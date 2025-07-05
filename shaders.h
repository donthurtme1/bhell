#ifndef SHADERS_H
#define SHADERS_H

static const char *sth_vertex_src = "\
#version 460\n\
\n\
/*\n\
 * Uniform variables\n\
 */\n\
layout(binding = 0) uniform transform_matrix_uniform {\n\
	mat3 trans_matrix;\n\
};\n\
layout(binding = 1) uniform sprite_data_uniform {\n\
	vec2 sprite_positions[512];\n\
};\n\
layout(binding = 2) uniform tmp_sprite_colour {\n\
	vec3 spr_clr;\n\
};\n\
\n\
/*\n\
 * Vertex attributes\n\
 */\n\
layout(location = 0) in vec2 vertex_position;\n\
layout(location = 1) in vec2 texture_coords;\n\
\n\
layout(location = 0) out vec2 out_texcoords;\n\
layout(location = 1) out vec3 sprite_colour;\n\
\n\
void\n\
main()\n\
{\n\
	vec2 t_sprite_pos;\n\
	t_sprite_pos.x = float(sprite_positions[gl_InstanceID].x) * (2.0f / 400.0f) - 1.0f;\n\
	t_sprite_pos.y = float(sprite_positions[gl_InstanceID].y) * (2.0f / 540.0f) - 1.0f;\n\
	gl_Position = vec4(\n\
			vertex_position.x + t_sprite_pos.x,\n\
			vertex_position.y + t_sprite_pos.y,\n\
			0.0f, 1.0f);\n\
	out_texcoords = texture_coords;\n\
	sprite_colour = spr_clr;\n\
}\n\
";

static const char *sth_fragment_src = "\
#version 460\n\
\n\
layout(binding = 0) uniform sampler2D sprite_texture;\n\
layout(binding = 1) uniform sampler2D sprite_texture2;\n\
\n\
layout(location = 0) in vec2 texture_coords;\n\
layout(location = 1) in vec3 spr_clr;\n\
\n\
layout(location = 0) out vec4 output_colour;\n\
\n\
void main()\n\
{\n\
	output_colour = vec4(spr_clr.xyz, 1.0f);\n\
}\n\
";

#endif