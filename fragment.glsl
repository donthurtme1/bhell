#version 460

layout(binding = 0) uniform sampler2D sprite_texture;
layout(binding = 1) uniform sampler2D sprite_texture2;

layout(location = 0) in vec2 texture_coords;

layout(location = 0) out vec4 output_colour;

void main()
{
	output_colour = texture(sprite_texture, texture_coords);
	//output_colour = vec4(0.8f, 0.05f, 0.05f, 1.0f);
}
