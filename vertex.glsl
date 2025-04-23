#version 460

layout(location = 0) in vec2 vertex_position;

void main()
{
	gl_Position = vec4(vertex_position, 0.0f, 1.0f);
}
