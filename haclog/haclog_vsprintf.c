#include "haclog_vsprintf.h"
#include "haclog/haclog_thread.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "haclog/haclog_time.h"
#include "haclog/haclog_stacktrace.h"
#include "haclog/haclog_sleep.h"
#include "haclog/haclog_err.h"
#include "haclog/haclog_context.h"

#define HACLOG_PRINTF_SPEC_DYNAMIC -1

#define HACLOG_PRINTF_LENGTH_none 0 //!< without length
#define HACLOG_PRINTF_LENGTH_hh 0x6868 //!< (uint16_t)'h' | ((uint16_t)'h' << 8)
#define HACLOG_PRINTF_LENGTH_h 0x68 //!< (uint16_t)'h'
#define HACLOG_PRINTF_LENGTH_l 0x6C //!< (uint16_t)'l'
#define HACLOG_PRINTF_LENGTH_ll 0x6C6C //!< (uint16_t)'l' | ((uint16_t)'l' << 8)
#define HACLOG_PRINTF_LENGTH_j 0x6A //!< (uint16_t)'j'
#define HACLOG_PRINTF_LENGTH_z 0x7A //!< (uint16_t)'z'
#define HACLOG_PRINTF_LENGTH_t 0x74 //!< (uint16_t)'t'
#define HACLOG_PRINTF_LENGTH_L 0x4C //!< (uint16_t)'L'

#define HACLOG_PRINTF_FLAGS_LEFT 0x01 //!< '-'
#define HACLOG_PRINTF_FLAGS_PLUS 0x02 //!< '+'
#define HACLOG_PRINTF_FLAGS_SPACE 0x04 //!< ' '
#define HACLOG_PRINTF_FLAGS_SPECIAL 0x08 //!< '#'
#define HACLOG_PRINTF_FLAGS_ZEROPAD 0x10 //!< '0'

enum e_haclog_format_type {
	HACLOG_FT_NONE = 0,
	HACLOG_FT_STR,
	HACLOG_FT_DOUBLE,
	HACLOG_FT_LONG_DOUBLE,
	HACLOG_FT_SIZE,
	HACLOG_FT_PTR,
	HACLOG_FT_PTRDIFF,
	HACLOG_FT_CHAR,
	HACLOG_FT_UCHAR,
	HACLOG_FT_UBYTE,
	HACLOG_FT_BYTE,
	HACLOG_FT_USHORT,
	HACLOG_FT_SHORT,
	HACLOG_FT_UINT,
	HACLOG_FT_INT,
	HACLOG_FT_ULONG,
	HACLOG_FT_LONG,
	HACLOG_FT_LONGLONG,
	HACLOG_FT_ULONGLONG,
};

static bool haclog_printf_is_flags(char c)
{
	return c == '-' || c == '+' || c == ' ' || c == '#' || c == '0';
}

static bool haclog_printf_is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

static bool haclog_printf_is_length(char c)
{
	return c == 'h' || c == 'l' || c == 'j' || c == 'z' || c == 't' || c == 'L';
}

static bool haclog_printf_is_type(char c)
{
	return c == 'd' || c == 'i' || c == 'u' || c == 'o' || c == 'x' ||
		   c == 'X' || c == 'f' || c == 'F' || c == 'e' || c == 'E' ||
		   c == 'g' || c == 'G' || c == 'a' || c == 'A' || c == 'c' ||
		   c == 's' || c == 'p' || c == 'n';
}

/**
 * @brief get number in format width/precision
 *
 * @param fmt  format string pointer
 *
 * @return value of width/precision
 */
static int haclog_printf_get_num(const char **p_fmt)
{
	int ret = 0;
	if (**p_fmt == '*') {
		ret = HACLOG_PRINTF_SPEC_DYNAMIC;
		++(*p_fmt);
	} else {
		while (haclog_printf_is_digit(**p_fmt)) {
			ret = 10 * ret + ((**p_fmt) - '0');
			++(*p_fmt);
		}
	}

	return ret;
}

/**
 * @brief allocate memory space for printf primitive
 *
 * @param fmt         format string
 * @param num_params  number of parameters
 * @param num_args    number of arguments
 */
static haclog_printf_primitive_t *
haclog_printf_primitive_allocate(const char *fmt, unsigned int num_params,
								 unsigned int num_args)
{
	haclog_printf_primitive_t *primitive = NULL;

	primitive =
		(haclog_printf_primitive_t *)malloc(sizeof(haclog_printf_primitive_t));
	if (primitive == NULL) {
		haclog_set_error(HACLOG_ERR_ALLOC_MEM);
		return NULL;
	}
	memset(primitive, 0, sizeof(*primitive));

	primitive->fmt = fmt;
	primitive->fmt_len = (unsigned int)strlen(fmt);
	primitive->num_params = num_params;
	primitive->num_args = num_args;

	primitive->specs = (haclog_printf_spec_t *)malloc(
		sizeof(haclog_printf_spec_t) * num_params);
	if (primitive->specs == NULL) {
		haclog_printf_primitive_clean(primitive);
		haclog_set_error(HACLOG_ERR_ALLOC_MEM);
		return NULL;
	}

	return primitive;
}

