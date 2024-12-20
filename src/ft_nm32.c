#include "ft_nm.h"
#include <ctype.h>

static void name_mangling(void *ptr_to_bin);
static void display_symbol_table(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr, Elf32_Shdr *symtab);
static unsigned char get_symbol_letter(Elf32_Sym symbol, Elf32_Shdr *shdr);
static Elf32_Shdr* get_section_headers_table(Elf32_Ehdr *ehdr);
static Elf32_Shdr* find_section_by_type(Elf32_Shdr *shdrtab, uint16_t shnum, uint32_t type);
static Elf32_Shdr *get_symbol_table(Elf32_Shdr *shdrtab, Elf32_Ehdr *ehdr);
static int should_skip_symbol(const char *symbol_name, Elf32_Sym symbol);
static void check_ELF_file_integrity(void *binary_pointer, off_t size);
static void check_symtab_integrity(const Elf32_Ehdr *elf_header,
							const Elf32_Shdr *section_headers,
							const Elf32_Shdr *symbol_table);
static void check_section_headers_table_integrity(Elf32_Ehdr *ELF_header, Elf32_Shdr *section_header_table);
static void print_symbol(Elf32_Shdr *section_headers, unsigned long long int index);


void ft_nm32( void* ptr_to_bin, long long size_of_bin ){
	check_ELF_file_integrity(ptr_to_bin, size_of_bin);
	name_mangling(ptr_to_bin);
}

void print_symbol(Elf32_Shdr *section_headers, unsigned long long int index);

Elf32_Shdr *get_symbol_table(Elf32_Shdr *section_headers, Elf32_Ehdr *elf_header)
{
	return find_section_by_type(section_headers, elf_header->e_shnum, SHT_SYMTAB);
}

void check_section_headers_table_integrity(Elf32_Ehdr *ELF_header, Elf32_Shdr *section_header_table) {
	if (!section_header_table) {
		fprintf(stderr, "Error: could not find section headers table\n");
		unmap_file_and_exit_with_failure((void *) ELF_header, sizeof(Elf32_Ehdr));
	}

	if (section_header_table[ELF_header->e_shstrndx].sh_offset + section_header_table[ELF_header->e_shstrndx].sh_size
		> ELF_header->e_shoff) {
		fprintf(stderr, "Error: section header string table is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) ELF_header, sizeof(Elf32_Ehdr));
	}
}

void display_symbol_table(Elf32_Ehdr *elf_header, Elf32_Shdr *section_headers, Elf32_Shdr *symbol_table) {
	Elf32_Shdr *string_table_section = &section_headers[symbol_table->sh_link];
	symtab_strtab_ptr = (char *) elf_header + string_table_section->sh_offset;
	symbol_array32 = (Elf32_Sym *) ((char *) elf_header + symbol_table->sh_offset);
	if (G_FLAGS[ALL_SYMBOLS_FLAG])
		printf("%016d %c \n", 0 , 'a');
	g_master_node = (t_btree *) malloc(sizeof(t_btree));
	if (!g_master_node) {
		perror("Error allocating memory for btree node");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf32_Ehdr));
	}
	ft_bzero(g_master_node, sizeof(t_btree));
	unsigned long long symbol_count = symbol_table->sh_size / sizeof(Elf32_Sym);
	sort_symbols_with_btree(symbol_count);
	if (G_FLAGS[NO_SORT_FLAG]) {
		for (unsigned long long i = 0; i < symbol_count; ++i) {
			print_symbol(section_headers, i);
		}
	} else {
		print_btree32(g_master_node, section_headers);
	}
	free_btrees(g_master_node);
}

void print_btree32(t_btree *node, Elf32_Shdr *sdhdrtab) {
	if (node->left) {
		print_btree32(node->left, sdhdrtab);
	}
	unsigned long long index = node->index;
	print_symbol(sdhdrtab, index);
	if (node->right) {
		print_btree32(node->right, sdhdrtab);
	}
}

