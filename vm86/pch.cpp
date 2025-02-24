﻿// pch.cpp: 与预编译标头对应的源文件；编译成功所必需的

#include "pch.h"

// 一般情况下，忽略此文件，但如果你使用的是预编译标头，请保留它。
#pragma comment(lib,"tbox_mtd.lib")

#ifdef VM86_OLD


static tb_char_t const* vm86_proc_compiler_prepare(vm86_proc_t* proc, tb_char_t const* p, tb_char_t const* e, tb_size_t* instructions_count)
{
	// trace
	tb_trace_d("prepare: ..");

	// done
	tb_char_t           line[8192];
	tb_size_t           count = 0;
	tb_bool_t           is_data = tb_false;
	tb_char_t const*    start = tb_null;
	while (p < e)
	{
		// save the base
		tb_char_t const* b = p;

		// read line
		tb_size_t size = sizeof(line);
		p = 0;//vm86_proc_compiler_read_line(p, e, line, &size);

		// check
		tb_check_continue(size);
		tb_assert(size < sizeof(line));

		// attempt to prepare locals
		if (vm86_proc_compiler_prepare_locals(proc, line, line + size)) continue;

		// save the start position
		if (!start) start = b;

		// is data or code segment?
		if (!tb_strnicmp(line, ".data", 5))
		{
			is_data = tb_true;
			continue;
		}
		else if (!tb_strnicmp(line, ".code", 5))
		{
			is_data = tb_false;
			continue;
		}

		// is data?
		tb_check_continue(!is_data);

		// attempt to prepare label
		if (vm86_proc_compiler_prepare_label(proc, line, line + size, count)) continue;

		// is end?
		if (tb_strnistr(line, size, "endp")) break;

		// trace
//        tb_trace_d("line: %s", line);

		// update the instructions count
		count++;
	}

	// trace
	tb_trace_d("prepare: count: %lu", count);

	// save the instructions count
	*instructions_count = count;

	// ok?
	return start;
}
static tb_bool_t vm86_proc_compiler_compile_skip_label(vm86_proc_t* proc, tb_char_t const* p, tb_char_t const* e)
{
	// check
	tb_assert_and_check_return_val(proc && proc->labels, tb_false);

	// done
	tb_bool_t ok = tb_false;
	do
	{
		// init name
		while (p < e && !tb_isspace(*p) && *p != ':') p++;
		tb_check_break(p < e && *p == ':');

		// ok
		ok = tb_true;

	} while (0);

	// ok?
	return ok;
}
static tb_size_t vm86_proc_compiler_compile_done(vm86_proc_t* proc, tb_char_t const* p, tb_char_t const* e)
{
	// check
	tb_assert_and_check_return_val(proc, 0);

	// trace
	tb_trace_d("");
	tb_trace_d("compile: ..");

	// done
	tb_char_t line[8192];
	tb_size_t count = 0;
	tb_bool_t is_data = tb_false;
	while (p < e)
	{
		// read line
		tb_size_t size = sizeof(line);
		p = 0; //vm86_proc_compiler_read_line(p, e, line, &size);

		// check
		tb_check_continue(size);
		tb_assert(size < sizeof(line));

		// is data or code segment?
		if (!tb_strnicmp(line, ".data", 5))
		{
			// switch to the data segment
			is_data = tb_true;
			continue;
		}
		else if (!tb_strnicmp(line, ".code", 5))
		{
			// dump data
#ifdef __vm_debug__
			if (is_data) vm86_data_dump(vm86_machine_data(proc->machine));
#endif

			// switch to the code segment
			is_data = tb_false;
			continue;
		}

		// is data?
		if (is_data)
		{
			// compile data
			//if (!vm86_proc_compiler_compile_data(proc, line, line + size)) break;
		}
		// is code?
		else
		{
			// skip the label
			if (vm86_proc_compiler_compile_skip_label(proc, line, line + size)) continue;

			// is end?
			if (tb_strnistr(line, size, "endp")) break;

			// trace
			tb_trace_d("");
			tb_trace_d("line: %s", line);

			// compile code
			if (!vm86_instruction_compile(&proc->instructions[count], p, e - p, proc->machine, proc->labels, proc->locals))break;

			// update the instructions count
			count++;
		}
	}

	// trace
	tb_trace_d("compile: count: %lu", count);

	// ok?
	return count;
}
static tb_bool_t vm86_proc_compile_(vm86_proc_t* proc, tb_char_t const* code, tb_size_t size)
{
	// trace
	tb_trace_d("=====================================================================");

	// done
	tb_bool_t           ok = tb_false;
	tb_char_t const*    p = code;
	tb_char_t const*    e = code + size;
	do
	{
		// prepare some data and labels first before compiling code
		p = vm86_proc_compiler_prepare(proc, p, e, &proc->instructions_count);
		tb_assert_and_check_break(p < e && p && proc->instructions_count);

		// make instructions
		proc->instructions = vm86_instruction_init(proc->instructions_count);
		tb_assert_and_check_break(proc->instructions);

		// convert the labels offset to the instructions address
		tb_for_all_if(tb_hash_map_item_t*, item, proc->labels, item)
		{
			// check
			tb_assert((tb_uint32_t)item->data < proc->instructions_count);

			// the offset
			tb_uint32_t offset = (tb_uint32_t)(proc->instructions + (tb_uint32_t)item->data);

			// update the offset
			tb_iterator_copy(proc->labels, item_itor, (tb_pointer_t)offset);
		}

		// compile it
		tb_size_t count = vm86_proc_compiler_compile_done(proc, p, e);
		tb_assert_and_check_break(count == proc->instructions_count);

		// ok
		ok = tb_true;

	} while (0);

	// ok?
	return ok;
}