/**
 * @brief fillup printf specifier format type with type is d/i
 *
 * @param spec  print format specifier pointer
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
static int haclog_printf_spec_fillup_ft_dt(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		spec->fmt_type = HACLOG_FT_INT;
	} break;
	case HACLOG_PRINTF_LENGTH_hh: {
		spec->fmt_type = HACLOG_FT_CHAR;
	} break;
	case HACLOG_PRINTF_LENGTH_h: {
		spec->fmt_type = HACLOG_FT_SHORT;
	} break;
	case HACLOG_PRINTF_LENGTH_l: {
		spec->fmt_type = HACLOG_FT_LONG;
	} break;
	case HACLOG_PRINTF_LENGTH_ll: {
		spec->fmt_type = HACLOG_FT_LONGLONG;
	} break;
	case HACLOG_PRINTF_LENGTH_j: {
		spec->fmt_type = HACLOG_FT_LONGLONG;
	} break;
	case HACLOG_PRINTF_LENGTH_z: {
		spec->fmt_type = HACLOG_FT_SIZE;
	} break;
	case HACLOG_PRINTF_LENGTH_t: {
		spec->fmt_type = HACLOG_FT_PTRDIFF;
	} break;
	default: {
		haclog_debug_break();
		return HACLOG_ERR_PRINTF_TYPE;
	} break;
	}
	return 0;
}

/**
 * @brief expand printf specifier info with type is u/o/x/X
 *
 * @param spec  print format specifier pointer
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
static int haclog_printf_spec_fillup_ft_uoxX(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		spec->fmt_type = HACLOG_FT_UINT;
	} break;
	case HACLOG_PRINTF_LENGTH_hh: {
		spec->fmt_type = HACLOG_FT_UCHAR;
	} break;
	case HACLOG_PRINTF_LENGTH_h: {
		spec->fmt_type = HACLOG_FT_USHORT;
	} break;
	case HACLOG_PRINTF_LENGTH_l: {
		spec->fmt_type = HACLOG_FT_ULONG;
	} break;
	case HACLOG_PRINTF_LENGTH_ll: {
		spec->fmt_type = HACLOG_FT_ULONGLONG;
	} break;
	case HACLOG_PRINTF_LENGTH_j: {
		spec->fmt_type = HACLOG_FT_ULONGLONG;
	} break;
	case HACLOG_PRINTF_LENGTH_z: {
		spec->fmt_type = HACLOG_FT_SIZE;
	} break;
	case HACLOG_PRINTF_LENGTH_t: {
		spec->fmt_type = HACLOG_FT_PTRDIFF;
	} break;
	default: {
		haclog_debug_break();
		return HACLOG_ERR_PRINTF_TYPE;
	} break;
	}
	return 0;
}

/**
 * @brief expand printf specifier info with type is f/F/e/E/g/G/a/A
 *
 * @param spec  print format specifier pointer
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
static int haclog_printf_spec_fillup_ft_fFeEgGaA(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		spec->fmt_type = HACLOG_FT_DOUBLE;
	} break;
	case HACLOG_PRINTF_LENGTH_L: {
		spec->fmt_type = HACLOG_FT_LONG_DOUBLE;
	} break;
	case HACLOG_PRINTF_LENGTH_l: {
		// NOTE:
		//   HACLOG_PRINTF_LENGTH_l(%lf) is no longer part of the standard
		haclog_debug_break();
		return HACLOG_ERR_PRINTF_TYPE;
	} break;
	default: {
		haclog_debug_break();
		return HACLOG_ERR_PRINTF_TYPE;
	} break;
	}
	return 0;
}

/**
 * @brief expand printf specifier info with type is c
 *
 * @param spec  print format specifier pointer
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
static int haclog_printf_spec_fillup_ft_c(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		spec->fmt_type = HACLOG_FT_CHAR;
	} break;
	default: {
		haclog_debug_break();
		return HACLOG_ERR_PRINTF_TYPE;
	} break;
	}
	return 0;
}

/**
 * @brief expand printf specifier info with type is s
 *
 * @param spec  print format specifier pointer
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
static int haclog_printf_spec_fillup_ft_s(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		spec->fmt_type = HACLOG_FT_STR;
	} break;
	default: {
		haclog_debug_break();
		return HACLOG_ERR_PRINTF_TYPE;
	} break;
	}
	return 0;
}

/**
 * @brief expand printf specifier info with type is p
 *
 * @param spec  print format specifier pointer
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
static int haclog_printf_spec_fillup_ft_p(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		spec->fmt_type = HACLOG_FT_PTR;
	} break;
	default: {
		haclog_debug_break();
		return HACLOG_ERR_PRINTF_TYPE;
	} break;
	}
	return 0;
}

/**
 * @brief fillup printf specifier format type
 *
 * @param spec  print format specifier pointer
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
static int haclog_printf_spec_fillup_ft(haclog_printf_spec_t *spec)
{
	switch (spec->type) {
	case 'd':
	case 'i': {
		return haclog_printf_spec_fillup_ft_dt(spec);
	} break;
	case 'u':
	case 'o':
	case 'x':
	case 'X': {
		return haclog_printf_spec_fillup_ft_uoxX(spec);
	} break;
	case 'f':
	case 'F':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
	case 'a':
	case 'A': {
		return haclog_printf_spec_fillup_ft_fFeEgGaA(spec);
	} break;
	case 'c': {
		return haclog_printf_spec_fillup_ft_c(spec);
	} break;
	case 's': {
		return haclog_printf_spec_fillup_ft_s(spec);
	} break;
	case 'p': {
		return haclog_printf_spec_fillup_ft_p(spec);
	} break;
	default: {
		haclog_debug_break();
		return HACLOG_FT_NONE;
	} break;
	}

	return 0;
}

static int haclog_printf_spec_fillup_ext(haclog_printf_spec_t *spec)
{
	int ret = 0;
	ret = haclog_printf_spec_fillup_ft(spec);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

/**
 * @brief fillup printf specifier info
 *
 * @param primitive  printf primitive pointer
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
static int haclog_printf_spec_fillup(haclog_printf_primitive_t *primitive)
{
	const char *fmt = primitive->fmt;
	unsigned int idx_spec = 0;
	while (*fmt) {
		if (*fmt != '%') {
			++fmt;
			continue;
		} else {
			if (*(fmt + 1) == '%') {
				// %%
				fmt += 2;
				continue;
			}

			if (idx_spec >= primitive->num_params) {
				break;
			}

			haclog_printf_spec_t *spec = primitive->specs + idx_spec;
			memset(spec, 0, sizeof(*spec));
			spec->pos_begin = fmt - primitive->fmt;

			++fmt;

			// flags
			while (haclog_printf_is_flags(*fmt)) {
				switch (*fmt) {
				case '-': {
					spec->flags |= HACLOG_PRINTF_FLAGS_LEFT;
				} break;
				case '+': {
					spec->flags |= HACLOG_PRINTF_FLAGS_PLUS;
				} break;
				case ' ': {
					spec->flags |= HACLOG_PRINTF_FLAGS_SPACE;
				} break;
				case '#': {
					spec->flags |= HACLOG_PRINTF_FLAGS_SPECIAL;
				} break;
				case '0': {
					spec->flags |= HACLOG_PRINTF_FLAGS_ZEROPAD;
				} break;
				}
				++fmt;
			}

			// width
			spec->width = haclog_printf_get_num(&fmt);

			// .precision
			if (*fmt == '.') {
				++fmt;
				spec->precision = haclog_printf_get_num(&fmt);
			}

			// length
			while (haclog_printf_is_length(*fmt)) {
				if (spec->length > (uint16_t)0x01 << 8) {
					return HACLOG_ERR_PRINTF_SPEC_LENGTH;
				}
				if (spec->length == 0) {
					spec->length = (uint16_t)*fmt;
				} else {
					spec->length |= (((uint16_t)*fmt) << 8);
				}
				++fmt;
			}

			// type
			if (!haclog_printf_is_type(*fmt)) {
				return HACLOG_ERR_PRINTF_TYPE;
			}
			spec->type = *fmt;

			++fmt;
			spec->pos_end = fmt - primitive->fmt;

			// extend
			haclog_printf_spec_fillup_ext(spec);

			++idx_spec;
		}
	}

	return 0;
}

#if HACLOG_ENABLE_DEBUG_FUNCS

static void haclog_spec_type_di_output(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		fprintf(stdout, "int");
	} break;
	case HACLOG_PRINTF_LENGTH_hh: {
		fprintf(stdout, "signed char");
	} break;
	case HACLOG_PRINTF_LENGTH_h: {
		fprintf(stdout, "short int");
	} break;
	case HACLOG_PRINTF_LENGTH_l: {
		fprintf(stdout, "long int");
	} break;
	case HACLOG_PRINTF_LENGTH_ll: {
		fprintf(stdout, "long long int");
	} break;
	case HACLOG_PRINTF_LENGTH_j: {
		fprintf(stdout, "intmax_t");
	} break;
	case HACLOG_PRINTF_LENGTH_z: {
		fprintf(stdout, "size_t");
	} break;
	case HACLOG_PRINTF_LENGTH_t: {
		fprintf(stdout, "ptrdiff_t");
	} break;
	}
}

static void haclog_spec_type_uoxX_output(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		fprintf(stdout, "unsigned int");
	} break;
	case HACLOG_PRINTF_LENGTH_hh: {
		fprintf(stdout, "unsigned char ");
	} break;
	case HACLOG_PRINTF_LENGTH_h: {
		fprintf(stdout, "unsigned short int");
	} break;
	case HACLOG_PRINTF_LENGTH_l: {
		fprintf(stdout, "unsigned long int");
	} break;
	case HACLOG_PRINTF_LENGTH_ll: {
		fprintf(stdout, "unsigned long long int");
	} break;
	case HACLOG_PRINTF_LENGTH_j: {
		fprintf(stdout, "uintmax_t");
	} break;
	case HACLOG_PRINTF_LENGTH_z: {
		fprintf(stdout, "size_t");
	} break;
	case HACLOG_PRINTF_LENGTH_t: {
		fprintf(stdout, "ptrdiff_t");
	} break;
	}
}

static void haclog_spec_type_fFeEgGaA_output(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		fprintf(stdout, "double");
	} break;
	case HACLOG_PRINTF_LENGTH_L: {
		fprintf(stdout, "long double");
	} break;
	}
}

static void haclog_spec_type_c_output(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		fprintf(stdout, "int");
	} break;
	case HACLOG_PRINTF_LENGTH_l: {
		fprintf(stdout, "wint_t");
	} break;
	}
}

static void haclog_spec_type_s_output(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		fprintf(stdout, "char*");
	} break;
	case HACLOG_PRINTF_LENGTH_l: {
		fprintf(stdout, "wchar_t*");
	} break;
	}
}

static void haclog_spec_type_p_output(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		fprintf(stdout, "void*");
	} break;
	}
}

static void haclog_spec_type_n_output(haclog_printf_spec_t *spec)
{
	switch (spec->length) {
	case HACLOG_PRINTF_LENGTH_none: {
		fprintf(stdout, "int*");
	} break;
	case HACLOG_PRINTF_LENGTH_hh: {
		fprintf(stdout, "signed char*");
	} break;
	case HACLOG_PRINTF_LENGTH_h: {
		fprintf(stdout, "short int*");
	} break;
	case HACLOG_PRINTF_LENGTH_l: {
		fprintf(stdout, "long int*");
	} break;
	case HACLOG_PRINTF_LENGTH_ll: {
		fprintf(stdout, "long long int*");
	} break;
	case HACLOG_PRINTF_LENGTH_j: {
		fprintf(stdout, "intmax_t*");
	} break;
	case HACLOG_PRINTF_LENGTH_z: {
		fprintf(stdout, "size_t*");
	} break;
	case HACLOG_PRINTF_LENGTH_t: {
		fprintf(stdout, "ptrdiff_t*");
	} break;
	}
}

/*
| length | d i           | u o x X                | f F e E g G a A | c      | s        | p     | n              |
------------------------------------------------------------------------------------------------------------------
| (none) | int           | unsigned int           | double          | int    | char*    | void* | int*           |
| hh     | signed char   | unsigned char          | signed char*    |        |          |       |                |
| h      | short int     | unsigned short int     |                 |        |          |       | short int*     |
| l      | long int      | unsigned long int      |                 | wint_t | wchar_t* |       | long int*      |
| ll     | long long int | unsigned long long int |                 |        |          |       | long long int* |
| j      | intmax_t      | uintmax_t              |                 |        |          |       | intmax_t*      |
| z      | size_t        | size_t                 |                 |        |          |       | size_t*        |
| t      | ptrdiff_t     | ptrdiff_t              |                 |        |          |       | ptrdiff_t*     |
| L      |               |                        | long double     |        |          |       |                |
 * */

