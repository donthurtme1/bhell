/*
 * Render similar sprites
 */
void
draw_sprites(GLuint vertex_array_obj, int nsprites,
		GLuint position_array_ubuf, GLuint colour_ubuf)
{
	if (nsprites < 0)
		return;

	const uint8_t index_data[] = {
		0, 1, 2, 2, 3, 0
	};

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, position_array_ubuf);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, colour_ubuf);
	glBindVertexArray(vertex_array_obj);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, index_data, nsprites);
}

/*
 * Load and compile shaders,
 * Returns a GLuint representing a shader program.
 */
GLuint
createprogram()
{
	int compile_status = 0;
	int error_occured = 0;

	/*
	 * Create and compile shaders
	 */
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &sth_vertex_src, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		int len = 0;
		char infolog[128];
		glGetShaderInfoLog(vertex_shader, sizeof infolog, &len, infolog);
		fwrite("Vertex shader: ", sizeof(char), 15, stderr);
		fwrite(infolog, sizeof(char), len, stderr);
		putc('\n', stderr);
		error_occured = 1;
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &sth_fragment_src, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		int len = 0;
		char infolog[128];
		glGetShaderInfoLog(fragment_shader, sizeof infolog, &len, infolog);
		fwrite("Fragment shader: ", sizeof(char), 17, stderr);
		fwrite(infolog, sizeof(char), len, stderr);
		putc('\n', stderr);
		error_occured = 1;
	}

	/*
	 * Create and link shader program
	 */
	int link_status;
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glGetProgramiv(shader_program, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE)
	{
		int len = 0;
		char infolog[128];
		glGetShaderInfoLog(fragment_shader, sizeof infolog, &len, infolog);
		fwrite("Shader Program: ", sizeof(char), 16, stderr);
		fwrite(infolog, sizeof(char), len, stderr);
		putc('\n', stderr);
		error_occured = 1;
	}

	/* Cleanup */
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	if (error_occured > 0)
		exit(EXIT_FAILURE);

	return shader_program;
}

/*
 * Calls glCreateVertexArrays for `vertex_arrays`
 */
int
create_sprite_arrays(GLuint *vertex_arrays, GLuint *vertex_buffers, int size)
{
	float vertex_data[] = {
		/* position														tex_coords */
		-0.0025f * size, -0.0025f * optn.target_aspect_ratio * size,	0.0f, 1.0f,
		 0.0025f * size, -0.0025f * optn.target_aspect_ratio * size,	1.0f, 1.0f,
		 0.0025f * size,  0.0025f * optn.target_aspect_ratio * size,	1.0f, 0.0f,
		-0.0025f * size,  0.0025f * optn.target_aspect_ratio * size,	0.0f, 0.0f,
	};

	glCreateVertexArrays(1, vertex_arrays);
	glGenBuffers(1, vertex_buffers);

	glBindVertexArray(*vertex_arrays);
	glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffers);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertex_data, vertex_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)8);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	return 0;
}
