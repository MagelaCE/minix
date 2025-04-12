/* This file contains some dumping routines for debugging. */

#include "../h/const.h"
#include "../h/type.h"
#include "../h/callnr.h"
#include "../h/com.h"
#include "../h/error.h"
#include "const.h"
#include "type.h"
#include "glo.h"
#include "proc.h"

#define NSIZE 20
phys_bytes aout[NR_PROCS];	/* pointers to the program names */
char nbuff[NSIZE+1];
int vargv;

extern struct tasktab tasktab[];

/*===========================================================================*
 *				DEBUG routines here			     * 
 *===========================================================================*/
PUBLIC p_dmp()
{
/* Proc table dump */

  register struct proc *rp;
  char *np;
  phys_clicks base, size;
  phys_bytes dst;
  int index;
  extern phys_bytes umap();

  printf(
  "\r\nproc  --pid -pc- -sp flag -user- --sys-- -base- -size-  recv- command\r\n");

  dst = umap(cproc_addr(SYSTASK), D, (vir_bytes)nbuff, NSIZE);

  for (rp = BEG_PROC_ADDR; rp < END_PROC_ADDR; rp++)  {
	if (rp->p_flags & P_SLOT_FREE) continue;
	base = rp->p_map[T].mem_phys;
	size = rp->p_map[S].mem_phys + rp->p_map[S].mem_len - base;
	prname(proc_number(rp));
	printf("%5u %4lx %4lx %2x %7U %7U %5uK %5uK  ",
		rp->p_pid,
		(unsigned long) rp->p_reg.r16.pc,
		(unsigned long) rp->p_reg.r16.sp,
		rp->p_flags,
		rp->user_time, rp->sys_time,
		click_to_round_k(base), click_to_round_k(size));
		if (rp->p_flags == 0)
			printf("      ");
		else
			prname(rp->p_getfrom);

	/* Fetch the command string from the user process. */
	index = proc_number(rp);
	if (index > LOW_USER && aout[index] != 0) {
		phys_copy(aout[index], dst, (long) NSIZE);
		for (np = &nbuff[0]; np < &nbuff[NSIZE]; np++)
			if (*np <= ' ' || *np >= 0177) *np = 0;
		if (index == LOW_USER + 1)
			printf("/bin/sh");
		else
			printf("%s", nbuff);
	}
	printf("\r\n");
  }
  printf("\r\n");
}



PUBLIC map_dmp()
{
  register struct proc *rp;
  phys_clicks base, size;

  printf("\r\nPROC   -----TEXT-----  -----DATA-----  ----STACK-----  -BASE- -SIZE-\r\n");
  for (rp = BEG_SERV_ADDR; rp < END_PROC_ADDR; rp++)  {
	if (rp->p_flags & P_SLOT_FREE) continue;
	base = rp->p_map[T].mem_phys;
	size = rp->p_map[S].mem_phys + rp->p_map[S].mem_len - base;
	prname(proc_number(rp));
	printf(" %4x %4x %4x  %4x %4x %4x  %4x %4x %4x  %5uK %5uK\r\n", 
	    rp->p_map[T].mem_vir, rp->p_map[T].mem_phys, rp->p_map[T].mem_len,
	    rp->p_map[D].mem_vir, rp->p_map[D].mem_phys, rp->p_map[D].mem_len,
	    rp->p_map[S].mem_vir, rp->p_map[S].mem_phys, rp->p_map[S].mem_len,
	    click_to_round_k(base), click_to_round_k(size));
  }
}


PRIVATE prname(i)
int i;
{
  if (i == ANY)
	printf("ANY   ");
  else if ( (unsigned) (i + NR_TASKS) <= LOW_USER + NR_TASKS)
	printf("%s", tasktab[i + NR_TASKS].name);
  else
	printf("%4d  ", i);
}

PUBLIC set_name(proc_nr, ptr)
int proc_nr;
char *ptr;
{
/* When an EXEC call is done, the kernel is told about the stack pointer.
 * It uses the stack pointer to find the command line, for dumping
 * purposes.
 */

  extern phys_bytes umap();
  phys_bytes src, dst;

  if (ptr == (char *) 0) {
	aout[proc_nr] = (phys_bytes) 0;
	return;
  }

  src = umap(proc_addr(proc_nr), D, (vir_bytes)(ptr + 2), 2);
  if (src == 0) return;
  dst = umap(proc_addr(SYSTASK), D, (vir_bytes)&vargv, 2);
  phys_copy(src, dst, 2L);

  aout[proc_nr] = umap(proc_addr(proc_nr), D, (vir_bytes)vargv, NSIZE);
}
