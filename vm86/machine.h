#ifndef VM86_MACHINE_H
#define VM86_MACHINE_H

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

 /* //////////////////////////////////////////////////////////////////////////////////////
  * extern
  */
__tb_extern_c_enter__

/* //////////////////////////////////////////////////////////////////////////////////////
 * types
 */
// the machine register type
typedef union __vm86_register_t
{
	/// the u8
	tb_uint8_t          u8[4];

	/// the u16
	tb_uint16_t         u16[2];

	/// the u32
	tb_uint32_t         u32;

}vm86_register_t;

// the machine register enum
typedef enum __vm86_register_e
{
	VM86_REGISTER_EAX = 0
	, VM86_REGISTER_EBX = 1
	, VM86_REGISTER_ECX = 2
	, VM86_REGISTER_EDX = 3

	, VM86_REGISTER_AX = 1 << 4
	, VM86_REGISTER_BX = 5 << 4
	, VM86_REGISTER_CX = 9 << 4
	, VM86_REGISTER_DX = 13 << 4

	, VM86_REGISTER_AH = 2 << 4
	, VM86_REGISTER_BH = 6 << 4
	, VM86_REGISTER_CH = 10 << 4
	, VM86_REGISTER_DH = 14 << 4

	, VM86_REGISTER_AL = 3 << 4
	, VM86_REGISTER_BL = 7 << 4
	, VM86_REGISTER_CL = 11 << 4
	, VM86_REGISTER_DL = 15 << 4

	, VM86_REGISTER_ESP = 4
	, VM86_REGISTER_EBP = 5
	, VM86_REGISTER_ESI = 6
	, VM86_REGISTER_EDI = 7

	, VM86_REGISTER_CS = 8
	, VM86_REGISTER_DS = 9
	, VM86_REGISTER_SS = 10
	, VM86_REGISTER_ES = 11
	, VM86_REGISTER_FS = 12
	, VM86_REGISTER_GS = 13

	, VM86_REGISTER_EIP = 14
	, VM86_REGISTER_EFLAGS = 15

	, VM86_REGISTER_XMM0 = 16
	, VM86_REGISTER_XMM1 = 17

	, VM86_REGISTER_MAXN = 18
	, VM86_REGISTER_MASK = 0xf

}vm86_register_e;

// the machine register eflag enum
typedef enum __vm86_register_eflag_e
{
	VM86_REGISTER_EFLAG_CF = 1 << 0
	, VM86_REGISTER_EFLAG_PF = 1 << 2
	, VM86_REGISTER_EFLAG_AF = 1 << 4
	, VM86_REGISTER_EFLAG_ZF = 1 << 6
	, VM86_REGISTER_EFLAG_SF = 1 << 7
	, VM86_REGISTER_EFLAG_OF = 1 << 11

}vm86_register_eflag_e;

// the machine registers type
typedef vm86_register_t(vm86_registers_t)[VM86_REGISTER_MAXN];

/// the machine registers ref type
typedef vm86_register_t* vm86_registers_ref_t;

// the machine data ref type
typedef void*  vm86_data_ref_t;

// the machine stack ref type
typedef void*   vm86_stack_ref_t;

// the machine segment ref type
typedef void* vm86_segment_ref_t;

// the machine segment info ref type
typedef void* vm86_segment_info_ref_t;

// the machine proc ref type
typedef void* vm86_proc_ref_t;

// the machine text ref type
typedef void* vm86_text_ref_t;

// the machine instruction ref type
typedef void* vm86_instruction_ref_t;

/*! the machine func type
 *
 * @param machine               the machine
 */
typedef tb_void_t(*vm86_machine_func_t)(vm86_machine_ref_t machine);


/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "data.h"
#include "segment.h"
#include "text.h"
#include "stack.h"
#include "register.h"
#include "parser.h"
#include "instruction.h"
#include "proc.h"

 /* //////////////////////////////////////////////////////////////////////////////////////
  * interfaces
  */

  /*! the machine instance
   *
   * @return                      the machine
   */
vm86_machine_ref_t              vm86_machine(tb_noarg_t);

/*! init machine
 *
 * @param data_size             the data size
 * @param stack_size            the stack size
 *
 * @return                      the machine
 */
vm86_machine_ref_t       vm86_machine_init(tb_size_t data_size, tb_size_t stack_size);

/*! exit machine
 *
 * @param machine               the machine
 */
tb_void_t                          vm86_machine_exit(vm86_machine_ref_t machine);

/*! the machine lock
 *
 * @param machine               the machine
 */
tb_spinlock_ref_t               vm86_machine_lock(vm86_machine_ref_t machine);

/*! the machine data
 *
 * @param machine               the machine
 *
 * @return                      the data
 */
vm86_data_ref_t                 vm86_machine_data(vm86_machine_ref_t machine);

/*! the machine stack
 *
 * @param machine               the machine
 *
 * @return                      the stack
 */
vm86_stack_ref_t                vm86_machine_stack(vm86_machine_ref_t machine);

/*! the machine registers
 *
 * @param machine               the machine
 *
 * @return                      the registers
 */
vm86_registers_ref_t            vm86_machine_registers(vm86_machine_ref_t machine);

/*! get function from the machine
 *
 * @param machine               the machine
 * @param name                  the function name
 *
 * @return                      the function address
 */
vm86_machine_func_t             vm86_machine_function(vm86_machine_ref_t machine, tb_char_t const* name);

/*! set function to the machine
 *
 * @param machine               the machine
 * @param name                  the function name
 * @param func                  the function address
 */
tb_void_t                       vm86_machine_function_set(vm86_machine_ref_t machine, tb_char_t const* name, vm86_machine_func_t func);

/*! set proc to the machine
 *
 * @param machine           the machine
 * @param proc                  the proc address
 */
tb_void_t                       vm86_machine_proc_set(vm86_machine_ref_t machine, vm86_proc_ref_t proc);


/*! get proc from the machine
 *
 * @param machine           the machine
 * @param name                the proc name
 */
vm86_proc_ref_t vm86_machine_proc(vm86_machine_ref_t machine, const tb_char_t* name);

/*! get text from the machine
 *
 * @param machine           the machine
 */
vm86_text_ref_t vm86_machine_text(vm86_machine_ref_t machine);

/* //////////////////////////////////////////////////////////////////////////////////////
 * extern
 */
__tb_extern_c_leave__

#endif


