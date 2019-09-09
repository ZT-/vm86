/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME            "machine_inst"
#define TB_TRACE_MODULE_DEBUG           (1)

 /* //////////////////////////////////////////////////////////////////////////////////////
  * includes
  */
#include "machine.h"

  /* //////////////////////////////////////////////////////////////////////////////////////
   * types
   */
   // the machine instruction done ref type
struct __vm86_instruction_t;
struct __vm86_token_t;
struct __vm86_value_t;

typedef  tb_uint32_t(*vm86_instruction_done_ref_t)(struct __vm86_instruction_t* instruction, vm86_machine_ref_t machine);
typedef struct __vm86_value_t*(*vm86_token_done_ref_t)(struct __vm86_token_t* token, vm86_registers_ref_t registers);

typedef struct __vm86_value_t
{
	// is cstr? need free it
	tb_uint8_t                      is_cstr;

	// is register
	tb_uint8_t                      is_reg;

	//the value
	tb_value_t                      v;

	// the registers
	tb_uint8_t                      r;

	//value addr
	tb_uint32_t                   v_ref;

}vm86_value_t, *vm86_value_ref_t;

typedef struct __vm86_token_t
{
	// the registers
	tb_uint8_t                      r0;
	tb_uint8_t                      r1;

	// the values
	vm86_value_t				v0;
	vm86_value_t				v1;
	vm86_value_t				v2;
	vm86_value_t				ret;

	// the op: +, -, *
	tb_char_t                       op;

	//the type byte word
	tb_uint8_t						type;

	//done
	vm86_token_done_ref_t done;

}vm86_token_t, *vm86_token_ref_t;

// the machine instruction type
typedef struct __vm86_instruction_t
{
	// done it
	vm86_instruction_done_ref_t  done;

	//token done 
	vm86_token_ref_t token1;
	vm86_token_ref_t token2;
	vm86_token_ref_t token3;

	//the type byte word
	tb_uint8_t	ptr_type;

	//debug instruction string
	tb_value_t	debug_str;

}vm86_instruction_t;

// the machine instruction executor entry type
typedef struct __vm86_instruction_entry_t
{
	// the instruction name
	tb_char_t  const* name;

	// done it
	vm86_instruction_done_ref_t     done;

}vm86_instruction_entry_t, *vm86_instruction_entry_ref_t;

//the token executor entry type
typedef struct __vm86_token_entry_t
{
	// the instruction name
	tb_char_t  const     op;

	// done it
	vm86_token_done_ref_t     done;

}vm86_token_entry_t, *vm86_token_entry_ref_t;


/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */



#define vm_stack() vm86_machine_stack(machine)
#define vm_registers() vm86_machine_registers(machine)

#define get_r(r) vm86_registers_value(registers, r)
#define getr(r) vm86_registers_value(vm_registers(), r)
#define setr(r,v) vm86_registers_value_set(vm_registers(),r,v)
#define getef(ef)  vm86_registers_eflages(vm_registers(),ef)
 //#define setef(ef)  ((vm_registers()[VM86_REGISTER_EFLAGS].u32) += ef)

#define getv0()  instruction->token1->done(instruction->token1, vm_registers())
#define getv1()  instruction->token2->done(instruction->token2, vm_registers())
#define getv2()  instruction->token3->done(instruction->token3, vm_registers())

#define t_ptr(v) vm86_instruction_token_type_ptr(token->type, v)
#define getr_cstr(r) vm86_registers_cstr(r)


void vm86_registers_set_eflages(vm86_registers_ref_t registers, vm86_register_eflag_e eflag, tb_uint8_t flag)
{
	registers[VM86_REGISTER_EFLAGS].u32 += eflag;
}

//token 0 instruction
static tb_uint32_t vm86_instruction_done_cdq(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	//EAX < 80000000, EDX 为00000000；当EAX >= 80000000， EDX 则为FFFFFFFF
	if (getr(VM86_REGISTER_EAX) < 0x80000000)
	{
		setr(VM86_REGISTER_EDX, 0);
	}
	else
	{
		setr(VM86_REGISTER_EDX, 0xFFFFFFFF);
	}

	// trace
	tb_trace_d("cdq");

	// end
	return 1;
}
static tb_uint32_t vm86_instruction_done_leave(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// the stack
	const vm86_stack_ref_t stack = vm_stack();

	// mov esp, ebp
	setr(VM86_REGISTER_ESP, getr(VM86_REGISTER_EBP));

	// pop ebp 
	tb_uint32_t ebp = 0;
	vm86_stack_pop(stack, &ebp);

	// end
	return 1;
}
static tb_uint32_t vm86_instruction_done_retn(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// the stack
	const vm86_stack_ref_t stack = vm_stack();

	// pop the return address
	tb_uint32_t retn = 0;
	vm86_stack_pop(stack, &retn);

	// trace
	tb_trace_d("retn (%#x)", retn);

	// check
	return retn;
}

