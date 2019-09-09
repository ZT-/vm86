/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME            "machine_proc"
#define TB_TRACE_MODULE_DEBUG           (1)

 /* //////////////////////////////////////////////////////////////////////////////////////
  * includes
  */
#include "machine.h"

  /* //////////////////////////////////////////////////////////////////////////////////////
   * types
   */
   // the machine proc type
typedef struct __vm86_proc_t
{
	// the machine
	vm86_machine_ref_t          machine;

	// the name
	tb_char_t*                  name;

	// the locals
	tb_hash_map_ref_t           locals;

	//instructions start index
	tb_size_t						oep;

	// the instruction count
	tb_size_t                   instructions_count;

}vm86_proc_t;

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
static tb_bool_t vm86_proc_compiler_prepare_locals(vm86_proc_t* proc, tb_char_t const* p, tb_char_t const* e)
{
	// check
	tb_assert_and_check_return_val(proc && proc->locals, tb_false);

	// done
	tb_bool_t ok = tb_false;
	do
	{
		// save base
		tb_char_t const* b = p;

		// init name
		tb_char_t name[8192] = { 0 };
		if (!vm86_parser_get_variable_name(&p, e, name, sizeof(name))) break;

		// find '='
		while (p < e && *p != '=') p++;
		tb_check_break(p < e && *p == '=');
		p++;

		// skip space
		while (p < e && tb_isspace(*p)) p++;

		// init type
		tb_char_t type[8192] = { 0 };
		for (b = p; p < e && !tb_isspace(*p); p++) {}
		tb_check_break(p < e && p - b < sizeof(type));
		tb_memcpy(type, b, p - b);

		// check type
		if (tb_stricmp(type, "dword")
			&& tb_stricmp(type, "word")
			&& tb_stricmp(type, "byte")
			&& tb_stricmp(type, "qword")
			&& tb_stricmp(type, "xmmword")
			&& tb_stricmp(type, "__m128i")
			)
		{
			break;
		}

		// seek to the value
		while (p < e && !tb_isdigit16(*p) && *p != '-') p++;

		// init value
		tb_uint32_t value = 0;
		if (!vm86_parser_get_number_value(&p, e, &value)) break;

		// save local
		tb_hash_map_insert(proc->locals, name, (tb_pointer_t)value);

		// trace
		tb_trace_d("local: %s, type: %s, value: %d", name, type, value);

		// ok
		ok = tb_true;

	} while (0);

	// ok?
	return ok;
}
static tb_bool_t vm86_proc_compiler_prepare_label(tb_hash_map_ref_t labels, tb_char_t const* p, tb_char_t const* e, tb_size_t offset)
{
	// check
	tb_assert_and_check_return_val(labels && p && e, tb_false);

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
		tb_hash_map_insert(labels, name, (tb_cpointer_t)offset);

		// trace
		tb_trace_d("label: %s => %lu", name, offset);

		// ok
		ok = tb_true;

	} while (0);

	// ok?
	return ok;
}
static tb_bool_t vm86_proc_compiler_skip_label(tb_char_t const* p, tb_char_t const* e)
{
	// check
	tb_assert_and_check_return_val(p && e, tb_false);

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

		// ok
		ok = tb_true;

	} while (0);

	// ok?
	return ok;
}
static tb_bool_t vm86_proc_find(vm86_machine_ref_t machine, vm86_segment_ref_t segment, vm86_proc_t** proc, tb_uint32_t last_oep, tb_uint32_t* start, tb_uint32_t* end)
{
	tb_char_t const* p = 0;
	tb_char_t const*e = 0;
	tb_bool_t has_start = tb_false;

	vm86_text_ref_t text = vm86_machine_text(machine);
	const tb_uint32_t size = vm86_segment_size(segment);

	// make proc
	*proc = tb_malloc0_type(vm86_proc_t);
	tb_assert_and_check_return_val(proc, tb_false);

	// init locals
	(*proc)->locals = tb_hash_map_init(8, tb_element_str(tb_true), tb_element_uint32());

	// save machine
	(*proc)->machine = machine;
	(*proc)->oep = last_oep;

	for (tb_uint32_t i = *start; i < size; ++i)
	{
		const tb_char_t* line = vm86_segment_line(segment, i);

		tb_trace_d("line: [%d] %s", (*proc)->oep + (*proc)->instructions_count, line);

		//proc start flag
		p = tb_strstr(line, "proc near");
		if (p)
		{
			has_start = tb_true;
			*start = i;
			e = p;
			p = line;
			while (p <= e && !tb_isspace(*p)) p++;

			// save name
			(*proc)->name = tb_strndup(line, p - line);

			continue;
		}
		//proc end flag
		p = tb_strstr(line, "endp");
		if (has_start && p)
		{
			*end = i;
			tb_trace_d("proc end");

			return tb_true;
		}

		// attempt to prepare locals
		const tb_uint32_t line_size = tb_strlen(line);
		if (vm86_proc_compiler_prepare_locals(*proc, line, line + line_size))
			continue;

		// attempt to prepare label
		if (vm86_proc_compiler_prepare_label(vm86_text_labels(text), line, line + line_size, (*proc)->oep + (*proc)->instructions_count))
			continue;

		(*proc)->instructions_count++;
	}

	return tb_false;
}
tb_uint32_t vm86_proc_oep(vm86_proc_ref_t self)
{
	// check
	vm86_proc_t* proc = (vm86_proc_t*)self;
	tb_assert_and_check_return(proc);

	return proc->oep;
}
tb_void_t vm86_proc_exit(vm86_proc_ref_t self)
{
	// check
	vm86_proc_t* proc = (vm86_proc_t*)self;
	tb_assert_and_check_return(proc);

	// exit name
	if (proc->name) tb_free(proc->name);
	proc->name = tb_null;

	// exit labels
	//if (proc->labels) tb_hash_map_exit(proc->labels);
	//proc->labels = tb_null;

	// exit locals
	if (proc->locals) tb_hash_map_exit(proc->locals);
	proc->locals = tb_null;

	// exit instructions
	//if (proc->instructions)
	//{
	//	for (tb_uint32_t i = 0; i < proc->instructions_count; ++i)
	//	{
	//		vm86_instruction_ref_t p = proc->instructions + i;
	//		vm86_instruction_release(p);
	//	}
	//	// exit it
	//	tb_free(proc->instructions);
	//	proc->instructions = tb_null;
	//}

	// exit it
	tb_free(proc);
}
tb_char_t const* vm86_proc_name(vm86_proc_ref_t self)
{
	// check
	vm86_proc_t* proc = (vm86_proc_t*)self;
	tb_assert_and_check_return_val(proc, tb_null);

	// the name
	return proc->name;
}
tb_void_t vm86_proc_done(vm86_proc_ref_t self)
{
	// check
	vm86_proc_t* proc = (vm86_proc_t*)self;
	tb_assert_and_check_return(proc && proc->name);

	// trace
	tb_trace_d("=====================================================================");
	tb_trace_d("done: %s", proc->name);

	// the machine
	vm86_machine_ref_t machine = proc->machine;
	tb_assert_and_check_return(machine);

	// the stack
	vm86_stack_ref_t stack = vm86_machine_stack(machine);
	tb_assert_and_check_return(stack);

	// the stack
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert_and_check_return(registers);

	// push the stub return address
	vm86_stack_push(stack, 0xbeaf);

	// done it
	tb_uint32_t eip = proc->oep;
	vm86_registers_value_set(registers, VM86_REGISTER_EIP, eip);

	tb_uint32_t count = 0;
	do
	{
		tb_trace_d("done: %d", count);

		// execute it
		const tb_bool_t ret =vm86_instruction_done(machine, eip);
		tb_assert(ret);

		eip = vm86_registers_value(registers, VM86_REGISTER_EIP);

		count++;

	} while (eip != 0xbeaf);

	tb_uint32_t ret = vm86_registers_value(registers, VM86_REGISTER_EAX);
}
tb_bool_t vm86_proc_compile_done(vm86_proc_t* proc, vm86_text_ref_t text, vm86_segment_ref_t segment, tb_uint32_t start, tb_uint32_t end)
{
	// check
	tb_assert_and_check_return_val(proc && segment, tb_false);

	proc->instructions_count = 0;

	// done
	tb_bool_t   ok = tb_false;

	tb_hash_map_ref_t lables = vm86_text_labels(text);

	for (tb_uint32_t i = start; i < end; ++i)
	{
		const tb_char_t* line = vm86_segment_line(segment, i);
		const tb_uint32_t size = tb_strlen(line);

		//tb_trace_d("line: %s", line);

		// attempt to prepare locals
		if (vm86_proc_compiler_prepare_locals(proc, line, line + size)) continue;

		// skip label
		if (vm86_proc_compiler_skip_label(line, line + size)) continue;

		// compile code
		if (!vm86_instruction_compile(vm86_text_get_instruction(text, proc->oep + proc->instructions_count),
			line, size, proc->machine, lables, proc->locals))
		{
			tb_assert(0);
		}

		proc->instructions_count++;
	}

	ok = tb_true;
	// ok?
	return ok;
}
tb_bool_t vm86_proc_complie(vm86_machine_ref_t machine, vm86_segment_ref_t segment)
{
	// check
	tb_assert_and_check_return_val(machine && segment, tb_false);

	// done
	tb_bool_t   ok = tb_false;
	vm86_proc_t*    proc = tb_null;
	//text script index
	tb_uint32_t start = 0, end = 0;
	tb_uint32_t last_oep = 0;

	while (vm86_proc_find(machine, segment, &proc, last_oep, &start, &end))
	{
		ok = tb_false;

		if (!vm86_proc_compile_done(proc, vm86_machine_text(machine), segment, start + 1, end))
		{
			// exit it
			if (proc) vm86_proc_exit((vm86_proc_ref_t)proc);
			proc = tb_null;
		}

		//move start index to next proc
		start = end + 1;

		// save proc
		vm86_machine_proc_set(machine, proc);

		last_oep = proc->instructions_count + 1;
		ok = tb_true;
	}

	// ok?
	return ok;
}
