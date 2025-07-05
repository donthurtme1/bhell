#version 460

/*
 * Uniform variables
 */
layout(binding = 0) uniform transform_matrix_uniform {
	mat3 trans_matrix;
};
layout(binding = 1) uniform sprite_data_uniform {
	vec2 sprite_positions[512];
};
layout(binding = 2) uniform tmp_sprite_colour {
	vec3 spr_clr;
};

/*
 * Vertex attributes
 */
layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 texture_coords;

layout(location = 0) out vec2 out_texcoords;
layout(location = 1) out vec3 sprite_colour;

void
main()
{
	vec2 t_sprite_pos;
	t_sprite_pos.x = float(sprite_positions[gl_InstanceID].x) * (2.0f / 400.0f) - 1.0f;
	t_sprite_pos.y = float(sprite_positions[gl_InstanceID].y) * (2.0f / 540.0f) - 1.0f;
	gl_Position = vec4(
			vertex_position.x + t_sprite_pos.x,
			vertex_position.y + t_sprite_pos.y,
			0.0f, 1.0f);
	out_texcoords = texture_coords;
	sprite_colour = spr_clr;
}
