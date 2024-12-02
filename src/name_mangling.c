#include <ctype.h>
#include "ft_nm.h"

void print_symbol(Elf64_Shdr *section_headers, unsigned long long int index);

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

void display_symbol_table(Elf64_Ehdr *elf_header, Elf64_Shdr *section_headers, Elf64_Shdr *symbol_table) {
	Elf64_Shdr *string_table_section = &section_headers[symbol_table->sh_link];
	symtab_strtab_ptr = (char *) elf_header + string_table_section->sh_offset;
	symbol_array = (Elf64_Sym *) ((char *) elf_header + symbol_table->sh_offset);
	g_master_node = (t_btree *) malloc(sizeof(t_btree));
	if (!g_master_node) {
		perror("Error allocating memory for btree node");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf64_Ehdr));
	}
	ft_bzero(g_master_node, sizeof(t_btree));
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
	if (!should_skip_symbol(symbol_name, symbol_array[index])) {
		if (symbol_array[index].st_value == 0) {
			printf("                 %c %s\n", letter, symbol_name);
		} else {
			printf("%016lx %c %s\n", symbol_array[index].st_value, letter, symbol_name);
		}
	}
}

int should_skip_symbol(const char *symbol_name, Elf64_Sym symbol) {
	if (symbol_name == NULL || symbol_name[0] == '\0')
		return 1;
	unsigned char bind = ELF64_ST_BIND(symbol.st_info);
	if (G_FLAGS[ONLY_GLOBAL_SYMBOLS_FLAG] && bind == STB_LOCAL)
		return 1;
	if (G_FLAGS[ONLY_UNDIFINED_SYMBOLS_FLAG] && symbol.st_shndx != SHN_UNDEF)
		return 1;
	if (G_FLAGS[ALL_SYMBOLS_FLAG])
		return 0;

	int len = ft_strlen(symbol_name);
	if (len >= 2 && symbol_name[len - 2] == '.') {
		char last_char = symbol_name[len - 1];
		if (last_char == 'c' || last_char == 'o' || last_char == 'a' || last_char == 's' || last_char == 'h'
			|| last_char == 'S') {
			return 1;
		}
	}
	if (len >= 3 && ft_strncmp(symbol_name + len - 3, ".so", 3) == 0) {
		return 1;
	}
	if (len >= 4 && ft_strncmp(symbol_name + len - 4, ".out", 4) == 0) {
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