static void haclog_spec_type_output(haclog_printf_spec_t *spec)
{
	switch (spec->type) {
	case 'd':
	case 'i': {
		haclog_spec_type_di_output(spec);
	} break;
	case 'u':
	case 'o':
	case 'x':
	case 'X': {
		haclog_spec_type_uoxX_output(spec);
	} break;
	case 'f':
	case 'F':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
	case 'a':
	case 'A': {
		haclog_spec_type_fFeEgGaA_output(spec);
	} break;
	case 'c': {
		haclog_spec_type_c_output(spec);
	} break;
	case 's': {
		haclog_spec_type_s_output(spec);
	} break;
	case 'p': {
		haclog_spec_type_p_output(spec);
	} break;
	case 'n': {
		haclog_spec_type_n_output(spec);
	} break;
	}
}

static void haclog_printf_spec_show(const char *fmt, haclog_printf_spec_t *spec)
{
	fwrite("[", 1, 1, stdout);
	fwrite(&fmt[spec->pos_begin], 1, spec->pos_end - spec->pos_begin, stdout);
	fwrite("]: ", 1, 3, stdout);

	fwrite("%", 1, 1, stdout);

	// flags
	fwrite("[", 1, 1, stdout);
	if (spec->flags & HACLOG_PRINTF_FLAGS_LEFT) {
		fwrite("-", 1, 1, stdout);
	}
	if (spec->flags & HACLOG_PRINTF_FLAGS_PLUS) {
		fwrite("+", 1, 1, stdout);
	}
	if (spec->flags & HACLOG_PRINTF_FLAGS_SPACE) {
		fwrite(" ", 1, 1, stdout);
	}
	if (spec->flags & HACLOG_PRINTF_FLAGS_SPECIAL) {
		fwrite("#", 1, 1, stdout);
	}
	if (spec->flags & HACLOG_PRINTF_FLAGS_ZEROPAD) {
		fwrite("0", 1, 1, stdout);
	}
	fwrite("]", 1, 1, stdout);

	// width
	fwrite("[", 1, 1, stdout);
	if (spec->width != 0) {
		if (spec->width == HACLOG_PRINTF_SPEC_DYNAMIC) {
			fwrite("*", 1, 1, stdout);
		} else {
			fprintf(stdout, "%d", spec->width);
		}
	}
	fwrite("]", 1, 1, stdout);

	// precision
	fwrite("[", 1, 1, stdout);
	if (spec->precision != 0) {
		if (spec->precision == HACLOG_PRINTF_SPEC_DYNAMIC) {
			fwrite("*", 1, 1, stdout);
		} else {
			fprintf(stdout, "%d", spec->precision);
		}
	}
	fwrite("]", 1, 1, stdout);

	// length and specifiers
	haclog_spec_type_output(spec);

	fwrite("\n", 1, 1, stdout);
	fflush(stdout);
}

