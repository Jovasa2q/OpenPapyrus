/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2008, Jerome Fimes, Communications & Systemes <jerome.fimes@c-s.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 */
#include "opj_includes.h"
#pragma hdrstop
/**
 * Default size of the validation list, if not sufficient, data will be reallocated with a double size.
 */
#define OPJ_VALIDATION_SIZE 10

opj_procedure_list_t *  opj_procedure_list_create()
{
	/* memory allocation */
	opj_procedure_list_t * l_validation = (opj_procedure_list_t*)opj_calloc(1, sizeof(opj_procedure_list_t));
	if(!l_validation) {
		return 0;
	}
	/* initialization */
	l_validation->m_nb_max_procedures = OPJ_VALIDATION_SIZE;
	l_validation->m_procedures = (opj_procedure*)opj_calloc(OPJ_VALIDATION_SIZE, sizeof(opj_procedure));
	if(!l_validation->m_procedures) {
		SAlloc::F(l_validation);
		return 0;
	}
	return l_validation;
}

void  opj_procedure_list_destroy(opj_procedure_list_t * p_list)
{
	if(p_list) {
		/* initialization */
		SAlloc::F(p_list->m_procedures);
		SAlloc::F(p_list);
	}
}

boolint STDCALL opj_procedure_list_add_procedure(opj_procedure_list_t * p_validation_list, opj_procedure p_procedure, opj_event_mgr_t* p_manager)
{
	assert(p_manager != NULL);
	if(p_validation_list->m_nb_max_procedures == p_validation_list->m_nb_procedures) {
		opj_procedure * new_procedures;
		p_validation_list->m_nb_max_procedures += OPJ_VALIDATION_SIZE;
		new_procedures = (opj_procedure*)opj_realloc(p_validation_list->m_procedures, p_validation_list->m_nb_max_procedures * sizeof(opj_procedure));
		if(!new_procedures) {
			SAlloc::F(p_validation_list->m_procedures);
			p_validation_list->m_nb_max_procedures = 0;
			p_validation_list->m_nb_procedures = 0;
			opj_event_msg(p_manager, EVT_ERROR, "Not enough memory to add a new validation procedure\n");
			return FALSE;
		}
		else {
			p_validation_list->m_procedures = new_procedures;
		}
	}
	p_validation_list->m_procedures[p_validation_list->m_nb_procedures] = p_procedure;
	++p_validation_list->m_nb_procedures;
	return TRUE;
}

uint32_t FASTCALL opj_procedure_list_get_nb_procedures(const opj_procedure_list_t * p_validation_list)
{
	return p_validation_list->m_nb_procedures;
}

opj_procedure * FASTCALL opj_procedure_list_get_first_procedure(opj_procedure_list_t * p_validation_list)
{
	return p_validation_list->m_procedures;
}

void opj_procedure_list_clear(opj_procedure_list_t * p_validation_list)
{
	p_validation_list->m_nb_procedures = 0;
}
