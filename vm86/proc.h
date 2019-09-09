#ifndef VM86_PROC_H
#define VM86_PROC_H

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

/*! init proc
 *
 * @param machine          the machine
 * @param code              the proc code
 * @param size              the proc size
 *
 * @return                  the proc
 */
vm86_proc_ref_t      vm86_proc_init(vm86_machine_ref_t machine, tb_char_t const* code, tb_size_t size);

/*! exit proc 
 *
 * @param proc              the proc
 */
tb_void_t                   vm86_proc_exit(vm86_proc_ref_t proc);

/*! the proc name
 *
 * @param proc              the proc
 *
 * @return                  the name
 */
tb_char_t const*            vm86_proc_name(vm86_proc_ref_t proc);

/*! done proc
 *
 * @param name              the proc name
 */
tb_void_t                   vm86_proc_call(vm86_machine_ref_t machine,const tb_char_t* name);

/*! done proc
 *
 * @param proc              the proc
 */
tb_void_t                   vm86_proc_done(vm86_proc_ref_t proc);

/*! proc complie
 *
 * @param proc              the proc
 */
tb_bool_t vm86_proc_complie(vm86_machine_ref_t machine, vm86_segment_ref_t segment);


tb_uint32_t vm86_proc_oep(vm86_proc_ref_t proc);

/* //////////////////////////////////////////////////////////////////////////////////////
 * extern
 */
__tb_extern_c_leave__

#endif