void haclog_printf_primitive_show(haclog_printf_primitive_t *primitive)
{
	fprintf(stdout, "--------------------------------\n");
	fwrite(primitive->fmt, 1, primitive->fmt_len, stdout);
	fwrite("\n", 1, 1, stdout);
	fprintf(stdout, "total param size: %u\n", primitive->param_size);

	fprintf(stdout, "%d|%s:%d|%s\n", primitive->loc.level, primitive->loc.file,
			primitive->loc.line, primitive->loc.func);

	for (unsigned int i = 0; i < primitive->num_params; i++) {
		haclog_printf_spec_show(primitive->fmt, &primitive->specs[i]);
	}
}

#endif

haclog_printf_primitive_t *
haclog_printf_primitive_gen(const char *fmt, const haclog_printf_loc_t *loc)
{
	haclog_printf_primitive_t *primitive = NULL;
	unsigned int num_params = 0;
	unsigned int num_args = 0;
	int ret = 0;

	// get number of parameters and arguments
	ret = haclog_printf_num_params(fmt, &num_params, &num_args);
	if (ret != 0) {
		haclog_set_error(ret);
		return NULL;
	}

	// allocate memory space
	primitive = haclog_printf_primitive_allocate(fmt, num_params, num_args);
	if (primitive == NULL) {
		return NULL;
	}

	// fillup printf specifier info
	ret = haclog_printf_spec_fillup(primitive);
	if (ret != 0) {
		haclog_printf_primitive_clean(primitive);
		return NULL;
	}

	// fillup local info
	if (loc) {
		primitive->loc.file = loc->file;
		primitive->loc.func = loc->func;
		primitive->loc.line = loc->line;
		primitive->loc.level = loc->level;
	} else {
		haclog_printf_primitive_clean(primitive);
		haclog_set_error(HACLOG_ERR_ARGUMENTS);
		return NULL;
	}

	// set param size
	primitive->param_size = 0;
	for (unsigned int i = 0; i < num_params; ++i) {
		primitive->param_size +=
			haclog_printf_spec_param_size(&primitive->specs[i]);
	}

	return primitive;
}

void haclog_printf_primitive_clean(haclog_printf_primitive_t *primitive)
{
	if (primitive) {
		if (primitive->specs) {
			free(primitive->specs);
			primitive->specs = NULL;
		}

		free(primitive);
	}
}

