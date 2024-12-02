#include <strings.h>
#include <ctype.h>
#include "ft_nm.h"

Elf64_Sym *symbol_array;
char *symtab_strtab_ptr;

void print_symbol(Elf64_Shdr *section_headers, unsigned long long int index);

int main(int argc, char **argv) {
	int file_descriptor;
	struct stat file_stat;
	void *binary_pointer;

	check_arguments(argc, argv);

	file_descriptor = open_file(argv[argc - 1]);
	get_file_stat(file_descriptor, &file_stat);
	map_file_to_memory(file_descriptor, &file_stat, &binary_pointer);
	close(file_descriptor);
	check_ELF_file_integrity(binary_pointer, file_stat.st_size);
	name_mangling(binary_pointer);
	unmap_file(binary_pointer, file_stat.st_size);
	exit(EXIT_SUCCESS);
}

void check_ELF_file_integrity(void *binary_pointer, off_t size) {
	if ((unsigned long) size < sizeof(Elf64_Ehdr)) {
		fprintf(stderr, "Error: file is too small to be a valid ELF file\n");
		unmap_file_and_exit_with_failure((void *) binary_pointer, size);
	}
	Elf64_Ehdr *elf_header = (Elf64_Ehdr *) binary_pointer;
	verify_magic_number(get_magic_number_ELF(elf_header));
	if (elf_header->e_shoff + elf_header->e_shnum * sizeof(Elf64_Shdr) > (unsigned long) size) {
		fprintf(stderr, "Error: section headers table is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (elf_header->e_shstrndx >= elf_header->e_shnum) {
		fprintf(stderr, "Error: section header string table index is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}

}

Elf64_Shdr *get_symbol_table(Elf64_Shdr *section_headers, Elf64_Ehdr *elf_header) {
	return find_section_by_type(section_headers, elf_header->e_shnum, SHT_SYMTAB);
}

Elf64_Shdr *find_section_by_type(Elf64_Shdr *section_headers, uint16_t section_count, uint32_t type) {
	for (int i = 0; i < section_count; ++i) {
		if (section_headers[i].sh_type == type) {
			return &section_headers[i];
		}
	}
	return NULL;
}

void check_symtab_integrity(const Elf64_Ehdr *elf_header,
							const Elf64_Shdr *section_headers_table,
							const Elf64_Shdr *symbol_table) {
	if (symbol_table == NULL) {
		fprintf(stderr, "Error: could not find symbol table\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (symbol_table->sh_link >= elf_header->e_shnum) {
		fprintf(stderr, "Error: symbol table string table index is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (symbol_table->sh_offset + symbol_table->sh_size > elf_header->e_shoff) {
		fprintf(stderr, "Error: symbol table is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (symbol_table->sh_link >= elf_header->e_shnum) {
		fprintf(stderr, "Error: symbol table string table index is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (section_headers_table[symbol_table->sh_link].sh_type != SHT_STRTAB) {
		fprintf(stderr, "Error: symbol table string table is not of type STRTAB\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
}

// todo get some test binaries
//todo make a Makefile
