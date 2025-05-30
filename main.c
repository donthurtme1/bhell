#include <openblas/cblas.h>
#include <stdio.h>
#include <time.h>
#include "shaders.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define die(msg) { \
	fprintf(stderr, "Err: line %d: " msg "\n", __LINE__); \
	exit(EXIT_FAILURE); \
}
#define LENGTH(a) (int)(sizeof(a) / sizeof(a[0]))
#define ALIGN(x) __attribute__((aligned(x)))

struct PlayerSpriteData {
	float xpos_offset, ypos_offset;
	float xscale_factor, yscale_factor;
	int texture_num; /* OpenGL always has at least 16 textures numbered 0 - 15 */
} ALIGN(8);

struct EnemySpriteData {
	float xpos_offset, ypos_offset;
	float xscale_factor, yscale_factor;
	int texture_num; /* OpenGL always has at least 16 textures numbered 0 - 15 */
} ALIGN(8);

static struct OptionData {
	int target_fps;
	float target_aspect_ratio;
} optn = {
	.target_fps = 100,
	.target_aspect_ratio = (float)10 / 16,
};

/*
 * Game stuff
 */
struct Projectile {
	short xpos, ypos; /* Origin at top left */
	short width, height; /* Size of hitbox */
	int projectile_id;
} ALIGN(8);

struct EntityData {
	short xpos, ypos; /* Origin at top left */
	short width, height; /* Size of hitbox */
} ALIGN(8);

static struct EntityData player = {
	.xpos = 0x7fff,
	.ypos = 0x7fff,
};

#include "logic.c"

extern struct Projectile *
collision_check(struct EntityData *player, struct Projectile *enemy_bullets, int n);

/*
 * Render the player and the player's projectiles
 */
void
draw_player_sprites(int nsprites)
{
	const uint8_t index_data[] = {
		0, 1, 2, 2, 3, 0
	};

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, index_data, nsprites);
}

void
draw_enemy_sprites(int nsprites)
{
	const uint8_t index_data[] = {
		0, 1, 2, 2, 3, 0
	};

	//glDrawElementsInstanced();
}

/*
 * Load and compile shaders,
 * Returns a GLuint representing a shader program.
 */
GLuint
createprogram()
{
	int compile_status = 0;
	int error_occured = 0;

	/*
	 * Create and compile shaders
	 */
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
		putc('\n', stderr);
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
		putc('\n', stderr);
		error_occured = 1;
	}

	int link_status;

	/*
	 * Create and link shader program
	 */
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
		putc('\n', stderr);
		error_occured = 1;
	}

	/* Cleanup */
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	if (error_occured > 0)
		exit(EXIT_FAILURE);

	return shader_program;
}

/*
 * Calls glCreateVertexArrays for @vertex_arrays
 */
