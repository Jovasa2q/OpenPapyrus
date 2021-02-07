// GNUPLOT - interpol.c 
// Copyright 1986 - 1993, 1998, 2004   Thomas Williams, Colin Kelley
//
/*
 * C-Source file identification Header
 *
 * This file belongs to a project which is:
 *
 * done 1993 by MGR-Software, Asgard  (Lars Hanke)
 * written by Lars Hanke
 *
 * Contact me via:
 *
 *  InterNet: mgr@asgard.bo.open.de
 *      FIDO: Lars Hanke @ 2:243/4802.22   (as long as they keep addresses)
 *
 **************************************************************************
 *
 *   Project: gnuplot
 *    Module:
 *      File: interpol.c
 *
 *   Revisor: Lars Hanke
 *   Revised: 26/09/93
 *  Revision: 1.0
 *
 **************************************************************************
 *
 * LEGAL
 *  This module is part of gnuplot and distributed under whatever terms
 *  gnuplot is or will be published, unless exclusive rights are claimed.
 *
 * DESCRIPTION
 *  Supplies 2-D data interpolation and approximation routines
 *
 * IMPORTS
 *  plot.h
 *    - cp_extend()
 *    - structs: curve_points, coordval, coordinate
 *
 *  setshow.h
 *    - samples, axis array[] variables
 *    - plottypes
 *
 * EXPORTS
 *  gen_interp()
 *  sort_points()
 *  cp_implode()
 *
 * BUGS and TODO
 *  I would really have liked to use Gershon Elbers contouring code for
 *  all the stuff done here, but I failed. So I used my own code.
 *  If somebody is able to consolidate Gershon's code for this purpose
 *  a lot of gnuplot users would be very happy - due to memory problems.
 *
 **************************************************************************
 *
 * HISTORY
 * Changes:
 *  Nov 24, 1995  Markus Schuh (M.Schuh@meteo.uni-koeln.de):
 *      changed the algorithm for csplines
 *      added algorithm for approximation csplines
 *      copied point storage and range fix from plot2d.c
 *
 *  Jun 30, 1996 Jens Emmerich
 *      implemented handling of UNDEFINED points
 *  Dec 2019 EAM
 *	move solve_tri_diag from contour.c to here
 *	generalize cp_tridiag to work on any coordinate dimension
 */
#include <gnuplot.h>
#pragma hdrstop

/* in order to support multiple axes, and to simplify ranging in
 * parametric plots, we use arrays to store some things. For 2d plots,
 * elements are z=0,y1=1,x1=2,z2=4,y2=5,x2=6 these are given symbolic
 * names in plot.h
 */

#define spline_coeff_size 4
typedef double spline_coeff[spline_coeff_size];
typedef double five_diag[5];

static int next_curve(curve_points * plot, int * curve_start);
static int num_curves(curve_points * plot);
static double eval_kdensity(curve_points * cp, int first_point, int num_points, double x);
static void do_kdensity(curve_points * cp, int first_point, int num_points, GpCoordinate * dest);
static double * cp_binomial(int points);
static void eval_bezier(curve_points * cp, int first_point, int num_points, double sr, coordval * px,
    coordval * py, coordval * py2, double * c);
static void do_bezier(curve_points * cp, double * bc, int first_point, int num_points, GpCoordinate * dest);
static int solve_tri_diag(tri_diag m[], double r[], double x[], int n);
static int solve_five_diag(five_diag m[], double r[], double x[], int n);
static spline_coeff * cp_approx_spline(GpCoordinate * first_point, int num_points, int path_dim, int spline_dim, int w_dim);
static spline_coeff * cp_tridiag(GpCoordinate * first_point, int num_points, int path_dim, int spline_dim);
static void do_cubic(curve_points * plot, spline_coeff * sc, spline_coeff * sc2,
    int first_point, int num_points, GpCoordinate * dest);
static void do_freq(curve_points * plot,  int first_point, int num_points);
static int do_curve_cleanup(GpCoordinate * point, int npoints);
static int compare_points(SORTFUNC_ARGS p1, SORTFUNC_ARGS p2);
static int compare_z(SORTFUNC_ARGS p1, SORTFUNC_ARGS p2);

/*
 * position curve_start to index the next non-UNDEFINDED point,
 * start search at initial curve_start,
 * return number of non-UNDEFINDED points from there on,
 * if no more valid points are found, curve_start is set
 * to plot->p_count and 0 is returned
 */

static int next_curve(curve_points * plot, int * curve_start)
{
	int curve_length;
	// Skip undefined points 
	while(*curve_start < plot->p_count && plot->points[*curve_start].type == UNDEFINED) {
		(*curve_start)++;
	}
	curve_length = 0;
	// curve_length is first used as an offset, then the correct # points 
	while((*curve_start) + curve_length < plot->p_count && plot->points[(*curve_start) + curve_length].type != UNDEFINED) {
		curve_length++;
	}
	return (curve_length);
}

/*
 * determine the number of curves in plot->points, separated by
 * UNDEFINED points
 */

static int num_curves(curve_points * plot)
{
	int num_points;
	int first_point = 0;
	int curves = 0;
	while((num_points = next_curve(plot, &first_point)) > 0) {
		curves++;
		first_point += num_points;
	}
	return (curves);
}

/* PKJ - May 2008
   kdensity (short for Kernel Density) builds histograms using
   "Kernel Density Estimation" using Gaussian Kernels.
   Check: L. Wassermann: "All of Statistics" for example.

   The implementation is based closely on the implementation for Bezier
   curves, except for the way the actual interpolation is generated.
 */
static double kdensity_bandwidth = 0;

static void stats_kdensity(curve_points * cp,
    int first_point,    /* where to start in plot->points (to find x-range) */
    int num_points      /* to determine end in plot->points */
    ) {
	GpCoordinate * this_points = (cp->points) + first_point;
	double default_bandwidth;
	int i;
	double kdensity_avg = 0.0;
	double kdensity_sigma = 0.0;
	for(i = 0; i < num_points; i++) {
		kdensity_avg   += this_points[i].x;
		kdensity_sigma += this_points[i].x * this_points[i].x;
	}
	kdensity_avg /= (double)num_points;
	kdensity_sigma = sqrt(kdensity_sigma/(double)num_points - kdensity_avg*kdensity_avg);
	/* This is the optimal bandwidth if the point distribution is Gaussian.
	   (Applied Smoothing Techniques for Data Analysis
	   by Adrian W, Bowman & Adelchi Azzalini (1997)) */
	/* If the supplied bandwidth is zero of less, the default bandwidth is used. */
	default_bandwidth = pow(4.0/(3.0*num_points), 1.0/5.0) * kdensity_sigma;
	if(cp->smooth_parameter <= 0) {
		kdensity_bandwidth = default_bandwidth;
		cp->smooth_parameter = -default_bandwidth;
	}
	else
		kdensity_bandwidth = cp->smooth_parameter;
}
//
// eval_kdensity is a modification of eval_bezier 
//
static double eval_kdensity(curve_points * cp, int first_point/* where to start in plot->points (to find x-range) */,
    int num_points/* to determine end in plot->points */, double x/* x value at which to calculate y */) 
{
	GpCoordinate * this_points = (cp->points) + first_point;
	double period = cp->smooth_period;
	uint i;
	double Z;
	double y = 0.0;
	for(i = 0; i < num_points; i++) {
		double dist = fabs(x - this_points[i].x);
		if(period > 0 && dist > period/2)
			dist = period - dist;
		Z = dist / kdensity_bandwidth;
		y += this_points[i].y * exp(-0.5*Z*Z) / kdensity_bandwidth;
	}
	y /= sqrt(2.0*M_PI);
	return y;
}
//
// do_kdensity is based on do_bezier, except for the call to eval_bezier 
// EAM Feb 2015: Don't touch xrange, but recalculate y limits  
//
static void do_kdensity(curve_points * cp, int first_point/* where to start in plot->points */, int num_points/* to determine end in plot->points */,
    GpCoordinate * dest/* where to put the interpolated data */)
{
	int i;
	double x, y;
	double sxmin, sxmax, step;
	GPO.AxS.Idx_X = cp->AxIdx_X;
	GPO.AxS.Idx_Y = cp->AxIdx_Y;
	if(GPO.AxS.__X().log)
		GPO.IntWarn(NO_CARET, "kdensity components are Gaussian on x, not log(x)");
	sxmin = GPO.AxS.__X().min;
	sxmax = GPO.AxS.__X().max;
	step = (sxmax - sxmin) / (samples_1 - 1);
	stats_kdensity(cp, first_point, num_points);
	for(i = 0; i < samples_1; i++) {
		x = sxmin + i * step;
		y = eval_kdensity(cp, first_point, num_points, x);
		/* now we have to store the points and adjust the ranges */
		dest[i].type = INRANGE;
		dest[i].x = x;
		store_and_update_range(&dest[i].y, y, &dest[i].type, &GPO.AxS.__Y(), cp->noautoscale);
		dest[i].xlow = dest[i].xhigh = dest[i].x;
		dest[i].ylow = dest[i].yhigh = dest[i].y;
		dest[i].z = -1;
	}
}

