#version 460

layout(location = 0) in vec4 frag_colour;

layout(location = 0) out vec4 output_colour;

void main()
{
	//output_colour = vec4(0.8f, 0.1f, 0.1f, 1.0f);
	output_colour = frag_colour;
}
