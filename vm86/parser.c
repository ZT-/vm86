/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME            "machine_parser"
#define TB_TRACE_MODULE_DEBUG           (0)

 /* //////////////////////////////////////////////////////////////////////////////////////
  * includes
  */
#include "machine.h"

  /* //////////////////////////////////////////////////////////////////////////////////////
   * private implementation
   */
static tb_long_t vm86_parser_comp_register(tb_iterator_ref_t iterator, tb_cpointer_t item, tb_cpointer_t name)
{
	// check
	tb_assert(item);

	// hacking for register 
	return tb_stricmp(*((tb_char_t const**)item), (tb_char_t const*)name);
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
tb_bool_t vm86_parser_get_variable_name(tb_char_t const** pp, tb_char_t const* e, tb_char_t* name, tb_size_t maxn)
{
	// check
	tb_assert(pp && e && name && maxn);

	// done
	tb_bool_t           ok = tb_false;
	tb_char_t const*    p = *pp;
	do
	{
		// save base
		tb_char_t const* b = p;

		// check
		tb_check_break(p < e && (tb_isalpha(*p) || *p == '_'));
		p++;

		// get name
		while (p < e && (tb_isalpha(*p) || *p == '_' || tb_isdigit(*p))) p++;
		tb_check_break(p <= e && p - b < maxn);
		tb_memcpy(name, b, p - b);

		// end
		name[p - b] = '\0';

		// skip the space
		while (p < e && tb_isspace(*p)) p++;

		// ok
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
tb_bool_t vm86_parser_get_segment_name(tb_char_t const** pp, tb_char_t const* e, tb_char_t* name, tb_size_t maxn)
{
	// check
	tb_assert(pp && e && name && maxn);

	// done
	tb_bool_t           ok = tb_false;
	tb_char_t const*    p = *pp;
	do
	{
		// save base
		tb_char_t const* b = p;

		// get name
		while (p < e && tb_isalpha(*p)) p++;
		tb_check_break(p < e && p - b < maxn && *p == ':');
		tb_memcpy(name, b, p - b);

		// end
		name[p - b] = '\0';

		// skip ":"
		p++;

		// skip the space
		while (p < e && tb_isspace(*p)) p++;

		// ok
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
tb_bool_t vm86_parser_get_instruction_name(tb_char_t const** pp, tb_char_t const* e, tb_char_t* name, tb_size_t maxn)
{
	// check
	tb_assert(pp && e && name && maxn);

	// done
	tb_bool_t           ok = tb_false;
	tb_char_t const*    p = *pp;
	do
	{
		// save base
		tb_char_t const* b = p;

		// skip name
		while (p < e && (tb_isalpha(*p) || tb_isdigit(*p))) p++;
		tb_check_break(p <= e && p - b < maxn);

		// not instruction name?
		if (p < e && !tb_isspace(*p)) break;

		// save name
		tb_memcpy(name, b, p - b);

		// end
		name[p - b] = '\0';

		// skip the space
		while (p < e && tb_isspace(*p)) p++;

		// ok
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
tb_bool_t vm86_parser_get_register(tb_char_t const** pp, tb_char_t const* e, tb_uint16_t* r)
{
	// check
	tb_assert(pp && e && r);

	// done
	tb_bool_t           ok = tb_false;
	tb_char_t const*    p = *pp;
	do
	{
		// save base
		tb_char_t const* b = p;

		// get instruction name
		tb_char_t name[64] = { 0 };
		while (p < e && (tb_isalpha(*p) || tb_isdigit(*p))) p++;
		tb_check_break(p <= e && p - b < sizeof(name));
		tb_memcpy(name, b, p - b);

		// skip the space
		while (p < e && tb_isspace(*p)) p++;

		// the register entry type
		typedef struct __vm86_register_entry_t
		{
			// the register name
			tb_char_t const*        name;

			// the register index
			tb_uint8_t              index;

		}vm86_register_entry_t, *vm86_register_entry_ref_t;

		// the registers
		static vm86_register_entry_t s_registers[] =
		{
			{ "ah",     VM86_REGISTER_EAX | VM86_REGISTER_AH }
		,   { "al",     VM86_REGISTER_EAX | VM86_REGISTER_AL }
		,   { "ax",     VM86_REGISTER_EAX | VM86_REGISTER_AX }
		,   { "bh",     VM86_REGISTER_EBX | VM86_REGISTER_BH }
		,   { "bl",     VM86_REGISTER_EBX | VM86_REGISTER_BL }
		,   { "bx",     VM86_REGISTER_EBX | VM86_REGISTER_BX }
		,   { "ch",     VM86_REGISTER_ECX | VM86_REGISTER_CH }
		,   { "cl",     VM86_REGISTER_ECX | VM86_REGISTER_CL }
		,   { "cx",     VM86_REGISTER_ECX | VM86_REGISTER_CX }
		,   { "dh",     VM86_REGISTER_EDX | VM86_REGISTER_DH }
		,   { "dl",     VM86_REGISTER_EDX | VM86_REGISTER_DL }
		,   { "dx",     VM86_REGISTER_EDX | VM86_REGISTER_DX }
		,   { "eax",    VM86_REGISTER_EAX }
		,   { "ebp",    VM86_REGISTER_EBP }
		,   { "ebx",    VM86_REGISTER_EBX }
		,   { "ecx",    VM86_REGISTER_ECX }
		,   { "edi",    VM86_REGISTER_EDI }
		,   { "edx",    VM86_REGISTER_EDX }
		,   { "esi",    VM86_REGISTER_ESI }
		,   { "esp",    VM86_REGISTER_ESP }
		,   { "xmm0",    VM86_REGISTER_XMM0 }
		,   { "xmm1",    VM86_REGISTER_XMM1 }
		};

		// init iterator
		tb_array_iterator_t array_iterator;
		tb_iterator_ref_t   iterator = tb_array_iterator_init_mem(&array_iterator, s_registers, tb_arrayn(s_registers), sizeof(vm86_register_entry_t));

		// find register by the binary search
		tb_size_t itor = tb_binary_find_all_if(iterator, vm86_parser_comp_register, name);
		tb_check_break(itor != tb_iterator_tail(iterator));

		// get the register
		vm86_register_entry_ref_t entry = (vm86_register_entry_ref_t)tb_iterator_item(iterator, itor);
		tb_assert_and_check_break(entry && (entry->index & VM86_REGISTER_MASK) < VM86_REGISTER_MAXN);

		// save register
		*r = entry->index;

		// trace
		tb_trace_d("register: %s: %x", name, entry->index);

		// ok 
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
tb_bool_t vm86_parser_get_value(tb_char_t const** pp, tb_char_t const* e, tb_value_t* value, tb_hash_map_ref_t proc_locals, tb_hash_map_ref_t proc_labels, vm86_data_ref_t data, tb_bool_t* is_str)
{
	// check
	tb_assert(pp && e && value && proc_labels);

	*is_str = tb_false;

	// try to get the number value first
	if (vm86_parser_get_number_value(pp, e, &value->u32)) return tb_true;

	// try to get the local value again
	if (vm86_parser_get_local_value(pp, e, &value->u32, proc_locals)) return tb_true;

	// try to get the offset value last
	if (vm86_parser_get_offset_value(pp, e, &value->u32, proc_labels, data)) return tb_true;

	// get func
	tb_char_t func[256] = { 0 };
	//try to get fun name
	const tb_bool_t  ret = vm86_parser_get_variable_name(pp, e, func, sizeof func);
	if (ret && is_str != 0)
	{
		*is_str = tb_true;
		value->str = tb_strdup(func);
	}

	return ret;
}
tb_bool_t vm86_parser_get_number_value(tb_char_t const** pp, tb_char_t const* e, tb_uint32_t* value)
{
	// check
	tb_assert(pp && e && value);

	// done
	tb_bool_t           ok = tb_false;
	tb_char_t const*    p = *pp;
	do
	{
		// check
		tb_check_break(p < e && (tb_isdigit16(*p) || *p == '-'));

		// is sign?
		tb_bool_t is_sign = tb_false;
		if (*p == '-')
		{
			is_sign = tb_true;
			p++;
		}

		// save the base
		tb_char_t const* b = p;

		// skip the value
		while (p < e && tb_isdigit16(*p)) p++;

		// is hex?
		tb_bool_t is_hex = tb_false;
		if (p < e && *p == 'h')
		{
			is_hex = tb_true;
			p++;
		}

		// check end, exclude variable name
		tb_check_break(!tb_isalpha(*p) && *p != '_');

		// save value
		*value = is_hex ? tb_s16tou32(b) : tb_s10tou32(b);
		if (is_sign) *value = (tb_uint32_t)(((tb_sint32_t)*value) * -1);

		// skip the space
		while (p < e && tb_isspace(*p)) p++;

		// ok
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
tb_bool_t vm86_parser_get_local_value(tb_char_t const** pp, tb_char_t const* e, tb_uint32_t* value, tb_hash_map_ref_t proc_locals)
{
	// check
	tb_assert(pp && e && value && proc_locals);

	// done
	tb_bool_t           ok = tb_false;
	tb_char_t const*    p = *pp;
	do
	{
		// get instruction name
		tb_char_t name[256] = { 0 };
		if (!vm86_parser_get_variable_name(&p, e, name, sizeof(name))) break;

		// get value
		if (tb_hash_map_find(proc_locals, name) != tb_iterator_tail(proc_locals))
			*value = (tb_uint32_t)tb_hash_map_get(proc_locals, name);
		else break;

		// trace
		tb_trace_d("local: %s: %u", name, *value);

		// ok
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
//tb_bool_t vm86_parser_get_offset_value_(tb_char_t const** pp, tb_char_t const* e, tb_uint32_t* value, vm86_machine_ref_t machine)
//{
//	vm86_text_ref_t text = vm86_machine_text(machine);
//	return vm86_parser_get_offset_value(pp, e, value, vm86_machine_data(machine));
//}
tb_bool_t vm86_parser_get_offset_value(tb_char_t const** pp, tb_char_t const* e, tb_uint32_t* value, tb_hash_map_ref_t proc_labels, vm86_data_ref_t data)
{
	// check
	tb_assert(pp && e && value && proc_labels);

	// done
	tb_bool_t           ok = tb_false;
	tb_char_t const*    p = *pp;
	do
	{
		if (p + 6 < e && !tb_strnicmp(p, "offset", 6))
		{
			// seek to the name
			p += 7;
		}

		// attempt to get segment name
		tb_char_t segment[16] = { 0 };
		tb_bool_t has_segment = vm86_parser_get_segment_name(&p, e, segment, sizeof(segment));

		// skip "short ..."
		if (p + 6 < e && !tb_strnicmp(p, "short ", 6)) p += 6;

		// skip the space
		while (p < e && tb_isspace(*p)) p++;

		// get variable name
		tb_char_t name[256] = { 0 };
		if (!vm86_parser_get_variable_name(&p, e, name, sizeof(name))) break;

		// is .data segment?
		if (has_segment && !tb_stricmp(segment, "ds"))
			*value = vm86_data_get(data, name, tb_null);
		// is .code segment?
		else if (has_segment && !tb_stricmp(segment, "cs"))
			*value = (tb_uint32_t)tb_hash_map_get(proc_labels, name);
		else
		{
			// get value
			if (tb_hash_map_find(proc_labels, name) != tb_iterator_tail(proc_labels))
				*value = (tb_uint32_t)tb_hash_map_get(proc_labels, name);
			else if (vm86_data_is(data, name))
				*value = vm86_data_get(data, name, tb_null);
			else break;
		}

		// check
		tb_assert(*value < TB_MAXU32);

		// trace
		tb_trace_d("offset: %s: %x", name, *value);

		// ok
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
tb_bool_t vm86_parser_get_op(tb_char_t const** pp, tb_char_t const* e, tb_char_t* p_op)
{
	// check
	tb_assert(pp && e && p_op);

	// done
	tb_bool_t           ok = tb_false;
	tb_char_t const*    p = *pp;
	do
	{
		const tb_char_t op = *p;
		if (op != '+' && op != '*' && op != '-')
			break;

		p++;
		*p_op = op;
		// trace
		tb_trace_d("op : %c", op);

		// ok
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
tb_bool_t vm86_parser_get_segment_type(tb_char_t const** pp, tb_char_t const* e, tb_char_t* p_segment)
{
	// check
	tb_assert(pp && e && p_segment);

	// done
	tb_bool_t  ok = tb_false;
	tb_char_t const* p = *pp;
	tb_char_t  type = 0;
	do
	{
		//check flag
		if (tb_strncmp(p, "ds:", 3) == 0)
		{
			p += 3;//strlen("byte ptr ")== 9 
			type = 'd';
		}
		else break;

		*p_segment = type;
		// trace
		tb_trace_d("segment : %c", type);

		// ok
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
tb_bool_t vm86_parser_get_ptr_type(tb_char_t const** pp, tb_char_t const* e, tb_uint8_t* p_type)
{
	// check
	tb_assert(pp && e && p_type);

	// done
	tb_bool_t  ok = tb_false;
	tb_char_t const* p = *pp;
	tb_uint8_t  type = 0;
	do
	{
		//check flag
		if (tb_strncmp(p, "byte ptr ", 9) == 0)
		{
			p += 9;//strlen("byte ptr ")== 9 
			type = 3;
		}
		else if (tb_strncmp(p, "word ptr ", 9) == 0)
		{
			p += 9;//strlen("word ptr ")== 9 
			type = 2;
		}
		else if (tb_strncmp(p, "dword ptr ", 10) == 0)
		{
			p += 10;//strlen("word ptr ")== 10
			type = 0;
		}
		else if (tb_strncmp(p, "xmmword ptr ", 10) == 0)
		{
			p += 12;//strlen("word ptr ")== 10
			type = 8;
		}
		else break;

		*p_type = type;
		// trace
		tb_trace_d("ptr_type : %c", type);

		// ok
		ok = tb_true;

	} while (0);

	// update the code pointer if ok
	if (ok) *pp = p;

	// ok?
	return ok;
}
tb_bool_t vm86_parser_get_token(tb_char_t const** pp, tb_char_t const* e, tb_char_t* token, tb_size_t maxn)
{
	// check
	tb_assert_and_check_return_val(pp && e && token && maxn, tb_false);

	// done
	tb_bool_t ok = tb_false;
	tb_char_t const*  p = *pp;

	do
	{
		// skip the space
		while (p < e && tb_isspace(*p)) p++;
		tb_check_break(p < e);

		// save base
		tb_char_t const* b = p;

		// get name
		while (p < e &&  *p != ',') p++;
		tb_memcpy(token, b, p - b);
		tb_check_break(p <= e && p - b < maxn);

		// end
		token[p - b] = '\0';

		//skip ,
		if (p < e) p++;

		// ok 
		ok = tb_true;
	} while (0);

	if (ok) *pp = p;

	// ok?
	return ok;
}
