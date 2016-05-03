render: render.c
	g++ -o render render.c -O2 -lGLU -lGL -lm -lglut -lOSMesa -lGLEW -lpng -lassimp -lIL -pthread -I. -I./util -I./DevIL/include -I./glm -g -O2 -MT render.o -MD -MP 
clean:
	rm render
	rm *.o
