#include "ft_nm.h"

char G_FLAGS[5] = {0, 0, 0, 0, 0};
/* -a flag : display all symbols
 * -g flag : display only global symbols
 * -u flag : display only undefined symbols
 * -r flag : sort in reverse order
 * -p flag : don't sort, display in the order the symbols appear in the file
 * */
void check_flag_conformity(int argc, char **argv);

void check_flags(int argc, char **argv) {
	check_flag_conformity(argc, argv);
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			int j = 1;
			while (argv[i][j]) {
				switch (argv[i][j]) {
					case 'a': G_FLAGS[ALL_SYMBOLS_FLAG] = 1;
						break;
					case 'g': G_FLAGS[ONLY_GLOBAL_SYMBOLS_FLAG] = 1;
						break;
					case 'u': G_FLAGS[ONLY_UNDIFINED_SYMBOLS_FLAG] = 1;
						break;
					case 'r': G_FLAGS[REVERSE_ORDER_FLAG] = 1;
						break;
					case 'p': G_FLAGS[NO_SORT_FLAG] = 1;
						break;
					default: printf("Error: flag %c not recognized\n", argv[i][1]);
						exit(EXIT_FAILURE);
				}
				++j;
			}
		}
	}
	if (G_FLAGS[ONLY_UNDIFINED_SYMBOLS_FLAG] == 1)
		G_FLAGS[ONLY_GLOBAL_SYMBOLS_FLAG] = 0;
	if (G_FLAGS[ONLY_GLOBAL_SYMBOLS_FLAG] == 1)
		G_FLAGS[ALL_SYMBOLS_FLAG] = 0;
}

void check_flag_conformity(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: ft_nm [-agurp] <binary>\n");
		exit(EXIT_FAILURE);
	}
	//last argument must not be a flag
	if (argv[argc - 1][0] == '-') {
		printf("Usage: ft_nm [-agurp] <binary>\n");
		exit(EXIT_FAILURE);
	}

	//the binary should be the last argument
	if (argc > 2) {
		for (int i = 1; i < argc - 1; ++i) {
			if (argv[i][0] != '-') {
				printf("Usage: ft_nm [-agurp] <binary>\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == '\0') {
				printf("Error: flag %s not recognized\n", argv[i]);
				exit(EXIT_FAILURE);
			}
		}
	}

}
