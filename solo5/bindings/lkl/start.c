/*
 * Copyright (c) 2015-2019 Contributors as noted in the AUTHORS file
 *
 * This file is part of Solo5, a sandboxed execution environment.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "bindings.h"
#include "../crt_init.h"
#include "solo5_version.h"

extern void init_liblkl(void *heap_start, unsigned long heap_size, solo5_handle_t handle);
extern void __init_libc(char *envp[], const char *);

static void
start_lkl(void *heap_start, unsigned long heap_size)
{
	char	*envp[] = { 0 };
	solo5_handle_t	handle;
	struct solo5_net_info	info;

	if (solo5_net_acquire(NULL, &handle, &info) != SOLO5_R_OK)
		handle = 0;
	__init_libc(envp, "lkl");
	init_liblkl(heap_start, heap_size, handle);
}

void _start(void *arg)
{
   /* These should be disabled. __init_libc() will manage ssp, tls. */
#if 0
    crt_init_ssp();
    crt_init_tls();
#endif

    static struct solo5_start_info si;

    platform_init(arg);
    si.cmdline = cmdline_parse(platform_cmdline());

    log(INFO, "            |      ___|\n");
    log(INFO, "  __|  _ \\  |  _ \\ __ \\\n");
    log(INFO, "\\__ \\ (   | | (   |  ) |\n");
    log(INFO, "____/\\___/ _|\\___/____/\n");
    log(INFO, "Solo5: Bindings version %s\n", SOLO5_VERSION);

    mem_init();
    block_init(arg);
    net_init(arg);

    mem_lock_heap(&si.heap_start, &si.heap_size);

    /* solo5 binding uses stack in this area. Stack size should be subtracted for now. */
    start_lkl((void *)si.heap_start, si.heap_size - 0x10000);

    solo5_exit(solo5_app_main(&si));
}

/*
 * Place the .interp section in this module, as it comes first in the link
 * order.
 */
DECLARE_ELF_INTERP

/*
 * The "ABI1" Solo5 ELF note is declared in this module.
 */
ABI1_NOTE_DECLARE_BEGIN
{
    .abi_target = SPT_ABI_TARGET,
    .abi_version = SPT_ABI_VERSION
}
ABI1_NOTE_DECLARE_END

/*
 * Pretend that we are an OpenBSD executable. See elf_abi.h for details.
 */
DECLARE_OPENBSD_NOTE
