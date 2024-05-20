#include <strings.h>
#include <ctype.h>
#include "nm.h"

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
	if (size < sizeof(Elf64_Ehdr)) {
		fprintf(stderr, "Error: file is too small to be a valid ELF file\n");
		unmap_file_and_exit_with_failure((void *) binary_pointer, size);
	}
	Elf64_Ehdr *elf_header = (Elf64_Ehdr *) binary_pointer;
	verify_magic_number(get_magic_number_ELF(elf_header));
	if (elf_header->e_shoff + elf_header->e_shnum * sizeof(Elf64_Shdr) > size) {
		fprintf(stderr, "Error: section headers table is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	if (elf_header->e_shstrndx >= elf_header->e_shnum) {
		fprintf(stderr, "Error: section header string table index is out of bounds\n");
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

void check_section_headers_table_integrity(Elf64_Ehdr *ELF_header, Elf64_Shdr *section_header_table) {
	if (!section_header_table) {
		fprintf(stderr, "Error: could not find section headers table\n");
		unmap_file_and_exit_with_failure((void *) ELF_header, sizeof(Elf64_Ehdr));
	}

	if (section_header_table[ELF_header->e_shstrndx].sh_offset + section_header_table[ELF_header->e_shstrndx].sh_size
		> ELF_header->e_shoff) {
		fprintf(stderr, "Error: section header string table is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) ELF_header, sizeof(Elf64_Ehdr));
	}
}

Elf64_Shdr *get_section_headers_table(Elf64_Ehdr *elf_header) {
	return (Elf64_Shdr *) ((char *) elf_header + elf_header->e_shoff);
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

void display_symbol_table(Elf64_Ehdr *elf_header, Elf64_Shdr *section_headers, Elf64_Shdr *symbol_table) {
	Elf64_Shdr *string_table_section = &section_headers[symbol_table->sh_link];
	symtab_strtab_ptr = (char *) elf_header + string_table_section->sh_offset;
	symbol_array = (Elf64_Sym *) ((char *) elf_header + symbol_table->sh_offset);
	g_master_node = (t_btree *) malloc(sizeof(t_btree));
	if (!g_master_node) {
		perror("Error allocating memory for btree node");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	bzero(g_master_node, sizeof(t_btree));
	unsigned long long symbol_count = symbol_table->sh_size / sizeof(Elf64_Sym);
	sort_symbols_with_btree(symbol_count);
	if (G_FLAGS[NO_SORT_FLAG]) {
		for (unsigned long long i = 0; i < symbol_count; ++i) {
			print_symbol(section_headers, i);
		}
	} else {
		print_btree(g_master_node, section_headers);
	}
	free_btrees(g_master_node);
}

void print_btree(t_btree *node, Elf64_Shdr *section_headers) {
	if (node->left) {
		print_btree(node->left, section_headers);
	}
	unsigned long long index = node->index;
	print_symbol(section_headers, index);
	if (node->right) {
		print_btree(node->right, section_headers);
	}
}

void print_symbol(Elf64_Shdr *section_headers, unsigned long long int index) {
	char *symbol_name = symtab_strtab_ptr + symbol_array[index].st_name;
	unsigned char letter = get_symbol_letter(symbol_array[index], section_headers);
	if (!should_skip_symbol(symbol_name, letter, symbol_array[index])) {
		if (symbol_array[index].st_value == 0) {
			printf("                 %c %s\n", letter, symbol_name);
		} else {
			printf("%016llx %c %s\n", symbol_array[index].st_value, letter, symbol_name);
		}
	}
}

int should_skip_symbol(const char *symbol_name, const char letter, Elf64_Sym symbol) {
	if (symbol_name == NULL || symbol_name[0] == '\0')
		return 1;
	unsigned char bind = ELF64_ST_BIND(symbol.st_info);
	if (G_FLAGS[ONLY_GLOBAL_SYMBOLS_FLAG] && bind == STB_LOCAL)
		return 1;
	if (G_FLAGS[ONLY_UNDIFINED_SYMBOLS_FLAG] && symbol.st_shndx != SHN_UNDEF)
		return 1;
	if (G_FLAGS[ALL_SYMBOLS_FLAG])
		return 0;

	int len = strlen(symbol_name);
	if (len >= 2 && symbol_name[len - 2] == '.') {
		char last_char = symbol_name[len - 1];
		if (last_char == 'c' || last_char == 'o' || last_char == 'a' || last_char == 's' || last_char == 'h'
			|| last_char == 'S') {
			return 1;
		}
	}
	if (len >= 3 && strcmp(symbol_name + len - 3, ".so") == 0) {
		return 1;
	}
	if (len >= 4 && strcmp(symbol_name + len - 4, ".out") == 0) {
		return 1;
	}
	return 0;
}

unsigned char get_symbol_letter(Elf64_Sym symbol, Elf64_Shdr *section_headers) {
	unsigned char bind = ELF64_ST_BIND(symbol.st_info);
	unsigned char type = ELF64_ST_TYPE(symbol.st_info);
	Elf64_Half section_index = symbol.st_shndx;

	if (section_index >= SHN_LORESERVE) {
		if (section_index == SHN_ABS) {
			if (bind == STB_LOCAL)
				return 'a';
			else
				return 'A';
		}
		return '?';
	}
	Elf32_Word section_type = section_headers[section_index].sh_type;
	Elf32_Xword section_flags = section_headers[section_index].sh_flags;
	unsigned char letter = '?';

	if (bind == STB_GNU_UNIQUE) {
		letter = 'u';
	} else if (bind == STB_WEAK) {
		if (type == STT_OBJECT) {
			letter = (section_index == SHN_UNDEF) ? 'v' : 'V';
		} else {
			letter = (section_index == SHN_UNDEF) ? 'w' : 'W';
		}
	} else if (section_index == SHN_UNDEF) {
		letter = 'U';
	} else if (section_index == SHN_ABS) {
		letter = 'A';
	} else if (section_index == SHN_COMMON) {
		letter = 'C';
	} else if (section_type == SHT_NOBITS && section_flags == (SHF_ALLOC | SHF_WRITE)) {
		letter = 'B';
	} else if (section_type == SHT_PROGBITS) {
		if (section_flags & SHF_ALLOC && section_flags & SHF_EXECINSTR) {
			letter = 'T';
		} else if (section_flags & SHF_ALLOC && section_flags & SHF_WRITE) {
			letter = 'D';
		}
	} else if (section_type == SHT_DYNAMIC) {
		letter = 'D';
	} else if (section_type == SHT_NOTE) {
		letter = 'N';
	} else if (type == STT_OBJECT || type == STT_COMMON) {
		if (section_flags & SHF_WRITE) {
			letter = 'D';
		} else {
			letter = 'R';
		}
	}
	if (section_flags & SHF_ALLOC && !(section_flags & SHF_WRITE) && !(section_flags & SHF_EXECINSTR)) {
		letter = 'R';
	}
	if (bind == STB_LOCAL && letter != '?') {
		letter = tolower(letter);
	}
	return letter;
}
// todo remove bzero and change strcmp to ft_strncmp
// todo get some test binaries
