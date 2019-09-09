/* //////////////////////////////////////////////////////////////////////////////////////
 * Description
 *
 * vm86 text script complier
 */

 /* //////////////////////////////////////////////////////////////////////////////////////
  * includes
  */
#include "machine.h"

  /* //////////////////////////////////////////////////////////////////////////////////////
   * types
   */
typedef struct __vm86_text_t
{
	// the labels
	tb_hash_map_ref_t           labels;

	// the instructions
	vm86_instruction_ref_t*      instructions;

	// the instruction count
	tb_size_t                   instructions_count;

}vm86_text_t;

vm86_text_ref_t vm86_text_init()
{
	vm86_text_t* text = tb_malloc0_type(vm86_text_t);

	// init labels
	text->labels = tb_hash_map_init(8, tb_element_str(tb_true), tb_element_uint32());
	tb_assert_and_check_break(text->labels);
	return text;
}
tb_hash_map_ref_t vm86_text_labels(vm86_text_ref_t self)
{
	vm86_text_t* text = (vm86_text_t*)self;
	tb_assert_and_check_return_val(text, tb_null);
	return text->labels;
}
vm86_instruction_ref_t vm86_text_get_instruction(vm86_text_ref_t self, tb_size_t n)
{
	vm86_text_t* text = (vm86_text_t*)self;
	tb_assert_and_check_return_val(text, tb_null);

	if (n > text->instructions_count)
		return tb_null;

	return text->instructions[n];
}
tb_bool_t vm86_text_set_instruction(vm86_text_ref_t self, vm86_instruction_ref_t instruction, tb_size_t n)
{
	vm86_text_t* text = (vm86_text_t*)self;
	tb_assert_and_check_return_val(text, tb_false);

	if (n > text->instructions_count)
		return tb_false;

	text->instructions[n] = instruction;

	return tb_true;
}


tb_bool_t vm86_text_prepare_label_done(vm86_text_ref_t self, tb_char_t const* p, tb_char_t const* e, tb_size_t offset)
{
	vm86_text_t* text = (vm86_text_t*)self;

	// check
	tb_assert_and_check_return_val(text->labels && p && e, tb_false);

	// done
	tb_bool_t ok = tb_false;
	do
	{
		// save base
		tb_char_t const* b = p;

		// init name
		tb_char_t name[8192] = { 0 };
		while (p < e && !tb_isspace(*p) && *p != ':') p++;
		tb_check_break(p < e && p - b < sizeof(name) && *p == ':');
		tb_memcpy(name, b, p - b);

		// save offset
		tb_hash_map_insert(text->labels, name, (tb_cpointer_t)offset);

		// trace
		tb_trace_d("label: %s => %lu", name, offset);

		// ok
		ok = tb_true;

	} while (0);

	// ok?
	return ok;
}

tb_bool_t vm86_text_compile(vm86_machine_ref_t machine, tb_char_t const* code, tb_size_t size)
{
	// check
	tb_assert_and_check_return_val(machine && code && size, tb_null);

	// done
	tb_bool_t  ok = tb_false;

	do
	{
		vm86_text_t* text = (vm86_text_t*)vm86_machine_text(machine);

		//init segments
		const vm86_segment_info_ref_t segment_info = vm86_segment_info_init();

		//complie segments
		if (!vm86_segment_complie(machine, segment_info, code, size)) break;

		const vm86_segment_ref_t segment_data = vm86_segment_get_data(segment_info);

		//complie global data segment
		if (segment_data && !vm86_data_complie(machine, segment_data)) break;

		const vm86_segment_ref_t segment_code = vm86_segment_get_code(segment_info);

		//init instructions
		tb_size_t count = vm86_segment_size(segment_code);
		text->instructions = vm86_instruction_init(count);
		tb_assert_and_check_break(text->instructions);
		text->instructions_count = count;

		//complie global code segment
		if (!vm86_proc_complie(machine, segment_code)) break;

		// ok
		ok = tb_true;
	} while (0);

	// ok?
	return ok;
}