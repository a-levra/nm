#include <strings.h>
#include <ctype.h>
#include "ft_nm.h"

Elf64_Sym *symbol_array;
char *symtab_strtab_ptr;
char *file_name;

int is32bits(void *ptr_to_bin);

int main(int argc, char **argv) {
	int file_descriptor;
	struct stat file_stat;
	void *ptr_to_binary;

	check_arguments(argc, argv);

	file_descriptor = open_file(argv[argc - 1]);
	file_name = ft_strdup(ft_strrchr(argv[argc - 1], '/'));
	get_file_stat(file_descriptor, &file_stat);
	map_file_to_memory(file_descriptor, &file_stat, &ptr_to_binary);
	close(file_descriptor);
	verify_magic_number(get_magic_number_ELF((Elf64_Ehdr *) ptr_to_binary));
	if (is32bits(ptr_to_binary))
		return 0; //ft_nm32(ptr_to_binary, file_stat.st_size);
	else
		ft_nm64(ptr_to_binary, file_stat.st_size);

	unmap_file(ptr_to_binary, file_stat.st_size);
	exit(EXIT_SUCCESS);
}

int is32bits(void *ptr_to_bin) {
	Elf64_Ehdr *elf_header = (Elf64_Ehdr *) ptr_to_bin;
	return elf_header->e_ident[EI_CLASS] == ELFCLASS32;
}

//void ft_nm32( void* ptr_to_bin, long long size_of_bin ){
//	check_ELF_file_integrity(ptr_to_bin, size_of_bin);
//	name_mangling(ptr_to_bin);
//}