/* HBB 990205: rewrote the 'bezier' interpolation routine,
 * to prevent numerical overflow and other undesirable things happening
 * for large data files (num_data about 1000 or so), where binomial
 * coefficients would explode, and powers of 'sr' (0 < sr < 1) become
 * extremely small. Method used: compute logarithms of these
 * extremely large and small numbers, and only go back to the
 * real numbers once they've cancelled out each other, leaving
 * a reasonable-sized one. */

/*
 * cp_binomial() computes the binomial coefficients needed for BEZIER stuff
 *   and stores them into an array which is hooked to sdat.
 * (MGR 1992)
 */
static double * cp_binomial(int points)
{
	double * coeff;
	int n, k;
	int e;
	e = points;             /* well we're going from k=0 to k=p_count-1 */
	coeff = (double*)gp_alloc(e * sizeof(double), "bezier coefficients");
	n = points - 1;
	e = n / 2;
	/* HBB 990205: calculate these in 'logarithmic space',
	* as they become _very_ large, with growing n (4^n) */
	coeff[0] = 0.0;
	for(k = 0; k < e; k++) {
		coeff[k + 1] = coeff[k] + log(((double)(n - k)) / ((double)(k + 1)));
	}
	for(k = n; k >= e; k--)
		coeff[k] = coeff[n - k];
	return (coeff);
}

/* This is a subfunction of do_bezier() for BEZIER style computations.
 * It is passed the step fraction (STEP/MAXSTEPS) and the addresses of
 * the double values holding the next x and y coordinates.
 * (MGR 1992)
 * Feb 2020: Do yhigh also so that filledcurves can use it
 */

static void eval_bezier(curve_points * cp,
    int first_point,            /* where to start in plot->points (to find x-range) */
    int num_points,             /* to determine end in plot->points */
    double sr,                  /* position inside curve, range [0:1] */
    coordval * px,               /* OUTPUT: x and y */
    coordval * py,
    coordval * py2,              /*         used for 2nd border of fillcurves */
    double * c)                  /* Bezier coefficient array */
{
	uint n = num_points - 1;
	GpCoordinate * this_points = (cp->points) + first_point;
	if(sr == 0.0) {
		*px = this_points[0].x;
		*py = this_points[0].y;
		*py2 = this_points[0].yhigh;
	}
	else if(sr == 1.0) {
		*px = this_points[n].x;
		*py = this_points[n].y;
		*py2 = this_points[n].yhigh;
	}
	else {
		/* HBB 990205: do calculation in 'logarithmic space',
		 * to avoid over/underflow errors, which would exactly cancel
		 * out each other, anyway, in an exact calculation
		 */
		uint i;
		double lx = 0.0, ly = 0.0, ly2 = 0.0;
		double log_dsr_to_the_n = n * log(1 - sr);
		double log_sr_over_dsr = log(sr) - log(1 - sr);
		for(i = 0; i <= n; i++) {
			double u = exp(c[i] + log_dsr_to_the_n + i * log_sr_over_dsr);
			lx += this_points[i].x * u;
			ly += this_points[i].y * u;
			ly2 += this_points[i].yhigh * u;
		}
		*px = lx;
		*py = ly;
		*py2 = ly2;
	}
}
// 
// Generate a new set of coordinates representing the bezier curve.
// Note that these are sampled evenly across the x range (from "set samples N")
// rather than corresponding to x values of the original data points.
// 
static void do_bezier(curve_points * cp, double * bc/* Bezier coefficient array */, int first_point/* where to start in plot->points */,
    int num_points/* to determine end in plot->points */, GpCoordinate * dest/* where to put the interpolated data */)
{
	coordval x, y, yhigh;
	GPO.AxS.Idx_X = cp->AxIdx_X;
	GPO.AxS.Idx_Y = cp->AxIdx_Y;
	for(int i = 0; i < samples_1; i++) {
		eval_bezier(cp, first_point, num_points, (double)i / (double)(samples_1 - 1), &x, &y, &yhigh, bc);
		dest[i].type = INRANGE;
		store_and_update_range(&dest[i].x, x, &dest[i].type, &GPO.AxS.__X(), GPO.AxS.__X().autoscale);
		store_and_update_range(&dest[i].y, y, &dest[i].type, &GPO.AxS.__Y(), GPO.AxS.__Y().autoscale);
		dest[i].xlow = dest[i].xhigh = dest[i].x;
		dest[i].ylow = dest[i].yhigh = dest[i].y;
		dest[i].z = -1;
		dest[i].yhigh = yhigh;
	}
}
/*
 * Solve five diagonal linear system equation. The five diagonal matrix is
 * defined via matrix M, right side is r, and solution X i.e. M * X = R.
 * Size of system given in n. Return TRUE if solution exist.
 *  G. Engeln-Muellges/ F.Reutter:
 *  "Formelsammlung zur Numerischen Mathematik mit Standard-FORTRAN-Programmen"
 *  ISBN 3-411-01677-9
 *
 * /  m02 m03 m04   0   0   0   0    .       .       . \   /  x0  \    / r0  \
 * I  m11 m12 m13 m14   0   0   0    .       .       . I   I  x1  I   I  r1  I
 * I  m20 m21 m22 m23 m24   0   0    .       .       . I * I  x2  I = I  r2  I
 * I    0 m30 m31 m32 m33 m34   0    .       .       . I   I  x3  I   I  r3  I
 *      .   .   .   .   .   .   .    .       .       .        .        .
 * \                           m(n-3)0 m(n-2)1 m(n-1)2 /   \x(n-1)/   \r(n-1)/
 *
 */
static int solve_five_diag(five_diag m[], double r[], double x[], int n)
{
	int i;
	five_diag * hv = (five_diag *)gp_alloc((n + 1) * sizeof(five_diag), "five_diag help vars");
	hv[0][0] = m[0][2];
	if(hv[0][0] == 0) {
		SAlloc::F(hv);
		return FALSE;
	}
	hv[0][1] = m[0][3] / hv[0][0];
	hv[0][2] = m[0][4] / hv[0][0];

	hv[1][3] = m[1][1];
	hv[1][0] = m[1][2] - hv[1][3] * hv[0][1];
	if(hv[1][0] == 0) {
		SAlloc::F(hv);
		return FALSE;
	}
	hv[1][1] = (m[1][3] - hv[1][3] * hv[0][2]) / hv[1][0];
	hv[1][2] = m[1][4] / hv[1][0];

	for(i = 2; i < n; i++) {
		hv[i][3] = m[i][1] - m[i][0] * hv[i - 2][1];
		hv[i][0] = m[i][2] - m[i][0] * hv[i - 2][2] - hv[i][3] * hv[i - 1][1];
		if(hv[i][0] == 0) {
			SAlloc::F(hv);
			return FALSE;
		}
		hv[i][1] = (m[i][3] - hv[i][3] * hv[i - 1][2]) / hv[i][0];
		hv[i][2] = m[i][4] / hv[i][0];
	}

	hv[0][4] = 0;
	hv[1][4] = r[0] / hv[0][0];
	for(i = 1; i < n; i++) {
		hv[i + 1][4] = (r[i] - m[i][0] * hv[i - 1][4] - hv[i][3] * hv[i][4]) / hv[i][0];
	}

	x[n - 1] = hv[n][4];
	x[n - 2] = hv[n - 1][4] - hv[n - 2][1] * x[n - 1];
	for(i = n - 3; i >= 0; i--)
		x[i] = hv[i + 1][4] - hv[i][1] * x[i + 1] - hv[i][2] * x[i + 2];

	SAlloc::F(hv);
	return TRUE;
}

/*
 * Calculation of approximation cubic splines
 * Returns matrix of spline coefficients
 * Dec 2019 EAM - modified original routine cp_approx_spline for use with
 *                multi-dimensional splines
 * original code: created spline for y given x = control, variable z = weight
 * revised code:  create spline for coordinate indexed by spline_dim
 *                given control variable indexed by path_dim
 *                weights indexed by w_dim
 */
