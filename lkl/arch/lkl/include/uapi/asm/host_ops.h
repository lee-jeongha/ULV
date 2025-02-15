#ifndef _ASM_UAPI_LKL_HOST_OPS_H
#define _ASM_UAPI_LKL_HOST_OPS_H

/* Defined in {posix,nt}-host.c */
struct lkl_sem;
struct lkl_tls_key;
typedef unsigned long lkl_thread_t;
struct lkl_jmp_buf {
	unsigned long buf[128];
};
struct lkl_pci_dev;

/**
 * lkl_dev_pci_ops - PCI host operations
 *
 * These operations would be a wrapper of userspace PCI drvier and
 * must be provided by a host library or by the application.
 *
 * @add - add a new PCI device; returns a handler or NULL if fails
 * @remove - release resources
 * @init_irq - allocate resources for interrupts
 * @read - read the PCI Configuration Space
 * @write - write the PCI Configuration Space
 * @resource_alloc - map BARx and return the mapped address. x is resource_index
 *
 * @map_page - return the DMA address of pages; vaddr might not be page-aligned
 * @unmap_page - cleanup DMA region if needed
 *
 */
struct lkl_dev_pci_ops {
	struct lkl_pci_dev *(*add)(const char *name, void *kernel_ram,
				   unsigned long ram_size);
	void (*remove)(struct lkl_pci_dev *dev);
	int (*irq_init)(struct lkl_pci_dev *dev, int irq);
	int (*read)(struct lkl_pci_dev *dev, int where, int size, void *val);
	int (*write)(struct lkl_pci_dev *dev, int where, int size, void *val);
	void *(*resource_alloc)(struct lkl_pci_dev *dev,
				unsigned long resource_size,
				int resource_index);
	unsigned long long (*map_page)(struct lkl_pci_dev *dev, void *vaddr,
				       unsigned long size);
	void (*unmap_page)(struct lkl_pci_dev *dev,
			   unsigned long long dma_handle, unsigned long size);
};

/**
 * lkl_host_operations - host operations used by the Linux kernel
 *
 * These operations must be provided by a host library or by the application
 * itself.
 *
 * @virtio_devices - string containg the list of virtio devices in virtio mmio
 * command line format. This string is appended to the kernel command line and
 * is provided here for convenience to be implemented by the host library.
 *
 * @print - optional operation that receives console messages
 *
 * @panic - called during a kernel panic
 *
 * @thread_create - create a new thread and run f(arg) in its context; returns a
 * thread handle or 0 if the thread could not be created
 * @thread_exit - terminates the current thread
 * for success, -1 otherwise
 *
 * @mem_alloc - allocate memory
 * @mem_free - free memory
 * @page_alloc - allocate page aligned memory
 * @page_free - free memory allocated by page_alloc
 *
 * @timer_create - allocate a host timer that runs fn(arg) when the timer
 * fires.
 * @timer_free - disarms and free the timer
 * @timer_set_oneshot - arm the timer to fire once, after delta ns.
 * @timer_set_periodic - arm the timer to fire periodically, with a period of
 * delta ns.
 *
 * @ioremap - searches for an I/O memory region identified by addr and size and
 * returns a pointer to the start of the address range that can be used by
 * iomem_access
 * @iomem_acess - reads or writes to and I/O memory region; addr must be in the
 * range returned by ioremap
 *
 * @memcpy - copy memory
 * @pci_ops - pointer to PCI host operations
 */
struct lkl_host_operations {
	const char	*virtio_devices;

	void (*print)(const char *str, int len);
	void (*panic)(void);

	void (*thread_switch)(lkl_thread_t prev, lkl_thread_t next);
	lkl_thread_t (*thread_create)(void *(*f)(void *), void *arg);
	void (*thread_exit)(lkl_thread_t lthrd);
	lkl_thread_t (*thread_self)(void);

	void* (*mem_alloc)(unsigned long);
	void (*mem_free)(void *);
	void* (*page_alloc)(unsigned long size);
	void (*page_free)(void *addr, unsigned long size);

	unsigned long long (*time)(void);

	void (*clockevent_alloc)(int irq);
	void (*clockevent_set_next)(unsigned long ns_delta);
	void (*clockevent_free)(void);

	void* (*ioremap)(long addr, int size);
	int (*iomem_access)(const volatile void *addr, void *val, int size,
			    int write);

	void* (*memcpy)(void *dest, const void *src, unsigned long count);
	struct lkl_dev_pci_ops *pci_ops;
};

/**
 * lkl_start_kernel - registers the host operations and starts the kernel
 *
 * The function returns only after the kernel is shutdown with lkl_sys_halt.
 *
 * @lkl_ops - pointer to host operations
 * @mem_start - start memory address
 * @mem_size - memory size
 * generate the Linux kernel command line
 */
int lkl_start_kernel(struct lkl_host_operations *lkl_ops, void *mem_start, unsigned long mem_size);

/**
 * lkl_is_running - returns 1 if the kernel is currently running
 */
int lkl_is_running(void);

#endif
