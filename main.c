#include <strings.h>
#include "nm.h"

t_btree *master_node = NULL;
Elf64_Sym *sym_array;
char *symtab_strtab_p;

void print_custom_btree(t_btree *master_node);

unsigned char get_letter_info(unsigned char info);

int main(int argc, char **argv) {
	int fd;
	struct stat st;
	void *ptr_to_bin;
	check_args(argc);

	fd = try_open(argv[1]);
	check_fstat(fd, &st);
	map_file_to_memory(fd, &st, &ptr_to_bin);
	close(fd);
	nm(ptr_to_bin);
	unmap_file(ptr_to_bin, st.st_size);
	exit(EXIT_SUCCESS);

}

void nm(void *ptr_to_bin) {
	//case ELF
	// ehdr means ELF header
	Elf64_Ehdr *ehdr = ( Elf64_Ehdr *)ptr_to_bin;
	check_magic_number(get_magic_number_ELF(ehdr));
	read_section_header(ehdr);
}

Elf64_Shdr* get_section_headers_table(Elf64_Ehdr *ehdr) {
	return (Elf64_Shdr *)((char *)ehdr + ehdr->e_shoff);
}

Elf64_Shdr* get_section_header_table_string_table(Elf64_Ehdr *ehdr, Elf64_Shdr *shdrtab) {
	return &shdrtab[ehdr->e_shstrndx];
}

char* get_section_header_table_string_table_ptr(Elf64_Ehdr *ehdr, Elf64_Shdr *shstrtab) {
	return (char *)ehdr + shstrtab->sh_offset;
}

void print_section_names(Elf64_Ehdr *ehdr, Elf64_Shdr *shdrtab, char *shstrtab_p) {
	//print section names and their indexes
	for (int i = 0; i < ehdr->e_shnum; i++) {
		printf("%d: %s\n", i, shstrtab_p + shdrtab[i].sh_name);
	}
}

Elf64_Shdr* find_section_by_type(Elf64_Shdr *shdrtab, uint16_t shnum, uint32_t type) {
	for (int i = 0; i < shnum; ++i) {
		if (shdrtab[i].sh_type == type) {
			return &shdrtab[i];
		}
	}
	return NULL;
}

void read_section_header(Elf64_Ehdr *ehdr) {
	Elf64_Shdr *shdrtab = get_section_headers_table(ehdr);
//	Elf64_Shdr *shtab_strtab = get_section_header_table_string_table(ehdr, shdrtab);
//	char *shtab_strtab_p = get_section_header_table_string_table_ptr(ehdr, shtab_strtab);
//	print_section_names(ehdr, shdrtab, shtab_strtab_p);
	Elf64_Shdr* symtab = find_section_by_type(shdrtab, ehdr->e_shnum, SHT_SYMTAB);
	if (symtab == NULL) {
		printf("Error: could not find symbol table\n");
		exit(EXIT_FAILURE);
	}
	print_symbol_table(ehdr, shdrtab, symtab);
}

void print_symbol_table(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr, Elf64_Shdr *symtab) {
	Elf64_Shdr *symtab_strtab = &shdr[symtab->sh_link];							//ici, je récupère la string table de la symtab ( il y a le nom des symboles dedans)
	symtab_strtab_p = (char *)ehdr + symtab_strtab->sh_offset;			//la, c'est le ptr "point de départ" de la string table de la symtab
	sym_array = (Elf64_Sym *)((char *)ehdr + symtab->sh_offset);		//ici, je récupère un pointeur pour me permettre de parcourir la symtab
	unsigned long long number_of_symbols = symtab->sh_size / sizeof(Elf64_Sym);
	master_node = (t_btree *)malloc(sizeof(t_btree));
	if (!master_node) {
		printf("Error: could not allocate memory for btree node\n");
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	bzero(master_node, sizeof(t_btree));
	sort_with_custom_btree(number_of_symbols);
	print_custom_btree(master_node);
}

void print_custom_btree(t_btree *node) {
	if (node->left)
		print_custom_btree(node->left);

	if (sym_array[node->index].st_value == 0) {
		printf("                 %c %s\n", get_letter_info(sym_array[node->index].st_info), symtab_strtab_p + sym_array[node->index].st_name);
	} else {
		printf("%016llx %c %s\n", sym_array[node->index].st_value, get_letter_info(sym_array[node->index].st_info), symtab_strtab_p + sym_array[node->index].st_name);
	}

	if (node->right)
		print_custom_btree(node->right);
}



unsigned char get_letter_info(unsigned char info) {
	switch (info & 0xf) {
		case STT_NOTYPE:
			return ' ';
		case STT_OBJECT:
			return 'd';
		case STT_FUNC:
			return 't';
		case STT_SECTION:
			return 's';
		case STT_FILE:
			return 'f';
		default:
			return '?';
	}
}

void insert_in_btree(t_btree **child_node, t_btree *parent_node, unsigned long long sym_entry_index){
	if (!*child_node)
	{
		create_node(child_node);
		(*child_node)->parent = parent_node;
		(*child_node)->index = sym_entry_index;
	}
	else
		cmp_and_insert_in_btree(sym_entry_index, *child_node);
}

void create_node(t_btree **p_btree) {
	*p_btree = (t_btree *)malloc(sizeof(t_btree));
	if (!*p_btree) {
		printf("Error: could not allocate memory for btree node\n");
		perror("malloc");
		free_btrees(master_node);
		exit(EXIT_FAILURE);
	}
	bzero(*p_btree, sizeof(t_btree));
}

void free_btrees(t_btree *node) {
	if (node->left)
		free_btrees(node->left);
	if (node->right)
		free_btrees(node->right);
	free(node);
}

void cmp_and_insert_in_btree(unsigned long long sym_entry_index, t_btree *node){
	if (strcmp(symtab_strtab_p + sym_array[node->index].st_name,symtab_strtab_p + sym_array[sym_entry_index].st_name) < 0)
		insert_in_btree(&(node->right), node, sym_entry_index);
	else
		insert_in_btree(&(node->left), node, sym_entry_index);
}

t_btree *sort_with_custom_btree(unsigned long long number_of_symbols) {
	for( unsigned long long i = 1; i < number_of_symbols; ++i){
		cmp_and_insert_in_btree(i, master_node);
	}
	return master_node;
}

//todo remove bzero and change strcmp to ft_strncmp
//understand the letters