static spline_coeff * cp_approx_spline(GpCoordinate * points, int num_points, int path_dim, int spline_dim, int w_dim)
{
	spline_coeff * sc;
	five_diag * m;
	double * r, * x, * h, * xp, * yp;
	int i;
	// Define an overlay onto GpCoordinate that lets us select whichever
	// of x,y,z,... is needed by specifying an index 0-6
	struct gen_coord {
		enum coord_type type;
		coordval dimension[7];
	};
	struct gen_coord * this_point;
	if(num_points < 4)
		GPO.IntError(NO_CARET, "Can't calculate approximation splines, need at least 4 points");
	this_point = (struct gen_coord *)(points);
	for(i = 0; i < num_points; i++)
		if(this_point[i].dimension[w_dim] <= 0)
			GPO.IntError(NO_CARET, "Can't calculate approximation splines, all weights have to be > 0");
	sc = (spline_coeff *)gp_alloc((num_points) * sizeof(spline_coeff), "spline matrix");
	m = (five_diag *)gp_alloc((num_points - 2) * sizeof(five_diag), "spline help matrix");
	r = (double *)gp_alloc((num_points - 2) * sizeof(double), "spline right side");
	x = (double *)gp_alloc((num_points - 2) * sizeof(double), "spline solution vector");
	h = (double *)gp_alloc((num_points - 1) * sizeof(double), "spline help vector");
	xp = (double *)gp_alloc((num_points) * sizeof(double), "x pos");
	yp = (double *)gp_alloc((num_points) * sizeof(double), "y pos");
	xp[0] = this_point[0].dimension[path_dim];
	yp[0] = this_point[0].dimension[spline_dim];
	for(i = 1; i < num_points; i++) {
		xp[i] = this_point[i].dimension[path_dim];
		yp[i] = this_point[i].dimension[spline_dim];
		h[i - 1] = xp[i] - xp[i - 1];
	}
	/* set up the matrix and the vector */
	for(i = 0; i <= num_points - 3; i++) {
		r[i] = 3 * ((yp[i + 2] - yp[i + 1]) / h[i + 1] - (yp[i + 1] - yp[i]) / h[i]);
		if(i < 2)
			m[i][0] = 0;
		else
			m[i][0] = 6 / this_point[i].dimension[w_dim] / h[i - 1] / h[i];

		if(i < 1)
			m[i][1] = 0;
		else
			m[i][1] = h[i] - 6 / this_point[i].dimension[w_dim] / h[i] * (1 / h[i - 1] + 1 / h[i])
			    - 6 / this_point[i + 1].dimension[w_dim] / h[i] * (1 / h[i] + 1 / h[i + 1]);

		m[i][2] = 2 * (h[i] + h[i + 1])
		    + 6 / this_point[i].dimension[w_dim] / h[i] / h[i]
		    + 6 / this_point[i + 1].dimension[w_dim] * (1 / h[i] + 1 / h[i + 1]) * (1 / h[i] + 1 / h[i + 1])
		    + 6 / this_point[i + 2].dimension[w_dim] / h[i + 1] / h[i + 1];

		if(i > num_points - 4)
			m[i][3] = 0;
		else
			m[i][3] = h[i + 1] - 6 / this_point[i + 1].dimension[w_dim] / h[i + 1] * (1 / h[i] + 1 / h[i + 1])
			    - 6 / this_point[i + 2].dimension[w_dim] / h[i + 1] * (1 / h[i + 1] + 1 / h[i + 2]);

		if(i > num_points - 5)
			m[i][4] = 0;
		else
			m[i][4] = 6 / this_point[i + 2].dimension[w_dim] / h[i + 1] / h[i + 2];
	}

	/* solve the matrix */
	if(!solve_five_diag(m, r, x, num_points - 2)) {
		SAlloc::F(sc);
		SAlloc::F(h);
		SAlloc::F(x);
		SAlloc::F(r);
		SAlloc::F(m);
		SAlloc::F(xp);
		SAlloc::F(yp);
		GPO.IntError(NO_CARET, "Can't calculate approximation splines");
	}
	sc[0][2] = 0;
	for(i = 1; i <= num_points - 2; i++)
		sc[i][2] = x[i - 1];
	sc[num_points - 1][2] = 0;

	sc[0][0] = yp[0] + 2 / this_point[0].dimension[w_dim] / h[0] * (sc[0][2] - sc[1][2]);
	for(i = 1; i <= num_points - 2; i++)
		sc[i][0] = yp[i] - 2 / this_point[i].dimension[w_dim] * (sc[i - 1][2] / h[i - 1] - sc[i][2] * (1 / h[i - 1] + 1 / h[i]) + sc[i + 1][2] / h[i]);
	sc[num_points - 1][0] = yp[num_points - 1] - 2 / this_point[num_points - 1].dimension[w_dim] / h[num_points - 2] * (sc[num_points - 2][2] - sc[num_points - 1][2]);

	for(i = 0; i <= num_points - 2; i++) {
		sc[i][1] = (sc[i + 1][0] - sc[i][0]) / h[i] - h[i] / 3 * (sc[i + 1][2] + 2 * sc[i][2]);
		sc[i][3] = (sc[i + 1][2] - sc[i][2]) / 3 / h[i];
	}
	SAlloc::F(h);
	SAlloc::F(x);
	SAlloc::F(r);
	SAlloc::F(m);
	SAlloc::F(xp);
	SAlloc::F(yp);
	return (sc);
}
/*
 * Calculation of cubic splines
 * This can be treated as a special case of approximation cubic splines, with
 * all weights -> infinity.
 *
 * Returns matrix of spline coefficients
 *
 * Dec 2019 EAM - modified original routine cp_tridiag() for use to
 * create multi-dimensional splines
 *   original code: created a spline for y using x as the control variable
 *   revised code:  spline for arbitrary coord using another coordinate as control
 *
 * Previous call to cp_tridiag(plot, start, n)
 *      becomes     cp_tridiag(&plot->points[start], n, 0, 1)
 *                                                      X  Y  <==
 * To create a spline for an arbitrary coordinate, e.g. x, as a function of PATH
 *	load path increments into points[i].CRD_PATH
 *      cp_tridiag(points, n, PATHCOORD, 0)
 *
 */
static spline_coeff * cp_tridiag(GpCoordinate * points, int num_points, int path_dim, int spline_dim)
{
	spline_coeff * sc;
	tri_diag * m;
	double * r, * x, * h, * xp, * yp;
	int i;
	/* Define an overlay onto GpCoordinate that lets us select whichever
	 * of x,y,z,... is needed by specifying an index 0-6
	 */
	struct gen_coord {
		enum coord_type type;
		coordval dimension[7];
	};
	struct gen_coord * this_point;
	if(num_points < 3)
		GPO.IntError(NO_CARET, "Can't calculate splines, need at least 3 points");
	this_point = (struct gen_coord *)(points);
	sc = (spline_coeff *)gp_alloc((num_points) * sizeof(spline_coeff), "spline matrix");
	m = (tri_diag *)gp_alloc((num_points - 2) * sizeof(tri_diag), "spline help matrix");
	r = (double *)gp_alloc((num_points - 2) * sizeof(double), "spline right side");
	x = (double *)gp_alloc((num_points - 2) * sizeof(double), "spline solution vector");
	h = (double *)gp_alloc((num_points - 1) * sizeof(double), "spline help vector");
	xp = (double *)gp_alloc((num_points) * sizeof(double), "x pos");
	yp = (double *)gp_alloc((num_points) * sizeof(double), "y pos");
	xp[0] = this_point[0].dimension[path_dim];
	yp[0] = this_point[0].dimension[spline_dim];
	for(i = 1; i < num_points; i++) {
		xp[i] = this_point[i].dimension[path_dim];
		yp[i] = this_point[i].dimension[spline_dim];
		h[i - 1] = xp[i] - xp[i - 1];
	}

	/* set up the matrix and the vector */

	for(i = 0; i <= num_points - 3; i++) {
		r[i] = 3 * ((yp[i + 2] - yp[i + 1]) / h[i + 1] - (yp[i + 1] - yp[i]) / h[i]);
		if(i < 1)
			m[i][0] = 0;
		else
			m[i][0] = h[i];

		m[i][1] = 2 * (h[i] + h[i + 1]);

		if(i > num_points - 4)
			m[i][2] = 0;
		else
			m[i][2] = h[i + 1];
	}

	/* solve the matrix */
	if(!solve_tri_diag(m, r, x, num_points - 2)) {
		SAlloc::F(sc);
		SAlloc::F(h);
		SAlloc::F(x);
		SAlloc::F(r);
		SAlloc::F(m);
		SAlloc::F(xp);
		SAlloc::F(yp);
		GPO.IntError(NO_CARET, "Can't calculate cubic splines");
	}
	sc[0][2] = 0;
	for(i = 1; i <= num_points - 2; i++)
		sc[i][2] = x[i - 1];
	sc[num_points - 1][2] = 0;
	for(i = 0; i <= num_points - 1; i++)
		sc[i][0] = yp[i];
	for(i = 0; i <= num_points - 2; i++) {
		sc[i][1] = (sc[i + 1][0] - sc[i][0]) / h[i] - h[i] / 3 * (sc[i + 1][2] + 2 * sc[i][2]);
		sc[i][3] = (sc[i + 1][2] - sc[i][2]) / 3 / h[i];
	}
	SAlloc::F(h);
	SAlloc::F(x);
	SAlloc::F(r);
	SAlloc::F(m);
	SAlloc::F(xp);
	SAlloc::F(yp);
	return (sc);
}

