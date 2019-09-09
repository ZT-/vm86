#include "machine.h"
#include <fstream>

using namespace std;

void vm86_base64(string code)
{
	// the machine
	vm86_machine_ref_t machine = vm86_machine();
	if (machine)
	{
		// the lock
		tb_spinlock_ref_t lock = vm86_machine_lock(machine);

		// enter
		tb_spinlock_enter(lock);

		// the stack
		vm86_stack_ref_t stack = vm86_machine_stack(machine);

		// compile proc
		tb_bool_t ret = vm86_text_compile(machine, code.c_str(), code.length());
		if (ret)
		{
			// add function
			//vm86_machine_function_set(machine, "printf", vm86_demo_proc_func_printf);

			const vm86_proc_ref_t proc = vm86_machine_proc(machine, "sub_401020");

			char str1[] = "a";
			int src_len = tb_strlen(str1);
			char str2[20] = { 0 }; //注意长度要给够
			int len = 0;

			// init arguments
			vm86_stack_push(stack, (tb_uint32_t)&len);
			vm86_stack_push(stack, (tb_uint32_t)str2);
			vm86_stack_push(stack, (tb_uint32_t)src_len);
			vm86_stack_push(stack, (tb_uint32_t)str1);

			// done proc
			vm86_proc_done(proc);

			// restore stack
			auto registers = vm86_machine_registers(machine);
			tb_uint32_t ret = vm86_registers_value(registers, VM86_REGISTER_EAX);

			//vm86_stack_pop(stack, &ret);

		}

		// leave
		tb_spinlock_leave(lock);
	}
}

void vm86_test_hex2byte(string code)
{
	// the machine
	vm86_machine_ref_t machine = vm86_machine();
	if (machine)
	{
		// the lock
		tb_spinlock_ref_t lock = vm86_machine_lock(machine);

		// enter
		tb_spinlock_enter(lock);

		// the stack
		vm86_stack_ref_t stack = vm86_machine_stack(machine);

		// compile proc
		tb_bool_t ret = vm86_text_compile(machine, code.c_str(), code.length());
		if (ret)
		{
			// add function
			//vm86_machine_function_set(machine, "printf", vm86_demo_proc_func_printf);

			const vm86_proc_ref_t proc = vm86_machine_proc(machine, "sub_401000");

			char str1[] = "6C";
			unsigned char str2[20] = { 0 }; //注意长度要给够
			int len = strlen(str1);

			// init arguments
			vm86_stack_push(stack, (tb_uint32_t)len);
			vm86_stack_push(stack, (tb_uint32_t)str2);
			vm86_stack_push(stack, (tb_uint32_t)str1);

			// done proc
			vm86_proc_done(proc);

			// restore stack
			auto registers = vm86_machine_registers(machine);
			tb_uint32_t ret = vm86_registers_value(registers, VM86_REGISTER_EAX);

			//vm86_stack_pop(stack, &ret);
		}

		// leave
		tb_spinlock_leave(lock);
	}
}

void vm86_test_toupper(string code)
{
	// the machine
	vm86_machine_ref_t machine = vm86_machine();
	if (machine)
	{
		// the lock
		tb_spinlock_ref_t lock = vm86_machine_lock(machine);

		// enter
		tb_spinlock_enter(lock);

		// the stack
		vm86_stack_ref_t stack = vm86_machine_stack(machine);

		// compile proc
		tb_bool_t ret = vm86_text_compile(machine, code.c_str(), code.length());
		if (ret)
		{
			// add function
			//vm86_machine_function_set(machine, "printf", vm86_demo_proc_func_printf);

			const vm86_proc_ref_t proc = vm86_machine_proc(machine, "sub_401000");

			char str1[] = "a";
			unsigned char str2[20] = { 0 }; //注意长度要给够
			int len = strlen(str1);

			vm86_stack_push(stack, (tb_uint32_t)str1[0]);

			// done proc
			vm86_proc_done(proc);

			// restore stack
			auto registers = vm86_machine_registers(machine);
			tb_uint32_t ret = vm86_registers_value(registers, VM86_REGISTER_EAX);

			tb_assert(ret == 'A');

		}

		// leave
		tb_spinlock_leave(lock);
	}
}


tb_int_t main(tb_int_t argc, tb_char_t** argv)
{
	if (!tb_init(tb_null, tb_null))
		return 0;

	ifstream in_file("F:\\vm86\\asm\\test_shl.asm");
	if (!in_file)
	{
		MessageBox(0, L"Error", L"File Not Found!", 0);
		return -1;
	}

	std::string string_buff((std::istreambuf_iterator<char>(in_file)),
		std::istreambuf_iterator<char>());

	vm86_test_hex2byte(string_buff);

	tb_exit();
	return 0;
}


