#include "ft_nm.h"

t_btree *g_master_node = NULL;

void insert_in_btree(t_btree **child_node, unsigned long long sym_entry_index) {
	if (!*child_node)
	{
		create_node(child_node);
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
		free_btrees(g_master_node);
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

int compare_symtab_entries(unsigned long long index1, unsigned long long index2) {
	if (is32){
		const char *name1 = symtab_strtab_ptr + symbol_array32[index1].st_name;
		const char *name2 = symtab_strtab_ptr + symbol_array32[index2].st_name;

		return ft_strncmp_custom(name1, name2, 100) * (G_FLAGS[REVERSE_ORDER_FLAG] ? -1 : 1);
	}
	const char *name1 = symtab_strtab_ptr + symbol_array64[index1].st_name;
	const char *name2 = symtab_strtab_ptr + symbol_array64[index2].st_name;

	return ft_strncmp_custom(name1, name2, 100) * (G_FLAGS[REVERSE_ORDER_FLAG] ? -1 : 1);
}

void cmp_and_insert_in_btree(unsigned long long sym_entry_index, t_btree *node) {
	if (compare_symtab_entries(node->index, sym_entry_index) < 0)
		insert_in_btree(&(node->right), sym_entry_index);
	else
		insert_in_btree(&(node->left), sym_entry_index);
}

t_btree *sort_symbols_with_btree(unsigned long long number_of_symbols) {
	for( unsigned long long i = 1; i < number_of_symbols; ++i){
		cmp_and_insert_in_btree(i, g_master_node);
	}
	return g_master_node;
}

