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
	vec2 sprite_positions;
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
	vec3 t_sprite_pos = trans_matrix * vec3(sprite_positions, 1.0f);
	gl_Position = vec4(
			vertex_position.x + t_sprite_pos.x,
			vertex_position.y + t_sprite_pos.y,
			0.0f, 1.0f);
	out_texcoords = texture_coords;
}
