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
#include <stdbool.h>
#include "elf.h"

typedef struct s_btree {
	struct s_btree *left;
	struct s_btree *right;
	unsigned long long index;
} t_btree;
extern t_btree *g_master_node;


/* sym_array is the symbol table
 * in a format of an array of Elf64_Sym */
extern Elf64_Sym *symbol_array64;
extern Elf32_Sym *symbol_array32;
// is32 is a flag to indicate if the binary is 32 or 64 bits
extern bool is32;
/* symtab_strtab_p is a ptr to the string table of the symbol table */
extern char *symtab_strtab_ptr;
extern char *file_name;


extern char G_FLAGS[6];
enum FLAGS {ALL_SYMBOLS_FLAG, ONLY_GLOBAL_SYMBOLS_FLAG, ONLY_UNDIFINED_SYMBOLS_FLAG, REVERSE_ORDER_FLAG, NO_SORT_FLAG, DEBUG_FLAG};

void check_arguments(int argc, char **argv);
int open_file(char *string);
void get_file_stat(int fd, struct stat *st);
void map_file_to_memory(int fd, struct stat *st, void **ptr);
void check_flags(int argc, char **argv);

//global nm
void ft_nm64( void* ptr_to_bin, long long size_of_bin );
void ft_nm32( void* ptr_to_bin, long long size_of_bin );

void verify_magic_number(uint32_t magic_number);
uint32_t get_magic_number_ELF(Elf64_Ehdr *ehdr); //this is cross architecture

//custom btree
void create_node(t_btree **p_btree);
void free_btrees(struct s_btree *node);
void cmp_and_insert_in_btree(unsigned long long sym_entry_index, t_btree *node);
t_btree *sort_symbols_with_btree(unsigned long long number_of_symbols);
void print_btree64(t_btree *node, Elf64_Shdr *sdhdrtab);
void print_btree32(t_btree *node, Elf32_Shdr *sdhdrtab);

//exit routine
void unmap_file(void *p_void, off_t size);
void unmap_file_and_exit_with_failure(void *p_void, off_t size);

// utils
void ft_bzero(void *s, size_t n);
int ft_strncmp(const char *s1, const char *s2, size_t n);
int ft_strncmp_custom(const char *s1, const char *s2, size_t n);
size_t ft_strlen(const char *s);
char* ft_strrchr(const char *s_origin, int c);
void error(char *message);
char	*ft_strdup(const char *str);

#endif //UNTITLED__NM_H_
