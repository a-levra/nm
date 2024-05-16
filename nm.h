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
#include "elf.h"

typedef struct s_btree {
	struct s_btree *parent;
	struct s_btree *left;
	struct s_btree *right;
	unsigned long long index;
} t_btree;
extern t_btree *master_node;

/* sym_array is the symbol table
 * in a format of an array of Elf64_Sym */
extern Elf64_Sym *sym_array;
/* symtab_strtab_p is a ptr to the string table of the symbol table */
extern char *symtab_strtab_p;


void check_args(int argc);
int try_open(char *string);
void check_fstat(int fd, struct stat *st);
void map_file_to_memory(int fd, struct stat *st, void **ptr);
void unmap_file(void *p_void, off_t size);
void nm(void *ptr_to_bin);
void check_magic_number(uint32_t magic_number);
uint32_t get_magic_number_ELF(Elf64_Ehdr *ehdr);
void read_section_header(Elf64_Ehdr *ehdr);
void print_symbol_table(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr, Elf64_Shdr *symtab);
void create_node(t_btree **p_btree);
void free_btrees(struct s_btree *node);
void cmp_and_insert_in_btree(unsigned long long sym_entry_index, t_btree *node);
t_btree *sort_with_custom_btree(unsigned long long number_of_symbols);


#endif //UNTITLED__NM_H_
