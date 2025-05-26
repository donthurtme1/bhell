#ifndef SHADERS_H
#define SHADERS_H

static const char *sth_vertex_src = "\
#version 460\n\
\n\
struct sprite_data {\n\
	vec2 position;\n\
	int tex;\n\
};\n\
\n\
layout(binding = 0) uniform sampler2D sprite_texture_1;\n\
layout(binding = 1) uniform sprite_data_uniform {\n\
	vec2 sprite_positions[2];\n\
};\n\
\n\
layout(location = 0) in vec2 vertex_position;\n\
layout(location = 1) in vec2 texture_coords;\n\
\n\
layout(location = 0) out vec4 frag_colour;\n\
\n\
void\n\
main()\n\
{\n\
	gl_Position = vec4(\n\
			vertex_position.x + sprite_positions[gl_InstanceID].x,\n\
			vertex_position.y + sprite_positions[gl_InstanceID].y,\n\
			0.0f, 1.0f);\n\
	frag_colour = texture(sprite_texture_1, texture_coords);\n\
}\n\
";

static const char *sth_fragment_src = "\
#version 460\n\
\n\
layout(location = 0) in vec4 frag_colour;\n\
\n\
layout(location = 0) out vec4 output_colour;\n\
\n\
void main()\n\
{\n\
	//output_colour = vec4(0.8f, 0.1f, 0.1f, 1.0f);\n\
	output_colour = frag_colour;\n\
}\n\
";

#endif