static vm86_instruction_ref_t vm86_instruction_done_leave(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the stack
	vm86_stack_ref_t stack = vm86_machine_stack(machine);
	tb_assert(stack);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// mov esp, ebp
	vm86_registers_value_set(registers, VM86_REGISTER_ESP, vm86_registers_value(registers, VM86_REGISTER_EBP));

	// pop ebp 
	tb_uint32_t ebp = 0;
	vm86_stack_pop(stack, &ebp);

	// end
	return tb_null;
}
static vm86_instruction_ref_t vm86_instruction_done_retn(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the stack
	vm86_stack_ref_t stack = vm86_machine_stack(machine);
	tb_assert(stack);

	// pop the return address
	tb_uint32_t retn = 0;
	vm86_stack_pop(stack, &retn);

	// trace
	tb_trace_d("retn(%#x)", retn);

	// check
	tb_assert(retn == 0xbeaf);

	// end
	return tb_null;
}
static vm86_instruction_ref_t vm86_instruction_done_call(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// get the function name
	tb_char_t const* name = instruction->v0.cstr;
	tb_assert(name && instruction->is_cstr);

	// get the function
	vm86_machine_func_t func = vm86_machine_function(machine, name);

	// trace
	tb_trace_d("call %s(%#x)", name, func);

	// check
	tb_assert(func);

	// call the function
	func(machine);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_push_r0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// the stack
	vm86_stack_ref_t stack = vm86_machine_stack(machine);
	tb_assert(stack);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// trace
	tb_trace_d("push %s(%#x)", vm86_registers_cstr(instruction->r0), r0);

	// push r0
	vm86_stack_push(stack, r0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_dec_r0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	r0--;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0);

	// trace
	tb_trace_d("dec %s(%#x)", vm86_registers_cstr(instruction->r0), r0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_push_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);
	tb_used(registers);

	// the stack
	vm86_stack_ref_t stack = vm86_machine_stack(machine);
	tb_assert(stack);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// trace
	tb_trace_d("push %#x", v0);

	// push r0
	vm86_stack_push(stack, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_pop_r0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// the stack
	vm86_stack_ref_t stack = vm86_machine_stack(machine);
	tb_assert(stack);

	// pop r0
	tb_uint32_t r0 = 0;
	vm86_stack_pop(stack, &r0);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0);

	// trace
	tb_trace_d("pop %s(%#x)", vm86_registers_cstr(instruction->r0), r0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_mov_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r1);

	// trace
	tb_trace_d("mov %s, %s(%#x)", vm86_registers_cstr(instruction->r0), vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_mov_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, v0);

	// trace
	tb_trace_d("mov %s, %#x", vm86_registers_cstr(instruction->r0), v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_mov_r0_$r1_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, *((tb_uint32_t*)(r1 + v0)));

	// trace
	tb_trace_d("mov %s, [%s(%#x) + %#x]", vm86_registers_cstr(instruction->r0), vm86_registers_cstr(instruction->r1), r1, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_mov_$r0_add_v0$_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set [r0 + v0] = r1
	*((tb_uint32_t*)(r0 + v0)) = r1;

	// trace
	tb_trace_d("mov [%s(%#x) + %#x], %s(%#x)", vm86_registers_cstr(instruction->r0), r0, v0, vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_mov_$r0_add_v0$_v1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// get v1
	tb_uint32_t v1 = instruction->v1.u32;

	// set [r0 + v0] = v1
	*((tb_uint32_t*)(r0 + v0)) = v1;

	// trace
	tb_trace_d("mov [%s(%#x) + %#x], %#x", vm86_registers_cstr(instruction->r0), r0, v0, v1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_mov_$r0_add_v0_op_v1$_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// get v1
	tb_uint32_t v1 = instruction->v1.u32;

	// set [r0 + v0 + v1] = v1
	if (instruction->op == '+')
		*((tb_uint32_t*)(r0 + v0 + v1)) = r1;
	else
		// set [r0 + v0 * v1] = v1
		*((tb_uint32_t*)(r0 + v0 * v1)) = r1;

	// trace
	tb_trace_d("mov [%s(%#x) + %#x op %#x], %s(%#x)",
		vm86_registers_cstr(instruction->r0), r0,
		v0, v1,
		op,
		vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_mov_$r0_add_v0_op_v1$_v2(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// get v1
	tb_uint32_t v1 = instruction->v1.u32;

	// get v2
	tb_uint32_t v2 = instruction->v2.u32;

	// set [r0 + v0 + v1] = v1
	if (instruction->op == '+')
		*((tb_uint32_t*)(r0 + v0 + v1)) = v2;
	else
		// set [r0 + v0 * v1] = v1
		*((tb_uint32_t*)(r0 + v0 * v1)) = v2;

	// trace
	tb_trace_d("mov [%s(%#x) + %#x op %#x],%#x",
		vm86_registers_cstr(instruction->r0), r0,
		v0, v1,
		op,
		v2);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_movzx_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r1);

	// trace
	tb_trace_d("movzx %s, %s(%#x)", vm86_registers_cstr(instruction->r0), vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_add_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 + r1);

	// trace
	tb_trace_d("add %s(%#x), %s(%#x)", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_add_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 + v0);

	// trace
	tb_trace_d("add %s, %#x", vm86_registers_cstr(instruction->r0), v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_add_r0_$r1_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 + *((tb_uint32_t*)(r1 + v0)));

	// trace
	tb_trace_d("add %s(%#x), [%s(%#x) + %#x]", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_sub_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 - r1);

	// trace
	tb_trace_d("sub %s(%#x), %s(%#x)", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_sub_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 - v0);

	// trace
	tb_trace_d("sub %s, %#x", vm86_registers_cstr(instruction->r0), v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_sub_r0_$r1_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 - *((tb_uint32_t*)(r1 + v0)));

	// trace
	tb_trace_d("sub %s(%#x), [%s(%#x) + %#x]", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_mov_r0_$r1_add_v0_op_v1$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get op
	tb_char_t op = instruction->op;
	tb_assert(op == '+' || op == '*');

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// get v1
	tb_uint32_t v1 = instruction->v1.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0,
		op == '+' ? (r1 + v0 + v1) : (r1 + v0 * v1)
	);

	// trace
	tb_trace_d("mov %s, [%s(%#x) + %#x %c %#x]",
		vm86_registers_cstr(instruction->r0),
		vm86_registers_cstr(instruction->r1), r1, v0, op, v1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_lea_r0_$r1_add_r2_op_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get op
	tb_char_t op = instruction->op;
	tb_assert(op == '+' || op == '*');

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get r2
	tb_uint32_t r2 = vm86_registers_value(registers, instruction->r2);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, op == '+' ? (r1 + r2 + v0) : (r1 + r2 * v0));

	// trace
	tb_trace_d("lea %s, [%s(%#x) + %s(%#x) %c %#x]", vm86_registers_cstr(instruction->r0), vm86_registers_cstr(instruction->r1), r1, vm86_registers_cstr(instruction->r2), r2, op, v0);

	// ok
	return instruction + 1;
}
static tb_bool_t vm86_instruction_done_jxx(tb_uint32_t eflags, tb_char_t h1, tb_char_t h2)
{
	// done
	tb_bool_t ok = tb_false;
	do
	{
		// jmp?
		if (h1 == 'm') ok = tb_true;
		// jz?
		else if (h1 == 'z') ok = (eflags & VM86_REGISTER_EFLAG_ZF);
		// jnx?
		else if (h1 == 'n')
		{
			// jnb?
			if (h2 == 'b') ok = !(eflags & VM86_REGISTER_EFLAG_CF);
			// jnz?
			else if (h2 == 'z') ok = !(eflags & VM86_REGISTER_EFLAG_ZF);
			// error
			else
			{
				// abort
				tb_assert(0);
			}
		}
		// jbx?
		else if (h1 == 'b')
		{
			// jbe?
			if (h2 == 'e') ok = (eflags & VM86_REGISTER_EFLAG_ZF) || (eflags & VM86_REGISTER_EFLAG_CF);
			// error
			else
			{
				// abort
				tb_assert(0);
			}
		}
		// jax?
		else if (h1 == 'a')
		{
			// ja?
			if (h2 == '\0') ok = !((eflags & VM86_REGISTER_EFLAG_ZF) || (eflags & VM86_REGISTER_EFLAG_CF));
			else
			{
				// abort
				tb_assert(0);
			}
		}

	} while (0);

	// ok?
	return ok;
}
static vm86_instruction_ref_t vm86_instruction_done_jxx_r0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the hint
	tb_char_t h1 = tb_tolower(instruction->hint[1]);
	tb_char_t h2 = tb_tolower(instruction->hint[2]);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// ok?
	tb_bool_t ok = vm86_instruction_done_jxx(registers[VM86_REGISTER_EFLAGS].u32, h1, h2);
	if (!ok)
	{
		// trace
		tb_trace_d("j%c%c %s(%#x), ok: %u", h1, h2 ? h2 : ' ', vm86_registers_cstr(instruction->r0), vm86_registers_value(registers, instruction->r0), ok);

		// continue
		return instruction + 1;
	}

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// trace
	tb_trace_d("j%c%c %s(%#x), ok: %u", h1, h2 ? h2 : ' ', vm86_registers_cstr(instruction->r0), r0, ok);

	// goto it
	return (vm86_instruction_ref_t)r0;
}
static vm86_instruction_ref_t vm86_instruction_done_jxx_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the hint
	tb_char_t h1 = tb_tolower(instruction->hint[1]);
	tb_char_t h2 = tb_tolower(instruction->hint[2]);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;
	tb_assert(v0);

	// ok?
	tb_bool_t ok = vm86_instruction_done_jxx(registers[VM86_REGISTER_EFLAGS].u32, h1, h2);

	// trace
	tb_trace_d("j%c%c %#x, ok: %u", h1, h2 ? h2 : ' ', v0, ok);

	// goto the next instruction
	return ok ? (vm86_instruction_ref_t)v0 : instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_jxx_v0$r0_mul_v1$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the hint
	tb_char_t h1 = tb_tolower(instruction->hint[1]);
	tb_char_t h2 = tb_tolower(instruction->hint[2]);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// get v1
	tb_uint32_t v1 = instruction->v1.u32;

	// ok?
	tb_bool_t ok = vm86_instruction_done_jxx(registers[VM86_REGISTER_EFLAGS].u32, h1, h2);
	if (!ok)
	{
		// trace
		tb_trace_d("j%c%c %#x[%s(%#x) * %#x]: %#x, ok: %u", h1, h2 ? h2 : ' ', v0, vm86_registers_cstr(instruction->r0), vm86_registers_value(registers, instruction->r0), v1, *((tb_uint32_t*)(v0 + (vm86_registers_value(registers, instruction->r0) * v1))), ok);

		// continue
		return instruction + 1;
	}

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// the offset
	tb_uint32_t offset = *((tb_uint32_t*)(v0 + (r0 * v1)));
	tb_assert(offset);

	// trace
	tb_trace_d("j%c%c %#x[%s(%#x) * %#x]: %#x, ok: %u", h1, h2 ? h2 : ' ', v0, vm86_registers_cstr(instruction->r0), r0, v1, offset, ok);

	// goto it
	return (vm86_instruction_ref_t)offset;
}
static vm86_instruction_ref_t vm86_instruction_done_mov_r0_v0$r1_mul_v1$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// get v1
	tb_uint32_t v1 = instruction->v1.u32;

	//get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// the offset
	const tb_uint32_t offset = *((tb_uint32_t*)(v0 + (r1 * v1)));
	tb_assert(offset);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, offset);

	// trace
	tb_trace_d("mov %s(%#x), %#x[%s(%#x) * %#x]: offset:%#x", vm86_registers_cstr(instruction->r0), r0,
		v0,
		vm86_registers_cstr(instruction->r1), r1,
		v1,
		offset
	);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_xor_r0_v0$r1_mul_v1$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// get v1
	tb_uint32_t v1 = instruction->v1.u32;

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	//get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// the offset
	const tb_uint32_t offset = *((tb_uint32_t*)(v0 + (r1 * v1)));
	tb_assert(offset);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 ^ offset);

	// trace
	tb_trace_d("xor %s(%#x), %#x[%s(%#x) * %#x]: offset:%#x", vm86_registers_cstr(instruction->r0), r0,
		v0,
		vm86_registers_cstr(instruction->r1), r1,
		v1,
		offset
	);

	// ok
	return instruction + 1;
}
static tb_uint32_t vm86_instruction_done_cmp(tb_uint32_t v0, tb_uint32_t v1)
{
	// TODO
	// set eflags
	tb_uint32_t eflags = 0;
	if (v0 == v1) eflags |= VM86_REGISTER_EFLAG_ZF;
	if (v0 < v1) eflags |= VM86_REGISTER_EFLAG_SF;
	if (v0 < v1) eflags |= VM86_REGISTER_EFLAG_CF;

	// ok?
	return eflags;
}
static vm86_instruction_ref_t vm86_instruction_done_cmp_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// compare it
	registers[VM86_REGISTER_EFLAGS].u32 = vm86_instruction_done_cmp(r0, r1);

	// trace
	tb_trace_d("cmp %s(%#x), %s(%#x): %#x", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1, registers[VM86_REGISTER_EFLAGS].u32);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_cmp_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// compare it
	registers[VM86_REGISTER_EFLAGS].u32 = vm86_instruction_done_cmp(r0, v0);

	// trace
	tb_trace_d("cmp %s(%#x), %#x: %#x", vm86_registers_cstr(instruction->r0), r0, v0, registers[VM86_REGISTER_EFLAGS].u32);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_cmp_r0_$r1_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// compare it
	registers[VM86_REGISTER_EFLAGS].u32 = vm86_instruction_done_cmp(r0, *((tb_uint32_t*)(r1 + v0)));

	// trace
	tb_trace_d("cmp %s(%#x), [%s(%#x), %#x]: %#x", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1, v0, registers[VM86_REGISTER_EFLAGS].u32);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_cmp_$r0_add_v0$_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// compare it
	registers[VM86_REGISTER_EFLAGS].u32 = vm86_instruction_done_cmp(*((tb_uint32_t*)(r0 + v0)), r1);

	// trace
	tb_trace_d("cmp [%s(%#x), %#x], %s(%#x): %#x", vm86_registers_cstr(instruction->r0), r0, v0, vm86_registers_cstr(instruction->r1), r1, registers[VM86_REGISTER_EFLAGS].u32);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_cmp_$r0_add_v0$_v1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// get v1
	tb_uint32_t v1 = instruction->v1.u32;

	// compare it
	registers[VM86_REGISTER_EFLAGS].u32 = vm86_instruction_done_cmp(*((tb_uint32_t*)(r0 + v0)), v1);

	// trace
	tb_trace_d("cmp [%s(%#x), %#x], %#x: %#x", vm86_registers_cstr(instruction->r0), r0, v0, v1, registers[VM86_REGISTER_EFLAGS].u32);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_shrd_r0_r1_r2(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get r2
	tb_uint32_t r2 = vm86_registers_value(registers, instruction->r2);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r1 >> r2);

	// trace
	tb_trace_d("shrd %s, %s(%#x), %s(%#x)", vm86_registers_cstr(instruction->r0), vm86_registers_cstr(instruction->r1), r1, vm86_registers_cstr(instruction->r2), r2);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_shr_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 >> r1);

	// trace
	tb_trace_d("shr %s(%#x), %s(%#x)", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_shr_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 >> v0);

	// trace
	tb_trace_d("shr %s(%#x), %#x", vm86_registers_cstr(instruction->r0), r0, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_shl_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 << r1);

	// trace
	tb_trace_d("shl %s(%#x), %s(%#x)", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_shl_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 << v0);

	// trace
	tb_trace_d("shl %s(%#x), %#x", vm86_registers_cstr(instruction->r0), r0, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_sar_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, ((tb_sint32_t)r0) >> r1);

	// trace
	tb_trace_d("sar %s(%#x), %s(%#x)", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_sar_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, ((tb_sint32_t)r0) >> v0);

	// trace
	tb_trace_d("sar %s(%#x), %#x", vm86_registers_cstr(instruction->r0), r0, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_and_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 & r1);

	// trace
	tb_trace_d("and %s(%#x), %s(%#x)", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_and_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 & v0);

	// trace
	tb_trace_d("and %s(%#x), %#x", vm86_registers_cstr(instruction->r0), r0, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_and_r0_$r1_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 & *((tb_uint32_t*)(r1 + v0)));

	// trace
	tb_trace_d("and %s(%#x), [%s(%#x) + %#x]", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_xor_r0_r1(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 ^ r1);

	// trace
	tb_trace_d("xor %s(%#x), %s(%#x)", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_xor_r0_$r1$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 ^ *(tb_uint32_t*)r1);

	// trace
	tb_trace_d("xor %s(%#x), %s(%#x)", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_xor_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 ^ v0);

	// trace
	tb_trace_d("xor %s(%#x), %#x", vm86_registers_cstr(instruction->r0), r0, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_xor_r0_$r1_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 ^ *((tb_uint32_t*)(r1 + v0)));

	// trace
	tb_trace_d("xor %s(%#x), [%s(%#x) + %#x]", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_or_r0_v0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 | v0);

	// trace
	tb_trace_d("or %s(%#x), %#x", vm86_registers_cstr(instruction->r0), r0, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_or_r0_$r1_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// set r0
	vm86_registers_value_set(registers, instruction->r0, r0 | *((tb_uint32_t*)(r1 + v0)));

	// trace
	tb_trace_d("or %s(%#x), [%s(%#x) + %#x]", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1, v0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_not_r0(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// set r0
	vm86_registers_value_set(registers, instruction->r0, ~r0);

	// trace
	tb_trace_d("not %s(%#x)", vm86_registers_cstr(instruction->r0), r0);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_mul_$r0_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// the multiplicand
	tb_uint32_t multiplicand = registers[VM86_REGISTER_EAX].u32;

	// the multiplier
	tb_uint32_t multiplier = *((tb_uint32_t*)(r0 + v0));

	// the result
	tb_uint64_t result = (tb_uint64_t)multiplicand * multiplier;

	// set result
	vm86_registers_value_set(registers, VM86_REGISTER_EAX, (tb_uint32_t)result);
	vm86_registers_value_set(registers, VM86_REGISTER_EDX, (tb_uint32_t)(result >> 32));

	// trace
	tb_trace_d("mul [%s(%#x), %#x]: %u * %u = %llu", vm86_registers_cstr(instruction->r0), r0, v0, multiplicand, multiplier, result);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_imul_r0_$r1_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get r1
	tb_uint32_t r1 = vm86_registers_value(registers, instruction->r1);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// the multiplicand
	tb_uint32_t multiplicand = r0;

	// the multiplier
	tb_uint32_t multiplier = *((tb_uint32_t*)(r1 + v0));

	// the result
	tb_uint64_t result = (tb_uint64_t)multiplicand * multiplier;

	// set result
	vm86_registers_value_set(registers, instruction->r0, (tb_uint32_t)result);

	// trace
	tb_trace_d("imul %s(%#x), [%s(%#x), %#x]: %u * %u = %llu", vm86_registers_cstr(instruction->r0), r0, vm86_registers_cstr(instruction->r1), r1, v0, multiplicand, multiplier, result);

	// ok
	return instruction + 1;
}
static vm86_instruction_ref_t vm86_instruction_done_div_$r0_add_v0$(vm86_instruction_ref_t instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get r0
	tb_uint32_t r0 = vm86_registers_value(registers, instruction->r0);

	// get v0
	tb_uint32_t v0 = instruction->v0.u32;

	// the dividend
	tb_uint64_t dividend = ((tb_uint64_t)registers[VM86_REGISTER_EDX].u32 << 32) | registers[VM86_REGISTER_EAX].u32;

	// the divisor
	tb_uint32_t divisor = *((tb_uint32_t*)(r0 + v0));

	// the quotient
	tb_uint32_t quotient = (tb_uint32_t)(dividend / divisor);

	// the remainder
	tb_uint32_t remainder = (tb_uint32_t)(dividend % divisor);

	// set result
	vm86_registers_value_set(registers, VM86_REGISTER_EAX, quotient);
	vm86_registers_value_set(registers, VM86_REGISTER_EDX, remainder);

	// trace
	tb_trace_d("div [%s(%#x), %#x]: %llu / %u = %u, %%%u = %u", vm86_registers_cstr(instruction->r0), r0, v0, dividend, divisor, quotient, divisor, remainder);

	// ok
	return instruction + 1;
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * globals
 */


 // the xxx func entries
static vm86_instruction_entry_t g_xxx_func[] =
{
	{ "call",   vm86_instruction_done_call           }
};

// the xxx r0 entries
static vm86_instruction_entry_t g_xxx_r0[] =
{
	{ "dec",   vm86_instruction_done_dec_r0       }
,	{ "ja",     vm86_instruction_done_jxx_r0         }
,   { "jbe",    vm86_instruction_done_jxx_r0         }
,   { "jmp",    vm86_instruction_done_jxx_r0         }
,   { "jnb",    vm86_instruction_done_jxx_r0         }
,   { "jnz",    vm86_instruction_done_jxx_r0         }
,   { "jz",     vm86_instruction_done_jxx_r0         }
,   { "not",    vm86_instruction_done_not_r0         }
,   { "pop",    vm86_instruction_done_pop_r0         }
,   { "push",   vm86_instruction_done_push_r0   }
};

// the xxx v0 entries
static vm86_instruction_entry_t g_xxx_v0[] =
{
	{ "ja",     vm86_instruction_done_jxx_v0         }
,   { "jbe",    vm86_instruction_done_jxx_v0         }
,   { "jmp",    vm86_instruction_done_jxx_v0         }
,   { "jnb",    vm86_instruction_done_jxx_v0         }
,   { "jnz",    vm86_instruction_done_jxx_v0         }
,   { "jz",     vm86_instruction_done_jxx_v0         }
,   { "push",   vm86_instruction_done_push_v0        }
};

// the xxx r0, r1 entries
static vm86_instruction_entry_t g_xxx_r0_r1[] =
{
	{ "add",    vm86_instruction_done_add_r0_r1      }
,   { "and",    vm86_instruction_done_and_r0_r1      }
,   { "cmp",    vm86_instruction_done_cmp_r0_r1      }
,   { "mov",    vm86_instruction_done_mov_r0_r1      }
,   { "movzx",  vm86_instruction_done_movzx_r0_r1    }
,   { "sar",    vm86_instruction_done_sar_r0_r1      }
,   { "shl",    vm86_instruction_done_shl_r0_r1      }
,   { "shr",    vm86_instruction_done_shr_r0_r1      }
,   { "sub",    vm86_instruction_done_sub_r0_r1      }
,   { "xor",    vm86_instruction_done_xor_r0_r1      }
};

// the xxx r0, r1 entries
static vm86_instruction_entry_t g_xxx_r0_$r1$[] =
{
  { "xor",    vm86_instruction_done_xor_r0_$r1$      }
};

// the xxx r0, r1, r2 entries
static vm86_instruction_entry_t g_xxx_r0_r1_r2[] =
{
	{ "shrd",   vm86_instruction_done_shrd_r0_r1_r2  }
};

// the xxx r0, v0 entries
static vm86_instruction_entry_t g_xxx_r0_v0[] =
{
	{ "add",    vm86_instruction_done_add_r0_v0      }
,   { "and",    vm86_instruction_done_and_r0_v0      }
,   { "cmp",    vm86_instruction_done_cmp_r0_v0      }
,   { "mov",    vm86_instruction_done_mov_r0_v0      }
,   { "or",     vm86_instruction_done_or_r0_v0       }
,   { "sar",    vm86_instruction_done_sar_r0_v0      }
,   { "shl",    vm86_instruction_done_shl_r0_v0      }
,   { "shr",    vm86_instruction_done_shr_r0_v0      }
,   { "sub",    vm86_instruction_done_sub_r0_v0      }
,   { "xor",    vm86_instruction_done_xor_r0_v0      }
};

// the xxx r0, [r1 + v0] entries
static vm86_instruction_entry_t g_xxx_r0_$r1_add_v0$[] =
{
	{ "add",    vm86_instruction_done_add_r0_$r1_add_v0$     }
,   { "and",    vm86_instruction_done_and_r0_$r1_add_v0$     }
,   { "cmp",    vm86_instruction_done_cmp_r0_$r1_add_v0$     }
,   { "imul",   vm86_instruction_done_imul_r0_$r1_add_v0$    }
,   { "mov",    vm86_instruction_done_mov_r0_$r1_add_v0$     }
,   { "or",     vm86_instruction_done_or_r0_$r1_add_v0$      }
,   { "sub",    vm86_instruction_done_sub_r0_$r1_add_v0$     }
,   { "xor",    vm86_instruction_done_xor_r0_$r1_add_v0$     }
,   { "movzx",  vm86_instruction_done_movzx_r0_r1    }
,   { "sar",    vm86_instruction_done_sar_r0_r1      }
,   { "shl",    vm86_instruction_done_shl_r0_r1      }
,   { "shr",    vm86_instruction_done_shr_r0_r1      }
};

// the xxx [r0], [r1 + v0 op v1] entries
static vm86_instruction_entry_t g_xxx_r0_$r1_add_v0_op_v1$[] =
{
	{ "mov",    vm86_instruction_done_mov_r0_$r1_add_v0_op_v1$  }
};

// the xxx r0, [r1 + r2 op v0] entries
static vm86_instruction_entry_t g_xxx_r0_$r1_add_r2_op_v0$[] =
{
	{ "lea",    vm86_instruction_done_lea_r0_$r1_add_r2_op_v0$  }
};

// the xxx v0[r0 * v1] entries
static vm86_instruction_entry_t g_xxx_v0$r0_mul_v1$[] =
{
	{ "ja",     vm86_instruction_done_jxx_v0$r0_mul_v1$      }
,   { "jbe",   vm86_instruction_done_jxx_v0$r0_mul_v1$      }
,   { "jmp",  vm86_instruction_done_jxx_v0$r0_mul_v1$      }
,   { "jnb",   vm86_instruction_done_jxx_v0$r0_mul_v1$      }
,   { "jnz",   vm86_instruction_done_jxx_v0$r0_mul_v1$      }
,   { "jz",     vm86_instruction_done_jxx_v0$r0_mul_v1$      }
};

// the xxx r0,v0[r1 * v1] entries
static vm86_instruction_entry_t g_xxx_r0_v0$r1_mul_v1$[] =
{
	{ "mov",  vm86_instruction_done_mov_r0_v0$r1_mul_v1$ },
	{ "xor",    vm86_instruction_done_xor_r0_v0$r1_mul_v1$ }
};

// the xxx [r0 + v0] entries
static vm86_instruction_entry_t g_xxx_$r0_add_v0$[] =
{
	{ "div",    vm86_instruction_done_div_$r0_add_v0$        }
,   { "mul",    vm86_instruction_done_mul_$r0_add_v0$        }
};

// the xxx [r0 + v0], r1 entries
static vm86_instruction_entry_t g_xxx_$r0_add_v0$_r1[] =
{
	{ "cmp",    vm86_instruction_done_cmp_$r0_add_v0$_r1     }
,   { "mov",    vm86_instruction_done_mov_$r0_add_v0$_r1     }
};

// the xxx [r0 + v0], v1 entries
static vm86_instruction_entry_t g_xxx_$r0_add_v0$_v1[] =
{
	{ "cmp",    vm86_instruction_done_cmp_$r0_add_v0$_v1     }
,   { "mov",    vm86_instruction_done_mov_$r0_add_v0$_v1     }
};

//the xxx [r0 + v0 op v1],r1
static vm86_instruction_entry_t g_xxx_$r0_add_v0_op_v1$_r1[] =
{
	 { "mov", vm86_instruction_done_mov_$r0_add_v0_op_v1$_r1     }
};

//the xxx [r0 + v0 op v1],v2
static vm86_instruction_entry_t g_xxx_$r0_add_v0_op_v1$_v2[] =
{
	 { "mov", vm86_instruction_done_mov_$r0_add_v0_op_v1$_v2     }
};
#endif


#ifdef VM86_OLD

// get instruction name
tb_char_t name[64] = { 0 };
if (!vm86_parser_get_instruction_name(&p, e, name, sizeof(name))) break;

// init instruction hint
instruction->hint[0] = name[0];
instruction->hint[1] = name[1];
instruction->hint[2] = name[2];

// init executor
instruction->done = tb_null;

// xxx?
if (p == e)
{
	// init instruction
	instruction->done = vm86_instruction_find(name, g_xxx, tb_arrayn(g_xxx));
}
// xxx [ ... ], ... ?
else if (*p == '[')
{
	// get r0
	p++;
	if (!vm86_parser_get_register(&p, e, &r0)) break;

	// [r0 +- v0]?
	if (*p != ']')
	{
		// skip op
		tb_assert(*p == '+');
		p++;

		// skip space
		while (p < e && tb_isspace(*p)) p++;

		// get v0
		if (!vm86_parser_get_value(&p, e, &v0, proc_locals, proc_labels, data)) break;
	}

	// xxx [r0 + v0 op v1]?
	if (vm86_parser_get_op(&p, e, &op))
	{
		// get v1
		if (!vm86_parser_get_value(&p, e, &v1, proc_locals, proc_labels, data)) break;

		// skip "], "
		while (p < e && (tb_isspace(*p) || *p == ',' || *p == ']')) p++;

		// xxx [r0 + v0 op v1],r1?
		if (vm86_parser_get_register(&p, e, &r1))
		{
			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->r1 = (tb_uint8_t)r1;
			instruction->v0.u32 = v0;
			instruction->v1.u32 = v1;
			instruction->done = vm86_instruction_find(name, g_xxx_$r0_add_v0_op_v1$_r1, tb_arrayn(g_xxx_$r0_add_v0_op_v1$_r1));
		}
		// xxx [r0 + v0 op v1],v2?
		else if (vm86_parser_get_value(&p, e, &v2, proc_locals, proc_labels, data))
		{
			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->v0.u32 = v0;
			instruction->v1.u32 = v1;
			instruction->v2.u32 = v2;
			instruction->done = vm86_instruction_find(name, g_xxx_$r0_add_v0_op_v1$_v2, tb_arrayn(g_xxx_$r0_add_v0_op_v1$_v2));
		}
	}
	else
	{
		// skip "], "
		while (p < e && (tb_isspace(*p) || *p == ',' || *p == ']')) p++;

		// xxx [r0 + v0]?
		if (p == e)
		{
			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->v0.u32 = v0;
			instruction->done = vm86_instruction_find(name, g_xxx_$r0_add_v0$, tb_arrayn(g_xxx_$r0_add_v0$));
		}
		// xxx [r0 + v0], r1?
		else if (vm86_parser_get_register(&p, e, &r1))
		{
			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->r1 = (tb_uint8_t)r1;
			instruction->v0.u32 = v0;
			instruction->done = vm86_instruction_find(name, g_xxx_$r0_add_v0$_r1, tb_arrayn(g_xxx_$r0_add_v0$_r1));
		}
		// xxx [r0 + v0], v1?
		else
		{
			// get v1
			if (!vm86_parser_get_value(&p, e, &v1, proc_locals, proc_labels, data)) break;

			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->v0.u32 = v0;
			instruction->v1.u32 = v1;
			instruction->done = vm86_instruction_find(name, g_xxx_$r0_add_v0$_v1, tb_arrayn(g_xxx_$r0_add_v0$_v1));
		}
	}
}
// xxx r0, ...?
else if (vm86_parser_get_register(&p, e, &r0))
{
	// skip ',' and space
	while (p < e && (*p == ',' || tb_isspace(*p))) p++;

	// xxx r0?
	if (p == e)
	{
		// init instruction
		instruction->r0 = (tb_uint8_t)r0;
		instruction->done = vm86_instruction_find(name, g_xxx_r0, tb_arrayn(g_xxx_r0));
	}
	// xxx r0, r1, ...?
	else if (vm86_parser_get_register(&p, e, &r1))
	{
		// skip ',' and space
		while (p < e && (*p == ',' || tb_isspace(*p))) p++;

		// xxx r0, r1?
		if (p == e)
		{
			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->r1 = (tb_uint8_t)r1;
			instruction->done = vm86_instruction_find(name, g_xxx_r0_r1, tb_arrayn(g_xxx_r0_r1));
		}
		// xxx r0, r1, r2?
		else if (vm86_parser_get_register(&p, e, &r2))
		{
			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->r1 = (tb_uint8_t)r1;
			instruction->r2 = (tb_uint8_t)r2;
			instruction->done = vm86_instruction_find(name, g_xxx_r0_r1_r2, tb_arrayn(g_xxx_r0_r1_r2));
		}
		else break;
	}
	// xxx r0, [r1 + ...]?
	else if (*p == '[')
	{
		// get r1
		p++;
		if (!vm86_parser_get_register(&p, e, &r1)) break;

		if (*p != ']')
		{
			// skip op
			tb_assert(*p == '+');
			p++;

			// skip space
			while (p < e && tb_isspace(*p)) p++;

			// xxx r0, [r1 + r2 op v0]?
			if (vm86_parser_get_register(&p, e, &r2))
			{
				// get op
				vm86_parser_get_op(&p, e, &op);

				// skip space
				while (p < e && tb_isspace(*p)) p++;

				// get v0
				if (!vm86_parser_get_value(&p, e, &v0, proc_locals, proc_labels, data)) break;

				// init instruction
				instruction->r0 = (tb_uint8_t)r0;
				instruction->r1 = (tb_uint8_t)r1;
				instruction->r2 = (tb_uint8_t)r2;
				instruction->op = op;
				instruction->v0.u32 = v0;
				instruction->done = vm86_instruction_find(name, g_xxx_r0_$r1_add_r2_op_v0$, tb_arrayn(g_xxx_r0_$r1_add_r2_op_v0$));
			}
			// xxx r0, [r1 + v0 ... ]?
			else
			{
				// get v0
				if (!vm86_parser_get_value(&p, e, &v1, proc_locals, proc_labels, data)) break;

				// get op
				// xxx r0, [r1 + v0 op v1 ]?
				if (vm86_parser_get_op(&p, e, &op))
				{
					// get v1
					if (!vm86_parser_get_value(&p, e, &v0, proc_locals, proc_labels, data)) break;

					instruction->r0 = (tb_uint8_t)r0;
					instruction->op = op;
					instruction->v0.u32 = v0;
					instruction->v1.u32 = v1;
					instruction->done = vm86_instruction_find(name, g_xxx_r0_$r1_add_v0_op_v1$, tb_arrayn(g_xxx_r0_$r1_add_v0_op_v1$));
				}
				// xxx r0, [r1 + v0]?
				else
				{
					// init instruction
					instruction->r0 = (tb_uint8_t)r0;
					instruction->r1 = (tb_uint8_t)r1;
					instruction->v0.u32 = v0;
					instruction->done = vm86_instruction_find(name, g_xxx_r0_$r1_add_v0$, tb_arrayn(g_xxx_r0_$r1_add_v0$));
				}
			}
		}
		//xxx r0, [r1]
		else
		{
			// skip ]
			p++;

			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->r1 = (tb_uint8_t)r1;
			instruction->done = vm86_instruction_find(name, g_xxx_r0_$r1$, tb_arrayn(g_xxx_r0_$r1$));
		}
	}
	// xxx r0, offset label?
	else if (p + 6 < e && !tb_strnicmp(p, "offset", 6))
	{
		// seek to the name
		p += 6;
		while (p < e && tb_isspace(*p)) p++;

		// get v0
		if (!vm86_parser_get_offset_value(&p, e, &v0, proc_labels, data)) break;

		// init instruction
		instruction->r0 = (tb_uint8_t)r0;
		instruction->v0.u32 = v0;
		instruction->done = vm86_instruction_find(name, g_xxx_r0_v0, tb_arrayn(g_xxx_r0_v0));
	}
	// xxx r0, v0...?
	else if (vm86_parser_get_value(&p, e, &v0, proc_locals, proc_labels, data))
	{
		// xxx r0, v0[r1 * v1]?
		if (*p == '[')
		{
			// get r1
			p++;
			if (!vm86_parser_get_register(&p, e, &r1)) break;

			// skip op
			tb_assert(*p == '*');
			p++;

			// skip space
			while (p < e && tb_isspace(*p)) p++;

			// get v1
			if (!vm86_parser_get_value(&p, e, &v1, proc_locals, proc_labels, data)) break;

			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->r1 = (tb_uint8_t)r1;
			instruction->v0.u32 = v0;
			instruction->v1.u32 = v1;
			instruction->done = vm86_instruction_find(name, g_xxx_r0_v0$r1_mul_v1$, tb_arrayn(g_xxx_r0_v0$r1_mul_v1$));
		}
		// xxx r0, v0?
		else
		{
			// init instruction
			instruction->r0 = (tb_uint8_t)r0;
			instruction->v0.u32 = v0;
			instruction->done = vm86_instruction_find(name, g_xxx_r0_v0, tb_arrayn(g_xxx_r0_v0));
		}
	}
	else break;
}
// xxx v0...?
else if (vm86_parser_get_value(&p, e, &v0, proc_locals, proc_labels, data))
{
	// xxx v0[r0 * v1]?
	if (p < e && *p == '[')
	{
		// get r0
		p++;
		if (!vm86_parser_get_register(&p, e, &r0)) break;

		// skip op
		tb_assert(*p == '*');
		p++;

		// skip space
		while (p < e && tb_isspace(*p)) p++;

		// get v1
		if (!vm86_parser_get_value(&p, e, &v1, proc_locals, proc_labels, data)) break;

		// init instruction
		instruction->r0 = (tb_uint8_t)r0;
		instruction->v0.u32 = v0;
		instruction->v1.u32 = v1;
		instruction->done = vm86_instruction_find(name, g_xxx_v0$r0_mul_v1$, tb_arrayn(g_xxx_v0$r0_mul_v1$));
	}
	// xxx v0?
	else
	{
		// init instruction
		instruction->v0.u32 = v0;
		instruction->done = vm86_instruction_find(name, g_xxx_v0, tb_arrayn(g_xxx_v0));
	}
}
// xxx func?
else
{
	// get func
	tb_char_t func[512] = { 0 };
	if (!vm86_parser_get_variable_name(&p, e, func, sizeof(func))) break;

	// init instruction
	instruction->is_cstr = tb_true;
	instruction->v0.cstr = tb_strdup(func);
	instruction->done = vm86_instruction_find(name, g_xxx_func, tb_arrayn(g_xxx_func));
}
#endif