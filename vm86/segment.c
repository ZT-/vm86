/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME            "machine_segment"
#define TB_TRACE_MODULE_DEBUG           (0)

 /* //////////////////////////////////////////////////////////////////////////////////////
  * includes
  */
#include "machine.h"


typedef struct __vm86_segment_t
{
	tb_char_t*               line[8192];
	tb_size_t                  size;
}vm86_segment_t;

typedef struct __vm86_segment_info_t
{
	vm86_segment_t* data;
	vm86_segment_t* code;
}vm86_segment_info_t;


vm86_segment_info_ref_t vm86_segment_info_init()
{
	return  tb_malloc0_type(vm86_segment_info_t);
}
vm86_segment_ref_t vm86_segment_get_data(vm86_segment_info_ref_t self)
{
	vm86_segment_info_t*  segment_info = (vm86_segment_info_t*)self;
	return segment_info->data;
}
vm86_segment_ref_t vm86_segment_get_code(vm86_segment_info_ref_t self)
{
	vm86_segment_info_t*  segment_info = (vm86_segment_info_t*)self;
	return segment_info->code;
}
tb_uint32_t vm86_segment_size(vm86_segment_ref_t self)
{
	vm86_segment_t*  segment = (vm86_segment_t*)self;
	return segment->size;
}
const tb_char_t* vm86_segment_line(vm86_segment_ref_t self, const tb_uint32_t n)
{
	vm86_segment_t*  segment = (vm86_segment_t*)self;
	return segment->line[n];
}
static tb_char_t const* vm86_segment_read_line(tb_char_t const* p, tb_char_t const* e, tb_char_t* line, tb_size_t* size)
{
	// done
	tb_char_t   ch = 0;
	tb_char_t*  q = line;
	tb_bool_t   is_space = tb_true;
	tb_bool_t   is_comment = tb_false;
	tb_size_t   maxn = *size;
	while (p < e)
	{
		// read char
		ch = *p++;

		// is line?
		if (ch == '\n')
		{
			// finish line
			if (q > line && q[-1] == '\r')
				q--;

			while (q[-1] == ' ')
			{
				q--;
			}

			*q = '\0';

			// save size
			*size = q - line;

			// ok
			return p;
		}
		// is comment?
		else if (ch == ';') is_comment = tb_true;
		// append char to line
		else if (!is_comment)
		{
			// exists non-space character
			if (!tb_isspace(ch)) is_space = tb_false;

			// append it
			if (q - line < maxn - 1 && (!tb_isspace(ch) || !is_space)) *q++ = ch;

			// no data?
			if (!ch) break;
		}
	}

	// end
	*q = '\0';

	// save size
	*size = q - line;

	// ok?
	return p;
}
static tb_bool_t vm86_segment_add_line(vm86_segment_t* segment, tb_char_t const* code, tb_size_t size)
{
	tb_char_t**  line = &segment->line[segment->size];
	*line = tb_malloc_cstr(size + 1);
	tb_strcpy(*line, code);
	segment->size++;
	return tb_true;
}
static void vm86_segment_release(vm86_segment_t* segment)
{
	for (tb_size_t i = 0; i < segment->size; ++i)
	{
		tb_free(segment->line[i]);
		segment->line[i] = 0;
	}
	tb_free(segment);
	segment = 0;
}
void vm86_segment_info_release(vm86_segment_info_ref_t self)
{
	vm86_segment_info_t*  segment_info = (vm86_segment_info_t*)self;

	vm86_segment_release(segment_info->code);
	tb_free(segment_info->code);

	vm86_segment_release(segment_info->data);
	tb_free(segment_info->data);

	tb_free(segment_info);
	segment_info = 0;
}
tb_bool_t vm86_segment_complie(vm86_machine_ref_t machine, vm86_segment_info_ref_t self, tb_char_t const* code, tb_size_t code_size)
{
	// check
	tb_assert_and_check_return_val(machine && code && code_size, tb_false);

	vm86_segment_info_t*  segment_info = (vm86_segment_info_t*)self;

	vm86_text_ref_t  text = vm86_machine_text(machine);

	// done
	tb_char_t           line[8192];
	tb_bool_t           is_data = tb_false;
	tb_char_t const*    p = code;
	tb_char_t const*    e = code + code_size;
	tb_bool_t           ok = tb_false;

	while (p < e)
	{
		// save the base
		tb_char_t const* b = p;

		// read line
		tb_size_t size = sizeof line;
		p = vm86_segment_read_line(p, e, line, &size);

		// check
		tb_check_continue(size);
		tb_assert(size < sizeof line);

		tb_trace_d("%s", line);

		// is data or code segment?
		if (!tb_strnicmp(line, ".data", 5))
		{
			is_data = tb_true;
			segment_info->data = tb_malloc_type(vm86_segment_t);
			segment_info->data->size = 0;

			continue;
		}
		else if (!tb_strnicmp(line, ".code", 5))
		{
			is_data = tb_false;
			segment_info->code = tb_malloc_type(vm86_segment_t);
			segment_info->code->size = 0;
			continue;
		}

		if (is_data)
		{
			vm86_segment_add_line(segment_info->data, line, size);
		}
		else
		{
			vm86_segment_add_line(segment_info->code, line, size);
		}
	}

	ok = tb_true;

	return ok;
}
