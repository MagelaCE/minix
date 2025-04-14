/* EXTERN should be extern except for the table file */
#ifdef _TABLE
#undef EXTERN
#define EXTERN
#endif


/* File System global variables */
EXTERN struct fproc *fp;	/* pointer to caller's fproc struct */
EXTERN int super_user;		/* 1 if caller is super_user, else 0 */
EXTERN int dont_reply;		/* normally 0; set to 1 to inhibit reply */
EXTERN int susp_count;		/* number of procs suspended on pipe */
EXTERN int reviving;		/* number of pipe processes to be revived */
EXTERN off_t rdahedpos;		/* position to read ahead */
EXTERN struct inode *rdahed_inode;	/* pointer to inode to read ahead */
EXTERN char fstack[FS_STACK_BYTES];	/* the File System's stack. */

/* The parameters of the call are kept here. */
EXTERN message m;		/* the input message itself */
EXTERN message m1;		/* the output message used for reply */
EXTERN int who;			/* caller's proc number */
EXTERN int fs_call;		/* system call number */
EXTERN char user_path[PATH_MAX];/* storage for user path name */

/* The following variables are used for returning results to the caller. */
EXTERN int err_code;		/* temporary storage for error number */
EXTERN int rdwt_err;		/* status of last disk i/o request */

/* Data which needs initialization. */
typedef unsigned short u16_t;	/* belongs in h/type.h */
extern u16_t data_org[INFO + 2];	/* origin and sizes of init */
				/* belongs in h/build.h */
extern int (*call_vector[])();	/* table of system calls handled by FS */
extern max_major;		/* maximum major device (+ 1) */

/* Functions. */

/* cache.c */
zone_nr alloc_zone();
void flushall();
void free_zone();
struct buf *get_block();
void invalidate();
void put_block();
void rw_block();
void rw_scattered();

/* device.c */
void dev_close();
int dev_io();
int do_ioctl();
int dev_open();
void no_call();
void rw_dev();
void rw_dev2();
int tty_exit();
void tty_open();

/* filedes.c */
struct filp *find_filp();
int get_fd();
struct filp *get_filp();

/* inode.c */
struct inode *alloc_inode();
void dup_inode();
void free_inode();
struct inode *get_inode();
void put_inode();
void rw_inode();
void wipe_inode();

/* link.c */
int do_link();
int do_unlink();
void truncate();

/* main.c */
void main();
void reply();

/* misc.c */
int do_dup();
int do_exit();
int do_fcntl();
int do_fork();
int do_revive();
int do_set();
int do_sync();

/* mount.c */
int do_mount();
int do_umount();

/* open.c */
int do_close();
int do_creat();
int do_lseek();
int do_mknod();
int do_open();

/* path.c */
struct inode *advance();
int search_dir();
struct inode *eat_path();
struct inode *last_dir();

/* pipe.c */
int do_pipe();
int do_unpause();
int pipe_check();
void release();
void revive();
void suspend();

/* protect.c */
int do_access();
int do_chmod();
int do_chown();
int do_umask();
int forbidden();
int read_only();

/* putc.c */
void putc();

/* read.c */
int do_read();
struct buf *rahead();
void read_ahead();
block_nr read_map();
int read_write();
int rw_user();

/* stadir.c */
int do_chdir();
int do_chroot();
int do_fstat();
int do_stat();

/* super.c */
bit_nr alloc_bit();
void free_bit();
struct super_block *get_super();
int load_bit_maps();
int mounted();
void rw_super();
int scale_factor();
int unload_bit_maps();

/* time.c */
int do_stime();
int do_time();
int do_tims();
int do_utime();

/* utility.c */
time_t clock_time();
int cmp_string();
void copy();
int fetch_name();
int no_sys();
void panic();

/* write.c */
void clear_zone();
int do_write();
struct buf *new_block();
void zero_block();

/* library */
void printk();
int receive();
int send();
int sendrec();
void sys_abort();
void sys_copy();
void sys_kill();
void sys_times();