void print_symbol(Elf32_Shdr *section_headers, unsigned long long int index) {
	char *symbol_name = symtab_strtab_ptr + symbol_array32[index].st_name;
	unsigned char letter = get_symbol_letter(symbol_array32[index], section_headers);
	if (!should_skip_symbol(symbol_name, symbol_array32[index])) {
		if (symbol_array32[index].st_value == 0 &&( letter == 'w' || letter == 'U')) {
			printf("                 %c %s\n", letter, symbol_name);
		} else {
			printf("%016lx %c %s\n", (unsigned long) symbol_array32[index].st_value, letter, symbol_name);
		}
	}
}

int should_skip_symbol(const char *symbol_name, Elf32_Sym symbol) {
	if (symbol_name == NULL || symbol_name[0] == '\0')
		return 1;
	unsigned char bind = ELF32_ST_BIND(symbol.st_info);
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

unsigned char get_symbol_letter(Elf32_Sym symbol, Elf32_Shdr *section_headers) {
	unsigned char bind = ELF32_ST_BIND(symbol.st_info);
	unsigned char type = ELF32_ST_TYPE(symbol.st_info);
	Elf32_Half section_index = symbol.st_shndx;

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
		letter -= 'A' - 'a';
	}
	return letter;
}

void check_ELF_file_integrity(void *binary_pointer, off_t size) {
	if ((unsigned long) size < sizeof(Elf32_Ehdr)) {
		error("file is too small to be a valid ELF file\n");
		unmap_file_and_exit_with_failure((void *) binary_pointer, size);
	}
	Elf32_Ehdr *elf_header = (Elf32_Ehdr *) binary_pointer;

	if (elf_header->e_shoff + elf_header->e_shnum * sizeof(Elf32_Shdr) > (unsigned long) size) {
		error("section headers table is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf32_Ehdr));
	}
	if (elf_header->e_shstrndx >= elf_header->e_shnum) {
		error("section header string table index is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf32_Ehdr));
	}

}

void name_mangling(void *binary_pointer) {
	Elf32_Ehdr *elf_header = (Elf32_Ehdr *) binary_pointer;
	Elf32_Shdr *section_headers_table = get_section_headers_table(elf_header);
	check_section_headers_table_integrity(elf_header, section_headers_table);
	Elf32_Shdr *symbol_table = get_symbol_table(section_headers_table, elf_header);

	check_symtab_integrity(elf_header, section_headers_table, symbol_table);
	display_symbol_table(elf_header, section_headers_table, symbol_table);
}


Elf32_Shdr *find_section_by_type(Elf32_Shdr *section_headers, uint16_t section_count, uint32_t type) {
	for (int i = 0; i < section_count; ++i) {
		if (section_headers[i].sh_type == type) {
			return &section_headers[i];
		}
	}
	return NULL;
}

Elf32_Shdr *get_section_headers_table(Elf32_Ehdr *elf_header) {
	return (Elf32_Shdr *) ((char *) elf_header + elf_header->e_shoff);
}

void check_symtab_integrity(const Elf32_Ehdr *elf_header,
							const Elf32_Shdr *section_headers_table,
							const Elf32_Shdr *symbol_table) {
	if (symbol_table == NULL) {
		error("could not find symbol table\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf32_Ehdr));
	}
	if (symbol_table->sh_link >= elf_header->e_shnum) {
		error("symbol table string table index is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf32_Ehdr));
	}
	if (symbol_table->sh_offset + symbol_table->sh_size > elf_header->e_shoff) {
		error("symbol table is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf32_Ehdr));
	}
	if (symbol_table->sh_link >= elf_header->e_shnum) {
		error("symbol table string table index is out of bounds\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf32_Ehdr));
	}
	if (section_headers_table[symbol_table->sh_link].sh_type != SHT_STRTAB) {
		error("symbol table string table is not of type STRTAB\n");
		unmap_file_and_exit_with_failure((void *) elf_header, sizeof(Elf32_Ehdr));
	}
}