#version 460

layout(binding = 0) uniform sampler2D sprite_texture;
layout(binding = 1) uniform sampler2D sprite_texture2;

layout(location = 0) in vec2 texture_coords;
layout(location = 1) in vec3 spr_clr;

layout(location = 0) out vec4 output_colour;

void main()
{
	output_colour = vec4(spr_clr.xyz, 1.0f);
}