/*
 * Solve tri diagonal linear system equation. The tri diagonal matrix is
 * defined via matrix M, right side is r, and solution X i.e. M * X = R.
 * Size of system given in n. Return TRUE if solution exist.
 */
static int solve_tri_diag(tri_diag m[], double r[], double x[], int n)
{
	int i;
	double t;
	for(i = 1; i < n; i++) { /* Eliminate element m[i][i-1] (lower diagonal). */
		if(m[i - 1][1] == 0)
			return FALSE;
		t = m[i][0] / m[i - 1][1]; /* Find ratio between the two lines. */
		m[i][1] = m[i][1] - m[i - 1][2] * t;
		r[i] = r[i] - r[i - 1] * t;
	}
	/* Back substitution - update the solution vector X */
	if(m[n - 1][1] == 0)
		return FALSE;
	x[n - 1] = r[n - 1] / m[n - 1][1]; /* Find last element. */
	for(i = n - 2; i >= 0; i--) {
		if(m[i][1] == 0)
			return FALSE;
		x[i] = (r[i] - x[i + 1] * m[i][2]) / m[i][1];
	}
	return TRUE;
}

void gen_interp_unwrap(curve_points * plot)
{
	double y, diff;
	int curves = num_curves(plot);
	int first_point = 0;
	for(int i = 0; i < curves; i++) {
		int num_points = next_curve(plot, &first_point);
		double lasty = 0; /* make all plots start the same place */
		for(int j = first_point; j < first_point + num_points; j++) {
			if(plot->points[j].type == UNDEFINED)
				continue;
			y = plot->points[j].y;
			do {
				diff = y - lasty;
				if(diff >  M_PI) y -= 2*M_PI;
				if(diff < -M_PI) y += 2*M_PI;
			} while(fabs(diff) > M_PI);
			plot->points[j].y = y;

			lasty = y;
		}
		do_freq(plot, first_point, num_points);
		first_point += num_points + 1;
	}
}

static void do_cubic(curve_points * plot/* still contains old plot->points */,
    spline_coeff * sc/* generated by cp_tridiag */, spline_coeff * sc2/* optional spline for yhigh */,
    int first_point/* where to start in plot->points */, int num_points/* to determine end in plot->points */,
    GpCoordinate * dest/* where to put the interpolated data */)
{
	double xdiff, temp, x, y;
	double xstart, xend;    /* Endpoints of the sampled x range */
	int i, l;
	GpCoordinate * this_points;
	GPO.AxS.Idx_X = plot->AxIdx_X;
	GPO.AxS.Idx_Y = plot->AxIdx_Y;
	this_points = (plot->points) + first_point;
	l = 0;
	/* HBB 20010727: Sample only across the actual x range, not the
	 * full range of input data */
#if SAMPLE_CSPLINES_TO_FULL_RANGE
	xstart = this_points[0].x;
	xend = this_points[num_points - 1].x;
#else
	xstart = MAX(this_points[0].x, GPO.AxS.__X().min);
	xend = MIN(this_points[num_points - 1].x, GPO.AxS.__X().max);

	if(xstart >= xend) {
		/* This entire segment lies outside the current x range. */
		for(i = 0; i < samples_1; i++)
			dest[i].type = OUTRANGE;
		return;
	}
#endif
	xdiff = (xend - xstart) / (samples_1 - 1);
	for(i = 0; i < samples_1; i++) {
		x = xstart + i * xdiff;
		/* Move forward to the spline interval this point is in */
		while((x >= this_points[l + 1].x) && (l < num_points - 2))
			l++;
		temp = x - this_points[l].x;
		/* Evaluate cubic spline polynomial */
		y = ((sc[l][3] * temp + sc[l][2]) * temp + sc[l][1]) * temp + sc[l][0];
		dest[i].type = INRANGE;
		store_and_update_range(&dest[i].x, x, &dest[i].type, &GPO.AxS.__X(), GPO.AxS.__X().autoscale);
		store_and_update_range(&dest[i].y, y, &dest[i].type, &GPO.AxS.__Y(), GPO.AxS.__Y().autoscale);
		dest[i].xlow = dest[i].xhigh = dest[i].x;
		dest[i].ylow = dest[i].yhigh = dest[i].y;
		dest[i].z = -1;
		/* This case is used when smoothing "x y yhigh with filledcurves" */
		if(sc2) {
			y = ((sc2[l][3] * temp + sc2[l][2]) * temp + sc2[l][1]) * temp + sc2[l][0];
			dest[i].yhigh = y;
		}
	}
}
/*
 * do_freq() is like the other smoothers only in that it
 * needs to adjust the plot ranges. We don't have to copy
 * approximated curves or anything like that.
 */
static void do_freq(curve_points * plot/* still contains old plot->points */, int first_point/* where to start in plot->points */, int num_points/* to determine end in plot->points */)
{
	int x_axis = plot->AxIdx_X;
	int y_axis = plot->AxIdx_Y;
	GpCoordinate * p_this = (plot->points) + first_point;
	for(int i = 0; i < num_points; i++) {
		const double x = p_this[i].x;
		const double y = p_this[i].y;
		p_this[i].type = INRANGE;
		/* Overkill.  All we really want to do is update the x and y range */
		store_and_update_range(&p_this[i].x, x, &p_this[i].type, &GPO.AxS.__X(), GPO.AxS.__X().autoscale);
		store_and_update_range(&p_this[i].y, y, &p_this[i].type, &GPO.AxS.__Y(), GPO.AxS.__Y().autoscale);
		p_this[i].xlow = p_this[i].xhigh = p_this[i].x;
		p_this[i].ylow = p_this[i].yhigh = p_this[i].y;
		p_this[i].z = -1;
	}
}
/*
 * Frequency plots have don't need new points allocated; we just need
 * to adjust the plot ranges. Wedging this into gen_interp() would
 * make that code even harder to read.
 */
void gen_interp_frequency(curve_points * plot)
{
	int i, j;
	int first_point, num_points;
	double y;
	double y_total = 0.0;
	const int curves = num_curves(plot);
	if(oneof2(plot->plot_smooth, SMOOTH_FREQUENCY_NORMALISED, SMOOTH_CUMULATIVE_NORMALISED)) {
		first_point = 0;
		for(i = 0; i < curves; i++) {
			num_points = next_curve(plot, &first_point);
			for(j = first_point; j < first_point + num_points; j++) {
				if(plot->points[j].type == UNDEFINED)
					continue;
				y_total += plot->points[j].y;
			}
			first_point += num_points + 1;
		}
	}
	first_point = 0;
	for(i = 0; i < curves; i++) {
		num_points = next_curve(plot, &first_point);
		/* If cumulative, replace the current y-value with the
		   sum of all previous y-values. This assumes that the
		   data has already been sorted by x-values. */
		if(plot->plot_smooth == SMOOTH_CUMULATIVE) {
			y = 0;
			for(j = first_point; j < first_point + num_points; j++) {
				if(plot->points[j].type == UNDEFINED)
					continue;

				y += plot->points[j].y;
				plot->points[j].y = y;
			}
		}

		/* Alternatively, cumulative normalised means replace the
		   current y-value with the sum of all previous y-values
		   divided by the total sum of all values.  This assumes the
		   data is sorted as before.  Normalising in this way allows
		   comparison of the CDF of data sets with differing total
		   numbers of samples.  */

		if(plot->plot_smooth == SMOOTH_CUMULATIVE_NORMALISED) {
			y = 0;
			for(j = first_point; j < first_point + num_points; j++) {
				if(plot->points[j].type == UNDEFINED)
					continue;
				y += plot->points[j].y;
				plot->points[j].y = y / y_total;
			}
		}
		/* Finally, normalized frequency smoothing means that we take our
		   existing histogram and divide each value by the total */
		if(plot->plot_smooth == SMOOTH_FREQUENCY_NORMALISED) {
			for(j = first_point; j < first_point + num_points; j++) {
				if(plot->points[j].type == UNDEFINED)
					continue;
				plot->points[j].y /= y_total;
			}
		}
		do_freq(plot, first_point, num_points);
		first_point += num_points + 1;
	}
}
/*
 * This is the shared entry point used for the original smoothing options
 * csplines acsplines bezier sbezier
 */
