//
//
#include "alloc.h"
#include "mem.h"
#include "memento.h"
#include "outf.h"
#include "xml.h"
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* These str_*() functions realloc buffer as required. All return 0 or -1 with
   errno set. */

/* Appends first <s_len> chars of string <s> to *p. */
static int str_catl(char** p, const char* s, int s_len)
{
	size_t p_len = (*p) ? strlen(*p) : 0;
	if(extract_realloc2(p, p_len + 1, p_len + s_len + 1)) 
		return -1;
	memcpy(*p + p_len, s, s_len);
	(*p)[p_len + s_len] = 0;
	return 0;
}

/* Appends a char.  */
static int str_catc(char** p, char c)
{
	return str_catl(p, &c, 1);
}

/* Unused but usefult o keep code here. */
#if 0
/* Appends a string. */
static int str_cat(char** p, const char* s)
{
	return str_catl(p, s, strlen(s));
}
#endif

char* extract_xml_tag_attributes_find(extract_xml_tag_t* tag, const char* name)
{
	int i;
	for(i = 0; i<tag->attributes_num; ++i) {
		if(!strcmp(tag->attributes[i].name, name)) {
			char* ret = tag->attributes[i].value;
			return ret;
		}
	}
	outf("Failed to find attribute '%s'", name);
	return NULL;
}

int extract_xml_tag_attributes_find_float(extract_xml_tag_t*  tag, const char*         name, float*              o_out)
{
	const char* value = extract_xml_tag_attributes_find(tag, name);
	if(!value) {
		errno = ESRCH;
		return -1;
	}
	if(extract_xml_str_to_float(value, o_out)) return -1;
	return 0;
}

int extract_xml_tag_attributes_find_double(extract_xml_tag_t*  tag, const char*         name, double*             o_out)
{
	const char* value = extract_xml_tag_attributes_find(tag, name);
	if(!value) {
		errno = ESRCH;
		return -1;
	}
	if(extract_xml_str_to_double(value, o_out)) return -1;
	return 0;
}

int extract_xml_tag_attributes_find_int(extract_xml_tag_t*  tag, const char*         name, int*                o_out)
{
	const char* text = extract_xml_tag_attributes_find(tag, name);
	return extract_xml_str_to_int(text, o_out);
}

int extract_xml_tag_attributes_find_uint(extract_xml_tag_t*  tag, const char*         name, unsigned*           o_out)
{
	const char* text = extract_xml_tag_attributes_find(tag, name);
	return extract_xml_str_to_uint(text, o_out);
}

int extract_xml_tag_attributes_find_size(extract_xml_tag_t*  tag, const char*         name, size_t*             o_out)
{
	const char* text = extract_xml_tag_attributes_find(tag, name);
	return extract_xml_str_to_size(text, o_out);
}

int extract_xml_str_to_llint(const char* text, long long* o_out)
{
	char* endptr;
	long long x;
	if(!text) {
		errno = ESRCH;
		return -1;
	}
	if(text[0] == 0) {
		errno = EINVAL;
		return -1;
	}
	errno = 0;
	x = strtoll(text, &endptr, 10 /*base*/);
	if(errno) {
		return -1;
	}
	if(*endptr) {
		errno = EINVAL;
		return -1;
	}
	*o_out = x;
	return 0;
}

int extract_xml_str_to_ullint(const char* text, unsigned long long* o_out)
{
	char* endptr;
	unsigned long long x;
	if(!text) {
		errno = ESRCH;
		return -1;
	}
	if(text[0] == 0) {
		errno = EINVAL;
		return -1;
	}
	errno = 0;
	x = strtoull(text, &endptr, 10 /*base*/);
	if(errno) {
		return -1;
	}
	if(*endptr) {
		errno = EINVAL;
		return -1;
	}
	*o_out = x;
	return 0;
}

int extract_xml_str_to_int(const char* text, int* o_out)
{
	long long x;
	if(extract_xml_str_to_llint(text, &x)) return -1;
	if(x > INT_MAX || x < INT_MIN) {
		errno = ERANGE;
		return -1;
	}
	*o_out = (int)x;
	return 0;
}

int extract_xml_str_to_uint(const char* text, unsigned* o_out)
{
	unsigned long long x;
	if(extract_xml_str_to_ullint(text, &x)) return -1;
	if(x > UINT_MAX) {
		errno = ERANGE;
		return -1;
	}
	*o_out = (unsigned)x;
	return 0;
}

