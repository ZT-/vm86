#ifndef VM86_INSTRUCTION_H
#define VM86_INSTRUCTION_H

  /* //////////////////////////////////////////////////////////////////////////////////////
   * extern
   */
__tb_extern_c_enter__


/* //////////////////////////////////////////////////////////////////////////////////////
 * interfaces
 */


vm86_instruction_ref_t vm86_instruction_init(tb_uint32_t n);

/*! release the instruction
 *
 * @param instruction       the instruction
 */
void  vm86_instruction_release(vm86_instruction_ref_t instruction);

 /*! compile the instruction
  *
  * @param instruction       the instruction
  * @param code              the instruction code
  * @param size              the instruction size
  * @param machine           the machine
  * @param proc_labels       the proc labels
  * @param proc_locals       the proc labels
  */
tb_bool_t  vm86_instruction_compile(vm86_instruction_ref_t instruction, tb_char_t const* code, tb_size_t size, vm86_machine_ref_t machine, tb_hash_map_ref_t proc_labels, tb_hash_map_ref_t proc_locals);



tb_bool_t vm86_instruction_done(vm86_machine_ref_t machine, tb_uint32_t eip);

/* //////////////////////////////////////////////////////////////////////////////////////
 * extern
 */
__tb_extern_c_leave__

#endif


