OBJS	= main.o common.o pt1.o pt2.o pt3.o wind.o height.o moisture.o
SOURCE	= src/main.c src/common.c src/pt/t1.c src/pt/t2.c src/pt/t3.c src/wind.c src/height.c src/moisture.c
HEADER	= include/main.h include/common.h include/pt/t1.h include/pt/t2.h include/pt/t3.h include/wind.h include/height.h include/moisture.h
OUT	= bin/meteo
CC	 = gcc
FLAGS	 = -g -c -Wall -Wextra -pedantic
IFLAGS	 = -Iinclude
LFLAGS	 = -lm

all: clean $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

common.o: src/common.c
	$(CC) $(FLAGS) $(IFLAGS) src/common.c

pt1.o: src/pt/t1.c
	$(CC) $(FLAGS) $(IFLAGS) src/pt/t1.c -o pt1.o

pt2.o: src/pt/t2.c
	$(CC) $(FLAGS) $(IFLAGS) src/pt/t2.c -o pt2.o

pt3.o: src/pt/t3.c
	$(CC) $(FLAGS) $(IFLAGS) src/pt/t3.c -o pt3.o

wind.o: src/wind.c
	$(CC) $(FLAGS) $(IFLAGS) src/wind.c

height.o: src/height.c
	$(CC) $(FLAGS) $(IFLAGS) src/height.c

moisture.o: src/moisture.c
	$(CC) $(FLAGS) $(IFLAGS) src/moisture.c

main.o: src/main.c
	$(CC) $(FLAGS) $(IFLAGS) src/main.c

doxygen:
	doxygen Doxyfile

clean:
	rm -f $(OBJS)

cleanall:
	rm -f $(OBJS) $(OUT)
