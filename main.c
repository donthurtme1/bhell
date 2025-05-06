#include <stdio.h>
#include <time.h>
#include "shaders.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <SDL3/SDL.h>

#define die(msg) { \
	fprintf(stderr, "Err: line %d: " msg "\n", __LINE__); \
	exit(EXIT_FAILURE); \
}

static struct {
	int target_fps;
} optn = {
	.target_fps = 30, /* Only needs to be small because pokemon game lol */
};

static __attribute__((aligned(8))) struct {
	int xpos, ypos;
} player = {
	.xpos = 0,
	.ypos = 0,
};

/* Points to a list of array buffers to draw  */
static GLuint *render_queue;

void
renderscene(GLuint *vertex_array)
{
	GLuint sprite_position_uniform_buffer;

	glBindVertexArray(vertex_array[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void *)0);
	glBindVertexArray(vertex_array[1]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void *)0);

	/*
	 * Draw every sprite with seperate render call
	 * TODO: minimize amount of render calls
	 * glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, render_queue, render_queue_len);
	 */
	//for (int i = 0;
	//		render_queue == 0;
	//		i++)
	//{
	//	size_t render_queue_len = 1;
	//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void *)0);
	//	break;
	//}
}

/*
 * Load and compile shaders,
 * Returns a GLuint representing a shader program
 */
GLuint
createprogram()
{
	int compile_status = 0;
	int error_occured = 0;

	/* Create and compile shaders */
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &sth_vertex_src, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		int len = 0;
		char infolog[128];
		glGetShaderInfoLog(vertex_shader, sizeof infolog, &len, infolog);
		fwrite("Vertex shader: ", sizeof(char), 15, stderr);
		fwrite(infolog, sizeof(char), len, stderr);
		error_occured = 1;
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &sth_fragment_src, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		int len = 0;
		char infolog[128];
		glGetShaderInfoLog(fragment_shader, sizeof infolog, &len, infolog);
		fwrite("Fragment shader: ", sizeof(char), 17, stderr);
		fwrite(infolog, sizeof(char), len, stderr);
		error_occured = 1;
	}

	int link_status;

	/* Create and link shader program */
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glGetProgramiv(shader_program, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE)
	{
		int len = 0;
		char infolog[128];
		glGetShaderInfoLog(fragment_shader, sizeof infolog, &len, infolog);
		fwrite("Shader Program: ", sizeof(char), 16, stderr);
		fwrite(infolog, sizeof(char), len, stderr);
		error_occured = 1;
	}

	/* Cleanup */
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	if (error_occured > 0)
		exit(EXIT_FAILURE);

	return shader_program;
}

int
main(int argc, char *argv[])
{
	/* Setup SDL and glut */
	int sdl_result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_Window *window = SDL_CreateWindow("sprites", 0, 0,
			SDL_WINDOW_OPENGL |
			SDL_WINDOW_RESIZABLE);
	if (window == NULL)
		die("Failed to create window");

	SDL_GLContext context = SDL_GL_CreateContext(window);
	int mkcurrent_result = SDL_GL_MakeCurrent(window, context);
	if (mkcurrent_result < 0)
		die("Failed to make context current");

	glutInit(&argc, argv);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	GLuint shader_program = createprogram();
	glUseProgram(shader_program);

	/* Create vertex arrays */
	GLuint vertex_array[2];
	glCreateVertexArrays(2, vertex_array);
	glBindVertexArray(vertex_array[0]);

	float vertex_data[] = {
		-0.1f, -0.1f,
		 0.1f, -0.1f,
		 0.1f,  0.1f,
		-0.1f,  0.1f,
	};
	GLuint vertex_buffer[2];
	glGenBuffers(2, vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertex_data, vertex_data, GL_STATIC_DRAW);

	uint8_t index_data[] = {
		0, 1, 2, 2, 3, 0
	};
	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof index_data, index_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);


	glBindVertexArray(vertex_array[1]);
	float vertex_data2[] = {
		-0.1f + 0.25, -0.1f + 0.25,
		 0.1f + 0.25, -0.1f + 0.25,
		 0.1f + 0.25,  0.1f + 0.25,
		-0.1f + 0.25,  0.1f + 0.25,
	};
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertex_data2, vertex_data2, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	struct timespec monotime = { .tv_sec = 0, .tv_nsec = 1000000000 / optn.target_fps };
	int run = 1;
	while (run > 0) {
		/* Input */
		static SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					run = 0;
					break;
			}
		}

		/* Game */

		/* Render */
		static int w, h;
		SDL_GetWindowSize(window, &w, &h);
		glViewport(0, 0, w, h);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderscene(vertex_array);
		SDL_GL_SwapWindow(window);

		/* Frame advance */
		monotime.tv_nsec += (1000000000 / optn.target_fps);
		if (monotime.tv_nsec >= 1000000000) {
			monotime.tv_nsec -= 1000000000;
			monotime.tv_sec++;
		}
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &monotime, NULL);
	}

	free(render_queue);
	SDL_Quit();
	return 0;
}
