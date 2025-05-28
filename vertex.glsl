#version 460

struct sprite_data {
	vec2 position;
	int tex;
};

layout(binding = 0) uniform sampler2D sprite_texture[2];
layout(binding = 1) uniform sprite_data_uniform {
	vec2 sprite_positions[2];
};

layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 texture_coords;

layout(location = 0) out vec4 frag_colour;

void
main()
{
	gl_Position = vec4(
			vertex_position.x + sprite_positions[gl_InstanceID].x,
			vertex_position.y + sprite_positions[gl_InstanceID].y,
			0.0f, 1.0f);
	frag_colour = texture(sprite_texture[0], texture_coords);
}