void gen_interp(curve_points * plot)
{
	spline_coeff * sc = NULL;
	spline_coeff * sc2 = NULL;
	double * bc;
	int i;
	int num_points;
	int curves = num_curves(plot);
	GpCoordinate * new_points = (GpCoordinate *)gp_alloc((samples_1 + 1) * curves * sizeof(GpCoordinate), "interpolation table");
	int first_point = 0;
	for(i = 0; i < curves; i++) {
		num_points = next_curve(plot, &first_point);
		switch(plot->plot_smooth) {
			case SMOOTH_CSPLINES:
			    /* 0 and 1 signify x and y, the first two dimensions in GpCoordinate */
			    /* for FILLEDCURVES_BETWEEN we do it again for x and yhigh */
			    sc = cp_tridiag(&plot->points[first_point], num_points, 0, 1);
			    if(plot->plot_style == FILLEDCURVES && oneof3(plot->filledcurves_options.closeto, FILLEDCURVES_BETWEEN, FILLEDCURVES_ABOVE, FILLEDCURVES_BELOW))
				    sc2 = cp_tridiag(&plot->points[first_point], num_points, 0, 4);
			    do_cubic(plot, sc, sc2, first_point, num_points,
				new_points + i * (samples_1 + 1));
			    SAlloc::F(sc);
			    SAlloc::F(sc2);
			    break;
			case SMOOTH_ACSPLINES:
			    /* 0 = control axis x,  1 = spline on y,  2 = weights held in z */
			    sc = cp_approx_spline(&plot->points[first_point], num_points, 0, 1, 2);
			    if(plot->plot_style == FILLEDCURVES && oneof3(plot->filledcurves_options.closeto, FILLEDCURVES_BETWEEN, FILLEDCURVES_ABOVE, FILLEDCURVES_BELOW))
				    sc2 = cp_approx_spline(&plot->points[first_point], num_points, 0, 4, 2);
			    do_cubic(plot, sc, sc2, first_point, num_points,
				new_points + i * (samples_1 + 1));
			    SAlloc::F(sc);
			    SAlloc::F(sc2);
			    break;

			case SMOOTH_BEZIER:
			case SMOOTH_SBEZIER:
			    bc = cp_binomial(num_points);
			    do_bezier(plot, bc, first_point, num_points,
				new_points + i * (samples_1 + 1));
			    SAlloc::F((char*)bc);
			    break;
			case SMOOTH_KDENSITY:
			    do_kdensity(plot, first_point, num_points,
				new_points + i * (samples_1 + 1));
			    break;
			default: /* keep gcc -Wall quiet */
			    ;
		}
		new_points[(i + 1) * (samples_1 + 1) - 1].type = UNDEFINED;
		first_point += num_points;
	}
	SAlloc::F(plot->points);
	plot->points = new_points;
	plot->p_max = curves * (samples_1 + 1);
	plot->p_count = plot->p_max - 1;
}
/*
 * sort_points
 */

static int compare_points(SORTFUNC_ARGS arg1, SORTFUNC_ARGS arg2)
{
	GpCoordinate const * p1 = (GpCoordinate const *)arg1;
	GpCoordinate const * p2 = (GpCoordinate const *)arg2;
	if(p1->x > p2->x)
		return (1);
	if(p1->x < p2->x)
		return (-1);
	return (0);
}

static int compare_z(SORTFUNC_ARGS arg1, SORTFUNC_ARGS arg2)
{
	GpCoordinate const * p1 = (GpCoordinate const *)arg1;
	GpCoordinate const * p2 = (GpCoordinate const *)arg2;
	if(p1->z > p2->z)
		return (1);
	if(p1->z < p2->z)
		return (-1);
	return (0);
}

void sort_points(curve_points * plot)
{
	int first_point, num_points;

	first_point = 0;
	while((num_points = next_curve(plot, &first_point)) > 0) {
		/* Sort this set of points, does qsort handle 1 point correctly? */
		qsort(plot->points + first_point, num_points,
		    sizeof(GpCoordinate), compare_points);
		first_point += num_points;
	}
	return;
}

/*
 * Sort on z rather than x
 * used by "smooth zsort"
 */
