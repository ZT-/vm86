/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME            "machine_data"
#define TB_TRACE_MODULE_DEBUG           (0)

 /* //////////////////////////////////////////////////////////////////////////////////////
  * includes
  */
#include "machine.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * types
 */

 // the machine data chunk type
typedef struct __vm86_data_chunk_t
{
	// the offset
	tb_uint32_t                     offset;

	// the size
	tb_uint32_t                     size;

}vm86_data_chunk_t, *vm86_data_chunk_ref_t;

// the machine data data type
typedef struct __vm86_data_t
{
	// the data
	tb_byte_t*                      data;

	// the size
	tb_uint32_t                     size;

	// the base
	tb_uint32_t                     base;

	// the labels
	tb_hash_map_ref_t               labels;

}vm86_data_t;

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
vm86_data_ref_t vm86_data_init(tb_size_t size)
{
	// check
	tb_assert_and_check_return_val(size, tb_null);

	// done
	tb_bool_t       ok = tb_false;
	vm86_data_t*    data = tb_null;
	do
	{
		// make data
		data = tb_malloc0_type(vm86_data_t);
		tb_assert_and_check_break(data);

		// make data
		data->data = tb_malloc0_bytes(size);
		tb_assert_and_check_break(data->data);

		// init labels
		data->labels = tb_hash_map_init(8, tb_element_str(tb_true), tb_element_mem(sizeof(vm86_data_chunk_t), tb_null, tb_null));
		tb_assert_and_check_break(data->labels);

		// init data
		data->size = size;
		data->base = 0;

		// ok
		ok = tb_true;

	} while (0);

	// failed?
	if (!ok)
	{
		// exit it
		if (data) vm86_data_exit((vm86_data_ref_t)data);
		data = tb_null;
	}

	// ok?
	return (vm86_data_ref_t)data;
}
tb_void_t vm86_data_exit(vm86_data_ref_t self)
{
	// check
	vm86_data_t* data = (vm86_data_t*)self;
	tb_assert_and_check_return(data);

	// exit data
	if (data->data) tb_free(data->data);
	data->data = tb_null;

	// exit labels
	if (data->labels) tb_hash_map_exit(data->labels);
	data->labels = tb_null;

	// exit it
	tb_free(data);
}
tb_bool_t vm86_data_is(vm86_data_ref_t self, tb_char_t const* name)
{
	// check
	vm86_data_t* data = (vm86_data_t*)self;
	tb_assert_and_check_return_val(data && data->labels && name, tb_false);

	// is data?
	return tb_hash_map_find(data->labels, name) != tb_iterator_tail(data->labels);
}
tb_uint32_t vm86_data_get(vm86_data_ref_t self, tb_char_t const* name, tb_size_t* psize)
{
	// check
	vm86_data_t* data = (vm86_data_t*)self;
	tb_assert_and_check_return_val(data && data->labels && name, 0);

	// get the data chunk
	vm86_data_chunk_ref_t chunk = (vm86_data_chunk_ref_t)tb_hash_map_get(data->labels, name);
	tb_assert(chunk && chunk->offset < data->size);

	// save size
	if (psize) *psize = chunk->size;

	// the data offset
	return tb_p2u32(data->data + chunk->offset);
}
tb_void_t vm86_data_add(vm86_data_ref_t self, tb_char_t const* name, tb_byte_t const* buff, tb_size_t size)
{
	// check
	vm86_data_t* data = (vm86_data_t*)self;
	tb_assert_and_check_return(data && data->labels && name);

	// get the data chunk
	vm86_data_chunk_ref_t chunk = (vm86_data_chunk_ref_t)tb_hash_map_get(data->labels, name);
	if (chunk)
	{
		// must be the last chunk
		tb_assert(chunk->offset + chunk->size == data->base);

		// check
		tb_assert(data->base + size <= data->size);

		// add data
		if (buff && size) tb_memcpy(data->data + data->base, buff, size);

		// update size
		chunk->size += size;

		// update base
		data->base += size;
	}
	else
	{
		// check
		tb_assert(data->base + size <= data->size);

		// init chunk
		vm86_data_chunk_t item = { data->base, size };

		// add chunk
		tb_hash_map_insert(data->labels, name, &item);

		// add data
		if (buff && size) tb_memcpy(data->data + data->base, buff, size);

		// update base
		data->base += size;
	}
}
tb_char_t const* vm86_data_read(vm86_machine_ref_t machine, tb_char_t const* p, tb_char_t const* e, tb_byte_t* data, tb_size_t* size)
{
	// check
	tb_assert_and_check_return_val(machine && p && e && data && size, tb_null);

	// done
	tb_bool_t   ok = tb_false;
	tb_byte_t*  qb = data;
	tb_byte_t*  qe = data + *size;
	do
	{
		// skip the space
		while (p < e && tb_isspace(*p)) p++;

		// save base
		tb_char_t const* b = p;

		// init type
		tb_char_t type[16] = { 0 };
		while (p < e && !tb_isspace(*p)) p++;
		tb_check_break(p < e && p - b < sizeof(type));
		tb_memcpy(type, b, p - b);

		// trace
		tb_trace_d("type: %s", type);

		// byte?
		if (!tb_stricmp(type, "db"))
		{
			// done
			tb_char_t ch;
			tb_bool_t in_str = tb_false;
			while (p < e && qb < qe)
			{
				// the character
				ch = *p;

				// in string?
				if (!in_str)
				{
					// space or ,? skip it
					if (tb_isspace(ch) || ch == ',') p++;
					// string?
					else if (ch == '\"' || ch == '\'')
					{
						// enter string
						in_str = tb_true;
						p++;
					}
					// value?
					else if (tb_isdigit16(ch))
					{
						// get value
						tb_uint32_t value = 0;
						if (!vm86_parser_get_number_value(&p, e, &value)) break;
						tb_assert(value <= TB_MAXU8);

						// append data
						*qb++ = (tb_byte_t)value;
					}
					// comment? end it
					else if (ch == ';') p = e;
					// invalid character
					else
					{
						// trace
						tb_trace_e("invalid character: %c", ch);
						tb_assert(0);
					}
				}
				else
				{
					// leave?
					if (ch == '\"' || ch == '\'')
					{
						// leave string
						in_str = tb_false;
						p++;
					}
					// append data
					else *qb++ = *p++;
				}
			}
		}
		// dword?
		else if (!tb_stricmp(type, "dd"))
		{
			// done
			tb_char_t ch;
			while (p < e && qb < qe)
			{
				// the character
				ch = *p;

				// space or ,? skip it
				if (tb_isspace(ch) || ch == ',') p++;
				// offset?
				else if (p + 6 < e && !tb_strnicmp(p, "offset", 6))
				{
					// seek to the name
					p += 6;
					while (p < e && tb_isspace(*p)) p++;

					//TODO:
					tb_assert(0);
					// get the offset value
					//tb_uint32_t value = 0;
					//if (!vm86_parser_get_offset_value(&p, e, &value, proc->labels, vm86_machine_data(machine))) break;

					// append data
					//tb_bits_set_u32_ne(qb, value);
					qb += 4;
				}
				// value?
				else if (tb_isdigit16(ch))
				{
					// get value
					tb_uint32_t value = 0;
					if (!vm86_parser_get_number_value(&p, e, &value)) break;
					tb_assert(value <= TB_MAXU32);

					// append data
					tb_bits_set_u32_ne(qb, value);
					qb += 4;
				}
				// comment? end it
				else if (ch == ';') p = e;
				// invalid character
				else
				{
					// trace
					tb_trace_e("invalid character: %c", ch);
					tb_assert(0);
				}
			}
		}
		// word?
		else if (!tb_stricmp(type, "dw"))
		{
			// TODO
			tb_assert(0);
		}
		else
		{
			// TODO
			tb_assert(0);
		}

		// ok
		ok = tb_true;

	} while (0);

	// ok?
	if (ok)
	{
		// save size
		*size = qb - data;
	}
	else p = tb_null;

	// ok?
	return p;
}
tb_bool_t vm86_data_complie(vm86_machine_ref_t machine, vm86_segment_ref_t segment)
{
	// check
	tb_assert_and_check_return_val(machine && segment, tb_false);

	// the data
	vm86_data_ref_t data = vm86_machine_data(machine);
	tb_assert_and_check_return_val(data, tb_false);

	// done
	tb_bool_t ok = tb_false;

	tb_char_t last_data_name[8192] = { 0 };
	const tb_uint32_t size = vm86_segment_size(segment);
	for (tb_size_t i = 0; i < size; ++i)
	{
		// save base
		const tb_char_t* p = vm86_segment_line(segment, i);
		const tb_char_t* b = p;
		const tb_char_t* e = p + tb_strlen(p);

		// init name
		tb_char_t name[8192] = { 0 };
		while (p < e && !tb_isspace(*p)) p++;
		tb_check_break(p < e && p - b < sizeof(name));
		tb_memcpy(name, b, p - b);

		// only data? restore the pointer offset
		tb_bool_t only_data = tb_false;
		if (!tb_stricmp(name, "dd") || !tb_stricmp(name, "dw") || !tb_stricmp(name, "db"))
		{
			p -= 2;
			only_data = tb_true;
		}

		// done
		tb_byte_t  buff[8192];
		tb_size_t   maxn = sizeof(buff);
		tb_byte_t*  base = buff;
		while (p < e)
		{
			// read data
			tb_size_t read = maxn;
			p = vm86_data_read(machine, p, e, base, &read);
			tb_assert(p && read);

			// update the buffer
			maxn -= read;
			base += read;
		}

		// check
		tb_assert(base > buff);

		// only data? append to the last data
		if (only_data)
		{
			// add data
			vm86_data_add(data, last_data_name, buff, base - buff);
		}
		// exists name? add new data
		else
		{
			// add data
			vm86_data_add(data, name, buff, base - buff);

			// save the last data name
			tb_strlcpy(last_data_name, name, sizeof(last_data_name));
		}

		// ok
		ok = tb_true;

	} while (0);

	// ok?
	return ok;
}

#ifdef __vm_debug__
tb_void_t vm86_data_dump(vm86_data_ref_t self)
{
	// check
	vm86_data_t* data = (vm86_data_t*)self;
	tb_assert_and_check_return(data && data->labels);

	// trace
	tb_trace_i("");
	tb_trace_i(".data");

	// done
	tb_for_all_if(tb_hash_map_item_t*, item, data->labels, item && item->data)
	{
		// the chunk
		vm86_data_chunk_ref_t chunk = (vm86_data_chunk_ref_t)item->data;

		// dump data
		tb_dump_data(data->data + chunk->offset, chunk->size);

		// trace
		tb_trace_i("data: name: %s, offset: %lu, size: %lu", item->name, chunk->offset, chunk->size);
	}
}
#endif
