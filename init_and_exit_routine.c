#include "nm.h"

uint32_t get_magic_number_ELF(Elf64_Ehdr *ehdr) {
	uint32_t magic_number = ehdr->e_ident[EI_MAG0] | ehdr->e_ident[EI_MAG1] << 8 | ehdr->e_ident[EI_MAG2] << 16 | ehdr->e_ident[EI_MAG3] << 24;
	return magic_number;
}

void check_magic_number(uint32_t magic_number) {
	switch (magic_number) {
		case 0x7f454c46:
		case 0x464c457f:
			break;
		default:
			printf("Error: magic number not recognized (0x%x)\n", magic_number);
			exit(EXIT_FAILURE);
	}

}

void unmap_file(void *p_void, off_t size) {
	if (munmap(p_void, size) == -1) {
		printf("Error: could not unmap file\n");
		perror("munmap");
		exit(EXIT_FAILURE);
	}
}

void map_file_to_memory(int fd, struct stat *st, void **ptr) {
	if ((*ptr = mmap(NULL, (*st).st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
		printf("Error: could not map file\n");
		perror("mmap");
		exit(EXIT_FAILURE);
	}
}

void check_fstat(int fd, struct stat *st) {
	if (fstat(fd, st) == -1) {
		printf("Error: could not get file stats\n");
		perror("fstat");
		exit(EXIT_FAILURE);
	}
}

int try_open(char *string) {
	int fd = open(string, O_RDONLY);
	if (fd == -1) {
		printf("Error: could not open file %s\n", string);
		perror("open");
		exit(EXIT_FAILURE);
	}
	return fd;
}

void check_args(int argc) {
	if (argc < 2) {
		printf("Usage: ft_nm <binary>\n");
		exit(EXIT_FAILURE);
	}
}