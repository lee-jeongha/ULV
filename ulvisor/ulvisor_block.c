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

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <assert.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <seccomp.h>
#include <stdbool.h>

#include "block_attach.h"
#include "ulvisor.h"

static int	fd_block;
static off_t	capacity;

static int
handle_cmdarg(char *cmdarg)
{
	char	path[256];
	int	n_scanned;

	if (strncmp("--block=", cmdarg, 8) != 0)
		return -1;

	n_scanned = sscanf(cmdarg, "--block=%256s", path);
	if (n_scanned != 1)
		return -1;

	fd_block = block_attach(path, &capacity);

	{
		extern int ulvisor_add_block(int fd);

		ulvisor_add_block(fd_block);
	}
	return 0;
}

static char *
usage(void)
{
	return "--block:NAME=PATH (attach block device/file at PATH as block storage NAME)";
}

DECLARE_MODULE(block,
	       .setup = NULL,
	       .handle_cmdarg = handle_cmdarg,
	       .usage = usage)
