#ifndef VM86_TEXT_H
#define VM86_TEXT_H

 /* //////////////////////////////////////////////////////////////////////////////////////
  * extern
  */
__tb_extern_c_enter__

/* //////////////////////////////////////////////////////////////////////////////////////
 * interfaces
 */

vm86_text_ref_t vm86_text_init();

/*! compile proc
 *
 * @param text              the text
 * @param code              the proc code
 *
 * @return                  the proc
 */
tb_bool_t  vm86_text_compile(vm86_machine_ref_t machine, tb_char_t const* code, tb_size_t size);

tb_hash_map_ref_t vm86_text_labels(vm86_text_ref_t text);

vm86_instruction_ref_t vm86_text_get_instruction(vm86_text_ref_t text, tb_size_t n);

tb_bool_t vm86_text_prepare_label(vm86_text_ref_t  text, tb_char_t const* p, tb_char_t const* e, tb_size_t offset);

/* //////////////////////////////////////////////////////////////////////////////////////
 * extern
 */
__tb_extern_c_leave__

#endif