void zsort_points(curve_points * plot)
{
	int i, first_point, num_points;

	/* save variable color into GpCoordinate */
	if(plot->varcolor) {
		for(i = 0; i < plot->p_count; i++)
			plot->points[i].CRD_COLOR = plot->varcolor[i];
	}

	first_point = 0;
	while((num_points = next_curve(plot, &first_point)) > 0) {
		qsort(plot->points + first_point, num_points,
		    sizeof(GpCoordinate), compare_z);
		first_point += num_points;
	}

	/* restore variable color */
	if(plot->varcolor) {
		for(i = 0; i < plot->p_count; i++)
			plot->varcolor[i] = plot->points[i].CRD_COLOR;
	}
	return;
}
// 
// cp_implode() if averaging is selected this function computes the new
//   entries and shortens the whole thing to the necessary size
// MGR Addendum
// 
//void cp_implode(curve_points * cp)
void GnuPlot::CpImplode(curve_points * pCp)
{
	int first_point, num_points;
	int i, j, k;
	double x = 0.0;
	double y = 0.0;
	double sux = 0.0;
	double slx = 0.0;
	double suy = 0.0;
	double sly = 0.0;
	double weight = 1.0; /* used for acsplines */
	bool all_inrange = FALSE;
	AxS.Idx_X = pCp->AxIdx_X;
	AxS.Idx_Y = pCp->AxIdx_Y;
	j = 0;
	first_point = 0;
	while((num_points = next_curve(pCp, &first_point)) > 0) {
		bool last_point = FALSE;
		k = 0;
		for(i = first_point; i <= first_point + num_points; i++) {
			if(i == first_point + num_points) {
				if(k == 0)
					break;
				last_point = TRUE;
			}
			if(!last_point && pCp->points[i].type == UNDEFINED)
				continue;
			if(k == 0) {
				x = pCp->points[i].x;
				y = pCp->points[i].y;
				sux = pCp->points[i].xhigh;
				slx = pCp->points[i].xlow;
				suy = pCp->points[i].yhigh;
				sly = pCp->points[i].ylow;
				weight = pCp->points[i].z;
				all_inrange = (pCp->points[i].type == INRANGE);
				k = 1;
			}
			else if(!last_point && pCp->points[i].x == x) {
				y += pCp->points[i].y;
				sux += pCp->points[i].xhigh;
				slx += pCp->points[i].xlow;
				suy += pCp->points[i].yhigh;
				sly += pCp->points[i].ylow;
				weight += pCp->points[i].z;
				if(pCp->points[i].type != INRANGE)
					all_inrange = FALSE;
				k++;
			}
			else {
				pCp->points[j].x = x;
				if(oneof4(pCp->plot_smooth, SMOOTH_FREQUENCY, SMOOTH_FREQUENCY_NORMALISED, SMOOTH_CUMULATIVE, SMOOTH_CUMULATIVE_NORMALISED))
					k = 1;
				pCp->points[j].y = y /= (double)k;
				pCp->points[j].xhigh = sux / (double)k;
				pCp->points[j].xlow = slx / (double)k;
				pCp->points[j].yhigh = suy / (double)k;
				pCp->points[j].ylow = sly / (double)k;
				pCp->points[j].z = weight / (double)k;
				// HBB 20000405: I wanted to use STORE_AND_FIXUP_RANGE here,
				// but won't: it assumes we want to modify the range, and
				// that the range is given in 'input' coordinates.
				pCp->points[j].type = INRANGE;
				if(!all_inrange) {
					if(((x < AxS.__X().min) && !(AxS.__X().autoscale & AUTOSCALE_MIN))
					    || ((x > AxS.__X().max) && !(AxS.__X().autoscale & AUTOSCALE_MAX))
					    || ((y < AxS.__Y().min) && !(AxS.__Y().autoscale & AUTOSCALE_MIN))
					    || ((y > AxS.__Y().max) && !(AxS.__Y().autoscale & AUTOSCALE_MAX)))
						pCp->points[j].type = OUTRANGE;
				} /* if (! all inrange) */
				j++; /* next valid entry */
				k = 0; /* to read */
				i--; /* from this (-> last after for(;;)) entry */
			} /* else (same x position) */
		} /* for(points in curve) */
		// FIXME: Monotonic cubic splines support only a single curve per data set 
		if(j < pCp->p_count && pCp->plot_smooth == SMOOTH_MONOTONE_CSPLINE)
			break;
		// insert invalid point to separate curves 
		if(j < pCp->p_count) {
			pCp->points[j].type = UNDEFINED;
			j++;
		}
		first_point += num_points;
	}                       /* end while */
	pCp->p_count = j;
	cp_extend(pCp, j);
}
// 
// EAM December 2013
// monotonic cubic spline using the Fritsch-Carlson algorithm
// FN Fritsch & RE Carlson (1980). "Monotone Piecewise Cubic Interpolation".
// SIAM Journal on Numerical Analysis (SIAM) 17 (2): 238–246. doi:10.1137/0717021.
// 
void mcs_interp(curve_points * pPlot)
{
	// These track the original (pre-sorted) data points 
	int N = pPlot->p_count;
	GpCoordinate * p = (GpCoordinate *)gp_realloc(pPlot->points, (N+1) * sizeof(GpCoordinate), "mcs");
	int i;
	// These will track the resulting smoothed curve (>= 3X original count) 
	// Larger number of samples gives smoother curve (no surprise!) 
	int Nsamp = (samples_1 > 2*N) ? samples_1 : 2*N;
	int Ntot = N + Nsamp;
	GpCoordinate * new_points = (GpCoordinate *)gp_alloc((Ntot) * sizeof(GpCoordinate), "mcs");
	double xstart = MAX(p[0].x, GPO.AxS.__X().min);
	double xend = MIN(p[N-1].x, GPO.AxS.__X().max);
	double xstep = (xend - xstart) / (Nsamp - 1);
	// Load output x coords for sampling 
	for(i = 0; i<N; i++)
		new_points[i].x = p[i].x;
	for(; i<Ntot; i++)
		new_points[i].x = xstart + (i-N)*xstep;
	// Sort output x coords 
	qsort(new_points, Ntot, sizeof(GpCoordinate), compare_points);
	// Displace any collisions 
	for(i = 1; i<Ntot-1; i++) {
		double delta = new_points[i].x - new_points[i-1].x;
		if(new_points[i+1].x - new_points[i].x < delta/1000.)
			new_points[i].x -= delta/2.;
	}
	// Calculate spline coefficients 
#define DX      xlow
#define SLOPE   xhigh
#define C1      ylow
#define C2      yhigh
#define C3      z
	for(i = 0; i < N-1; i++) {
		p[i].DX = p[i+1].x - p[i].x;
		p[i].SLOPE = (p[i+1].y - p[i].y) / p[i].DX;
	}
	// The SIAM paper only mentions setting the final slope to zero if the
	// calculation is otherwise ill-behaved (how would one detect that?).
	// Retaining the data-derived slope makes the handling at the two ends
	// of the data range consistent. See Bug #2055
	// p[N-1].SLOPE = 0; 
	p[N-1].SLOPE = p[N-2].SLOPE;
	p[0].C1 = p[0].SLOPE;
	for(i = 0; i < N-1; i++) {
		if(p[i].SLOPE * p[i+1].SLOPE <= 0) {
			p[i+1].C1 = 0;
		}
		else {
			double sum = p[i].DX + p[i+1].DX;
			p[i+1].C1 = (3.0 * sum) / ((sum + p[i+1].DX) /  p[i].SLOPE + (sum + p[i].DX) /  p[i+1].SLOPE);
		}
	}
	p[N].C1 = p[N-1].SLOPE;
	for(i = 0; i < N; i++) {
		double temp = p[i].C1 + p[i+1].C1 - 2*p[i].SLOPE;
		p[i].C2 = (p[i].SLOPE - p[i].C1 -temp) / p[i].DX;
		p[i].C3 = temp / (p[i].DX * p[i].DX);
	}
	// Use the coefficients C1, C2, C3 to interpolate over the requested range 
	for(i = 0; i < Ntot; i++) {
		double x = new_points[i].x;
		double y;
		bool exact = FALSE;
		if(x == p[N-1].x) { /* Exact value for right-most point of original data */
			y = p[N-1].y;
			exact = TRUE;
		}
		else {
			int low = 0;
			int mid;
			int high = N-1;
			while(low <= high) {
				mid = floor((low + high) / 2);
				if(p[mid].x < x)
					low = mid + 1;
				else if(p[mid].x > x)
					high = mid - 1;
				else { /* Exact value for some point in original data */
					y = p[mid].y;
					exact = TRUE;
					break;
				}
			}
			if(!exact) {
				int j = MAX(0, high);
				double diff = x - p[j].x;
				y = p[j].y + p[j].C1 * diff + p[j].C2 * diff * diff + p[j].C3 * diff * diff * diff;
			}
		}
		xstart = GPO.AxS.__X().min;
		xend = GPO.AxS.__X().max;
		if(inrange(x, xstart, xend))
			new_points[i].type = INRANGE;
		else
			new_points[i].type = OUTRANGE;
		// FIXME:  simpler test for outrange would be sufficient 
		GPO.AxS.Idx_Y = pPlot->AxIdx_Y;
		store_and_update_range(&new_points[i].y, y, &new_points[i].type, &GPO.AxS.__Y(), pPlot->noautoscale);
	}
	// Replace original data with the interpolated curve 
	SAlloc::F(p);
	pPlot->points = new_points;
	pPlot->p_count = Ntot;
	pPlot->p_max = Ntot + 1;
#undef DX
#undef SLOPE
#undef C1
#undef C2
#undef C3
}

/*
 * Binned histogram of input values.
 *
 *   plot FOO using N:(1) bins{=<nbins>} {binrange=[binlow:binhigh]}
 *                        {binwidth=<width>} with boxes
 *
 * If no binrange is given, binlow and binhigh are taken from the x range of the data.
 * In either of these cases binlow is the midpoint x-coordinate of the first bin
 * and binhigh is the midpoint x-coordinate of the last bin.
 * Points that lie exactly on a bin boundary are assigned to the upper bin.
 * Bin assignments are not affected by "set xrange".
 * Notes:
 *    binwidth = (binhigh-binlow) / (nbins-1)
 *        xmin = binlow - binwidth/2
 *        xmax = binhigh + binwidth/2
 *    first bin holds points with (xmin =< x < xmin + binwidth)
 *    last bin holds points with (xmax-binwidth =< x < binhigh + binwidth)
 */