int haclog_printf_num_params(const char *fmt, unsigned int *num_params,
							 unsigned int *num_args)
{
	*num_params = 0;
	*num_args = 0;

	while (*fmt) {
		if (*fmt != '%') {
			++fmt;
			continue;
		} else {
			if (*(fmt + 1) == '%') {
				// %%
				fmt += 2;
				continue;
			}

			++fmt;

			// flags
			while (haclog_printf_is_flags(*fmt)) {
				++fmt;
			}

			// width
			if (haclog_printf_get_num(&fmt) == HACLOG_PRINTF_SPEC_DYNAMIC) {
				++(*num_args);
			}

			// .precision
			int length = 0;
			if (*fmt == '.') {
				++fmt;
				if (haclog_printf_get_num(&fmt) == HACLOG_PRINTF_SPEC_DYNAMIC) {
					++(*num_args);
				}
			}

			// length
			while (haclog_printf_is_length(*fmt)) {
				if (length > (uint16_t)0x01 << 8) {
					return HACLOG_ERR_PRINTF_SPEC_LENGTH;
				}
				if (length == 0) {
					length = (uint16_t)*fmt;
				} else {
					length |= ((uint16_t)*fmt << 8);
				}
				++fmt;
			}

			// specifier
			if (!haclog_printf_is_type(*fmt)) {
				return HACLOG_ERR_PRINTF_TYPE;
			}

			++fmt;
			++(*num_params);
			++(*num_args);
		}
	}

	return 0;
}

int haclog_printf_spec_param_size(haclog_printf_spec_t *spec)
{
	int param_size = 0;
	switch (spec->fmt_type) {
	case HACLOG_FT_NONE: {
		haclog_debug_break();
		param_size = sizeof(haclog_serialize_placeholder);
	} break;
	case HACLOG_FT_STR: {
		param_size = sizeof(haclog_serialize_placeholder);
	} break;
	case HACLOG_FT_DOUBLE: {
		param_size = sizeof(haclog_serialize_placeholder);
	} break;
	case HACLOG_FT_LONG_DOUBLE: {
		param_size = sizeof(long double);
	} break;
	case HACLOG_FT_SIZE:
	case HACLOG_FT_PTR:
	case HACLOG_FT_PTRDIFF:
	case HACLOG_FT_CHAR:
	case HACLOG_FT_UCHAR:
	case HACLOG_FT_UBYTE:
	case HACLOG_FT_BYTE:
	case HACLOG_FT_USHORT:
	case HACLOG_FT_SHORT:
	case HACLOG_FT_UINT:
	case HACLOG_FT_INT:
	case HACLOG_FT_ULONG:
	case HACLOG_FT_LONG:
	case HACLOG_FT_LONGLONG:
	case HACLOG_FT_ULONGLONG: {
		param_size = sizeof(haclog_serialize_placeholder);
	} break;
	default: {
		haclog_debug_break();
		param_size = sizeof(haclog_serialize_placeholder);
	} break;
	}

	if (spec->width == HACLOG_PRINTF_SPEC_DYNAMIC) {
		param_size += sizeof(haclog_serialize_placeholder);
	}
	if (spec->precision == HACLOG_PRINTF_SPEC_DYNAMIC) {
		param_size += sizeof(haclog_serialize_placeholder);
	}

	return param_size;
}

typedef struct haclog_str_cache {
	const char *s; //!< string pointer
	unsigned int slen; //!< string size in bytes buffer
	unsigned int copy_slen; //!< string copy length
} haclog_str_cache_t;

#define HACLOG_MAX_STR_CACHE 128

#define HACLOG_SERIALIZE(p, v)                                       \
	static_assert(sizeof(v) <= sizeof(haclog_serialize_placeholder), \
				  "value not le sizeof(placeholder)");               \
	memcpy(p, &v, sizeof(v));                                        \
	p += sizeof(haclog_serialize_placeholder);

#define HACLOG_SERIALIZE_VA_ARG(p, type) \
	type v = va_arg(args, type);         \
	HACLOG_SERIALIZE(p, v);

#define HACLOG_FETCH_W(bytes_buf, r, w, num_bytes, ret)                     \
	fetch_loop_cnt = 0;                                                     \
	do {                                                                    \
		ret = haclog_bytes_buffer_w_fc(bytes_buf, num_bytes, r, w);         \
		if (ret >= 0) {                                                     \
			break;                                                          \
		}                                                                   \
		if (ret == -1) {                                                    \
			haclog_debug_break();                                           \
			return;                                                         \
		}                                                                   \
		if (fetch_loop_cnt++) {                                             \
			haclog_thread_yield();                                          \
		}                                                                   \
		r = haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed); \
	} while (1);

