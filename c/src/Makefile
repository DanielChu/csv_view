BIN_PATH=../bin

all: csv_view 

OS=LINUX

csv_view: csv_view.c csv.o
	gcc -Wall -O3 -I /usr/include/ncursesw/ csv_view.c csv.o -o ${BIN_PATH}/csv_view -lncursesw -D $(OS)

csv.o: csv.c
	gcc -Wall -O3 csv.c -c  -o csv.o

debug_csv_view: csv_view.c debug_csv.o
	gcc -g -Wall -I /usr/include/ncursesw/ csv_view.c csv.o -o ${BIN_PATH}/debug_csv_view -lncursesw  

debug_csv.o: csv.c
	gcc -g  -Wall csv.c -c -pg -o debug_csv.o
