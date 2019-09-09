#ifndef VM86_SEGMENT_H
#define VM86_SEGMENT_H

 /* //////////////////////////////////////////////////////////////////////////////////////
  * extern
  */
__tb_extern_c_enter__

vm86_segment_info_ref_t vm86_segment_info_init();
void vm86_segment_info_release(vm86_segment_info_ref_t segment_info);
vm86_segment_ref_t vm86_segment_get_data(vm86_segment_info_ref_t segment_info);
vm86_segment_ref_t vm86_segment_get_code(vm86_segment_info_ref_t segment_info);
tb_uint32_t vm86_segment_size(vm86_segment_ref_t segment);
const tb_char_t* vm86_segment_line(vm86_segment_ref_t segment, const tb_uint32_t n);
tb_bool_t vm86_segment_complie(vm86_machine_ref_t machine, vm86_segment_info_ref_t segment_info, tb_char_t const* code, tb_size_t code_size);

/* //////////////////////////////////////////////////////////////////////////////////////
 * extern
 */
__tb_extern_c_leave__

#endif