void haclog_printf_primitive_serialize(haclog_bytes_buffer_t *bytes_buf,
									   haclog_printf_primitive_t *primitive,
									   const char *fmt_str, ...)
{
	haclog_context_t *ctx = haclog_context_get();
	if (primitive->loc.level < ctx->level) {
		return;
	}

	const haclog_atomic_int hdr_size =
		(haclog_atomic_int)sizeof(haclog_serialize_hdr_t);
	static haclog_thread_local haclog_atomic_int r = 0;
	// haclog_atomic_int r =
	//     haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed);
	haclog_atomic_int w = bytes_buf->w;
	haclog_atomic_int w_hdr = 0;
	haclog_atomic_int w_const_args = 0;
	haclog_atomic_int w_str = 0;
	haclog_atomic_int w_cache_line = 0;
	char *p = NULL;
	unsigned int fetch_loop_cnt = 0;

	HACLOG_FETCH_W(bytes_buf, r, w, hdr_size, w_hdr);
	w = w_hdr + hdr_size;

	HACLOG_FETCH_W(bytes_buf, r, w, primitive->param_size, w_const_args);
	w = w_const_args + primitive->param_size;

	p = haclog_bytes_buffer_get(bytes_buf, w_const_args);

	// serialize const arguments
	haclog_str_cache_t str_cache[HACLOG_MAX_STR_CACHE + 1];
	unsigned int n_str_idx = 0;
	unsigned int n_str = 0;
	unsigned long extra_len = 0;

	va_list args;
	va_start(args, fmt_str);
	for (unsigned int i = 0; i < primitive->num_params; ++i) {
		haclog_printf_spec_t *spec = &primitive->specs[i];
		unsigned long precision = 0;

		if (spec->width == HACLOG_PRINTF_SPEC_DYNAMIC) {
			int width = va_arg(args, int);
			HACLOG_SERIALIZE(p, width);
		}

		if (spec->precision == HACLOG_PRINTF_SPEC_DYNAMIC) {
			precision = va_arg(args, int);
			HACLOG_SERIALIZE(p, precision);
		} else {
			precision = spec->precision;
		}

		switch (spec->fmt_type) {
		case HACLOG_FT_STR: {
			if (n_str_idx == HACLOG_MAX_STR_CACHE) {
				str_cache[n_str_idx].s = "(haclog str cache full)";
				str_cache[n_str_idx].copy_slen = strlen(str_cache[n_str_idx].s);
				str_cache[n_str_idx].slen = HACLOG_ROUND_TO_2POWX(
					str_cache[n_str_idx].copy_slen + 1, 8);

				HACLOG_SERIALIZE(p, extra_len);
				extra_len += str_cache[n_str_idx].slen;
			} else {
				const char *s = va_arg(args, const char *);
				unsigned long slen = 0;
				unsigned long copy_slen = 0;
				if (spec->precision == 0) {
					copy_slen = strlen(s);
				} else {
					while (s[copy_slen]) {
						if (copy_slen >= precision) {
							break;
						}
						copy_slen++;
					}
				}
				slen = HACLOG_ROUND_TO_2POWX(copy_slen + 1, 8);

				str_cache[n_str_idx].s = s;
				str_cache[n_str_idx].copy_slen = copy_slen;
				str_cache[n_str_idx].slen = slen;
				++n_str_idx;

				HACLOG_SERIALIZE(p, extra_len);
				extra_len += slen;
			}

			++n_str;
		} break;
		case HACLOG_FT_DOUBLE: {
			HACLOG_SERIALIZE_VA_ARG(p, double);
		} break;
		case HACLOG_FT_LONG_DOUBLE: {
			long double v = va_arg(args, long double);
			memcpy(p, &v, sizeof(v));
			p += sizeof(v);
		} break;
		case HACLOG_FT_SIZE: {
			HACLOG_SERIALIZE_VA_ARG(p, size_t);
		} break;
		case HACLOG_FT_PTR: {
			HACLOG_SERIALIZE_VA_ARG(p, void *);
		} break;
		case HACLOG_FT_PTRDIFF: {
			HACLOG_SERIALIZE_VA_ARG(p, ptrdiff_t);
		} break;
		case HACLOG_FT_CHAR:
		case HACLOG_FT_BYTE: {
			char v = (char)va_arg(args, int);
			HACLOG_SERIALIZE(p, v);
		} break;
		case HACLOG_FT_UCHAR:
		case HACLOG_FT_UBYTE: {
			unsigned char v = (unsigned char)va_arg(args, unsigned int);
			HACLOG_SERIALIZE(p, v);
		} break;
		case HACLOG_FT_SHORT: {
			short v = (short)va_arg(args, int);
			HACLOG_SERIALIZE(p, v);
		} break;
		case HACLOG_FT_USHORT: {
			unsigned short v = (unsigned short)va_arg(args, unsigned int);
			HACLOG_SERIALIZE(p, v);
		} break;
		case HACLOG_FT_INT: {
			HACLOG_SERIALIZE_VA_ARG(p, int);
		} break;
		case HACLOG_FT_UINT: {
			HACLOG_SERIALIZE_VA_ARG(p, unsigned int);
		} break;
		case HACLOG_FT_LONG: {
			HACLOG_SERIALIZE_VA_ARG(p, long int);
		} break;
		case HACLOG_FT_ULONG: {
			HACLOG_SERIALIZE_VA_ARG(p, unsigned long);
		} break;
		case HACLOG_FT_LONGLONG: {
			HACLOG_SERIALIZE_VA_ARG(p, long long);
		} break;
		case HACLOG_FT_ULONGLONG: {
			HACLOG_SERIALIZE_VA_ARG(p, unsigned long long);
		} break;
		default: {
			haclog_debug_break();
		} break;
		}
	}
	va_end(args);

	// serialize string arguments
	HACLOG_FETCH_W(bytes_buf, r, w, extra_len, w_str);
	w = w_str + extra_len;

	p = haclog_bytes_buffer_get(bytes_buf, w_str);

	for (unsigned int i = 0; i < n_str_idx; ++i) {
		memcpy(p, str_cache[i].s, str_cache[i].copy_slen);
		*(p + str_cache[i].copy_slen) = '\0';
		p += str_cache[i].slen;
	}
	for (unsigned int i = n_str_idx; i < n_str; ++i) {
		memcpy(p, str_cache[HACLOG_MAX_STR_CACHE].s,
			   str_cache[HACLOG_MAX_STR_CACHE].copy_slen);
		*(p + str_cache[HACLOG_MAX_STR_CACHE].copy_slen) = '\0';
		p += str_cache[HACLOG_MAX_STR_CACHE].slen;
	}

	// cache line
	HACLOG_FETCH_W(bytes_buf, r, w, HACLOG_CACHE_INTERVAL, w_cache_line);
	w = w_cache_line + HACLOG_CACHE_INTERVAL;

	// fillup hdr
	haclog_serialize_hdr_t *hdr =
		(haclog_serialize_hdr_t *)haclog_bytes_buffer_get(bytes_buf, w_hdr);
	haclog_realtime_get(hdr->ts);
	hdr->pos_const = w_const_args;
	hdr->pos_str = w_str;
	hdr->extra_len = extra_len;
	hdr->pos_end = w;
	hdr->primitive = primitive;

	// move writer
	haclog_bytes_buffer_w_move(bytes_buf, w);
}

