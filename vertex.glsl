#version 460

struct sprite_data {
	vec2 position;
	int tex;
};

layout(binding = 0) uniform sampler2D sprite_texture;

/*
 * Uniform variables
 */
layout(binding = 0) uniform transform_matrix_uniform {
	mat3 trans_matrix;
};
layout(binding = 1) uniform sprite_data_uniform {
	uvec4 sprite_positions[512];
	/* .x -> .xpos
	   .y -> .ypos
	   .z -> .width
	   .w -> .height */
};

/*
 * Vertex attributes
 */
layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 texture_coords;

layout(location = 0) out vec2 out_texcoords;

void
main()
{
	vec2 t_sprite_pos;
	t_sprite_pos.x = float(sprite_positions[gl_InstanceID].x >> 16) * (2.0f / 400.0f) - 1.0f;
	t_sprite_pos.y = float(sprite_positions[gl_InstanceID].y >> 16) * (2.0f / 540.0f) - 1.0f;
	gl_Position = vec4(
			vertex_position.x + t_sprite_pos.x,
			vertex_position.y + t_sprite_pos.y,
			0.0f, 1.0f);
	out_texcoords = texture_coords;
}
