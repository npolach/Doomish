all: openg

openg: openg.cpp
	g++ openg.cpp -Wall -lX11 -lGL -lGLU -lm ./libggfonts.a -o openg

clean:
	rm -f openg


