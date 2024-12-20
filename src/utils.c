#include "ft_nm.h"

uint32_t get_magic_number_ELF(Elf64_Ehdr *ehdr) {
	uint32_t magic_number = ehdr->e_ident[EI_MAG0] | ehdr->e_ident[EI_MAG1] << 8 | ehdr->e_ident[EI_MAG2] << 16 | ehdr->e_ident[EI_MAG3] << 24;
	return magic_number;
}

void verify_magic_number(uint32_t magic_number) {
	switch (magic_number) {
		case 0x7f454c46:
		case 0x464c457f:
			break;
		default:
			printf("Error: magic number not recognized (0x%x)\n", magic_number);
			exit(EXIT_FAILURE);
	}

}

void unmap_file(void *p_void, off_t size) {
	if (munmap(p_void, size) == -1) {
		printf("Error: could not unmap file\n");
		perror("munmap");
		exit(EXIT_FAILURE);
	}
}

void unmap_file_and_exit_with_failure(void *p_void, off_t size) {
	unmap_file(p_void, size);
	exit(EXIT_FAILURE);
}

void map_file_to_memory(int fd, struct stat *st, void **ptr) {
	if ((*ptr = mmap(NULL, (*st).st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
		printf("Error: could not map file\n");
		perror("mmap");
		exit(EXIT_FAILURE);
	}
}

void get_file_stat(int fd, struct stat *st) {
	if (fstat(fd, st) == -1) {
		printf("Error: could not get file stats\n");
		perror("fstat");
		exit(EXIT_FAILURE);
	}
}

int open_file(char *string) {
	int fd = open(string, O_RDONLY);
	if (fd == -1) {
		printf("Error: could not open file %s\n", string);
		perror("open");
		exit(EXIT_FAILURE);
	}
	return fd;
}

void check_arguments(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: ft_nm <binary>\n");
		exit(EXIT_FAILURE);
	}
	check_flags(argc, argv);
}

void ft_bzero(void *s, size_t n) {
	unsigned char *p = s;
	while (n--)
		*p++ = 0;
}

int ft_strncmp(const char *s1, const char *s2, size_t n) {
	unsigned char *p1 = (unsigned char *) s1;
	unsigned char *p2 = (unsigned char *) s2;
	while (n--) {
		if (*p1 != *p2) {
			return *p1 - *p2;
		}
		if (*p1 == '\0') {
			return 0;
		}
		++p1;
		++p2;
	}
	return 0;
}

//will skip underscores and ignore case
int ft_strncmp_custom(const char *s1, const char *s2, size_t n) {
	unsigned char *p1 = (unsigned char *) s1;
	unsigned char *p2 = (unsigned char *) s2;
	size_t n1;
	size_t n2;

	while (n--) {
		n1 = n;
		n2 = n;
		//skip underscores for s1
		if (*p1 == '_')
			while (n1-- && *p1 == '_')
				++p1;

		//skip underscores for s2
		if (*p2 == '_')
			while (n2-- && *p2 == '_')
				++p2;

		n = (n1 < n2 ? n1 : n2);

		//ignore case
		char tolower_p1 = 0;
		if (*p1 >= 'A' && *p1 <= 'Z')
			tolower_p1 = 'A' - 'a';

		char tolower_p2 = 0;
		if (*p2 >= 'A' && *p2 <= 'Z')
			tolower_p2 = 'A' - 'a';

		if ((*p1 - tolower_p1) != (*p2 - tolower_p2)) {
			return (*p1 - tolower_p1) - (*p2 - tolower_p2);
		}
		if (*p1 == '\0') {
			return 0;
		}
		++p1;
		++p2;
	}
	return 0;
}

size_t ft_strlen(const char *s) {
	size_t len = 0;
	while (s[len]) {
		++len;
	}
	return len;
}

char	*ft_strrchr(const char *s, int c)
{
	size_t	i;
	char	char_c;
	int		res;

	char_c = (char)c;
	res = -1;
	i = 0;
	if (char_c == '\0')
	{
		while (s[i])
			i++;
		return ((char *)s + i);
	}
	while (s[i])
	{
		if (s[i] == char_c)
			res = i;
		i++;
	}
	if (res == -1)
		return (NULL);
	else
		return ((char *)s + res);
}

void error(char *message){
	fprintf(stderr, "ft_nm: %s: ", file_name);
	if (G_FLAGS[DEBUG_FLAG])
		fprintf(stderr, "%s\n", message);
	else
		fprintf(stderr, "file format not recognized\n");
}

char	*ft_strdup(const char *str)
{
	char	*new;
	size_t	i;

	new = malloc(sizeof(char) * ft_strlen(str) + 1);
	if (!new)
		return (NULL);
	i = 0;
	while (str[i])
	{
		new[i] = str[i];
		i++;
	}
	new[i] = 0;
	return (new);
}
