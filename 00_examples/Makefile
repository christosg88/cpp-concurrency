.PHONY: all

all: 01_print_in_order_asan 01_print_in_order_tsan

01_print_in_order_asan: 01_print_in_order.cpp Makefile
	clang++ -Wall -Wextra -Werror -std=c++17 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -O0 -g -o 01_print_in_order_asan 01_print_in_order.cpp

01_print_in_order_tsan: 01_print_in_order.cpp Makefile
	clang++ -Wall -Wextra -Werror -std=c++17 -fsanitize=thread -O0 -g -o 01_print_in_order_tsan 01_print_in_order.cpp

