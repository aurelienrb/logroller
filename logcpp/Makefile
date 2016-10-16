CXX=g++
CXXFLAGS=-W -Wall -ansi -pedantic
LDFLAGS=
EXEC=test_raii

all: $(EXEC)

test_raii: Resource.o ResourceManager.o main.o
	$(CXX) -o test_raii Resource.o ResourceManager.o main.o $(LDFLAGS)

Resource.o: ../Resource.cpp ../Resource.h
	$(CXX) -o Resource.o -c ../Resource.cpp $(CXXFLAGS)

ResourceManager.o: ../ResourceManager.cpp ../ResourceManager.h ../Resource.h
	$(CXX) -o ResourceManager.o -c ../ResourceManager.cpp $(CXXFLAGS)

main.o: ../main.cpp ../ResourceManager.h ../Resource.h
	$(CXX) -o main.o -c ../main.cpp $(CXXFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