void make_bins(curve_points * plot, int nbins, double binlow, double binhigh, double binwidth)
{
	int i, binno;
	double * bin;
	double bottom, top, range;
	GpAxis * xaxis = &GPO.AxS[plot->AxIdx_X];
	GpAxis * yaxis = &GPO.AxS[plot->AxIdx_Y];
	double ymax = 0;
	int N = plot->p_count;
	// Find the range of points to be binned 
	if(binlow != binhigh) {
		/* Explicit binrange [min:max] in the plot command */
		bottom = binlow;
		top = binhigh;
	}
	else {
		/* Take binrange from the data itself */
		bottom = VERYLARGE; top = -VERYLARGE;
		for(i = 0; i<N; i++) {
			if(bottom > plot->points[i].x)
				bottom = plot->points[i].x;
			if(top < plot->points[i].x)
				top = plot->points[i].x;
		}
		if(top <= bottom)
			GPO.IntWarn(NO_CARET, "invalid bin range [%g:%g]", bottom, top);
	}

	/* If a fixed binwidth was provided, find total number of bins */
	if(binwidth > 0) {
		double temp;
		nbins = 1 + (top - bottom) / binwidth;
		temp = nbins * binwidth - (top - bottom);
		bottom -= temp/2.;
		top += temp/2.;
	}
	/* otherwise we use (N-1) intervals between midpoints of bin 1 and bin N */
	else {
		binwidth = (top - bottom) / (nbins - 1);
		bottom -= binwidth/2.;
		top += binwidth/2.;
	}
	range = top - bottom;
	FPRINTF((stderr, "make_bins: %d bins from %g to %g, binwidth %g\n", nbins, bottom, top, binwidth));
	bin = (double *)gp_alloc(nbins*sizeof(double), "bins");
	for(i = 0; i<nbins; i++)
		bin[i] = 0;
	for(i = 0; i<N; i++) {
		if(plot->points[i].type == UNDEFINED)
			continue;
		binno = floor(nbins * (plot->points[i].x - bottom) / range);
		if(0 <= binno && binno < nbins)
			bin[binno] += plot->points[i].y;
	}
	if(xaxis->autoscale & AUTOSCALE_MIN) {
		if(xaxis->min > bottom)
			xaxis->min = bottom;
	}
	if(xaxis->autoscale & AUTOSCALE_MAX) {
		if(xaxis->max < top)
			xaxis->max = top;
	}

	/* Replace the original data with one entry per bin.
	 * new x = midpoint of bin
	 * new y = number of points in the bin
	 */
	plot->p_count = nbins;
	plot->points = (GpCoordinate *)gp_realloc(plot->points, nbins * sizeof(GpCoordinate), "curve_points");
	for(i = 0; i<nbins; i++) {
		double bincent = bottom + (0.5 + (double)i) * binwidth;
		plot->points[i].type = INRANGE;
		plot->points[i].x     = bincent;
		plot->points[i].xlow  = bincent - binwidth/2.;
		plot->points[i].xhigh = bincent + binwidth/2.;
		plot->points[i].y     = bin[i];
		plot->points[i].ylow  = plot->points[i].y;
		plot->points[i].yhigh = plot->points[i].y;
		plot->points[i].z = 0; /* FIXME: leave it alone? */
		if(inrange(bincent, xaxis->min, xaxis->max)) {
			if(ymax < bin[i])
				ymax = bin[i];
		}
		else {
			plot->points[i].type = OUTRANGE;
		}
		FPRINTF((stderr, "bin[%d] %g %g\n", i, plot->points[i].x, plot->points[i].y));
	}

	if(yaxis->autoscale & AUTOSCALE_MIN) {
		if(yaxis->min > 0)
			yaxis->min = 0;
	}
	if(yaxis->autoscale & AUTOSCALE_MAX) {
		if(yaxis->max < ymax)
			yaxis->max = ymax;
	}

	/* Recheck range on y */
	for(i = 0; i<nbins; i++)
		if(!inrange(plot->points[i].y, yaxis->min, yaxis->max))
			plot->points[i].type = OUTRANGE;

	/* Clean up */
	SAlloc::F(bin);
}

/*
 * spline approximation of 3D lines
 */

/*
 * Replace one isocurve with a 3D natural cubic spline interpolation.
 * If there are multiple isocurves, or multiple curves with isocurves,
 * the caller must sort that out and call here separately for each one.
 * TODO:
 *	number of spline samples should be independent of "set samples"
 */
static void do_3d_cubic(struct iso_curve * curve, enum PLOT_SMOOTH smooth_option)
{
	int i, l;
	int nseg = samples_1;
	GpCoordinate * new_points;
	double xrange, yrange, zrange;
	double dx, dy, dz;
	double maxdx, maxdy, maxdz;
	double t, tsum, tstep;
	spline_coeff * sc_x = NULL;
	spline_coeff * sc_y = NULL;
	spline_coeff * sc_z = NULL;
	GpCoordinate * old_points = curve->points;
	/*
	 * Sanity check axis ranges.
	 * This catches curves that lie in a plane of constant x or y.
	 * The fixup prints a warning to the user but we don't see it here.
	 */
	GPO.AxisCheckedExtendEmptyRange(FIRST_X_AXIS, "at time of spline generation");
	GPO.AxisCheckedExtendEmptyRange(FIRST_Y_AXIS, "at time of spline generation");
	// prevent gross mismatch of x/y/z units 
	xrange = fabs(GPO.AxS[FIRST_X_AXIS].max - GPO.AxS[FIRST_X_AXIS].min);
	yrange = fabs(GPO.AxS[FIRST_Y_AXIS].max - GPO.AxS[FIRST_Y_AXIS].min);
	zrange = fabs(GPO.AxS[FIRST_Z_AXIS].max - GPO.AxS[FIRST_Z_AXIS].min);
	// Construct path-length vector; store it in unused slot of old_points 
	t = tsum = 0.0;
	maxdx = maxdy = maxdz = 0.0;
	old_points[0].CRD_PATH = 0;
	for(i = 1; i < curve->p_count; i++) {
		dx = (old_points[i].x - old_points[i-1].x) / xrange;
		dy = (old_points[i].y - old_points[i-1].y) / yrange;
		dz = (old_points[i].z - old_points[i-1].z) / zrange;
		tsum += sqrt(dx*dx + dy*dy + dz*dz);
		old_points[i].CRD_PATH = tsum;

		/* Track planarity */
		if(fabs(dx) > maxdx)
			maxdx = fabs(dx);
		if(fabs(dy) > maxdy)
			maxdy = fabs(dy);
		if(fabs(dz) > maxdz)
			maxdz = fabs(dz);
	}
	/* Normalize so that the path always runs from 0 to 1 */
	for(i = 1; i < curve->p_count; i++)
		old_points[i].CRD_PATH /= tsum;
	tstep = old_points[curve->p_count-1].CRD_PATH / (double)(nseg - 1);
	/* Create new list to hold interpolated points */
	new_points = (GpCoordinate *)gp_alloc((nseg+1) * sizeof(GpCoordinate), "3D spline");
	memzero(new_points, (nseg+1) * sizeof(GpCoordinate));
	/*
	 * If the curve being fitted lies entirely in one plane,
	 * we can do better by fitting a 2D spline rather than a 3D spline.
	 * This benefits the relatively common case of drawing a stack of
	 * 2D plots (e.g. fence plots).
	 * First check for a curve lying in the yz plane (x = constant).
	 */
	if(maxdx < FLT_EPSILON) {
		tstep = (old_points[curve->p_count-1].y - old_points[0].y) / (double)(nseg - 1);
		if(smooth_option == SMOOTH_ACSPLINES)
			sc_z = cp_approx_spline(curve->points, curve->p_count, 1, 2, 3);
		else
			sc_z = cp_tridiag(curve->points, curve->p_count, 1, 2);
		for(i = 0, l = 0; i < nseg; i++) {
			double temp;
			t = old_points[0].y + i * tstep;
			/* Move forward to the spline interval this point is in */
			while((t >= old_points[l + 1].y) && (l < curve->p_count- 2))
				l++;
			temp = t - old_points[l].y;
			new_points[i].x = old_points[l].x; /* All the same */
			new_points[i].y = t;
			new_points[i].z = ((sc_z[l][3] * temp + sc_z[l][2]) * temp + sc_z[l][1])
			    * temp + sc_z[l][0];
		}
	}

	/*
	 * Check for a curve lying in the xz plane (y = constant).
	 */
	else if(maxdy < FLT_EPSILON) {
		tstep = (old_points[curve->p_count-1].x - old_points[0].x) / (double)(nseg - 1);
		if(smooth_option == SMOOTH_ACSPLINES)
			sc_z = cp_approx_spline(curve->points, curve->p_count, 0, 2, 3);
		else
			sc_z = cp_tridiag(curve->points, curve->p_count, 0, 2);

		for(i = 0, l = 0; i < nseg; i++) {
			double temp;
			t = old_points[0].x + i * tstep;
			/* Move forward to the spline interval this point is in */
			while((t >= old_points[l + 1].x) && (l < curve->p_count- 2))
				l++;
			temp = t - old_points[l].x;
			new_points[i].x = t;
			new_points[i].y = old_points[l].y; /* All the same */
			new_points[i].z = ((sc_z[l][3] * temp + sc_z[l][2]) * temp + sc_z[l][1])
			    * temp + sc_z[l][0];
		}
	}

	/*
	 * Check for a curve lying in the xy plane (z = constant).
	 */
	else if(maxdz < FLT_EPSILON) {
		tstep = (old_points[curve->p_count-1].x - old_points[0].x) / (double)(nseg - 1);
		if(smooth_option == SMOOTH_ACSPLINES)
			sc_y = cp_approx_spline(curve->points, curve->p_count, 0, 1, 3);
		else
			sc_y = cp_tridiag(curve->points, curve->p_count, 0, 1);

		for(i = 0, l = 0; i < nseg; i++) {
			double temp;
			t = old_points[0].x + i * tstep;
			/* Move forward to the spline interval this point is in */
			while((t >= old_points[l + 1].x) && (l < curve->p_count- 2))
				l++;
			temp = t - old_points[l].x;
			new_points[i].x = t;
			new_points[i].y = ((sc_y[l][3] * temp + sc_y[l][2]) * temp + sc_y[l][1])
			    * temp + sc_y[l][0];
			new_points[i].z = old_points[l].z; /* All the same */
		}
	}

	/*
	 * This is the general case.
	 * Calculate spline coefficients for each dimension x, y, z
	 */
	else {
		if(smooth_option == SMOOTH_ACSPLINES) {
			sc_x = cp_approx_spline(curve->points, curve->p_count, PATHCOORD, 0, 3);
			sc_y = cp_approx_spline(curve->points, curve->p_count, PATHCOORD, 1, 3);
			sc_z = cp_approx_spline(curve->points, curve->p_count, PATHCOORD, 2, 3);
		}
		else {
			sc_x = cp_tridiag(curve->points, curve->p_count, PATHCOORD, 0);
			sc_y = cp_tridiag(curve->points, curve->p_count, PATHCOORD, 1);
			sc_z = cp_tridiag(curve->points, curve->p_count, PATHCOORD, 2);
		}
		for(i = 0, l = 0; i < nseg; i++) {
			double temp;
			t = i * tstep;
			/* Move forward to the spline interval this point is in */
			while((t >= old_points[l + 1].CRD_PATH) && (l < curve->p_count- 2))
				l++;
			temp = t - old_points[l].CRD_PATH;
			new_points[i].x = ((sc_x[l][3] * temp + sc_x[l][2]) * temp + sc_x[l][1]) * temp + sc_x[l][0];
			new_points[i].y = ((sc_y[l][3] * temp + sc_y[l][2]) * temp + sc_y[l][1]) * temp + sc_y[l][0];
			new_points[i].z = ((sc_z[l][3] * temp + sc_z[l][2]) * temp + sc_z[l][1]) * temp + sc_z[l][0];
		}
	}

	/* We're done with the spline coefficients */
	SAlloc::F(sc_x);
	SAlloc::F(sc_y);
	SAlloc::F(sc_z);
	/* Replace original data with spline approximation */
	SAlloc::F(curve->points);
	curve->points = new_points;
	curve->p_count = nseg;
	curve->p_max = nseg+1;  /* not sure why we asked for 1 extra */
}

