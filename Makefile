OBJS	= tests.o cards.o halfskat.o
SOURCE	= tests.cpp cards.cpp halfskat.cpp
HEADER	= cards.hpp halfskat.hpp
OUT	= tests
CC	 = clang++
FLAGS	 = -g -c -Wall -std=c++11 -stdlib=libc++
LFLAGS	 = -lgtest -lboost_thread -lboost_log -lboost_system

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

tests.o: tests.cpp
	$(CC) $(FLAGS) tests.cpp -o tests.o

cards.o: cards.cpp
	$(CC) $(FLAGS) cards.cpp  -o cards.o

halfskat.o: halfskat.cpp
	$(CC) $(FLAGS) halfskat.cpp -o halfskat.o

clean:
	rm -f $(OBJS) $(OUT)