#define HACLOG_MAX_FMT_ITEM 128

static int haclog_printf_primitive_format_str(const char *fmt,
											  haclog_printf_spec_t *spec,
											  int width, int precision,
											  const char *s, char *buf,
											  int buf_remain)
{
	char fmt_str[HACLOG_MAX_FMT_ITEM];
	int len = spec->pos_end - spec->pos_begin;
	if (len >= (int)sizeof(fmt_str)) {
		len = (int)sizeof(fmt_str) - 1;
	}
	memcpy(fmt_str, fmt + spec->pos_begin, len);
	fmt_str[len] = '\0';

	if (width > 0) {
		if (precision > 0) {
			return snprintf(buf, buf_remain, fmt_str, width, precision, s);
		} else {
			return snprintf(buf, buf_remain, fmt_str, width, s);
		}
	} else {
		if (precision > 0) {
			return snprintf(buf, buf_remain, fmt_str, precision, s);
		} else {
			return snprintf(buf, buf_remain, fmt_str, s);
		}
	}
}

#define HACLOG_FORMAT_VAL_FUNC(name, type, v)                                  \
	static int haclog_printf_primitive_format_##name(                          \
		const char *fmt, haclog_printf_spec_t *spec, int width, int precision, \
		type v, char *buf, int buf_remain)                                     \
	{                                                                          \
		char fmt_str[HACLOG_MAX_FMT_ITEM];                                     \
		int len = spec->pos_end - spec->pos_begin;                             \
		if (len >= (int)sizeof(fmt_str)) {                                     \
			len = (int)sizeof(fmt_str) - 1;                                    \
		}                                                                      \
		memcpy(fmt_str, fmt + spec->pos_begin, len);                           \
		fmt_str[len] = '\0';                                                   \
                                                                               \
		if (width > 0) {                                                       \
			if (precision > 0) {                                               \
				return snprintf(buf, buf_remain, fmt_str, width, precision,    \
								v);                                            \
			} else {                                                           \
				return snprintf(buf, buf_remain, fmt_str, width, v);           \
			}                                                                  \
		} else {                                                               \
			if (precision > 0) {                                               \
				return snprintf(buf, buf_remain, fmt_str, precision, v);       \
			} else {                                                           \
				return snprintf(buf, buf_remain, fmt_str, v);                  \
			}                                                                  \
		}                                                                      \
	}

#define HACLOG_FORMAT_VAL_CALL(name, v)                                    \
	haclog_printf_primitive_format_##name(primitive->fmt, spec, width,     \
										  precision, v, buf + total_bytes, \
										  buf_remain);

HACLOG_FORMAT_VAL_FUNC(double, double, v)
HACLOG_FORMAT_VAL_FUNC(long_double, long double, v)
HACLOG_FORMAT_VAL_FUNC(size, size_t, v)
HACLOG_FORMAT_VAL_FUNC(ptr, void *, v)
HACLOG_FORMAT_VAL_FUNC(ptrdiff, ptrdiff_t, v)
HACLOG_FORMAT_VAL_FUNC(char, char, v)
HACLOG_FORMAT_VAL_FUNC(uchar, unsigned char, v)
HACLOG_FORMAT_VAL_FUNC(short, short, v)
HACLOG_FORMAT_VAL_FUNC(ushort, unsigned short, v)
HACLOG_FORMAT_VAL_FUNC(int, int, v)
HACLOG_FORMAT_VAL_FUNC(uint, unsigned int, v)
HACLOG_FORMAT_VAL_FUNC(long_int, long int, v)
HACLOG_FORMAT_VAL_FUNC(u_long_int, unsigned long, v)
HACLOG_FORMAT_VAL_FUNC(long_long_int, long long, v)
HACLOG_FORMAT_VAL_FUNC(u_long_long_int, unsigned long long, v)

#define HACLOG_DESERIALIZE(p, v)                                     \
	static_assert(sizeof(v) <= sizeof(haclog_serialize_placeholder), \
				  "value not le sizeof(placeholder)");               \
	memcpy(&v, p, sizeof(v));                                        \
	p += sizeof(haclog_serialize_placeholder);

#define HACLOG_DESERIALIZE_VA_ARG(p, type, v) \
	type v;                                   \
	HACLOG_DESERIALIZE(p, v);

