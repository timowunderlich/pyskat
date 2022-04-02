#OBJS	= tests.o cards.o halfskat.o
OBJS	= cards.o halfskat.o
#SOURCE	= tests.cpp cards.cpp halfskat.cpp  pyskat_bindings.cpp
SOURCE	= cards.cpp halfskat.cpp  pyskat_bindings.cpp
SOURCEDIR = src/
HEADER	= cards.hpp halfskat.hpp
#OUT	= tests 
PYOUT = pyskat.so
CC	 = clang++
FLAGS = -O3 -g -c -Wall -std=c++14 -stdlib=libc++ -Iinclude/ `python3-config --include` -fvisibility=hidden
SOFLAGS = -shared -O3 -undefined dynamic_lookup -Wall -std=c++14 `python3-config --include`  -fPIC  -Iinclude/ -fvisibility=hidden
LFLAGS	 = -lgtest -lboost_thread -lboost_log -lboost_system
PYLFLAGS = -lboost_thread -lboost_log -lboost_system

all: $(OBJS) $(PYOUT)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

#tests.o: $(SOURCEDIR)tests.cpp
#	$(CC) $(FLAGS) $(SOURCEDIR)tests.cpp -o tests.o

cards.o: $(SOURCEDIR)cards.cpp
	$(CC) $(FLAGS) $(SOURCEDIR)cards.cpp  -o cards.o

halfskat.o: $(SOURCEDIR)halfskat.cpp
	$(CC) $(FLAGS) $(SOURCEDIR)halfskat.cpp -o halfskat.o

pyskat.so: $(SOURCEDIR)pyskat_bindings.cpp
	$(CC) $(SOFLAGS) $(PYLFLAGS) $(SOURCEDIR)pyskat_bindings.cpp $(SOURCEDIR)cards.cpp $(SOURCEDIR)halfskat.cpp -o $(PYOUT)

clean:
	rm -f $(OBJS) $(OUT) $(PYOUT)
