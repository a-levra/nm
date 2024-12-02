# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: alevra <alevra@student.42lyon.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/12/02 00:00:00 by alevra            #+#    #+#              #
#    Updated: 2024/12/02 00:00:00 by alevra           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ft_nm

HEADER = $(NAME).h

SRC =	btree_sorting.c \
        main.c \
        name_mangling.c \
        parse_args.c \
        utils.c \

FLAGS = -Wall -Wextra -Werror
FSAN = -fsanitize=address
OBJ = $(addprefix obj/,$(SRC:.c=.o))

all	: create_obj_folder
	make $(NAME)
	@make end_message

obj/%.o : src/%.c src/$(HEADER) Makefile
	cc -c -g3 ${FLAGS} $(FSAN) $< -o $@

$(NAME): $(OBJ)
	cc $(OBJ) $(FSAN) -g3 -o $(NAME)

create_obj_folder :
	@mkdir -p obj

clean:
	rm -f $(OBJ)
	@if [ -d "./obj" ]; then\
		rm -r obj;\
	fi

fclean: clean
	rm -f $(NAME)

re:
	make fclean
	make all

end_message:
	@echo "Done !"

test:
	@mkdir -p test_res
	./launch_test

.PHONY: all clean fclean re create_obj_folder end_message test