int
create_sprite_arrays(GLuint *vertex_arrays, GLuint *vertex_buffers, int narrays)
{
	float vertex_data[] = {
		/* position									tex_coords */
		-0.02f, -0.02f * optn.target_aspect_ratio,	0.0f, 1.0f,
		 0.02f, -0.02f * optn.target_aspect_ratio,	1.0f, 1.0f,
		 0.02f,  0.02f * optn.target_aspect_ratio,	1.0f, 0.0f,
		-0.02f,  0.02f * optn.target_aspect_ratio,	0.0f, 0.0f,
	};

	glCreateVertexArrays(narrays, vertex_arrays);
	glGenBuffers(narrays, vertex_buffers);
	for (int i = 0; i < narrays; i++)
	{
		glBindVertexArray(vertex_arrays[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof vertex_data, vertex_data, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)8);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	/*
	 * Setup SDL and glut
	 */
	int sdl_result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_Window *window = SDL_CreateWindow("sprites", 0, 0,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == NULL)
		die("Failed to create window");

	SDL_GLContext context = SDL_GL_CreateContext(window);
	int mkcurrent_result = SDL_GL_MakeCurrent(window, context);
	if (mkcurrent_result < 0)
		die("Failed to make context current");

	glutInit(&argc, argv);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);

	struct { int width, height; } winsize;
	SDL_GetWindowSize(window, &winsize.width, &winsize.height);
	glViewport(0, 0, 1080, 1920);
	glScissor(0, 0, 1080, 1920);

	/*
	 * Create shader program and vertex arrays
	 */
	GLuint shader_program = createprogram();
	glUseProgram(shader_program);
	GLuint vertex_arrays[2], vertex_buffers[2];
	create_sprite_arrays(vertex_arrays, vertex_buffers, 2);

	//GLuint render_framebuffer, output_framebuffer;
	//glGenFramebuffers(1, &render_framebuffer);
	//glGenFramebuffers(1, &output_framebuffer);

	/*
	 * Create uniform buffers
	 */
	GLuint transform_matrix_ubuf;
	float transform_matrix[9] = {
		2.0f / 21599.0f, 0, -1.0f,
		0, 2.0f / 34559.0f, -1.0f,
		0, 0, 0,
	};
	glGenBuffers(1, &transform_matrix_ubuf);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_matrix_ubuf);
	glNamedBufferData(transform_matrix_ubuf, sizeof transform_matrix, transform_matrix, GL_STATIC_DRAW);

	GLuint player_position_ubuf;
	float player_data[] = { 0.0f, 0.0f };
	glGenBuffers(1, &player_position_ubuf);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, player_position_ubuf);
	glNamedBufferData(player_position_ubuf, sizeof player_data, player_data, GL_DYNAMIC_DRAW);

	/*
	 * Load textures
	 */
	int texwidth[2], texheight[2], texchannels[2];
	unsigned char *texture_data[2];
	texture_data[0] = stbi_load("/home/basil/images/gameboy-flower.png",
			&texwidth[0], &texheight[0], &texchannels[0], 0);
	texture_data[1] = stbi_load("/home/basil/images/gameboy-flower.png",
			&texwidth[1], &texheight[1], &texchannels[1], 1);

	GLuint textures[2];
	glGenTextures(2, textures);
	for (int i = 0; i < 2; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texwidth[i], texheight[i],
				0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data[i]);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	/*
	 * Create projectile
	 */
	struct Projectile bullet = {
		.xpos = 0x0800, .ypos = 0x0800,
		.width = 0x0100, .height = 0x0100,
	};

	/*
	 * Main loop
	 */
	float move_diff[2] = { 0.0f, 0.0f };
	int input_dirs = 0;

	struct timespec monotime;
	clock_gettime(CLOCK_MONOTONIC, &monotime);

	int run = 1;
	int frame_time = 1000000000 / optn.target_fps;
	while (run > 0) {
		/* Input */
		static SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					run = 0;
					break;
				case SDL_EVENT_WINDOW_RESIZED:
					SDL_GetWindowSize(window, &winsize.width, &winsize.height);

					float current_aspect_ratio = (float)winsize.width / winsize.height;
					if (current_aspect_ratio == optn.target_aspect_ratio) {
						glViewport(0, 0, winsize.width, winsize.height);
						glScissor(0, 0, winsize.width, winsize.height);
					}
					else if (current_aspect_ratio > optn.target_aspect_ratio) {
						int width = winsize.height * optn.target_aspect_ratio;
						int offset = (winsize.width - width) / 2;
						glViewport(offset, 0, width, winsize.height);
						glScissor(offset, 0, width, winsize.height);
					}
					else {
						int height = winsize.width / optn.target_aspect_ratio;
						int offset = (winsize.height - height) / 2;
						glViewport(0, offset, winsize.width, height);
						glScissor(0, offset, winsize.width, height);
					}
					break;
				case SDL_EVENT_KEY_DOWN:
					switch (event.key.key) {
						case SDLK_S:
							input_dirs |= 1;
							move_diff[0] = -1.0f;
							break;
						case SDLK_F:
							input_dirs |= 2;
							move_diff[0] = 1.0f;
							break;
						case SDLK_D:
							input_dirs |= 4;
							move_diff[1] = -1.0f * optn.target_aspect_ratio;
							break;
						case SDLK_A:
							input_dirs |= 8;
							move_diff[1] = 1.0f * optn.target_aspect_ratio;
							break;
					}
					break;
				case SDL_EVENT_KEY_UP:
					switch (event.key.key) {
						case SDLK_S:
							input_dirs &= ~1;
							if (input_dirs & 2)
								move_diff[0] = 1.0f;
							else
								move_diff[0] = 0.0f;
							break;
						case SDLK_F:
							input_dirs &= ~2;
							if (input_dirs & 1)
								move_diff[0] = -1.0f;
							else
								move_diff[0] = 0.0f;
							break;
						case SDLK_D:
							input_dirs &= ~4;
							if (input_dirs & 8)
								move_diff[1] = 1.0f * optn.target_aspect_ratio;
							else
								move_diff[1] = 0.0f;
							break;
						case SDLK_A:
							input_dirs &= ~8;
							if (input_dirs & 4)
								move_diff[1] = -1.0f * optn.target_aspect_ratio;
							else
								move_diff[1] = 0.0f;
							break;
					}
					break;
			}
		}

		/* Game */
		float velocity = 0.008f;
		player_data[0] += move_diff[0] * velocity;
		player_data[1] += move_diff[1] * velocity;
		glNamedBufferData(player_position_ubuf, sizeof player_data, player_data, GL_DYNAMIC_DRAW);

		collision_check(&player, &bullet, 1);

		/*
		 * Render
		 */
		glDisable(GL_SCISSOR_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_framebuffer);
		draw_player_sprites(1);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		/* Copy contents from GL_READ_FRAMEBUFFER to GL_DRAW_FRAMEBUFFER
		 * scaled up by 2 times */
		//glBindFramebuffer(GL_READ_FRAMEBUFFER, render_framebuffer);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, output_framebuffer);
		//glBlitFramebuffer(0, 0, 304, 540, 0, 0, 608, 1080,
		//		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		SDL_GL_SwapWindow(window);

		/* Frame advance */
		monotime.tv_nsec += frame_time;
		if (monotime.tv_nsec >= 1000000000) {
			monotime.tv_nsec -= 1000000000;
			monotime.tv_sec++;
		}
		while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &monotime, NULL) > 0);
	}

	SDL_Quit();
	return 0;
}
