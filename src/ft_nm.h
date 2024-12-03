//
// Created by Aurelien Levra on 15/05/2024.
//

#ifndef UNTITLED__NM_H_
#define UNTITLED__NM_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <strings.h>
#include "elf.h"

typedef struct s_btree {
	struct s_btree *left;
	struct s_btree *right;
	unsigned long long index;
} t_btree;
extern t_btree *g_master_node;

/* sym_array is the symbol table
 * in a format of an array of Elf64_Sym */
extern Elf64_Sym *symbol_array;
/* symtab_strtab_p is a ptr to the string table of the symbol table */
extern char *symtab_strtab_ptr;

//init routine undi
extern char G_FLAGS[5];
enum FLAGS {ALL_SYMBOLS_FLAG, ONLY_GLOBAL_SYMBOLS_FLAG, ONLY_UNDIFINED_SYMBOLS_FLAG, REVERSE_ORDER_FLAG, NO_SORT_FLAG};

void check_arguments(int argc, char **argv);
int open_file(char *string);
void get_file_stat(int fd, struct stat *st);
void map_file_to_memory(int fd, struct stat *st, void **ptr);
void check_flags(int argc, char **argv);

//global nm
void name_mangling(void *ptr_to_bin);
void verify_magic_number(uint32_t magic_number);
uint32_t get_magic_number_ELF(Elf64_Ehdr *ehdr);
void display_symbol_table(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr, Elf64_Shdr *symtab);
unsigned char get_symbol_letter(Elf64_Sym symbol, Elf64_Shdr *shdr);
Elf64_Shdr* get_section_headers_table(Elf64_Ehdr *ehdr);
Elf64_Shdr* find_section_by_type(Elf64_Shdr *shdrtab, uint16_t shnum, uint32_t type);
Elf64_Shdr *get_symbol_table(Elf64_Shdr *shdrtab, Elf64_Ehdr *ehdr);
int should_skip_symbol(const char *symbol_name, Elf64_Sym symbol);
void check_ELF_file_integrity(void *binary_pointer, off_t size);
void check_symtab_integrity(const Elf64_Ehdr *elf_header,
							const Elf64_Shdr *section_headers,
							const Elf64_Shdr *symbol_table);
void check_section_headers_table_integrity(Elf64_Ehdr *ELF_header, Elf64_Shdr *section_header_table);

//custom btree
void create_node(t_btree **p_btree);
void free_btrees(struct s_btree *node);
void cmp_and_insert_in_btree(unsigned long long sym_entry_index, t_btree *node);
t_btree *sort_symbols_with_btree(unsigned long long number_of_symbols);
void print_btree(t_btree *node, Elf64_Shdr *sdhdrtab);

//exit routine
void unmap_file(void *p_void, off_t size);
void unmap_file_and_exit_with_failure(void *p_void, off_t size);

// utils
void ft_bzero(void *s, size_t n);
int ft_strncmp(const char *s1, const char *s2, size_t n);
int ft_strncmp_custom(const char *s1, const char *s2, size_t n);
size_t ft_strlen(const char *s);

#endif //UNTITLED__NM_H_
