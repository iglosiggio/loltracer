#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "jitdump.h"

pid_t gettid() {
	return syscall(SYS_gettid);
}

uint64_t get_timestamp() {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (uint64_t) tp.tv_sec * 1000000000 + tp.tv_nsec;
}

struct jitdump_header {
	uint32_t magic;		/* "JiTD" (0x4A695444 on little endian) */
	uint32_t version;	/* Currently set to 1 */
	uint32_t total_size;	/* Size in bytes of the header */
	uint32_t elf_mach;	/* ELF arch encodinf (e_machine from <elf.h> */
	uint32_t pad1;		/* Padding. Reserved for future use */
	uint32_t pid;		/* JIT runtime process identification */
	uint64_t timestamp;	/* timestamp of when the file was created */
	uint64_t flags;		/* a bitmask of flags */
};

enum jitdump_header_flags {
	/* Set if the jitdump file is using an architecture-specific clock
	 * source. */
	JITDUMP_FLAGS_ARCH_TIMESTAMP = 1
};

struct jitdump_record_header {
	uint32_t id;		/* Type of the record */
	uint32_t total_size;	/* Size in bytes of the record (w/header) */
	uint64_t timestamp;	/* When the record was created */
};

enum jitdump_record_type {
	JIT_CODE_LOAD 		= 0,	/* A jitted function */
	JIT_CODE_MOVE		= 1,	/* An already jitted function wich is moved */
	JIT_CODE_DEBUG_INFO	= 2,	/* Debug info of a jitted function */
	JIT_CODE_CLOSE		= 3,	/* End of the jit runtime (optional) */
	JIT_CODE_UNWINDING_INFO	= 4	/* Function unwinding information */
};

struct jitdump_code_load {
	uint32_t pid;		/* Process id of the runtime generating the code */
	uint32_t tid;		/* Thread id of the runtime generating the code */
	uint64_t vma;		/* virtual address of jitted code */
	uint64_t code_addr;	/* code start of the jitted code */
	uint64_t code_size;	/* Size of the jitted code */
	uint64_t code_index;	/* Unique identifier for the code */
	char name[];		/* Function name */
	/* Raw code follows... */
};

static FILE* fp;
static size_t code_idx;
static void* marker;
static size_t page_size;

void jitdump_open() {
	int fd;
	char filename[64];
	sprintf(filename, "jit-%d.dump", getpid());

	fd = open(filename, O_CREAT|O_TRUNC|O_RDWR, 0666);
	fp = fdopen(fd, "w+");

	page_size = sysconf(_SC_PAGESIZE);
	mmap(0, page_size, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);

	struct jitdump_header header = {
		.magic = 0x4A695444,
		.version = 1,
		.total_size = sizeof(header),
		.elf_mach = EM_X86_64, /* TODO: Hardcoded arch */
		.pad1 = 0,
		.pid = getpid(),
		.timestamp = get_timestamp(),
		.flags = 0
	};
	fwrite(&header, sizeof(header), 1, fp);
}

size_t jitdump_emit_load(const char* name, void* addr, size_t size, size_t offset) {
	size_t symbol_len = strlen(name) + 1;
	struct jitdump_record_header header = {
		.id = JIT_CODE_LOAD,
		.total_size = sizeof(header)
		            + sizeof(struct jitdump_code_load)
		            + symbol_len
		            + size,
		.timestamp = get_timestamp()
	};
	fwrite(&header, sizeof(header), 1, fp);

	struct jitdump_code_load load = {
		.pid = getpid(),
		.tid = gettid(),
		.vma = (uint64_t) addr,
		.code_addr = (uint64_t) addr + offset,
		.code_size = size - offset,
		.code_index = code_idx
	};
	fwrite(&load, sizeof(load), 1, fp);

	fwrite(name, symbol_len, 1, fp);

	fwrite(addr, size, 1, fp);

	return code_idx++;
}

void jitdump_close() {
	fclose(fp);
	fp = NULL;
	code_idx = 0;
	munmap(marker, page_size);
	marker = 0;
	page_size = 0;
}
