OBJS	= tests.o cards.o halfskat.o pyskat.so
SOURCE	= tests.cpp cards.cpp halfskat.cpp  pyskat.cpp
HEADER	= cards.hpp halfskat.hpp
OUT	= tests
CC	 = clang++
FLAGS = -O3 -g -c -Wall -std=c++14 -stdlib=libc++
SOFLAGS = -shared -O3 -undefined dynamic_lookup -Wall -std=c++14 `python3-config --include`  -fPIC 
LFLAGS	 = -lgtest -lboost_thread -lboost_log -lboost_system

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

tests.o: tests.cpp
	$(CC) $(FLAGS) tests.cpp -o tests.o

cards.o: cards.cpp
	$(CC) $(FLAGS) cards.cpp  -o cards.o

halfskat.o: halfskat.cpp
	$(CC) $(FLAGS) halfskat.cpp -o halfskat.o

pyskat.so: pyskat.cpp
	$(CC) $(SOFLAGS) pyskat.cpp cards.cpp -o pyskat.so

clean:
	rm -f $(OBJS) $(OUT)