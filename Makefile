CC = gcc
CFLAGS = -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors -fstack-protector-all -Wextra
PROGS = calendar public01 public02 public03 public04 public05 student_tests

.PHONY: clean

all: $(PROGS)

calendar: calendar.o my_memory_checker_216.o student_tests.o
	$(CC) -o calendar calendar.o my_memory_checker_216.o student_tests.o

public01: public01.o calendar.o  my_memory_checker_216.o 
	$(CC) -o public01 public01.o calendar.o my_memory_checker_216.o

public02: public02.o calendar.o my_memory_checker_216.o 
	$(CC) -o public02 public02.o calendar.o my_memory_checker_216.o 

public03: public03.o calendar.o my_memory_checker_216.o 
	$(CC) -o public03 public03.o calendar.o my_memory_checker_216.o 

public04: public04.o calendar.o my_memory_checker_216.o 
	$(CC) -o public04 public04.o calendar.o my_memory_checker_216.o 

public05: public05.o calendar.o my_memory_checker_216.o 
	$(CC) -o public05 public05.o calendar.o my_memory_checker_216.o 

student_tests: student_tests.o calendar.o my_memory_checker_216.o 
	$(CC) -o student_tests student_tests.o calendar.o my_memory_checker_216.o 

calendar.o: calendar.c calendar.h event.h my_memory_checker_216.h
	$(CC) $(CFLAGS) -c calendar.c

public01.o: public01.c event.h calendar.h my_memory_checker_216.h
	$(CC) $(CFLAGS) -c public01.c

public02.o: public02.c event.h calendar.h my_memory_checker_216.h
	$(CC) $(CFLAGS) -c public02.c

public03.o: public03.c event.h calendar.h my_memory_checker_216.h
	$(CC) $(CFLAGS) -c public03.c

public04.o: public04.c event.h calendar.h my_memory_checker_216.h
	$(CC) $(CFLAGS) -c public04.c

public05.o: public05.c event.h calendar.h my_memory_checker_216.h
	$(CC) $(CFLAGS) -c public05.c

student_tests.o: student_tests.c event.h calendar.h my_memory_checker_216.h
	$(CC) $(CFLAGS) -c student_tests.c

my_memory_checker_216.o: my_memory_checker_216.c my_memory_checker_216.h
	$(CC) $(CFLAGS) -c my_memory_checker_216.c


clean:
	@rm -f *.o $(PROGS) a.out



