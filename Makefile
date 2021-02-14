mandelbrot: mandelbrot.cpp
	g++ -o mandelbrot mandelbrot.cpp -pthread -lSDL2 -lGLEW -lX11 -lGLU -lGL
