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

static struct OptionData {
	int target_fps;
	float target_aspect_ratio;
} optn = {
	.target_fps = 100,
	.target_aspect_ratio = (float)40 / 54,
};

/*
 * Global input state
 */
struct InputState {
	enum {
		DIR_LEFT = 1,
		DIR_RIGHT = 2,
		DIR_DOWN = 4,
		DIR_UP = 8,
	} movement;
	int shoot;
} input_state;

/*
 * Game stuff
 */
typedef struct {
	float x, y; /* Origin at bottom left */
} ALIGN(8) Vec2;

struct Entity {
	Vec2 pos;
	Vec2 vel;
	int health;
	int fire_cooldown;
};

/*
 * Player variables
 */
struct Entity player_data = {
	.pos = { 200, 270 },
	.vel = { 0, 0 },
	.health = 100,
	.fire_cooldown = 0,
};
Vec2 player_bullets[256];
int n_player_bullets = 0;

/*
 * Enemy variables
 */
struct Entity enemy_array[64];
Vec2 enemy_bullets[1024];
int n_enemy_bullets = 0;

#include "input.c"
#include "game.c"
#include "render.c"

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

	GLuint player_sprite_varray, player_sprite_vbuf,
		   bullet_sprite_varray, bullet_sprite_vbuf;
	create_sprite_arrays(&player_sprite_varray, &player_sprite_vbuf, 8);
	create_sprite_arrays(&bullet_sprite_varray, &bullet_sprite_vbuf, 4);

	/*
	 * Create uniform buffers
	 */
	GLuint player_position_ubuf;
	glGenBuffers(1, &player_position_ubuf);
	glNamedBufferData(player_position_ubuf, sizeof player_data, &player_data, GL_DYNAMIC_DRAW);

	GLuint enemy_bullets_ubuf;
	glGenBuffers(1, &enemy_bullets_ubuf);
	glNamedBufferData(enemy_bullets_ubuf, 4 * sizeof(enemy_bullets[0]), enemy_bullets, GL_DYNAMIC_DRAW);

	GLuint player_bullets_ubuf;
	glGenBuffers(1, &player_bullets_ubuf);
	glNamedBufferData(player_bullets_ubuf, n_player_bullets, player_bullets, GL_DYNAMIC_DRAW);

	GLuint player_colour_ubuf;
	float colour_rosepine[3];
	colour_rosepine[0] = 0.24f;
	colour_rosepine[1] = 0.56f;
	colour_rosepine[2] = 0.69f;
	glGenBuffers(1, &player_colour_ubuf);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, player_colour_ubuf);
	glNamedBufferData(player_colour_ubuf, sizeof(colour_rosepine), colour_rosepine, GL_STATIC_DRAW);

	/*
	 * Load textures
	 */
	int texwidth[2], texheight[2], texchannels[2];
	unsigned char *texture_data[2];
	texture_data[0] = stbi_load("/home/basil/images/gameboy-flower.png",
			&texwidth[0], &texheight[0], &texchannels[0], 0);
	texture_data[1] = stbi_load("",
			&texwidth[1], &texheight[1], &texchannels[1], 0);

	GLuint textures[2];
	glGenTextures(2, textures);
	for (int i = 0; i < 1; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texwidth[i], texheight[i],
				0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data[i]);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	/*
	 * Main loop
	 */
	int move_diff[2] = { 0, 0 };
	int input_dirs = 0;

	struct timespec monotime;
	clock_gettime(CLOCK_MONOTONIC, &monotime);

	int frame_time = 1000000000 / optn.target_fps;
	while (1) {
		/*
		 * Input
		 */
		static SDL_Event event;
		while (SDL_PollEvent(&event)) {
			int event_result = handle_sdl_event(event, window);
			if (event_result < 0)
				goto end_main_loop;
		}

		static int shoot_cooldown = 0;
		if (input_state.shoot > 0 && shoot_cooldown == 0) {
			player_shoot(player_bullets, &n_player_bullets);
			shoot_cooldown = 6;
		}
		else if (shoot_cooldown > 0) {
			shoot_cooldown--;
		}

		/*
		 * Physics calculations
		 */
		player_data.pos.x += player_data.vel.x;
		player_data.pos.y += player_data.vel.y;

		/* Update player bullets */
		for (int i = 0;
				i < n_player_bullets &&
				i < 256; /* `n_bullets` should never exceed 256 */
				i++)
		{
			if (player_bullets[i].y + 5.2f > 540.0f) {
				n_player_bullets -= 1;
				player_bullets[i] = player_bullets[n_player_bullets];
			}

			player_bullets[i].y += 5.2f;
		}

		/* Update enemy bullets */
		static int enemy_bullet_cooldown = 0;
		if (enemy_bullet_cooldown > 0)
			enemy_bullet_cooldown--;
		else {
			spawn_enemy_bullet(enemy_bullets, &n_enemy_bullets);
			enemy_bullet_cooldown = 60;
		}
		for (int i = 0;
				i < n_enemy_bullets &&
				i < 1024; /* `n_enemy_bullets` should never exceed 1024 */
				i++)
		{
			enemy_bullets[i].y += -1.1f;
		}

		/*
		 * Update uniform buffers
		 */
		glNamedBufferData(player_position_ubuf, 8, &player_data.pos, GL_DYNAMIC_DRAW);
		glNamedBufferData(enemy_bullets_ubuf, n_enemy_bullets * 8, enemy_bullets, GL_DYNAMIC_DRAW);
		glNamedBufferData(player_bullets_ubuf, n_player_bullets * 8, player_bullets, GL_DYNAMIC_DRAW);

		/*
		 * Render
		 */
		glDisable(GL_SCISSOR_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(0.1f, 0.09f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*
		 * TODO: Copy contents from draw framebuffer to render framebuffer
		 * scaled up by 2 times to create a pixelated effect.
		 */
		/* Render player and enemies */
		draw_sprites(player_sprite_varray, 1, player_position_ubuf, player_colour_ubuf);
		/* Render bullets */
		draw_sprites(bullet_sprite_varray, n_enemy_bullets, enemy_bullets_ubuf, player_colour_ubuf);
		draw_sprites(bullet_sprite_varray, n_player_bullets, player_bullets_ubuf, player_colour_ubuf);
		SDL_GL_SwapWindow(window);

		/* Frame advance */
		monotime.tv_nsec += frame_time;
		if (monotime.tv_nsec >= 1000000000) {
			monotime.tv_nsec -= 1000000000;
			monotime.tv_sec++;
		}
		while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &monotime, NULL) > 0);
	}
end_main_loop:

	SDL_Quit();
	return 0;
}
