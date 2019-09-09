 /* //////////////////////////////////////////////////////////////////////////////////////
  * includes
  */
#include "machine.h"

  /* //////////////////////////////////////////////////////////////////////////////////////
   * types
   */

   // the machine machine type
typedef struct __vm86_machine_t
{
	// the procs
	tb_hash_map_ref_t       procs;

	// the data
	vm86_data_ref_t         data;

	// the text
	vm86_text_ref_t         text;

	// the stack
	vm86_stack_ref_t        stack;

	// the registers
	vm86_registers_t        registers;

	// the functions
	tb_hash_map_ref_t       functions;

	// the lock
	tb_spinlock_t           lock;

}vm86_machine_t;

/* //////////////////////////////////////////////////////////////////////////////////////
 * private implementation
 */
static tb_handle_t vm86_instance_init(tb_cpointer_t* ppriv)
{
	// init it
#ifdef __vm_small__
	return vm86_machine_init(1024, 1024);
#else
	return vm86_machine_init(4096, 4096);
#endif
}
static tb_void_t vm86_instance_exit(tb_handle_t machine, tb_cpointer_t priv)
{
	// exit it
	vm86_machine_exit((vm86_machine_ref_t)machine);
}
static tb_void_t vm86_text_proc_exit(tb_element_ref_t func, tb_pointer_t buff)
{
	// check
	tb_assert_and_check_return(buff);

	// the proc
	vm86_proc_ref_t proc = *((vm86_proc_ref_t*)buff);
	tb_assert_and_check_return(proc);

	// exit it
	vm86_proc_exit(proc);
}
/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
vm86_machine_ref_t vm86_machine()
{
	return (vm86_machine_ref_t)tb_singleton_instance(TB_SINGLETON_TYPE_USER + 5, vm86_instance_init, vm86_instance_exit, tb_null, tb_null);
}
vm86_machine_ref_t vm86_machine_init(tb_size_t data_size, tb_size_t stack_size)
{
	// check
	tb_assert_and_check_return_val(data_size && stack_size, tb_null);

	// done
	tb_bool_t           ok = tb_false;
	vm86_machine_t*     machine = tb_null;
	do
	{
		// check
		tb_assert_static(sizeof(tb_pointer_t) <= sizeof(tb_uint32_t));

		// make machine
		machine = tb_malloc0_type(vm86_machine_t);
		tb_assert_and_check_break(machine);

		// init lock
		if (!tb_spinlock_init(&machine->lock)) break;

		// init registers
		vm86_registers_clear(machine->registers);

		// make stack
		machine->stack = vm86_stack_init(stack_size, &machine->registers[VM86_REGISTER_ESP].u32);
		tb_assert_and_check_break(machine->stack);

		// make data
		machine->data = vm86_data_init(data_size);
		tb_assert_and_check_break(machine->data);

		// init text
		machine->text = vm86_text_init();
		tb_assert_and_check_break(machine->text);

		// init procs
		machine->procs = tb_hash_map_init(8, tb_element_str(tb_true), tb_element_ptr(vm86_text_proc_exit, tb_null));
		tb_assert_and_check_break(machine->procs);

		// make functions
		machine->functions = tb_hash_map_init(8, tb_element_str(tb_true), tb_element_ptr(tb_null, tb_null));
		tb_assert_and_check_break(machine->functions);

		// make functions
		machine->functions = tb_hash_map_init(8, tb_element_str(tb_true), tb_element_ptr(tb_null, tb_null));
		tb_assert_and_check_break(machine->functions);

		// ok
		ok = tb_true;

	} while (0);

	// failed?
	if (!ok)
	{
		// exit it
		if (machine) vm86_machine_exit((vm86_machine_ref_t)machine);
		machine = tb_null;
	}

	// ok?
	return (vm86_machine_ref_t)machine;
}
tb_void_t vm86_machine_exit(vm86_machine_ref_t self)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return(machine);

	// enter
	tb_spinlock_enter(&machine->lock);

	// exit text
	if (machine->procs) tb_hash_map_exit(machine->procs);
	machine->procs = tb_null;

	// exit data
	if (machine->data) vm86_data_exit(machine->data);
	machine->data = tb_null;

	// exit stack
	if (machine->stack) vm86_stack_exit(machine->stack);
	machine->stack = tb_null;

	// exit functions
	if (machine->functions) tb_hash_map_exit(machine->functions);
	machine->functions = tb_null;

	// leave
	tb_spinlock_leave(&machine->lock);

	// exit lock
	tb_spinlock_exit(&machine->lock);

	// exit it
	tb_free(machine);
}
tb_spinlock_ref_t vm86_machine_lock(vm86_machine_ref_t self)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return_val(machine, tb_null);

	// the lock
	return &machine->lock;
}
vm86_data_ref_t vm86_machine_data(vm86_machine_ref_t self)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return_val(machine, tb_null);

	// the data
	return machine->data;
}
vm86_proc_ref_t vm86_machine_proc(vm86_machine_ref_t self, tb_char_t const* name)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return_val(machine && name, tb_null);

	// find proc
	return (vm86_proc_ref_t)tb_hash_map_get(machine->procs, name);
}
vm86_stack_ref_t vm86_machine_stack(vm86_machine_ref_t self)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return_val(machine, tb_null);

	// the stack
	return machine->stack;
}
vm86_text_ref_t vm86_machine_text(vm86_machine_ref_t self)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return_val(machine, tb_null);

	// the text
	return machine->text;
}
vm86_registers_ref_t vm86_machine_registers(vm86_machine_ref_t self)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return_val(machine, tb_null);

	// the registers
	return machine->registers;
}
vm86_machine_func_t vm86_machine_function(vm86_machine_ref_t self, tb_char_t const* name)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return_val(machine && name, tb_null);

	// the function
	return (vm86_machine_func_t)tb_hash_map_get(machine->functions, name);
}
tb_void_t vm86_machine_function_set(vm86_machine_ref_t self, tb_char_t const* name, vm86_machine_func_t func)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return(machine && name);

	// set the function
	if (func) tb_hash_map_insert(machine->functions, name, func);
	else tb_hash_map_remove(machine->functions, name);
}
tb_void_t vm86_machine_proc_set(vm86_machine_ref_t self, vm86_proc_ref_t proc)
{
	// check
	vm86_machine_t* machine = (vm86_machine_t*)self;
	tb_assert_and_check_return(machine &&  proc);

	const tb_char_t* name = vm86_proc_name(proc);
	tb_assert_and_check_return(name);

	tb_hash_map_insert(machine->procs, name, proc);
}