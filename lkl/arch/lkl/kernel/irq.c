#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/hardirq.h>
#include <asm/irq_regs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/tick.h>
#include <asm/irqflags.h>
#include <asm/host_ops.h>

/*
 * To avoid much overhead we use an indirect approach: the irqs are marked using
 * a bitmap (array of longs) and a summary of the modified bits is kept in a
 * separate "index" long - one bit for each sizeof(long). Thus we can support
 * 4096 irqs on 64bit platforms and 1024 irqs on 32bit platforms.
 *
 * Whenever an irq is trigger both the array and the index is updated. To find
 * which irqs were triggered we first search the index and then the
 * corresponding part of the arrary.
 */
static unsigned long irq_status;

static inline unsigned long test_and_clear_irq_status(void)
{
	return __sync_fetch_and_and(&irq_status, 0);
}

void set_irq_pending(int irq)
{
	__sync_fetch_and_or(&irq_status, BIT(irq));
}

static struct irq_info {
	const char *user;
} irqs[NR_IRQS];

static bool irqs_enabled;

static struct pt_regs dummy;

static void run_irq(int irq)
{
	unsigned long flags;
	struct pt_regs *old_regs = set_irq_regs((struct pt_regs *)&dummy);

	/* interrupt handlers need to run with interrupts disabled */
	local_irq_save(flags);
	irq_enter();
	generic_handle_irq(irq);
	irq_exit();
	set_irq_regs(old_regs);
	local_irq_restore(flags);
}

/**
 * This function can be called from arbitrary host threads, so do not
 * issue any Linux calls (e.g. prink) if lkl_cpu_get() was not issued
 * before.
 */
int lkl_trigger_irq(int irq)
{
	if (!irq || irq > NR_IRQS)
		return -EINVAL;

	set_irq_pending(irq);
	return 0;
}

static inline void for_each_bit(unsigned long word, void (*f)(int, int), int j)
{
	int i = 0;

	while (word) {
		if (word & 1)
			f(i, j);
		word >>= 1;
		i++;
	}
}

static inline void deliver_irq(int bit, int unused)
{
	run_irq(bit);
}

void run_irqs(void)
{
	for_each_bit(test_and_clear_irq_status(), deliver_irq, 0);
}

int show_interrupts(struct seq_file *p, void *v)
{
	return 0;
}

int lkl_get_free_irq(const char *user)
{
	int i;
	int ret = -EBUSY;

	/* 0 is not a valid IRQ */
	for (i = 1; i < NR_IRQS; i++) {
		if (!irqs[i].user) {
			irqs[i].user = user;
			irq_set_chip_and_handler(i, &dummy_irq_chip, handle_simple_irq);
			ret = i;
			break;
		}
	}

	return ret;
}

void lkl_put_irq(int i, const char *user)
{
	if (!irqs[i].user || strcmp(irqs[i].user, user) != 0) {
		WARN("%s tried to release %s's irq %d", user, irqs[i].user, i);
		return;
	}

	irqs[i].user = NULL;
}

unsigned long arch_local_save_flags(void)
{
	return irqs_enabled;
}

void arch_local_irq_restore(unsigned long flags)
{
	if (flags == ARCH_IRQ_ENABLED && irqs_enabled == ARCH_IRQ_DISABLED &&
	    !in_interrupt())
		run_irqs();
	irqs_enabled = flags;
}

void init_IRQ(void)
{
	int i;

	for (i = 0; i < NR_IRQS; i++)
		irq_set_chip_and_handler(i, &dummy_irq_chip, handle_simple_irq);

	pr_info("lkl: irqs initialized\n");
}

void cpu_yield_to_irqs(void)
{
	cpu_relax();
}