int extract_xml_str_to_size(const char* text, size_t* o_out)
{
	unsigned long long x;
	if(extract_xml_str_to_ullint(text, &x)) return -1;
	if(x > SIZE_MAX) {
		errno = ERANGE;
		return -1;
	}
	*o_out = (size_t)x;
	return 0;
}

int extract_xml_str_to_double(const char* text, double* o_out)
{
	char* endptr;
	double x;
	if(!text) {
		errno = ESRCH;
		return -1;
	}
	if(text[0] == 0) {
		errno = EINVAL;
		return -1;
	}
	errno = 0;
	x = strtod(text, &endptr);
	if(errno) {
		return -1;
	}
	if(*endptr) {
		errno = EINVAL;
		return -1;
	}
	*o_out = x;
	return 0;
}

int extract_xml_str_to_float(const char* text, float* o_out)
{
	double x;
	if(extract_xml_str_to_double(text, &x)) {
		return -1;
	}
	if(x > FLT_MAX || x < -FLT_MAX) {
		errno = ERANGE;
		return -1;
	}
	*o_out = (float)x;
	return 0;
}

static int extract_xml_tag_attributes_append(extract_xml_tag_t*  tag, char * name, char * value)
{
	if(extract_realloc2(&tag->attributes, sizeof(extract_xml_attribute_t) * tag->attributes_num, sizeof(extract_xml_attribute_t) * (tag->attributes_num+1))) 
		return -1;
	tag->attributes[tag->attributes_num].name = name;
	tag->attributes[tag->attributes_num].value = value;
	tag->attributes_num += 1;
	return 0;
}

void extract_xml_tag_init(extract_xml_tag_t* tag)
{
	tag->name = NULL;
	tag->attributes = NULL;
	tag->attributes_num = 0;
	extract_astring_init(&tag->text);
}

void extract_xml_tag_free(extract_xml_tag_t* tag)
{
	int i;
	extract_free(&tag->name);
	for(i = 0; i<tag->attributes_num; ++i) {
		extract_xml_attribute_t* attribute = &tag->attributes[i];
		extract_free(&attribute->name);
		extract_free(&attribute->value);
	}
	extract_free(&tag->attributes);
	extract_astring_free(&tag->text);
	extract_xml_tag_init(tag);
}

/* Unused but useful to keep code here. */
#if 0
/* Like strcmp() but also handles NULL. */
static int extract_xml_strcmp_null(const char* a, const char* b)
{
	if(!a && !b) return 0;
	if(!a) return -1;
	if(!b) return 1;
	return strcmp(a, b);
}

#endif

/* Unused but usefult o keep code here. */
#if 0
/* Compares tag name, then attributes; returns -1, 0 or +1. Does not compare
   extract_xml_tag_t::text members. */
int extract_xml_compare_tags(const extract_xml_tag_t* lhs, const extract_xml_tag_t* rhs)
{
	int d;
	int i;
	d = extract_xml_strcmp_null(lhs->name, rhs->name);
	if(d) return d;
	for(i = 0;; ++i) {
		if(i >= lhs->attributes_num || i >= rhs->attributes_num) {
			break;
		}
		const extract_xml_attribute_t* lhs_attribute = &lhs->attributes[i];
		const extract_xml_attribute_t* rhs_attribute = &rhs->attributes[i];
		d = extract_xml_strcmp_null(lhs_attribute->name, rhs_attribute->name);
		if(d) return d;
		d = extract_xml_strcmp_null(lhs_attribute->value, rhs_attribute->value);
		if(d) return d;
	}
	if(lhs->attributes_num > rhs->attributes_num) return +1;
	if(lhs->attributes_num < rhs->attributes_num) return -1;
	return 0;
}

#endif