/*
 * Generate 2D splines along a path for each set of points in the plot,
 * smoothing option SMOOTH_PATH.
 * TODO:
 * - number of spline samples should be controlled by something other
 *   than "set samples"
 * - spline weights from an additional column
 */
void gen_2d_path_splines(curve_points * plot)
{
	int i;
	int ic, first_point;    /* indexes for original data */
	int is = 0;             /* index for new (splined) data */
	GpCoordinate * old_points = NULL;
	GpCoordinate * splined_points;
	spline_coeff * sc_x = NULL;
	spline_coeff * sc_y = NULL;
	const double xrange = fabs(GPO.AxS[plot->AxIdx_X].max - GPO.AxS[plot->AxIdx_X].min);
	const double yrange = fabs(GPO.AxS[plot->AxIdx_Y].max - GPO.AxS[plot->AxIdx_Y].min);
	const int curves = num_curves(plot);
	/* Allocate space to hold the interpolated points */
	splined_points = (GpCoordinate *)gp_alloc( (samples_1 * curves) * sizeof(GpCoordinate), NULL);
	memzero(splined_points, (samples_1 * curves) * sizeof(GpCoordinate));
	first_point = 0;
	for(ic = 0; ic < curves; ic++) {
		double t, tstep, tsum;
		double dx, dy;
		int l;
		int nold;
		int num_points = next_curve(plot, &first_point);
		bool closed = FALSE;
		/* Make a copy of the original points so that we don't corrupt the
		 * list by adding up to three new ones.
		 */
		old_points = (GpCoordinate *)gp_realloc(old_points, (num_points + 3) * sizeof(GpCoordinate), "spline points");
		memcpy(&old_points[1], &plot->points[first_point], num_points * sizeof(GpCoordinate));
		/* Remove any unusable points (NaN, missing, duplicates) before fitting a spline.
		 * If that leaves fewer than 3 points, skip it.
		 */
		nold = do_curve_cleanup(&old_points[1], num_points);
		if(nold < 3) {
			first_point += num_points;
			continue;
		}
		/* We expect one of two cases. Either this really is a closed
		 * curve (end point matches start point) or it is an open-ended
		 * path that may not be monotonic on x.
		 * For plot style "with filledcurves closed" we add an extra
		 * point at the end if it is not already there.
		 */
		if(old_points[1].x == old_points[nold].x &&  old_points[1].y == old_points[nold].y)
			closed = TRUE;
		if((plot->plot_style == FILLEDCURVES) && !closed) {
			old_points[++nold] = old_points[1];
			closed = TRUE;
		}
		if(closed) {
			/* Wrap around to one point before and one point after the path closure */
			nold += 2;
			old_points[0] = old_points[nold-3];
			old_points[nold-1] = old_points[2];
		}
		else {
			/* Dummy up an extension at either end */
			nold += 2;
			old_points[0].x = old_points[1].x + old_points[1].x - old_points[2].x;
			old_points[nold-1].x = old_points[nold-2].x + old_points[nold-2].x - old_points[nold-3].x;
			old_points[0].y = old_points[1].y + old_points[1].y - old_points[2].y;
			old_points[nold-1].y = old_points[nold-2].y + old_points[nold-2].y - old_points[nold-3].y;
		}

		/* Construct path-length vector; store it in unused slot of old_points */
		t = tsum = 0.0;
		old_points[0].CRD_PATH = 0;
		for(i = 1; i < nold; i++) {
			dx = (old_points[i].x - old_points[i-1].x) / xrange;
			dy = (old_points[i].y - old_points[i-1].y) / yrange;
			tsum += sqrt(dx*dx + dy*dy);
			old_points[i].CRD_PATH = tsum;
		}

		/* Normalize so that the path fraction always runs from 0 to 1 */
		for(i = 1; i < nold; i++)
			old_points[i].CRD_PATH /= tsum;
		tstep = 1.0 / (double)(samples_1 - 1);

		/* Calculate spline coefficients for x and for y as a function of path */
		sc_x = cp_tridiag(old_points, nold, PATHCOORD, 0);
		sc_y = cp_tridiag(old_points, nold, PATHCOORD, 1);

		/* First output point is the same as the original first point */
		splined_points[is++] = old_points[1];
		/* Skip the points in the overlap region */
		for(i = 0; i * tstep < old_points[1].CRD_PATH; i++)
			;
		/* Use spline coefficients to generate a new point at each sample interval. */
		for(l = 0; i < samples_1; i++) {
			double temp;
			t = i * tstep;
			/* Stop before wrapping around. Copy the original end point. */
			if(t > old_points[nold-2].CRD_PATH) {
				splined_points[is++] = old_points[nold-2];
				break;
			}
			/* Move forward to the spline interval this point is in */
			while((t >= old_points[l + 1].CRD_PATH) && (l < nold- 2))
				l++;
			temp = t - old_points[l].CRD_PATH;
			splined_points[is].x = ((sc_x[l][3] * temp + sc_x[l][2]) * temp + sc_x[l][1]) * temp + sc_x[l][0];
			splined_points[is].y = ((sc_y[l][3] * temp + sc_y[l][2]) * temp + sc_y[l][1]) * temp + sc_y[l][0];
			is++;
		}
		/* Done with spline coefficients */
		SAlloc::F(sc_x);
		SAlloc::F(sc_y);
		/* Add a seperator point after this set of splined points */
		splined_points[is++].type = UNDEFINED;
		first_point += num_points;
	}
	/* Replace original data with splined approximation */
	SAlloc::F(old_points);
	SAlloc::F(plot->points);
	plot->points = splined_points;
	plot->p_max = curves * samples_1;
	plot->p_count = is;
}
/*
 * Externally callable interface to 3D spline routines
 */
void gen_3d_splines(struct surface_points * plot)
{
	for(iso_curve * curve = plot->iso_crvs; curve; curve = curve->next) {
		// Remove any unusable points before fitting a spline 
		curve->p_count = do_curve_cleanup(curve->points, curve->p_count);
		if(curve->p_count > 3)
			do_3d_cubic(curve, plot->plot_smooth);
	}
}

static int do_curve_cleanup(GpCoordinate * point, int npoints)
{
	// Step through points in curve keeping only the usable ones. Discard duplicates
	int keep = 0;
	for(int i = 0; i < npoints; i++) {
		if(point[i].type == UNDEFINED)
			continue;
		if(isnan(point[i].x) || isnan(point[i].y) || isnan(point[i].z))
			continue;
		if(i != keep)
			point[keep] = point[i];
		// FIXME: should probably check fabs(this-prev) < EPS 
		if((keep > 0)  && (point[keep].x == point[keep-1].x) && (point[keep].y == point[keep-1].y) && (point[keep].z == point[keep-1].z))
			continue;
		keep++;
	}
	return keep;
}