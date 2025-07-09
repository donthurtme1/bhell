int
handle_sdl_event(SDL_Event event, SDL_Window *window)
{
	switch (event.type)
	{
		case SDL_EVENT_QUIT:
			return -1;
		case SDL_EVENT_WINDOW_RESIZED:
			struct { int width, height; } winsize;
			SDL_GetWindowSize(window, &winsize.width, &winsize.height);

			float current_aspect_ratio = (float)winsize.width / winsize.height;
			if (current_aspect_ratio == optn.target_aspect_ratio) {
				glViewport(0, 0, winsize.width, winsize.height);
				glScissor(0, 0, winsize.width, winsize.height);
			}
			else if (current_aspect_ratio > optn.target_aspect_ratio) {
				int width = winsize.height * optn.target_aspect_ratio;
				int offset = (winsize.width - width) / 2;
				glViewport(offset, 0, width, winsize.height);
				glScissor(offset, 0, width, winsize.height);
			}
			else {
				int height = winsize.width / optn.target_aspect_ratio;
				int offset = (winsize.height - height) / 2;
				glViewport(0, offset, winsize.width, height);
				glScissor(0, offset, winsize.width, height);
			}
			break;
		case SDL_EVENT_KEY_DOWN:
			switch (event.key.key) {
				case SDLK_S:
					input_state.mov_keys |= 1;
					player_data.vel.x = -1.5f;
					break;
				case SDLK_F:
					input_state.mov_keys |= 2;
					player_data.vel.x = 1.5f;
					break;
				case SDLK_D:
					input_state.mov_keys |= 4;
					player_data.vel.y = -1.5f;
					break;
				case SDLK_A:
					input_state.mov_keys |= 8;
					player_data.vel.y = 1.5f;
					break;

				case SDLK_K:
					input_state.shoot = 1;
					break;
			}
			break;
		case SDL_EVENT_KEY_UP:
			switch (event.key.key) {
				case SDLK_S:
					input_state.mov_keys &= ~1;
					if (input_state.mov_keys & 2)
						player_data.vel.x = 1.5f;
					else
						player_data.vel.x = 0;
					break;
				case SDLK_F:
					input_state.mov_keys &= ~2;
					if (input_state.mov_keys & 1)
						player_data.vel.x = -1.5f;
					else
						player_data.vel.x = 0;
					break;
				case SDLK_D:
					input_state.mov_keys &= ~4;
					if (input_state.mov_keys & 8)
						player_data.vel.y = 1.5f;
					else
						player_data.vel.y = 0;
					break;
				case SDLK_A:
					input_state.mov_keys &= ~8;
					if (input_state.mov_keys & 4)
						player_data.vel.y = -1.5f;
					else
						player_data.vel.y = 0;
					break;

				case SDLK_K:
					input_state.shoot = 0;
					break;
			}
			break;
	}

	return 0;
}

void
parse_input()
{
}