int extract_xml_pparse_init(extract_buffer_t* buffer, const char* first_line)
{
	char* first_line_buffer = NULL;
	int e = -1;
	if(first_line) {
		size_t first_line_len = strlen(first_line);
		size_t actual;
		if(extract_malloc(&first_line_buffer, first_line_len + 1)) goto end;

		if(extract_buffer_read(buffer, first_line_buffer, first_line_len, &actual)) {
			outf("error: failed to read first line.");
			goto end;
		}
		first_line_buffer[actual] = 0;
		if(strcmp(first_line, first_line_buffer)) {
			outf("Unrecognised prefix: ", first_line_buffer);
			errno = ESRCH;
			goto end;
		}
	}

	for(;;) {
		char c;
		int ee = extract_buffer_read(buffer, &c, 1, NULL);
		if(ee) {
			if(ee==1) errno = ESRCH; /* EOF. */
			goto end;
		}
		if(c == '<') {
			break;
		}
		else if(c == ' ' || c == '\n') {
		}
		else {
			outf("Expected '<' but found c=%i", c);
			goto end;
		}
	}
	e = 0;

end:
	extract_free(&first_line_buffer);
	return e;
}

static int s_next(extract_buffer_t* buffer, int* ret, char* o_c)
/* Reads next char, but if EOF sets *ret=+1, errno=ESRCH and returns +1. */
{
	int e = extract_buffer_read(buffer, o_c, 1, NULL);
	if(e == +1) {
		*ret = +1;
		errno = ESRCH;
	}
	return e;
}

static const char* extract_xml_tag_string(extract_xml_tag_t* tag)
{
	static char* buffer = NULL;
	extract_free(&buffer);
	extract_asprintf(&buffer, "<name=%s>", tag->name ? tag->name : "");
	return buffer;
}

int extract_xml_pparse_next(extract_buffer_t* buffer, extract_xml_tag_t* out)
{
	int ret = -1;
	char*   attribute_name = NULL;
	char*   attribute_value = NULL;
	char c;
	int i;
	if(0) outf("out is: %s", extract_xml_tag_string(out));
	extract_xml_tag_free(out);

	assert(buffer);

	/* Read tag name. */
	for(i = 0;; ++i) {
		int e = extract_buffer_read(buffer, &c, 1, NULL);
		if(e) {
			if(e == +1) ret = 1; /* EOF is not an error here. */
			goto end;
		}
		if(c == '>' || c == ' ') break;
		if(str_catc(&out->name, c)) goto end;
	}
	if(c == ' ') {
		/* Read attributes. */
		for(;;) {
			/* Read attribute name. */
			for(;;) {
				if(s_next(buffer, &ret, &c)) goto end;
				if(c == '=' || c == '>' || c == ' ') break;
				if(str_catc(&attribute_name, c)) goto end;
			}
			if(c == '>') break;

			if(c == '=') {
				/* Read attribute value. */
				int quote_single = 0;
				int quote_double = 0;
				size_t l;
				for(;;) {
					if(s_next(buffer, &ret, &c)) goto end;
					if(c == '\'') quote_single = !quote_single;
					else if(c == '"') quote_double = !quote_double;
					else if(!quote_single && !quote_double
					    && (c == ' ' || c == '/' || c == '>')
					    ) {
						/* We are at end of attribute value. */
						break;
					}
					else if(c == '\\') {
						// Escape next character.
						if(s_next(buffer, &ret, &c)) goto end;
					}
					if(str_catc(&attribute_value, c)) goto end;
				}

				/* Remove any enclosing quotes. */
				l = strlen(attribute_value);
				if(l >= 2) {
					if(
						(attribute_value[0] == '"' && attribute_value[l-1] == '"')
						||
						(attribute_value[0] == '\'' && attribute_value[l-1] == '\'')
						) {
						memmove(attribute_value, attribute_value+1, l-2);
						attribute_value[l-2] = 0;
					}
				}
			}

			if(extract_xml_tag_attributes_append(out, attribute_name, attribute_value)) goto end;
			attribute_name = NULL;
			attribute_value = NULL;
			if(c == '/') {
				if(s_next(buffer, &ret, &c)) goto end;
			}
			if(c == '>') break;
		}
	}

	/* Read plain text until next '<'. */
	for(;;) {
		/* We don't use s_next() here because EOF is not an error. */
		int e = extract_buffer_read(buffer, &c, 1, NULL);
		if(e == +1) {
			break; /* EOF is not an error here. */
		}
		if(e) goto end;
		if(c == '<') break;
		if(extract_astring_catc(&out->text, c)) goto end;
	}
	ret = 0;
end:
	extract_free(&attribute_name);
	extract_free(&attribute_value);
	if(ret) {
		extract_xml_tag_free(out);
	}
	return ret;
}