//token 1 instruction
static tb_uint32_t vm86_instruction_done_call(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	const vm86_value_ref_t v0 = getv0();

	// get the function name
	if (v0->is_cstr)
	{
		tb_char_t const* name = v0->v.cstr;

		// get the function
		const vm86_machine_func_t func = vm86_machine_function(machine, name);
		if (func)
		{
			// trace
			tb_trace_d("call %s(%#x)", name, func);

			// check
			tb_assert(func);

			// call the function
			func(machine);

			return 1;
		}

		const vm86_proc_ref_t proc = vm86_machine_proc(machine, name);

		v0->v.u32 = vm86_proc_oep(proc);
	}

	tb_uint32_t eip = getr(VM86_REGISTER_EIP);

	// the stack
	const vm86_stack_ref_t stack = vm_stack();

	// push the return address
	vm86_stack_push(stack, eip + 1);

	return v0->v.u32;
}
static tb_uint32_t vm86_instruction_done_dec(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	tb_assert(0);

	vm86_value_ref_t v0 = getv0();

	//*r0 = (*r0)--;

	// set r0
	//vm86_registers_value_set(registers, r0, r0);

	// trace
	//tb_trace_d("dec %s(%#x)", vm86_registers_cstr(instruction->r0), r0);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_inc(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}
static tb_uint32_t vm86_instruction_done_div(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	tb_uint32_t edx = getr(VM86_REGISTER_EDX);
	tb_uint32_t eax = getr(VM86_REGISTER_EAX);

	vm86_value_ref_t v0 = getv0();

	setr(VM86_REGISTER_EAX, eax / v0->v.u32);
	setr(VM86_REGISTER_EDX, eax % v0->v.u32);

	// trace
	tb_trace_d("div %#x", v0->v.u32);

	return 1;
}
static tb_uint32_t vm86_instruction_done_idiv(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	tb_int32_t edx = getr(VM86_REGISTER_EDX);
	tb_int32_t eax = getr(VM86_REGISTER_EAX);

	vm86_value_ref_t v0 = getv0();

	tb_int64_t d = edx * 0x100000000 + eax;

	setr(VM86_REGISTER_EAX, (tb_uint32_t)(d / v0->v.s32));
	setr(VM86_REGISTER_EDX, d % v0->v.s32);

	// trace
	tb_trace_d("idiv %#x", v0->v.s32);

	return 1;
}
static tb_uint32_t vm86_instruction_done_mul(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}
static tb_uint32_t vm86_instruction_done_ja(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// CF=0 and ZF=0
	tb_bool_t ok = getef(VM86_REGISTER_EFLAG_ZF) == 0 &&
		getef(VM86_REGISTER_EFLAG_CF) == 0;

	// get v0
	const vm86_value_ref_t v0 = getv0();

	// trace
	tb_trace_d("ja ret:%b %#x", ok, v0->v.u32);

	if (ok)
	{
		return v0->v.u32;
	}

	return 1;
}
static tb_uint32_t vm86_instruction_done_jb(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	//CF=1
	tb_bool_t ok = getef(VM86_REGISTER_EFLAG_CF) == 1;

	// get v0
	const vm86_value_ref_t v0 = getv0();

	// trace
	tb_trace_d("jb ret:%b %#x", ok, v0->v.u32);

	if (ok)
	{
		return v0->v.u32;
	}

	return 1;
}
static tb_uint32_t vm86_instruction_done_jbe(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	//CF=1 or ZF=1
	tb_bool_t ok = getef(VM86_REGISTER_EFLAG_CF) == 1||
		getef(VM86_REGISTER_EFLAG_ZF) == 1;

	// get v0
	const vm86_value_ref_t v0 = getv0();

	// trace
	tb_trace_d("jbe ret:%b %#x", ok, v0->v.u32);

	if (ok)
	{
		return v0->v.u32;
	}

	return 1;
}
static tb_uint32_t vm86_instruction_done_jg(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	//ZF=0 and SF=OF
	tb_uint32_t sf = getef(VM86_REGISTER_EFLAG_SF);
	tb_uint32_t of = getef(VM86_REGISTER_EFLAG_OF);
	tb_uint32_t zf = getef(VM86_REGISTER_EFLAG_ZF);
	tb_bool_t ok = (sf == of) && (zf == 0);

	// get v0
	const vm86_value_ref_t v0 = getv0();

	// trace
	tb_trace_d("jg ret:%b %#x", ok, v0->v.u32);

	if (ok)
	{
		return v0->v.u32;
	}

	return 1;
}
static tb_uint32_t vm86_instruction_done_jge(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// SF=OF
	tb_bool_t ok = getef(VM86_REGISTER_EFLAG_SF) == getef(VM86_REGISTER_EFLAG_OF);

	// get v0
	const vm86_value_ref_t v0 = getv0();

	// trace
	tb_trace_d("jge ret:%b %#x", ok, v0->v.u32);

	if (ok)
	{
		return v0->v.u32;
	}

	return 1;
}
static tb_uint32_t vm86_instruction_done_jl(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	//SF != OF
	tb_bool_t ok = getef(VM86_REGISTER_EFLAG_SF) != getef(VM86_REGISTER_EFLAG_OF);

	// get v0
	const vm86_value_ref_t v0 = getv0();

	// trace
	tb_trace_d("jl ret:%b %#x", ok, v0->v.u32);

	if (ok)
	{
		return v0->v.u32;
	}

	return 1;
}
static tb_uint32_t vm86_instruction_done_jle(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	//ZF != OF 或 ZF=1
	tb_bool_t ok = (getef(VM86_REGISTER_EFLAG_SF) != getef(VM86_REGISTER_EFLAG_OF)) ||
		getef(VM86_REGISTER_EFLAG_ZF) == 1;

	// get v0
	const vm86_value_ref_t v0 = getv0();

	// trace
	tb_trace_d("jle ret:%b %#x", ok, v0->v.u32);

	if (ok)
	{
		return v0->v.u32;
	}

	return 1;
}
static tb_uint32_t vm86_instruction_done_jmp(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get v0
	const vm86_value_ref_t v0 = getv0();

	// trace
	tb_trace_d("jmp %#x", v0->v.u32);

	// goto it
	return v0->v.u32;
}
static tb_uint32_t vm86_instruction_done_jnb(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}
static tb_uint32_t vm86_instruction_done_jns(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}
static tb_uint32_t vm86_instruction_done_jnz(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}
static tb_uint32_t vm86_instruction_done_jz(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}
static tb_uint32_t vm86_instruction_done_not(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}
static tb_uint32_t vm86_instruction_done_pop(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the stack
	const vm86_stack_ref_t stack = vm86_machine_stack(machine);
	tb_assert(stack);

	// pop r0
	tb_uint32_t r0 = 0;
	vm86_stack_pop(stack, &r0);

	// get v0
	vm86_value_ref_t v0 = getv0();

	// set r0
	setr(v0->r, r0);

	// trace
	tb_trace_d("pop %#x", r0);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_push(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// check
	tb_assert(instruction && machine);

	// the stack
	const vm86_stack_ref_t stack = vm86_machine_stack(machine);
	tb_assert(stack);

	// get v0
	vm86_value_ref_t v0 = getv0();

	// push r0
	vm86_stack_push(stack, v0->v.u32);

	// trace
	tb_trace_d("push %#x", v0->v.u32);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_nop(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}

//token 2 instruction
static tb_uint32_t vm86_instruction_done_add(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get v0
	const vm86_value_ref_t v0 = getv0();

	// get v1
	const vm86_value_ref_t v1 = getv1();

	if (v0->is_reg)
	{
		// set r0
		setr(v0->r, v0->v.u32 + v1->v.u32);
	}
	else
	{
		tb_assert(0);
	}

	// trace
	tb_trace_d("add %#x", v0->v.u32 + v1->v.u32);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_and(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get v
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	// set r0
	setr(v0->r, v0->v.u32 & v1->v.u32);

	// trace
	tb_trace_d("and %#x", v0->v.u32 & v1->v.u32);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_cmp(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// the registers
	vm86_registers_ref_t registers = vm_registers();
	tb_assert(registers);

	// get v
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	tb_int32_t ret = v0->v.u32 - v1->v.u32;
	tb_int32_t eflags = 0;

	if (ret == 0)
	{
		eflags += VM86_REGISTER_EFLAG_ZF;
	}
	else	if (ret < 0)
	{
		eflags += VM86_REGISTER_EFLAG_SF;
		eflags += VM86_REGISTER_EFLAG_CF;
	}

	registers[VM86_REGISTER_EFLAGS].u32 = eflags;

	// trace
	tb_trace_d("cmp %#x, %#x: %#x", v0->v.u32, v1->v.u32, eflags);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_imul(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}
static tb_uint32_t vm86_instruction_done_lea(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get v
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	// set r
	setr(v0->r, v0->v.u32 + v1->v.u32);

	// trace
	tb_trace_d("lea ");

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_or(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get r0
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	// set r0
	setr(v0->r, v0->v.u32 | v1->v.u32);

	// trace
	tb_trace_d("or  %#x", v0->v.u32 | v1->v.u32);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_sub(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get v0
	const vm86_value_ref_t v0 = instruction->token1->done(instruction->token1, registers);

	// get v1
	const vm86_value_ref_t v1 = instruction->token2->done(instruction->token2, registers);

	tb_uint32_t ret = v0->v.u32 - v1->v.u32;

	// set r0
	vm86_registers_value_set(registers, v0->r, ret);

	// trace
	tb_trace_d("sub ret:%#x", ret);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_xor(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get r0
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	// set r0
	setr(v0->r, v0->v.u32 ^ v1->v.u32);

	// trace
	tb_trace_d("xor  %#x", v0->v.u32 ^ v1->v.u32);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_sar(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	//算数右移

	// get r0
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	// set r0
	setr(v0->r, v0->v.s32 >> v1->v.s32);

	// trace
	tb_trace_d("shr %#x, %#x", v0->v.s32 >> v1->v.s32);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_shl(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get r0
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	// set r0
	setr(v0->r, v0->v.u32 << v1->v.u32);

	// trace
	tb_trace_d("shr %#x, %#x", v0->v.u32 << v1->v.u32);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_shr(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get r0
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	// set r0
	setr(v0->r, v0->v.u32 >> v1->v.u32);

	// trace
	tb_trace_d("shr %#x, %#x", v0->v.u32 >> v1->v.u32);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_test(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// the registers
	vm86_registers_ref_t registers = vm86_machine_registers(machine);
	tb_assert(registers);

	// get v
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	tb_uint32_t eflags = 0;
	if ((v0->v.u32 & v1->v.u32) == 0)
	{
		eflags |= VM86_REGISTER_EFLAG_ZF;
	}

	registers[VM86_REGISTER_EFLAGS].u32 = eflags;

	// trace
	tb_trace_d("test %#x", eflags);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_mov(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get v
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	// set r
	if (v0->is_reg)
	{
		setr(v0->r, v1->v.u32);
	}
	else
	{
		switch (instruction->ptr_type)
		{
		case 0:
			*(tb_uint32_t*)v0->v_ref = v1->v.u32;
			break;
		case 1:
			*(tb_uint16_t*)v0->v_ref = v1->v.u16;
			break;
		case 2:
			tb_assert(0);
			break;
		case 3:
			*(tb_uint8_t*)v0->v_ref = v1->v.u8;
			break;
		default:
			tb_assert(0);
		}
	}

	// trace
	tb_trace_d("mov");

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_movzx(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get v
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	// set r
	if (v0->is_reg)
	{
		setr(v0->r, v1->v.u16);
	}
	else
	{
		switch (instruction->ptr_type)
		{
		case 0:
		case 1:
			*(tb_uint16_t*)v0->v_ref = v1->v.u16;
			break;
		case 2:
			tb_assert(0);
			break;
		case 3:
			*(tb_uint8_t*)v0->v_ref = v1->v.u8;
			break;
		default:
			tb_assert(0);
		}
	}

	// trace
	tb_trace_d("movzx %#x", v1->v.u16);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_movsx(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get v
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();

	tb_uint32_t v = 0;

	if (v1->v.u16 < 0x8000)
	{
		v = v1->v.u16;
	}
	else
	{
		v = ((tb_int64_t)0xFFFF * 0x10000) + v1->v.u16;
	}

	// set r
	if (v0->is_reg)
	{
		setr(v0->r, v);
	}
	else
	{
		switch (instruction->ptr_type)
		{
		case 0:
		case 1:
			*(tb_uint16_t*)v0->v_ref = v;
			break;
		case 2:
			tb_assert(0);
			break;
		case 3:
			*(tb_uint8_t*)v0->v_ref = v;
			break;
		default:
			tb_assert(0);
		}
	}

	// trace
	tb_trace_d("movsx %#x", v);

	// ok
	return 1;
}
static tb_uint32_t vm86_instruction_done_movups(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}
static tb_uint32_t vm86_instruction_done_pxor(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	tb_assert(0);
	return 0;
}

//token 3 instruction
static tb_uint32_t vm86_instruction_done_shrd(vm86_instruction_t* instruction, vm86_machine_ref_t machine)
{
	// get v
	vm86_value_ref_t v0 = getv0();
	vm86_value_ref_t v1 = getv1();
	vm86_value_ref_t v2 = getv2();

	// set r
	setr(v0->r, v1->v.u32 >> v2->v.u32);

	// trace
	tb_trace_d("shrd %s, %s(%#x), %s(%#x)",
		getr_cstr(v0->r),
		getr_cstr(v1->r), v1->v.u32,
		getr_cstr(v2->r), v2->v.u32);

	// ok
	return 1;
}

//tokens
__tb_inline__ tb_uint32_t vm86_instruction_token_type_ptr(tb_uint8_t type, tb_uint32_t v)
{
	return v & (0xffffffff >> (8 * type));
}
static vm86_value_ref_t vm86_instruction_token_r0(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	token->ret.v.u32 = get_r(token->r0);
	token->ret.is_reg = 1;
	token->ret.r = token->r0;

	tb_trace_d("%s(%#x)", getr_cstr(token->ret.r), token->ret.v.u32);

	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_v0(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	if (token->v0.is_cstr)
	{
		token->ret.v.cstr = tb_strdup(token->v0.v.cstr);
		token->ret.is_cstr = token->v0.is_cstr;
		tb_trace_d("%s", token->ret.v.cstr);
	}
	else
	{
		token->ret.v.u32 = token->v0.v.u32;
		tb_trace_d("%#x", token->v0.v.u32);
	}
	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_$r0$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	token->ret.v.u32 = get_r(token->r0);
	token->ret.v_ref = t_ptr(token->ret.v.u32);
	tb_trace_d("[%s(%#x)]", getr_cstr(token->r0), token->ret.v.u32);
	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_$v0$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	token->ret.v.u32 = t_ptr(*(tb_uint32_t*)token->v0.v.u32);
	tb_trace_d("[%#x]", token->ret.v.u32);
	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_$r0_add_v0$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	const tb_uint32_t v0 = token->v0.v.u32;
	const tb_uint32_t r0 = get_r(token->r0);
	const tb_uint32_t v_ref = r0 + v0;
	token->ret.v_ref = v_ref;
	token->ret.v.u32 = t_ptr(*(tb_uint32_t*)v_ref);

	tb_trace_d("[%s(%#x)+%#x]=%#x(%#x)", getr_cstr(token->r0), r0, v0, v_ref, token->ret.v.u32);
	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_$r0_sub_v0$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	tb_assert(0);
	token->v0.v.u32 = *(tb_uint32_t*)(get_r(token->r0) - token->v0.v.u32);
	return &token->v0;
}
static vm86_value_ref_t vm86_instruction_token_$r0_mul_v0$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	tb_assert(0);
	token->v0.v.u32 = *(tb_uint32_t*)(get_r(token->r0) * token->v0.v.u32);
	return &token->v0;
}
static vm86_value_ref_t vm86_instruction_token_$r0_add_r1$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	const tb_uint32_t r0 = get_r(token->r0);
	const tb_uint32_t r1 = get_r(token->r1);
	token->ret.v_ref = r0 + r1;
	token->ret.v.u32 = t_ptr(*(tb_uint32_t*)token->ret.v_ref);

	tb_trace_d("[%s(%#x)+%s(%#x)]=%#x(%#x)",
		getr_cstr(token->r0), r0,
		getr_cstr(token->r1), r1,
		token->ret.v_ref,
		token->ret.v.u32);

	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_$r0_sub_r1$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	tb_assert(0);
	token->v0.v.u32 = *(tb_uint32_t*)(get_r(token->r0) - get_r(token->r1));
	return &token->v0;
}
static vm86_value_ref_t vm86_instruction_token_$r0_mul_r1$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	tb_assert(0);
	token->v0.v.u32 = *(tb_uint32_t*)(get_r(token->r0) * get_r(token->r1));
	return &token->v0;
}
static vm86_value_ref_t vm86_instruction_token_$r0_add_v0_op_v1$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	tb_assert(0);
	const tb_uint8_t r = get_r(token->r0);
	switch (token->op)
	{
	case '+':
		token->v0.v.u32 = *(tb_uint32_t*)(r + token->v0.v.u32 + token->v1.v.u32);
		break;
	case '-':
		token->v0.v.u32 = *(tb_uint32_t*)(r + token->v0.v.u32 - token->v1.v.u32);
		break;
	case '*':
		token->v0.v.u32 = *(tb_uint32_t*)(r + token->v0.v.u32 * token->v1.v.u32);
		break;
	default:
		tb_assert(0);
	}
	return &token->v0;
}
static vm86_value_ref_t vm86_instruction_token_$r0_add_r1_op_v0$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	const tb_uint8_t r0 = get_r(token->r0);
	const tb_uint8_t r1 = get_r(token->r1);
	tb_uint32_t ret = 0;

	switch (token->op)
	{
	case '+':
		ret = (r0 + r1 + token->v0.v.u32);
		break;
	case '-':
		ret = (r0 + r1 - token->v0.v.u32);
		break;
	case '*':
		ret = (r0 + r1 * token->v0.v.u32);
		break;
	default:
		tb_assert(0);
	}

	token->ret.v.u32 = t_ptr(ret);

	tb_trace_d("[%s(%#x)+%s(%#x) %c %#x]=%#x", getr_cstr(token->r0), r0,
		getr_cstr(token->r1), r1,
		token->op,
		token->v0.v.u32,
		token->ret.v.u32);

	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_$v0_add_v1_op_v2$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	tb_assert(0);
	switch (token->op)
	{
	case '+':
		token->v0.v.u32 = *(tb_uint32_t*)(token->v0.v.u32 + token->v1.v.u32 + token->v2.v.u32);
		break;
	case '-':
		token->v0.v.u32 = *(tb_uint32_t*)(token->v0.v.u32 + token->v1.v.u32 - token->v2.v.u32);
		break;
	case '*':
		token->v0.v.u32 = *(tb_uint32_t*)(token->v0.v.u32 + token->v1.v.u32 * token->v2.v.u32);
		break;
	default:
		tb_assert(0);
	}
	return &token->v0;
}
static vm86_value_ref_t vm86_instruction_token_v0$r0$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	const tb_uint8_t r0 = get_r(token->r0);
	tb_uint32_t ret = *(tb_uint32_t*)(token->v0.v.u32 + r0);
	token->ret.v.u32 = t_ptr(ret);

	tb_trace_d("%d[%s(%#x)] = %#x", token->v0.v.u32, getr_cstr(token->r0), r0, token->ret.v.u32);

	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_v0$r0_op_v1$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	const tb_uint8_t r0 = get_r(token->r0);
	tb_uint32_t ret = 0;

	switch (token->op)
	{
	case '+':
		ret = *(tb_uint32_t*)(token->v0.v.u32 + (r0 + token->v1.v.u32));
		break;
	case '-':
		ret = *(tb_uint32_t*)(token->v0.v.u32 + (r0 - token->v1.v.u32));
		break;
	case '*':
		ret = *(tb_uint32_t*)(token->v0.v.u32 + (r0 * token->v1.v.u32));
		break;
	default:
		tb_assert(0);
	}

	token->ret.v.u32 = t_ptr(ret);

	tb_trace_d("%#x[%s(%#x) %c %#x]= %#x",
		token->v0.v.u32,
		getr_cstr(token->r0), r0,
		token->op,
		token->v1.v.u32,
		token->ret.v.u32);

	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_v0$r0_op_r1$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	const tb_uint8_t r0 = get_r(token->r0);
	const tb_uint8_t r1 = get_r(token->r1);
	tb_uint32_t ret = 0;

	switch (token->op)
	{
	case '+':
		ret = *(tb_uint32_t*)(token->v0.v.u32 + (r0 + r1));
		break;
	case '-':
		ret = *(tb_uint32_t*)(token->v0.v.u32 + (r0 - r1));
		break;
	case '*':
		ret = *(tb_uint32_t*)(token->v0.v.u32 + (r0 * r1));
		break;
	default:
		tb_assert(0);
	}

	token->ret.v.u32 = t_ptr(ret);

	tb_trace_d("%#x[%s(%#x) %c %s(%#x)]= %#x",
		token->v0.v.u32,
		getr_cstr(token->r0), r0,
		token->op,
		getr_cstr(token->r1), r1,
		token->ret.v.u32);

	return &token->ret;
}
static vm86_value_ref_t vm86_instruction_token_v0$v1_op_v2$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	tb_assert(0);
	switch (token->op)
	{
	case '+':
		token->v0.v.u32 = *(tb_uint32_t*)(token->v0.v.u32 + (token->v1.v.u32 + token->v2.v.u32));
		break;
	case '-':
		token->v0.v.u32 = *(tb_uint32_t*)(token->v0.v.u32 + (token->v1.v.u32 - token->v2.v.u32));
		break;
	case '*':
		token->v0.v.u32 = *(tb_uint32_t*)(token->v0.v.u32 + (token->v1.v.u32 * token->v2.v.u32));
		break;
	default:
		tb_assert(0);
	}
	return &token->v0;
}
static vm86_value_ref_t vm86_instruction_token_v0_add_v1$r0_op_v2$(vm86_token_ref_t token, vm86_registers_ref_t registers)
{
	tb_assert(0);
	const tb_uint8_t r = get_r(token->r0);
	switch (token->op)
	{
	case '+':
		token->v0.v.u32 = *(tb_uint32_t*)(token->v0.v.u32 + token->v1.v.u32*(r + token->v2.v.u32));
		break;
	case '-':
		token->v0.v.u32 = *(tb_uint32_t*)(token->v0.v.u32 + token->v1.v.u32*(r - token->v2.v.u32));
		break;
	case '*':
		token->v0.v.u32 = *(tb_uint32_t*)(token->v0.v.u32 + token->v1.v.u32*(r * token->v2.v.u32));
		break;
	default:
		tb_assert(0);
	}
	return &token->v0;
}

// the instruction entries
static vm86_instruction_entry_t g_xxx_token_x0[] =
{
	{ "cdq",    vm86_instruction_done_cdq    }
,	{ "leave",   vm86_instruction_done_leave     }
,   { "retn",     vm86_instruction_done_retn        }
};

static vm86_instruction_entry_t g_xxx_token_x1[] =
{
	{ "call",   vm86_instruction_done_call      }
,	{ "dec",   vm86_instruction_done_dec    }
,	{ "div",    vm86_instruction_done_div     }
,	{ "idiv",    vm86_instruction_done_idiv     }
,	{ "inc",    vm86_instruction_done_inc    }
,	{ "ja",		vm86_instruction_done_ja        }
,   { "jb",		vm86_instruction_done_jb     }
,   { "jbe",	vm86_instruction_done_jbe      }
,   { "jg",		vm86_instruction_done_jg      }
,   { "jge",	vm86_instruction_done_jge     }
,   { "jl",		vm86_instruction_done_jl      }
,   { "jle",		vm86_instruction_done_jle      }
,   { "jmp",	vm86_instruction_done_jmp     }
,   { "jnb",	vm86_instruction_done_jnb     }
,   { "jns",		vm86_instruction_done_jns       }
,   { "jnz",		vm86_instruction_done_jnz       }
,   { "jz",		vm86_instruction_done_jz         }
,	{ "mul",    vm86_instruction_done_mul   }
,   { "nop",   vm86_instruction_done_nop    }
,   { "not",    vm86_instruction_done_not      }
,   { "pop",    vm86_instruction_done_pop    }
,   { "push",   vm86_instruction_done_push  }
};

static vm86_instruction_entry_t g_xxx_token_x2[] =
{
	{ "add",    vm86_instruction_done_add     }
,   { "and",    vm86_instruction_done_and     }
,	{ "cmp", vm86_instruction_done_cmp     }
,   { "imul",   vm86_instruction_done_imul    }
,	{ "lea",    vm86_instruction_done_lea  }
,	{ "mov", vm86_instruction_done_mov     }
,	{ "movsx", vm86_instruction_done_movsx     }
,   { "movups",  vm86_instruction_done_movups    }
,   { "movzx",  vm86_instruction_done_movzx    }
,   { "or",     vm86_instruction_done_or		 }
,   { "pxor", vm86_instruction_done_pxor		 }
,   { "sar",    vm86_instruction_done_sar      }
,   { "shl",    vm86_instruction_done_shl     }
,   { "shr",    vm86_instruction_done_shr      }
,   { "sub",    vm86_instruction_done_sub     }
,   { "test",    vm86_instruction_done_test     }
,   { "xor",    vm86_instruction_done_xor     }
};

static vm86_instruction_entry_t g_xxx_token_x3[] =
{
	{ "shrd",    vm86_instruction_done_shrd     }
};

// the token entries
static vm86_token_entry_t g_token_$r0_xxx_v0$[] =
{
	{'+', vm86_instruction_token_$r0_add_v0$},
	{ '-', vm86_instruction_token_$r0_sub_v0$},
	{ '*', vm86_instruction_token_$r0_mul_v0$},
};

static vm86_token_entry_t g_token_$r0_xxx_r1$[] =
{
	{'+', vm86_instruction_token_$r0_add_r1$},
	{'-', vm86_instruction_token_$r0_sub_r1$},
	{ '*', vm86_instruction_token_$r0_mul_r1$},
};

//static vm86_token_entry_t g_token_$v0_xxx_v1$[] =
//{
//	{"+", vm86_instruction_token_$v0_add_v1$},
//	{ "-", vm86_instruction_token_$v0_sub_v1$},
//	{ "*", vm86_instruction_token_$v0_mul_v1$},
//};

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */

static tb_long_t vm86_instruction_comp(tb_iterator_ref_t iterator, tb_cpointer_t item, tb_cpointer_t name)
{
	// check
	tb_assert(item);

	// comp it
	tb_char_t const* i_name = ((vm86_instruction_entry_ref_t)item)->name;
	return tb_stricmp(i_name, (tb_char_t const*)name);
}
static vm86_instruction_done_ref_t vm86_instruction_find(tb_char_t const* name, vm86_instruction_entry_ref_t entries, tb_size_t count)
{
	// init iterator
	tb_array_iterator_t array_iterator;
	tb_iterator_ref_t   iterator = tb_array_iterator_init_mem(&array_iterator, entries, count, sizeof(vm86_instruction_entry_t));

	// find executor by the binary search
	tb_size_t itor = tb_binary_find_all_if(iterator, vm86_instruction_comp, name);
	tb_assert(itor != tb_iterator_tail(iterator));

	// get the executor
	vm86_instruction_entry_ref_t entry = (vm86_instruction_entry_ref_t)tb_iterator_item(iterator, itor);
	tb_assert(entry && entry->done);

	// ok?
	return entry->done;
}
static vm86_token_done_ref_t vm86_instruction_op_find(tb_char_t const op, vm86_token_entry_ref_t entries, tb_size_t count)
{
	for (tb_size_t i = 0; i < count; ++i)
	{
		const vm86_token_entry_ref_t entry = entries + i;
		if (op == entry->op)
			return entry->done;
	}
	tb_assert(0);
	return 0;
}

vm86_instruction_ref_t vm86_instruction_init(tb_uint32_t n)
{
	vm86_instruction_t** ref = tb_nalloc0_type(n, vm86_instruction_t*);
	for (tb_uint32_t i = 0; i < n; ++i)
	{
		ref[i] = tb_malloc0_type(vm86_instruction_t);
	}
	return ref;
}
void  vm86_instruction_token_release(vm86_token_ref_t token)
{
	if (token->v1.is_cstr)
	{
		tb_free(token->v1.v.cstr);
		token->v1.v.cstr = 0;
	}
	if (token->v2.is_cstr)
	{
		tb_free(token->v2.v.cstr);
		token->v2.v.cstr = 0;
	}
	tb_free(token);
	token = 0;
}
void  vm86_instruction_release(vm86_instruction_t* instruction)
{
	if (instruction->token1)
	{
		vm86_instruction_token_release(instruction->token1);
	}
	if (instruction->token2)
	{
		vm86_instruction_token_release(instruction->token2);
	}
	if (instruction->token3)
	{
		vm86_instruction_token_release(instruction->token3);
	}
	tb_free(instruction);
	instruction = 0;
}
tb_bool_t vm86_instruction_parser_token(vm86_token_ref_t* p_token, const tb_char_t* pp, const tb_size_t maxn, vm86_data_ref_t data, tb_hash_map_ref_t proc_labels, tb_hash_map_ref_t proc_locals)
{
	// trace
	//tb_trace_d("token %s", pp);

	// check
	tb_assert_and_check_return_val(pp && maxn, tb_false);

	const vm86_token_ref_t  token = tb_malloc0_type(vm86_token_t);

	// done
	tb_bool_t ok = tb_false;
	const tb_size_t len = tb_strlen(pp);

	tb_char_t const*    p = pp;
	tb_char_t const*    e = pp + len;

	tb_uint16_t         r0 = 0;
	tb_uint16_t         r1 = 0;

	tb_value_t         v0;
	tb_value_t         v1;
	tb_value_t         v2;

	tb_char_t			 op;

	tb_bool_t			is_str;

	do
	{
		//check segment
		char segment = 0;
		if (vm86_parser_get_segment_type(&p, e, &segment))
		{
			//token->type = type;
		}

		//check type
		char type = 0;
		if (vm86_parser_get_ptr_type(&p, e, &type))
		{
			token->type = type;
		}

		//[ r0 ... ] ?
		if (*p == '[' && *(e - 1) == ']')
		{
			//skip [
			tb_assert(*p == '[');
			p++;

			//get r0
			if (!vm86_parser_get_register(&p, e, &r0)) break;

			///[ r0 ]
			if (*p == ']')
			{
				token->r0 = (tb_uint8_t)r0;
				token->done = vm86_instruction_token_$r0$;
			}
			else
			{
				//get op 
				if (!vm86_parser_get_op(&p, e, &op)) break;

				//get r1
				if (vm86_parser_get_register(&p, e, &r1))
				{
					//[ r0 op r1]
					if (*p == ']')
					{
						token->r0 = (tb_uint8_t)r0;
						token->r1 = (tb_uint8_t)r1;
						token->op = op;
						token->done = vm86_instruction_op_find(op, g_token_$r0_xxx_r1$, sizeof g_token_$r0_xxx_r1$);
					}
					//[ r0 op r1 op v0]
					else
					{
						//get op 
						if (!vm86_parser_get_op(&p, e, &op)) break;

						//get v0
						if (!vm86_parser_get_value(&p, e, &v0, proc_locals, proc_labels, data, &is_str)) break;

						token->r0 = (tb_uint8_t)r0;
						token->r1 = (tb_uint8_t)r1;
						token->v0.v = v0;
						token->op = op;
						token->done = vm86_instruction_token_$r0_add_r1_op_v0$;
					}
				}
				//get v0
				else	if (vm86_parser_get_value(&p, e, &v0, proc_locals, proc_labels, data, &is_str))
				{
					//[ r0 op v0]
					if (*p == ']')
					{
						token->r0 = (tb_uint8_t)r0;
						token->v0.v = v0;
						token->op = op;
						token->done = vm86_instruction_op_find(op, g_token_$r0_xxx_v0$, sizeof g_token_$r0_xxx_v0$);
					}
					//[ r0 op v0 op v1]
					else
					{
						//get op 
						if (!vm86_parser_get_op(&p, e, &op)) break;

						//get v1
						if (!vm86_parser_get_value(&p, e, &v1, proc_locals, proc_labels, data, &is_str)) break;

						token->r0 = (tb_uint8_t)r0;
						token->v0.v = v0;
						token->v1.v = v1;
						token->op = op;
						token->done = vm86_instruction_token_$r0_add_v0_op_v1$;
					}
				}
				else break;
			}
		}
		//v0[ ... ] ?
		else if (*p != '[' && *(e - 1) == ']')
		{
			//get v0
			if (!vm86_parser_get_value(&p, e, &v0, proc_locals, proc_labels, data, &is_str)) break;

			//type ptr v0+v1[r0*v2] like byte ptr dword_1110+1[eax * 4]
			if (*p == '+')
			{
				//skip +
				p++;

				//get number v1
				if (!vm86_parser_get_number_value(&p, e, &v1.u32)) break;

				//skip [
				tb_assert(*p == '[');
				p++;

				//get r0
				if (!vm86_parser_get_register(&p, e, &r0)) break;

				//get op 
				if (!vm86_parser_get_op(&p, e, &op)) break;

				//get number v1
				if (!vm86_parser_get_number_value(&p, e, &v2.u32)) break;

				token->r0 = (tb_uint8_t)r0;
				token->op = op;
				token->v0.v = v0;
				token->v1.v = v1;
				token->v2.v = v2;
				token->done = vm86_instruction_token_v0_add_v1$r0_op_v2$;
			}
			else
			{
				//skip [
				tb_assert(*p == '[');
				p++;

				//v0[r0...]?
				//get r0
				if (vm86_parser_get_register(&p, e, &r0))
				{
					//v0[r0]
					if (*p == ']')
					{
						token->r0 = (tb_uint8_t)r0;
						token->v0.v = v0;
						token->done = vm86_instruction_token_v0$r0$;
					}
					else
					{
						//get op 
						if (!vm86_parser_get_op(&p, e, &op)) break;

						//v0[r0 op r1]
						if (vm86_parser_get_register(&p, e, &r1))
						{
							token->r0 = (tb_uint8_t)r0;
							token->r1 = (tb_uint8_t)r1;
							token->v0.v = v0;
							token->op = op;
							token->done = vm86_instruction_token_v0$r0_op_r1$;
						}
						//v0[r0 op v1]
						else
						{
							//get v1
							if (!vm86_parser_get_value(&p, e, &v1, proc_locals, proc_labels, data, &is_str)) break;

							token->r0 = (tb_uint8_t)r0;
							token->v0.v = v0;
							token->v1.v = v1;
							token->op = op;
							token->done = vm86_instruction_token_v0$r0_op_v1$;
						}

					}
				}
				//v0[v1 op v2]
				//get v1
				else if (vm86_parser_get_value(&p, e, &v1, proc_locals, proc_labels, data, &is_str))
				{
					//get op 
					if (!vm86_parser_get_op(&p, e, &op)) break;

					//get v2
					if (!vm86_parser_get_value(&p, e, &v2, proc_locals, proc_labels, data, &is_str)) break;

					token->v0.v = v0;
					token->v1.v = v1;
					token->v2.v = v2;
					token->op = op;
					token->done = vm86_instruction_token_v0$v1_op_v2$;
				}
				else break;
			}
		}
		//xxx
		else
		{
			//r0
			if (vm86_parser_get_register(&p, e, &r0))
			{
				token->type = (r0 >> 4) & 3;
				token->r0 = (tb_uint8_t)r0;
				token->done = vm86_instruction_token_r0;
			}
			//v0
			else if (vm86_parser_get_value(&p, e, &v0, proc_locals, proc_labels, data, &is_str))
			{
				token->v0.v = v0;
				token->v0.is_cstr = is_str;
				token->done = vm86_instruction_token_v0;
			}
			else break;
		}

		ok = tb_true;

	} while (0);

	tb_assert(ok);

	*p_token = token;

	// ok?
	return ok;
}
tb_bool_t vm86_instruction_compile(vm86_instruction_t* instruction, tb_char_t const* code, tb_size_t size, vm86_machine_ref_t machine, tb_hash_map_ref_t proc_labels, tb_hash_map_ref_t proc_locals)
{
	// check
	tb_assert_and_check_return_val(instruction && code && size && machine && proc_labels, tb_false);

	instruction->debug_str.cstr = tb_strdup(code);

	// done
	tb_bool_t ok = tb_false;
	tb_char_t const*    p = code;
	tb_char_t const*    e = code + size;

	do
	{
		// the .data
		const vm86_data_ref_t data = vm86_machine_data(machine);
		tb_assert_and_check_break(data);

		// get instruction name
		tb_char_t name[32] = { 0 };
		if (!vm86_parser_get_instruction_name(&p, e, name, sizeof name)) break;

		// init instruction hint
		//instruction->hint[0] = name[0];
		//instruction->hint[1] = name[1];
		//instruction->hint[2] = name[2];

		//get tokens
#define token_max_num 3
		tb_size_t token_num = 0;
		tb_char_t token[token_max_num][64] = { {0} };
		do
		{
			tb_memset(token[token_num], 0, sizeof token[token_num]);
			if (vm86_parser_get_token(&p, e, token[token_num], sizeof token[token_num]))
			{
				token_num++;
				tb_assert(token_num <= token_max_num);
			}
		} while (p < e);

		// init executor
		instruction->done = tb_null;
		instruction->token1 = tb_null;
		instruction->token2 = tb_null;

		switch (token_num)
		{
		case 0:
			instruction->done = vm86_instruction_find(name, g_xxx_token_x0, tb_arrayn(g_xxx_token_x0));
			break;
		case 1:
			vm86_instruction_parser_token(&instruction->token1, token[0], sizeof token[0], data, proc_labels, proc_locals);
			instruction->done = vm86_instruction_find(name, g_xxx_token_x1, tb_arrayn(g_xxx_token_x1));
			instruction->ptr_type = instruction->token1->type;
			break;
		case 2:
			vm86_instruction_parser_token(&instruction->token1, token[0], sizeof token[0], data, proc_labels, proc_locals);
			vm86_instruction_parser_token(&instruction->token2, token[1], sizeof token[1], data, proc_labels, proc_locals);
			instruction->done = vm86_instruction_find(name, g_xxx_token_x2, tb_arrayn(g_xxx_token_x2));
			instruction->ptr_type = instruction->token1->type > instruction->token2->type ?
				instruction->token1->type :
				instruction->token2->type;
			break;
		case 3:
			vm86_instruction_parser_token(&instruction->token1, token[0], sizeof token[0], data, proc_labels, proc_locals);
			vm86_instruction_parser_token(&instruction->token2, token[1], sizeof token[1], data, proc_labels, proc_locals);
			vm86_instruction_parser_token(&instruction->token3, token[2], sizeof token[2], data, proc_labels, proc_locals);
			instruction->done = vm86_instruction_find(name, g_xxx_token_x3, tb_arrayn(g_xxx_token_x3));
			break;
		default:;
		}

		// check
		tb_assert(instruction->done);

		// ok 
		ok = tb_true;
	} while (0);

	// ok?
	return ok;
}
tb_bool_t vm86_instruction_done(vm86_machine_ref_t machine, tb_uint32_t eip)
{
	// check
	tb_assert(machine);

	vm86_text_ref_t text = vm86_machine_text(machine);
	tb_assert_and_check_return_val(text, tb_null);

	vm86_instruction_t* instruction = (vm86_instruction_t*)vm86_text_get_instruction(text, eip);
	tb_trace_d("[%d] %s", eip, instruction->debug_str.cstr);
	tb_assert_and_check_return_val(instruction && instruction->done, tb_null);

	tb_uint32_t ip = instruction->done(instruction, machine);

	if (ip == 1)
	{
		eip++;
	}
	else
	{
		eip = ip;
	}

	setr(VM86_REGISTER_EIP, eip);

	// the stack
	//const vm86_stack_ref_t stack = vm86_machine_stack(machine);
	//tb_assert(stack);
	//vm86_stack_dump(stack);
	//tb_uint32_t esp = getr(VM86_REGISTER_ESP);
	//tb_uint32_t ebp = getr(VM86_REGISTER_EBP);
	//tb_trace_d("esp(%#x),ebp(%#x)", esp, ebp);

	return tb_true;
}