#version 460

layout(binding = 0) uniform sprite_positions {
	vec2 sprite_positions[64];
};

layout(location = 0) in vec2 vertex_position;

void main()
{
	sprite_positions[gl_InstanceID];
	gl_Position = vec4(vertex_position, 0.0f, 1.0f);
}
