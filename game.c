void
player_shoot(Vec2 *player_bullets, int *len)
{
	if (*len >= 256)
		return;

	player_bullets[*len] = player_data.pos;
	*len += 1;
}

void
spawn_enemy_bullet(Vec2 *enemy_bullets, int *len)
{
	if (*len >= 1024)
		return;

	enemy_bullets[*len] = (Vec2){ 200, 540 };
	*len += 1;
}

/*
 * Returns 1 if there is a collision and 0 otherwise
 */
int
collision_test(Vec2 a_pos, Vec2 a_dimensions, Vec2 b_pos, Vec2 b_dimensions)
{
	return 0;
}
