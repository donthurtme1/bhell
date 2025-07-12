#ifndef _GAME_C
#define _GAME_C

#include <stdlib.h>

void
player_shoot(struct list_head *head)
{
	struct Bullet *new = malloc(sizeof(struct Bullet));
	new->pos = player_data.pos;
	list_add(&new->link, head->prev);
}

void
spawn_enemy_bullets(struct list_head *head, Vec2 spawn_pos, struct AttackData *attack_data)
{
	for (int i = 0;
			i < attack_data->bullet_count;
			i++)
	{
		struct Bullet *new = malloc(sizeof(struct Bullet));
		new->pos = spawn_pos;
		new->vel = attack_data->initial_velocities[i];
		list_add(&new->link, head->prev);
	}
}

void
spawn_enemy(struct list_head *head, Vec2 **preset_enemy_positions)
{
	struct Entity *new = malloc(sizeof(struct Entity));

	new->pos = (Vec2){ 0, 540 };
	new->vel = (Vec2){ 1.0f, -2.0f };
	new->accel = (Vec2){ 0, 0.008f };
	new->health = 10;
	new->fire_cooldown = 42;

	/* Quintuple spread shot */
	new->attack_data = malloc(sizeof(struct AttackData));
	new->attack_data->bullet_type = 0;
	new->attack_data->bullet_count = 5;
	new->attack_data->initial_velocities = malloc(sizeof(Vec2) * 5);
	new->attack_data->initial_accels = NULL;
	new->attack_data->initial_jerks = NULL;
	new->attack_data->initial_velocities[0] = (Vec2){ -0.595f, -2.222f };
	new->attack_data->initial_velocities[1] = (Vec2){ -0.3f,   -2.28f };
	new->attack_data->initial_velocities[2] = (Vec2){  0.0f,   -2.3f };
	new->attack_data->initial_velocities[3] = (Vec2){  0.3f,   -2.28f };
	new->attack_data->initial_velocities[4] = (Vec2){  0.595f, -2.222f };

	list_add(&new->link, head->prev);
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

#endif /* _GAME_C */
