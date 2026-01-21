#include <stdio.h>
#include <SDL.h>

#define true 1
#define false 0

#define TILE_SIZE 32
#define GRID_WIDTH 40
#define GRID_HEIGHT 25
#define WIDTH TILE_SIZE * GRID_WIDTH
#define HEIGHT TILE_SIZE * GRID_HEIGHT

#define FPS 60
#define FPS_DELAY 1000/FPS

typedef struct
{
	int x;
	int y;
	int button;
} Mouse;

void draw_grid(int grid[GRID_HEIGHT][GRID_WIDTH], SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect rect;
	rect.w = TILE_SIZE;
	rect.h = TILE_SIZE;
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			SDL_RenderDrawLine(renderer, x * TILE_SIZE, y * TILE_SIZE, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE);
			SDL_RenderDrawLine(renderer, x * TILE_SIZE, y * TILE_SIZE, x * TILE_SIZE + TILE_SIZE, y * TILE_SIZE);

			if (grid[y][x] == 1)
			{
				rect.x = x * TILE_SIZE;
				rect.y = y * TILE_SIZE;
				SDL_RenderFillRect(renderer, &rect);
			}

		}
	}
	SDL_RenderDrawLine(renderer, 0, HEIGHT - 1, WIDTH, HEIGHT - 1);
	SDL_RenderDrawLine(renderer, WIDTH - 1, 0, WIDTH - 1, HEIGHT);
}

void handle_mouse(Mouse* mouse, int grid[GRID_HEIGHT][GRID_WIDTH])
{
	if (mouse->button == SDL_BUTTON_LEFT)
	{
		int x = mouse->x / TILE_SIZE;
		int y = mouse->y / TILE_SIZE;

		grid[y][x] = 1;
	}
}

int main()
{
	SDL_Window* window = SDL_CreateWindow("Water Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	Mouse mouse = { -1, -1, -1 };
	int grid[GRID_HEIGHT][GRID_WIDTH] = { 0 };


	int running = true;
	SDL_Event event;

	while (running)
	{
		mouse.button = -1;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					running = false;
					break;

				case SDL_MOUSEBUTTONDOWN:
					mouse.button = event.button.button;
					mouse.x = event.button.x;
					mouse.y = event.button.y;
					break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		draw_grid(grid, renderer);

		handle_mouse(&mouse, grid);

		SDL_RenderPresent(renderer);
		SDL_Delay(FPS_DELAY);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}