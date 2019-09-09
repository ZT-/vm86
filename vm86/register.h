#ifndef VM86_REGISTER_H
#define VM86_REGISTER_H

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

 /* //////////////////////////////////////////////////////////////////////////////////////
  * extern
  */
__tb_extern_c_enter__


/* //////////////////////////////////////////////////////////////////////////////////////
 * interfaces
 */

static __tb_inline__ tb_uint8_t vm86_registers_eflages(vm86_registers_ref_t registers, vm86_register_eflag_e eflag)
{
	return (registers[VM86_REGISTER_EFLAGS].u32 & eflag) == 0 ? 0 : 1;
}
static __tb_inline__ tb_void_t vm86_registers_clear(vm86_registers_ref_t registers)
{
	// check
	tb_assert(registers);

	// clear it
	tb_memset(registers, 0, VM86_REGISTER_MAXN * sizeof(vm86_register_t));
}
static __tb_inline__ tb_uint32_t vm86_registers_value(vm86_registers_ref_t registers, tb_uint8_t index)
{
	// check
	tb_assert(registers);

	// get the register
	tb_size_t r = index & VM86_REGISTER_MASK;
	tb_assert(r < VM86_REGISTER_MAXN);

	// done
	tb_uint32_t value = 0;
	switch ((index >> 4) & 3)
	{
	case 0: value = registers[r].u32;       break;
	case 1: value = registers[r].u16[0];    break;
	case 2: value = registers[r].u8[1];     break;
	case 3: value = registers[r].u8[0];     break;
	default:
		tb_assert(0);
	}

	// ok?
	return value;
}
static __tb_inline__ tb_void_t vm86_registers_value_set(vm86_registers_ref_t registers, tb_uint8_t index, tb_uint32_t value)
{
	// check
	tb_assert(registers);

	// get the register
	tb_size_t r = index & VM86_REGISTER_MASK;
	tb_assert(r < VM86_REGISTER_MAXN);

	// done
	switch ((index >> 4) & 3)
	{
	case 0: registers[r].u32 = value;       break;
	case 1: registers[r].u16[0] = value;    break;
	case 2: registers[r].u8[1] = value;     break;
	case 3: registers[r].u8[0] = value;     break;
	default:
		tb_assert(0);
	}
}
static __tb_inline__ tb_char_t const* vm86_registers_cstr(tb_uint8_t index)
{
	// done
	tb_char_t const* cstr = tb_null;
	switch (index)
	{
	case VM86_REGISTER_EAX: cstr = "eax"; break;
	case VM86_REGISTER_EBX: cstr = "ebx"; break;
	case VM86_REGISTER_ECX: cstr = "ecx"; break;
	case VM86_REGISTER_EDX: cstr = "edx"; break;

	case VM86_REGISTER_ESP: cstr = "esp"; break;
	case VM86_REGISTER_EBP: cstr = "ebp"; break;
	case VM86_REGISTER_ESI: cstr = "esi"; break;
	case VM86_REGISTER_EDI: cstr = "edi"; break;

	case VM86_REGISTER_EAX | VM86_REGISTER_AL: cstr = "al"; break;
	case VM86_REGISTER_EAX | VM86_REGISTER_AH: cstr = "ah"; break;
	case VM86_REGISTER_EAX | VM86_REGISTER_AX: cstr = "ax"; break;

	case VM86_REGISTER_EBX | VM86_REGISTER_BL: cstr = "bl"; break;
	case VM86_REGISTER_EBX | VM86_REGISTER_BH: cstr = "bh"; break;
	case VM86_REGISTER_EBX | VM86_REGISTER_BX: cstr = "bx"; break;

	case VM86_REGISTER_ECX | VM86_REGISTER_CL: cstr = "cl"; break;
	case VM86_REGISTER_ECX | VM86_REGISTER_CH: cstr = "ch"; break;
	case VM86_REGISTER_ECX | VM86_REGISTER_CX: cstr = "cx"; break;

	case VM86_REGISTER_EDX | VM86_REGISTER_DL: cstr = "dl"; break;
	case VM86_REGISTER_EDX | VM86_REGISTER_DH: cstr = "dh"; break;
	case VM86_REGISTER_EDX | VM86_REGISTER_DX: cstr = "dx"; break;

	default: cstr = "unk"; break;
	}

	// ok?
	return cstr;
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * extern
 */
__tb_extern_c_leave__


#endif


