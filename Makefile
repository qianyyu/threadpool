CC      = g++
CFLAGS  = -Wall -O2 -std=c++11 -Werror -pthread

wc: distwc.o mapreduce.o threadpool.o
	$(CC) ${CFLAGS} -o wordcount distwc.o mapreduce.o threadpool.o
 
complie: distwc.o mapreduce.o threadpool.o

distwc.o: distwc.cc
	${CC} ${CFLAGS} -c distwc.cc
mapreduce.o: mapreduce.cc mapreduce.h
	${CC} ${CFLAGS} -c mapreduce.cc
threadpool.o: threadpool.cc threadpool.h
	${CC} ${CFLAGS} -c threadpool.cc

clean:
	rm *.o
	rm wordcount
	
compress:
	zip qian6.zip distwc.cc mapreduce.cc mapreduce.h threadpool.cc threadpool.h readme.md Makefile

clean-result:
	rm result-*.txt
clean-all: clean clean-result