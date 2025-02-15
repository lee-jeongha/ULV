#ifndef _LIBLKL_H_
#define _LIBLKL_H_

#define AT_FDCWD	-100

extern int mount_fs(const char *path_host, void **pfs);
extern int umount_fs(void *fs);
extern char *get_path(void *fs, const char *path);
extern void free_path(char *path);
extern void setup_network(int addr_my, int addr_gw);

long lkl_syscall(long, long *);

struct utsname {
	char sysname[9];
	char nodename[24];
	char release[32];
	char version[64];
	char machine[64];
	char dummy[1024];
};

struct timeval {
	unsigned long long	tv_sec;
	unsigned long long	tv_uec;
};

static __inline__ long lkl_sys_unlink(const char *path) { long lkl_params[6] = { (long)path, (long)0 }; return lkl_syscall(87, lkl_params); }
static __inline__ long lkl_sys_open(const char *path, int flags) { long lkl_params[6] = { (long)AT_FDCWD, (long)path, (long)flags }; return lkl_syscall(2, lkl_params); }
static __inline__ long lkl_sys_creat(const char *path, int mode) { long lkl_params[6] = { (long)AT_FDCWD, (long)path, (long)0102, (long)mode }; return lkl_syscall(85, lkl_params); }
static __inline__ long lkl_sys_close(int fd) { long lkl_params[6] = { (long)fd }; return lkl_syscall(3, lkl_params); }
static __inline__ long lkl_sys_uname(struct utsname *name) { long lkl_params[6] = { (long)name }; return lkl_syscall(63, lkl_params); }
static __inline__ long lkl_sys_umask(int mask) { long lkl_params[6] = { (long)mask }; return lkl_syscall(95, lkl_params); }
static __inline__ long lkl_sys_gettimeofday(struct timeval *tv) { long lkl_params[6] = { (long)tv, (long)0 }; return lkl_syscall(96, lkl_params); }

void free(void *ptr);
int strlen(const char *str);
int snprintf(char *str, size_t size, const char *format, ...);

#endif
