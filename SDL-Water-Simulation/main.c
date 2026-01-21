#include <stdio.h>
#include <SDL.h>

#define true 1
#define false 0

#define TILE_SIZE 8
#define GRID_WIDTH 160
#define GRID_HEIGHT 100
#define WIDTH TILE_SIZE * GRID_WIDTH
#define HEIGHT TILE_SIZE * GRID_HEIGHT

#define FPS 60
#define FPS_DELAY 1000/FPS

#define EMPTY_ID 0
#define WATER_ID 1
#define WALL_ID -1

#define WATER_MAX_ID 32

#define COLOR_RED_MIN 0
#define COLOR_GREEN_MIN 29
#define COLOR_BLUE_MIN 77

#define COLOR_RED_MAX 79
#define COLOR_GREEN_MAX 129
#define COLOR_BLUE_MAX 219

#define DRAW_LINES false

const int flow_dirs[3][2] = { {0, 1}, {-1, 0}, {1, 0} };

typedef struct
{
	int x;
	int y;
	int button;
} Mouse;

typedef struct
{
	int x;
	int y;
} Vec2;

typedef struct
{
	int x;
	int y;
	int z;
} Vec3;

int clamp(int val, int min, int max)
{
	if (val < min)
		return min;
	if (val > max)
		return max;
	return val;
}

void swap(int *a, int *b)
{
	int c = *a;
	*a = *b;
	*b = c;
}

Vec3 calculate_color(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
	int sum = 0;
	for (int i = y - 1; i >= 0; i--)
	{
		int id = grid[i][x];
		if (id == EMPTY_ID)
			break;
		if (id >= WATER_ID)
			sum++;
	}

	sum = clamp(sum, WATER_ID, WATER_MAX_ID);

	float t = (float)sum / (float)WATER_MAX_ID;

	int r = clamp((int)(COLOR_RED_MAX * (1.0f - t)), COLOR_RED_MIN, COLOR_RED_MAX);
	int g = clamp((int)(COLOR_GREEN_MAX * (1.0f - t)), COLOR_GREEN_MIN, COLOR_GREEN_MAX);
	int b = clamp((int)(COLOR_BLUE_MAX * (1.0f - t)), COLOR_BLUE_MIN, COLOR_BLUE_MAX);

	Vec3 color = { r, g, b };

	return color;
}

Vec2 process_water(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
	Vec2 newpos = { -1, -1 };
	int nx = x + flow_dirs[0][0];
	int ny = y + flow_dirs[0][1];
	if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT && grid[ny][nx] == EMPTY_ID)
	{
		grid[ny][nx] = grid[y][x];
		grid[y][x] = EMPTY_ID;
		newpos.x = nx;
		newpos.y = ny;
		return newpos;
	}

	int lateral[2] = { 1, 2 };
	if (rand() % 2)
	{
		int tmp = lateral[0];
		lateral[0] = lateral[1];
		lateral[1] = tmp;
	}

	for (int i = 0; i < 2; i++)
	{
		int idx = lateral[i];
		nx = x + flow_dirs[idx][0];
		ny = y + flow_dirs[idx][1];

		if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT && grid[ny][nx] == EMPTY_ID)
		{
			grid[ny][nx] = grid[y][x];
			grid[y][x] = EMPTY_ID;
			newpos.x = nx;
			newpos.y = ny;
			return newpos;
		}
	}

	return newpos;
}

void draw_grid(int grid[GRID_HEIGHT][GRID_WIDTH], SDL_Renderer* renderer)
{
	SDL_Rect rect;
	rect.w = TILE_SIZE - 1;
	rect.h = TILE_SIZE - 1;

	for (int y = GRID_HEIGHT - 1; y >= 0; y--)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			if (DRAW_LINES)
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderDrawLine(renderer, x * TILE_SIZE, y * TILE_SIZE, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE);
				SDL_RenderDrawLine(renderer, x * TILE_SIZE, y * TILE_SIZE, x * TILE_SIZE + TILE_SIZE, y * TILE_SIZE);
			}

			int id = grid[y][x];

			if (id == -1)
			{
				rect.x = x * TILE_SIZE + 1;
				rect.y = y * TILE_SIZE + 1;

				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderFillRect(renderer, &rect);
			}
			else if (id >= WATER_ID)
			{
				rect.x = x * TILE_SIZE + 1;
				rect.y = y * TILE_SIZE + 1;

				Vec3 color = calculate_color(grid, x, y);
				
				SDL_SetRenderDrawColor(renderer, color.x, color.y, color.z, 255);
				
				SDL_RenderFillRect(renderer, &rect);

			}

		}
	}
	SDL_RenderDrawLine(renderer, 0, HEIGHT - 1, WIDTH, HEIGHT - 1);
	SDL_RenderDrawLine(renderer, WIDTH - 1, 0, WIDTH - 1, HEIGHT);
}

void update_grid(int grid[GRID_HEIGHT][GRID_WIDTH])
{
	for (int y = GRID_HEIGHT - 1; y >= 0; y--)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			int id = grid[y][x];
			if (id >= WATER_ID)
				process_water(grid, x, y);
		}
	}
}

void handle_mouse(Mouse* mouse, int grid[GRID_HEIGHT][GRID_WIDTH])
{
	if (mouse->button == SDL_BUTTON_LEFT)
	{
		int x = mouse->x / TILE_SIZE;
		int y = mouse->y / TILE_SIZE;

		grid[y][x] = WALL_ID;
	}
	else if (mouse->button == SDL_BUTTON_RIGHT)
	{
		int x = mouse->x / TILE_SIZE;
		int y = mouse->y / TILE_SIZE;

		grid[y][x] = WATER_ID;
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

				/*case SDL_MOUSEBUTTONDOWN:
					mouse.button = event.button.button;
					mouse.x = event.button.x;
					mouse.y = event.button.y;
					break;*/
			}
		}

		Uint32 buttons = SDL_GetMouseState(&mouse.x, &mouse.y);

		if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			mouse.button = SDL_BUTTON_LEFT;
		}

		if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT))
		{
			mouse.button = SDL_BUTTON_RIGHT;
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		draw_grid(grid, renderer);
		update_grid(grid);

		handle_mouse(&mouse, grid);

		SDL_RenderPresent(renderer);
		SDL_Delay(FPS_DELAY);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}