#include "ft_nm.h"

void ft_nm64( void* ptr_to_bin, long long size_of_bin ){
	check_ELF_file_integrity(ptr_to_bin, size_of_bin);
	name_mangling(ptr_to_bin);
}

void check_ELF_file_integrity(void *binary_pointer, off_t size) {
	if ((unsigned long) size < sizeof(Elf64_Ehdr)) {
		error("file is too small to be a valid ELF file\n");
		unmap_file_and_exit_with_failure((void *) binary_pointer, size);
	}
	Elf64_Ehdr *elf_header = (Elf64_Ehdr *) binary_pointer;

	if (elf_header->e_shoff + elf_header->e_shnum * sizeof(Elf64_Shdr) > (unsigned long) size) {
		error("section headers table is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (elf_header->e_shstrndx >= elf_header->e_shnum) {
		error("section header string table index is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}

}

void name_mangling(void *binary_pointer) {
	Elf64_Ehdr *elf_header = (Elf64_Ehdr *) binary_pointer;
	Elf64_Shdr *section_headers_table = get_section_headers_table(elf_header);
	check_section_headers_table_integrity(elf_header, section_headers_table);
	Elf64_Shdr *symbol_table = get_symbol_table(section_headers_table, elf_header);

	check_symtab_integrity(elf_header, section_headers_table, symbol_table);
	display_symbol_table(elf_header, section_headers_table, symbol_table);
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
		error("could not find symbol table\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (symbol_table->sh_link >= elf_header->e_shnum) {
		error("symbol table string table index is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (symbol_table->sh_offset + symbol_table->sh_size > elf_header->e_shoff) {
		error("symbol table is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (symbol_table->sh_link >= elf_header->e_shnum) {
		error("symbol table string table index is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (section_headers_table[symbol_table->sh_link].sh_type != SHT_STRTAB) {
		error("symbol table string table is not of type STRTAB\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
}