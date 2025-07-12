#include <openblas/cblas.h>
#include <stdio.h>
#include <time.h>
#include "shaders.h"
#include "list.h"

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

typedef struct {
	float x, y; /* Origin at bottom left */
} ALIGN(8) Vec2;

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
	} mov_keys;
	Vec2 mov_dir;
	int shoot;
} input_state;

/*
 * Game stuff
 */
struct AttackData {
	int bullet_type;
	int bullet_count;

	Vec2 *initial_velocities;
	Vec2 *initial_accels;
	Vec2 *initial_jerks;
};

struct Entity {
	struct list_head link;

	Vec2 pos;
	Vec2 vel;
	Vec2 accel;

	int health;
	int fire_cooldown;
	struct AttackData *attack_data;
};

struct Bullet {
	struct list_head link;
	Vec2 pos;
	Vec2 vel;
};

/*
 * Gameplay variables
 */
struct Entity player_data = {
	.pos = { 200, 270 },
	.vel = { 0, 0 },
	.health = 100,
	.fire_cooldown = 0,
};

struct list_head playerbullets = LIST_HEAD_INIT(playerbullets);
struct list_head enemybullets = LIST_HEAD_INIT(enemybullets);
struct list_head enemies = LIST_HEAD_INIT(enemies);

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

	struct { int width, height; } win;
	SDL_GetWindowSize(window, &win.width, &win.height);
	glViewport(0, 0, 1080, 1920);
	glScissor(0, 0, 1080, 1920);

	/*
	 * Create shader program and vertex arrays
	 */
	GLuint shader_program = createprogram();
	glUseProgram(shader_program);

	GLuint player_sprite_varray, player_sprite_vbuf,
		   bullet_sprite_varray, bullet_sprite_vbuf,
		   enemy_sprite_varray, enemy_sprite_vbuf;
	create_sprite_arrays(&player_sprite_varray, &player_sprite_vbuf, 8);
	create_sprite_arrays(&bullet_sprite_varray, &bullet_sprite_vbuf, 4);
	create_sprite_arrays(&enemy_sprite_varray, &enemy_sprite_vbuf, 14);

	/*
	 * Create uniform buffers
	 */
	GLuint player_position_ubuf, player_bullets_ubuf;
	glGenBuffers(1, &player_position_ubuf);
	glGenBuffers(1, &player_bullets_ubuf);

	GLuint enemy_positions_ubuf, enemy_bullets_ubuf;
	glGenBuffers(1, &enemy_positions_ubuf);
	glGenBuffers(1, &enemy_bullets_ubuf);

	/* Static uniform buffers */
	GLuint player_colour_ubuf;
	GLuint enemy_colour_ubuf; /* enemy bullets */
	float colour_pine[3] = { 0.243f, 0.561f, 0.69f };
	glGenBuffers(1, &player_colour_ubuf);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, player_colour_ubuf);
	glNamedBufferData(player_colour_ubuf, sizeof(colour_pine), colour_pine, GL_STATIC_DRAW);
	float colour_rose[3] = { 0.922f, 0.435f, 0.573f };
	glGenBuffers(1, &enemy_colour_ubuf);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, enemy_colour_ubuf);
	glNamedBufferData(enemy_colour_ubuf, sizeof(colour_rose), colour_rose, GL_STATIC_DRAW);

	/*
	 * Initial setup
	 */
	//spawn_enemy(enemy_array, NULL, &n_enemies);
	//enemy_entity_positions[n_enemies - 1] = enemy_array[n_enemies - 1]->pos;

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

		if (player_data.fire_cooldown > 0) {
			player_data.fire_cooldown--;
		}
		else if (input_state.shoot > 0) {
			player_shoot(&playerbullets);
			player_data.fire_cooldown = 6;
		}

		/*
		 * Level stuff
		 */
		static int level_frame_count = 0;
		if (level_frame_count < 120)
			level_frame_count++;
		else {
			spawn_enemy(&enemies, NULL);
			level_frame_count = 0;
		}

		/*
		 * Physics calculations
		 */
		player_data.pos.x += player_data.vel.x;
		player_data.pos.y += player_data.vel.y;

		/* Update player bullets */
		for_each_bullet(bullet, &playerbullets)
		{
			if (bullet->pos.y + 6.8f > 540.0f) {
				list_del(&bullet->link);
				free(bullet);
				continue;
			}

			bullet->pos.y += 6.8f;
		}

		/* Update enemy entities */
		for_each_entity(enemy, &enemies)
		{
			enemy->vel.x += enemy->accel.x;
			enemy->vel.y += enemy->accel.y;
			enemy->pos.x += enemy->vel.x;
			enemy->pos.y += enemy->vel.y;

			if (enemy->fire_cooldown > 0)
				enemy->fire_cooldown -= 1;
			else {
				spawn_enemy_bullets(&enemybullets, enemy->pos, enemy->attack_data);
				enemy->fire_cooldown = 50;
			}

			if (enemy->pos.x > 400 || enemy->pos.x < 0 ||
					enemy->pos.y > 540 || enemy->pos.y < 0)
			{
				list_del(&enemy->link);
				free(enemy);
			}

			/* Test bullet collision */
			for_each_bullet(bullet, &playerbullets)
			{
				Vec2 enemy_box = { 14, 14 };
				Vec2 bullet_box = { 4, 4 };
				if (collision_test(enemy->pos, enemy_box, bullet->pos, bullet_box) == 0)
					continue;

				/* Damage calculations */
				enemy->health -= 1;
				if (enemy->health <= 0) {
					list_del(&enemy->link);
					free(enemy);
				}

				list_del(&bullet->link);
				free(bullet);
			}
		}

		/* Update enemy bullets */
		for_each_bullet(bullet, &enemybullets)
		{
			bullet->pos.x += bullet->vel.x;
			bullet->pos.y += bullet->vel.y;

			if (bullet->pos.x > 400.0f || bullet->pos.x < 0.0f ||
					bullet->pos.y > 540.0f || bullet->pos.y < 0.0f)
			{
				list_del(&bullet->link);
				free(bullet);
				continue;
			}
		}

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
		glNamedBufferData(player_position_ubuf, sizeof(Vec2), &player_data.pos, GL_DYNAMIC_DRAW);
		draw_sprites(player_sprite_varray, player_colour_ubuf, player_position_ubuf, 1);
		draw_bullets(&playerbullets, player_bullets_ubuf, player_colour_ubuf, bullet_sprite_varray);

		draw_entities(&enemies, enemy_positions_ubuf, enemy_colour_ubuf, enemy_sprite_varray);
		draw_bullets(&enemybullets, enemy_bullets_ubuf, enemy_colour_ubuf, bullet_sprite_varray);
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
