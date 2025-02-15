#include <linux/binfmts.h>
#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/personality.h>
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/start_kernel.h>
#include <linux/syscalls.h>
#include <linux/tick.h>
#include <asm/host_ops.h>
#include <asm/lkl_dbg.h>
#include <asm/irq.h>
#include <asm/unistd.h>
#include <asm/syscalls.h>

struct lkl_host_operations *lkl_ops;
static char cmd_line[COMMAND_LINE_SIZE];
static int is_running;
void (*pm_power_off)(void) = NULL;

long lkl_panic_blink(int state)
{
	lkl_ops->panic();
	return 0;
}

void __init setup_arch(char **cl)
{
	*cl = cmd_line;
	panic_blink = lkl_panic_blink;
	parse_early_param();
	bootmem_init();
}

static void *__init lkl_run_kernel(void *arg)
{
	threads_init();
	start_kernel();
	return NULL;
}

int __init lkl_start_kernel(struct lkl_host_operations *ops, void *mem_start, unsigned long mem_size)
{
	lkl_thread_t	thread;

	lkl_ops = ops;

	memory_start = (unsigned long)mem_start;
	memory_end = (unsigned long)((char *)mem_start + mem_size);

	if (ops->virtio_devices)
		strscpy(boot_command_line, ops->virtio_devices,
			COMMAND_LINE_SIZE);

	memcpy(cmd_line, boot_command_line, COMMAND_LINE_SIZE);

	thread = lkl_ops->thread_create(lkl_run_kernel, NULL);
	if (!thread)
		return -ENOMEM;

	handover_thread(thread);

	while (!is_running)
		schedule();

	return 0;
}

int lkl_is_running(void)
{
	return is_running;
}

void machine_halt(void)
{
	/* TODO: */
}

void machine_power_off(void)
{
	machine_halt();
}

void machine_restart(char *unused)
{
	machine_halt();
}

static int lkl_run_init(struct linux_binprm *bprm);

static struct linux_binfmt lkl_run_init_binfmt = {
	.module		= THIS_MODULE,
	.load_binary	= lkl_run_init,
};

static int lkl_run_init(struct linux_binprm *bprm)
{
	int	ret;

	if (strcmp("/init", bprm->filename) != 0)
		return -EINVAL;

	ret = flush_old_exec(bprm);
	if (ret)
		return ret;
	set_personality(PER_LINUX);
	setup_new_exec(bprm);
	install_exec_creds(bprm);

	set_binfmt(&lkl_run_init_binfmt);

	init_pid_ns.child_reaper = 0;

	current->mm->start_brk = current->mm->brk = memory_end;

	{
		extern void wakeup_user_thread(void);
		wakeup_user_thread();
	}

	is_running = 1;

	return 0;
}


/* skip mounting the "real" rootfs. ramfs is good enough. */
static int __init fs_setup(void)
{
	int fd;

	fd = sys_open("/init", O_CREAT, 0700);
	WARN_ON(fd < 0);
	sys_close(fd);

	register_binfmt(&lkl_run_init_binfmt);

	return 0;
}
late_initcall(fs_setup);
