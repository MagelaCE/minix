/* Functions, variables and data structures defined in various assembler
 * files. Each file should have a define like "#define KLIB88 .define" to
 * say what it is defining.
 */

	.text

#ifndef DB286
#define DB286 .extern
#endif
#ifndef KLIB88
#define KLIB88 .extern
#endif
#ifndef KLIB286
#define KLIB286 .extern
#endif
#ifndef MPX88
#define MPX88 .extern
#endif
#ifndef MPX286
#define MPX286 .extern
#endif
#ifndef RS2
#define RS2 .extern
#endif

/* Functions defined in db286.x. */

#ifdef i80286
#ifdef DEBUGGER
DB286		_debug2_exception
DB286		_patch_phys_copy
DB286		_unpatch_phys_copy
#endif
#endif

/* Functions defined in klib88.x. */

KLIB88		_ack_char
KLIB88		_build_sig
KLIB88		_check_mem
KLIB88		_cim_at_wini
KLIB88		_cim_printer
KLIB88		_cim_xt_wini
KLIB88		_cp_mess
KLIB88		cret
KLIB88		csv
KLIB88		_exit
KLIB88		.fat
KLIB88		_get_phys_byte
KLIB88		_get_chrome
KLIB88		_get_ega
KLIB88		_get_extmemsize
KLIB88		_get_memsize
KLIB88		_lock
KLIB88		_phys_copy
KLIB88		_porti_out
KLIB88		_port_read
KLIB88		_port_write
KLIB88		_reboot
KLIB88		_restore
KLIB88		_save_tty_vec
KLIB88		_scr_down
KLIB88		_scr_up
KLIB88		_sim_printer
KLIB88		_tasim_printer
KLIB88		_test_and_set
KLIB88		.trp
KLIB88		_unlock
KLIB88		_vid_copy
KLIB88		_wait_retrace
KLIB88		_wreboot

#ifndef DEBUGGER
KLIB88		_codeseg
KLIB88		_dataseg
KLIB88		_get_processor
KLIB88		_inportb
#endif

/* Functions defined in klib286.x. */

#ifdef i80286
KLIB286		_klib286_init		/* this patches the rest */
#endif

/* Functions defined in mpx88.x. */

MPX88		begtext
MPX88		_idle_task
MPX88		kernel_ds
MPX88		_restart
MPX88		save

MPX88		_int00			/* exception handlers */
MPX88		_int01
MPX88		_int02
MPX88		_int03
MPX88		_int04
MPX88		_int05
MPX88		_int06
MPX88		_int07
MPX88		_int08
MPX88		_int09
MPX88		_int10
MPX88		_int11
MPX88		_int12
MPX88		_int13
MPX88		_int14
MPX88		_int15
MPX88		_int16
MPX88		_trp

MPX88		_clock_int		/* interrupt handlers */
MPX88		_tty_int
#ifdef C_RS232_INT_HANDLERS
MPX88		_secondary_int
MPX88		_rs232_int
#ifdef i80286
MPX88		_psecondary_int
MPX88		_prs232_int
#endif
#endif
MPX88		_wini_int
MPX88		_disk_int
MPX88		_lpr_int

MPX88		_s_call			/* trap handlers */

/* Functions defined in mpx286.x. */

#ifdef i80286
MPX286		p2_errexception
MPX286		p2_save
MPX286		p2_resdone
MPX286		p_restart

MPX286		_divide_error		/* exception handlers */
MPX286		_nmi
MPX286		_overflow
MPX286		_bounds_check
MPX286		_inval_opcode
MPX286		_copr_not_available
MPX286		_double_fault
MPX286		_copr_seg_overrun
MPX286		_inval_tss
MPX286		_segment_not_present
MPX286		_stack_exception
MPX286		_general_protection

MPX286		_p2_s_call		/* trap handlers */
#endif /* i80286 */

/* Functions defined in rs2.x. */

#ifndef C_RS232_INT_HANDLERS
#ifdef i80286
RS2		_prs232_int
RS2		_psecondary_int
RS2		rs2_iret
#endif
RS2		_rs232_int
RS2		_secondary_int
#endif

/* C functions. */

.extern		_clock_handler
#ifdef DEBUGGER
.extern		_db_main	/* old real mode debugger */
#endif
.extern		_dp8390_int
.extern		_eth_stp
.extern		_exception
.extern		_init_8259
.extern		_interrupt
.extern		_keyboard
.extern		_main
.extern		_panic
.extern		_pr_char
#ifdef C_RS232_INT_HANDLERS
#ifdef OLD_TTY
.extern		_rs232
#else
.extern		_rs232_1handler
.extern		_rs232_2handler
#endif
#endif
.extern		_scan_keyboard
.extern		_sys_call
#ifndef OLD_TTY
.extern		_tty_wakeup
#endif
.extern		_unhold

		.bss

/* Variables defined in klib88.x. */

KLIB88		splimit
KLIB88		_vec_table

/* Variables defined in mpx88.x. */

MPX88		begbss
MPX88		begdata
MPX88		k_stktop
MPX88		_sizes

/* Variables defined in mpx286.x. */

#ifdef i80286
MPX286		ds_ex_number
MPX286		trap_errno
#endif

/* C variables. */

.extern		_blank_color
.extern		_boot_parameters
.extern		_held_head
.extern		_held_tail
.extern		_k_reenter
.extern		_pc_at
.extern		_port_65
.extern		_proc_ptr
.extern		_processor
.extern		_ps
.extern		_scan_code
.extern		_sizeof_bparam
.extern		_snow
#ifndef C_RS232_INT_HANDLERS
.extern		_rs_lines
.extern		_tty_events
#endif
.extern		_vid_mask
.extern		_vid_port

#ifdef i80286
#ifdef DEBUGGER
.extern		_bits32
.extern		_db_processor
.extern		_db_tss
.extern		_protected
#endif
.extern		_gdt
.extern		_tss
#endif

	.text
