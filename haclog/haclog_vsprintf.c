#include "haclog_vsprintf.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "haclog/haclog_err.h"

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
		// TODO: assert false
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
		// TODO: assert false
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
	default: {
		// TODO: assert false
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
		// TODO: assert false
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
		// TODO: assert false
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
		// TODO: assert false
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
		// TODO: assert false
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
				++fmt;
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
| l	     | long int	     | unsigned long int      |                 | wint_t | wchar_t* |       | long int*      |
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
				++fmt;
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
		// TODO: assert false
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
		// TODO: assert false
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

typedef struct haclog_str_cache {
	const char *s; //!< string pointer
	unsigned int slen; //!< string size in bytes buffer
	unsigned int copy_slen; //!< string copy length
} haclog_str_cache_t;

#define HACLOG_MAX_STR_CACHE 128

#define HACLOG_SERIALIZE_WRITE(p, num)                                 \
	static_assert(sizeof(num) <= sizeof(haclog_serialize_placeholder), \
				  "value not le sizeof(placeholder)");                 \
	memcpy(p, &num, sizeof(num));                                      \
	p += sizeof(haclog_serialize_placeholder);

#define HACLOG_SERIALIZE_VA_ARG(p, type) \
	type v = va_arg(args, type);         \
	HACLOG_SERIALIZE_WRITE(p, v);

void haclog_printf_primitive_serialize(haclog_bytes_buffer_t *bytes_buf,
									   haclog_printf_primitive_t *primitive,
									   ...)
{
	const haclog_atomic_int hdr_size =
		(haclog_atomic_int)sizeof(haclog_serialize_hdr_t);
	haclog_atomic_int r =
		haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed);
	haclog_atomic_int w = bytes_buf->w;
	haclog_atomic_int w_hdr = 0;
	haclog_atomic_int w_const_args = 0;
	haclog_atomic_int w_str = 0;
	char *p = NULL;

	w_hdr = haclog_bytes_buffer_w_fc(bytes_buf, hdr_size, &r, w);
	if (w_hdr == -1) {
		// TODO: assert false
		return;
	}
	w = w_hdr + hdr_size;

	w_const_args =
		haclog_bytes_buffer_w_fc(bytes_buf, primitive->param_size, &r, w);
	if (w_const_args == -1) {
		// TODO: assert false
		return;
	}
	w = w_const_args + primitive->param_size;

	p = haclog_bytes_buffer_get(bytes_buf, w_const_args);

	// serialize const arguments
	haclog_str_cache_t str_cache[HACLOG_MAX_STR_CACHE + 1];
	unsigned int n_str_idx = 0;
	unsigned int n_str = 0;
	unsigned long extra_len = 0;

	va_list args;
	va_start(args, primitive);
	for (unsigned int i = 0; i < primitive->num_params; ++i) {
		haclog_printf_spec_t *spec = &primitive->specs[i];
		unsigned long precision = 0;

		if (spec->width == HACLOG_PRINTF_SPEC_DYNAMIC) {
			int width = va_arg(args, int);
			HACLOG_SERIALIZE_WRITE(p, width);
		}

		if (spec->precision == HACLOG_PRINTF_SPEC_DYNAMIC) {
			precision = va_arg(args, int);
			HACLOG_SERIALIZE_WRITE(p, precision);
		}

		switch (spec->fmt_type) {
		case HACLOG_FT_STR: {
			if (n_str_idx == HACLOG_MAX_STR_CACHE) {
				str_cache[n_str_idx].s = "(haclog str cache full)";
				str_cache[n_str_idx].copy_slen = strlen(str_cache[n_str_idx].s);
				str_cache[n_str_idx].slen = HACLOG_ROUND_TO_2POWX(
					str_cache[n_str_idx].copy_slen + 1, 8);

				HACLOG_SERIALIZE_WRITE(p, extra_len);
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

				HACLOG_SERIALIZE_WRITE(p, extra_len);
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
			HACLOG_SERIALIZE_WRITE(p, v);
		} break;
		case HACLOG_FT_UCHAR:
		case HACLOG_FT_UBYTE: {
			unsigned char v = (unsigned char)va_arg(args, unsigned int);
			HACLOG_SERIALIZE_WRITE(p, v);
		} break;
		case HACLOG_FT_SHORT: {
			short v = (short)va_arg(args, int);
			HACLOG_SERIALIZE_WRITE(p, v);
		} break;
		case HACLOG_FT_USHORT: {
			unsigned short v = (unsigned short)va_arg(args, unsigned int);
			HACLOG_SERIALIZE_WRITE(p, v);
		} break;
		case HACLOG_FT_INT: {
			HACLOG_SERIALIZE_VA_ARG(p, int);
		} break;
		case HACLOG_FT_LONG: {
			HACLOG_SERIALIZE_VA_ARG(p, long int);
		} break;
		case HACLOG_FT_ULONG: {
			HACLOG_SERIALIZE_VA_ARG(p, unsigned long int);
		} break;
		case HACLOG_FT_LONGLONG: {
			HACLOG_SERIALIZE_VA_ARG(p, long long int);
		} break;
		case HACLOG_FT_ULONGLONG: {
			HACLOG_SERIALIZE_VA_ARG(p, unsigned long long int);
		} break;
		default: {
			// TODO: assert false
		} break;
		}
	}
	va_end(args);

	// serialize string arguments
	w_str = haclog_bytes_buffer_w_fc(bytes_buf, extra_len, &r, w);
	if (w_str == -1) {
		// TODO: assert false
		return;
	}
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

	// fillup hdr
	haclog_serialize_hdr_t *hdr =
		(haclog_serialize_hdr_t *)haclog_bytes_buffer_get(bytes_buf, w_hdr);
	hdr->pos_const = w_const_args;
	hdr->pos_str = w_str;
	hdr->extra_len = extra_len;
	hdr->primitive = primitive;

	// move writer
	if (extra_len > 0) {
		haclog_atomic_store(&bytes_buf->w, w_str + extra_len,
							haclog_memory_order_release);
	} else {
		haclog_atomic_store(&bytes_buf->w, w_const_args + primitive->param_size,
							haclog_memory_order_release);
	}
}