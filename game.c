#include <stdlib.h>

void
player_shoot(Vec2 *player_bullets, int *len)
{
	if (*len >= 256)
		return;

	player_bullets[*len] = player_data.pos;
	(*len) += 1;
}

void
spawn_enemy_bullets(Vec2 *bullet_pos, Vec2 *bullet_vel, int *n,
		Vec2 init_pos, struct AttackData *attack_data)
{
	if (*n + attack_data->num_bullets >= 1024)
		return;

	for (int i = 0; i < attack_data->num_bullets; i++)
	{
		bullet_pos[*n].x = init_pos.x;
		bullet_pos[*n].y = init_pos.y;
		bullet_vel[*n].x = attack_data->initial_velocities[i].x;
        bullet_vel[*n].y = attack_data->initial_velocities[i].y;
		(*n) += 1;
	}
}

void
spawn_enemy(struct Entity *enemies[], Vec2 **preset_enemy_positions, int *n)
{
	enemies[*n] = malloc(sizeof(struct Entity));

	enemies[*n]->pos = (Vec2){ 0, 500 };
	enemies[*n]->vel = (Vec2){ 0.8f, -2.8f };
	enemies[*n]->accel = (Vec2){ 0, 0.014f };
	enemies[*n]->health = 10;
	enemies[*n]->fire_cooldown = 42;

	/* Quintuple spread shot */
	enemies[*n]->attack_data = malloc(sizeof(struct AttackData));
	enemies[*n]->attack_data->bullet_type = 0;
	enemies[*n]->attack_data->num_bullets = 5;
	enemies[*n]->attack_data->initial_velocities = malloc(sizeof(Vec2) * 5);
	enemies[*n]->attack_data->initial_accels = NULL;
	enemies[*n]->attack_data->initial_jerks = NULL;
	enemies[*n]->attack_data->initial_velocities[0] = (Vec2){ -0.595f, -2.222f };
	enemies[*n]->attack_data->initial_velocities[1] = (Vec2){ -0.3f,   -2.28f };
	enemies[*n]->attack_data->initial_velocities[2] = (Vec2){  0.0f,   -2.3f };
	enemies[*n]->attack_data->initial_velocities[3] = (Vec2){  0.3f,   -2.28f };
	enemies[*n]->attack_data->initial_velocities[4] = (Vec2){  0.595f, -2.222f };

	(*n) += 1;
	//(*preset_enemy_positions) += 1;
}

void
remove_enemy(struct Entity *enemies[], int enemy_offset, int *n)
{
	//free(enemies[enemy_offset]->attack_data->initial_velocities);
	//free(enemies[enemy_offset]->attack_data);

	if (enemy_offset < *n - 1) {
		free(enemies[enemy_offset]);
		enemies[enemy_offset] = enemies[*n - 1];
		memset(enemies[*n - 1], 0, sizeof(enemies[*n - 1]));
	}
	*n -= 1;
}

/*
 * Returns 1 if there is a collision and 0 otherwise
 */
int
collision_test(Vec2 a_pos, Vec2 a_dimensions, Vec2 b_pos, Vec2 b_dimensions)
{
	if (a_pos.x + (0.5f * a_dimensions.x) < b_pos.x - (0.5f * b_dimensions.x))
		return 0;
	if (a_pos.x - (0.5f * a_dimensions.x) > b_pos.x + (0.5f * b_dimensions.x))
		return 0;
	if (a_pos.y + (0.5f * a_dimensions.y) < b_pos.y - (0.5f * b_dimensions.y))
		return 0;
	if (a_pos.y - (0.5f * a_dimensions.y) > b_pos.y + (0.5f * b_dimensions.y))
		return 0;
	return 1;
}
