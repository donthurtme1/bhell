LIBS = `pkg-config --libs gl glut sdl3 cblas` -lm
SHADERS = vertex.glsl fragment.glsl
WINDOWS = -mwindows -mwin32

make:
	sth -o shaders.h $(SHADERS)
	gcc -o sprites main.c $(LIBS)

windows:
	sth -o shaders.h $(SHADERS)
	gcc -o sprites.exe main.c $(LIBS) $(WINDOWS)
