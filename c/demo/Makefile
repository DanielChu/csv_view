all: demo

demo: demo.c ../src/csv.c
	gcc -Wall -O3 -I /usr/include/ncursesw/ -I ../src/ demo.c ../src/csv.c -o ./demo -lncursesw  
