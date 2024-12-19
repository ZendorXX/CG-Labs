CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic
LDFLAGS := -lsfml-graphics -lsfml-window -lsfml-system -lGLEW -lGL -lGLU

main: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o main.out $(LDFLAGS)

clean:
	rm -f *.out