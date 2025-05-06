#ifndef SHADERS_H
#define SHADERS_H

static const char *sth_vertex_src = "\
#version 460\n\
\n\
layout(binding = 0) uniform sprite_positions {\n\
	vec2 sprite_positions[64];\n\
};\n\
\n\
layout(location = 0) in vec2 vertex_position;\n\
\n\
void main()\n\
{\n\
	//gl_InstanceID\n\
	gl_Position = vec4(vertex_position, 0.0f, 1.0f);\n\
}\n\
";

static const char *sth_fragment_src = "\
#version 460\n\
\n\
layout(location = 0) out vec4 output_colour;\n\
\n\
void main()\n\
{\n\
	output_colour = vec4(0.8f, 0.1f, 0.1f, 1.0f);\n\
}\n\
";

#endif