int haclog_printf_primitive_format(haclog_bytes_buffer_t *bytes_buf,
								   haclog_meta_info_t *meta,
								   haclog_atomic_int w, char *buf,
								   size_t bufsize)
{
	if (bufsize < 1) {
		haclog_set_error(HACLOG_ERR_ARGUMENTS);
		return -1;
	}

	if (bytes_buf->r == w) {
		return -2;
	}

	if (bytes_buf->capacity - bytes_buf->r <
		(int)sizeof(haclog_serialize_hdr_t)) {
		bytes_buf->r = 0;
	}

	haclog_serialize_hdr_t *hdr =
		(haclog_serialize_hdr_t *)haclog_bytes_buffer_get(bytes_buf,
														  bytes_buf->r);
	int total_bytes = 0;
	unsigned int fmt_pos = 0;
	int buf_remain = (int)bufsize - 1;
	char *p = haclog_bytes_buffer_get(bytes_buf, hdr->pos_const);
	int n = 0;

	haclog_printf_primitive_t *primitive = hdr->primitive;
	for (unsigned int i = 0; i < primitive->num_params; ++i) {
		haclog_printf_spec_t *spec = primitive->specs + i;
		if (spec->pos_begin > fmt_pos) {
			n = spec->pos_begin - fmt_pos;
			if (n > buf_remain) {
				n = buf_remain;
			}

			if (n > 0) {
				memcpy(buf + total_bytes, primitive->fmt + fmt_pos, n);

				total_bytes += n;
				buf_remain -= n;
			}
		}
		fmt_pos = spec->pos_end;

		int width = 0;
		if (spec->width == HACLOG_PRINTF_SPEC_DYNAMIC) {
			HACLOG_DESERIALIZE(p, width);
		}

		int precision = 0;
		if (spec->precision == HACLOG_PRINTF_SPEC_DYNAMIC) {
			HACLOG_DESERIALIZE(p, precision);
		}

		n = 0;
		switch (spec->fmt_type) {
		case HACLOG_FT_STR: {
			unsigned long offset = 0;
			HACLOG_DESERIALIZE(p, offset);
			const char *s =
				haclog_bytes_buffer_get(bytes_buf, hdr->pos_str + offset);
			n = haclog_printf_primitive_format_str(primitive->fmt, spec, width,
												   precision, s,
												   buf + total_bytes,
												   buf_remain);
		} break;
		case HACLOG_FT_DOUBLE: {
			HACLOG_DESERIALIZE_VA_ARG(p, double, v);
			n = HACLOG_FORMAT_VAL_CALL(double, v);
		} break;
		case HACLOG_FT_LONG_DOUBLE: {
			long double v = 0.0;
			memcpy(&v, p, sizeof(v));
			p += sizeof(long double);
			n = HACLOG_FORMAT_VAL_CALL(long_double, v);
		} break;
		case HACLOG_FT_SIZE: {
			HACLOG_DESERIALIZE_VA_ARG(p, size_t, v);
			n = HACLOG_FORMAT_VAL_CALL(size, v);
		} break;
		case HACLOG_FT_PTR: {
			HACLOG_DESERIALIZE_VA_ARG(p, void *, v);
			n = HACLOG_FORMAT_VAL_CALL(ptr, v);
		} break;
		case HACLOG_FT_PTRDIFF: {
			HACLOG_DESERIALIZE_VA_ARG(p, ptrdiff_t, v);
			n = HACLOG_FORMAT_VAL_CALL(ptrdiff, v);
		} break;
		case HACLOG_FT_CHAR:
		case HACLOG_FT_BYTE: {
			HACLOG_DESERIALIZE_VA_ARG(p, char, v);
			n = HACLOG_FORMAT_VAL_CALL(char, v);
		} break;
		case HACLOG_FT_UCHAR:
		case HACLOG_FT_UBYTE: {
			HACLOG_DESERIALIZE_VA_ARG(p, unsigned char, v);
			n = HACLOG_FORMAT_VAL_CALL(uchar, v);
		} break;
		case HACLOG_FT_SHORT: {
			HACLOG_DESERIALIZE_VA_ARG(p, short, v);
			n = HACLOG_FORMAT_VAL_CALL(short, v);
		} break;
		case HACLOG_FT_USHORT: {
			HACLOG_DESERIALIZE_VA_ARG(p, unsigned short, v);
			n = HACLOG_FORMAT_VAL_CALL(ushort, v);
		} break;
		case HACLOG_FT_INT: {
			HACLOG_DESERIALIZE_VA_ARG(p, int, v);
			n = HACLOG_FORMAT_VAL_CALL(int, v);
		} break;
		case HACLOG_FT_UINT: {
			HACLOG_DESERIALIZE_VA_ARG(p, unsigned int, v);
			n = HACLOG_FORMAT_VAL_CALL(uint, v);
		} break;
		case HACLOG_FT_LONG: {
			HACLOG_DESERIALIZE_VA_ARG(p, long int, v);
			n = HACLOG_FORMAT_VAL_CALL(long_int, v);
		} break;
		case HACLOG_FT_ULONG: {
			HACLOG_DESERIALIZE_VA_ARG(p, unsigned long int, v);
			n = HACLOG_FORMAT_VAL_CALL(u_long_int, v);
		} break;
		case HACLOG_FT_LONGLONG: {
			HACLOG_DESERIALIZE_VA_ARG(p, long long int, v);
			n = HACLOG_FORMAT_VAL_CALL(long_long_int, v);
		} break;
		case HACLOG_FT_ULONGLONG: {
			HACLOG_DESERIALIZE_VA_ARG(p, unsigned long long int, v);
			n = HACLOG_FORMAT_VAL_CALL(u_long_long_int, v);
		} break;
		default: {
			haclog_debug_break();
		} break;
		}

		// NOTE:
		//   snprintf return the number of caracters would have been written to
		//   the fianl string **if enough space had been available**
		//   so need to manual compare n and buf_remain!!!
		//
		// e.g.
		//   char buf[4];
		//   memset(buf, 0, sizeof(buf));
		//   int n = snprintf(buf, sizeof(buf), "%s", "0123456789");
		//
		//   * the result:
		//     buf = "012\0"
		//     n = 10
		if (n > buf_remain) {
			n = buf_remain;
		}

		total_bytes += n;
		buf_remain -= n;

		if (buf_remain <= 0) {
			break;
		}
	}

	if (fmt_pos != primitive->fmt_len) {
		n = primitive->fmt_len - fmt_pos;
		if (n > buf_remain) {
			n = buf_remain;
		}

		memcpy(buf + total_bytes, primitive->fmt + fmt_pos, n);

		total_bytes += n;
		buf_remain -= n;
	}

	buf[total_bytes] = '\0';

	if (meta) {
		meta->loc = &primitive->loc;
		memcpy(&meta->ts, &hdr->ts, sizeof(meta->ts));
	}

	haclog_bytes_buffer_r_move(bytes_buf, hdr->pos_end);

	return total_bytes;
}
