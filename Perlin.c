#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// clear; gcc $(pkg-config --cflags --libs sdl2) Perlin.c -o Perlin; ./Perlin; rm ./Perlin

#define SCREEN_WIDTH 800	//window height
#define SCREEN_HEIGHT 800	//window width
#define GRID_SIZE 200

#define PI 3.14159265359

typedef struct Color {
  int r;
  int g;
  int b;
  int a;
} Color;

typedef struct Rect {
  int x;
  int y;
  int w;
  int h;
  Color color;
} Rect;

typedef struct Vector {
  int x;
  int y;
  double vx;
  double vy;
} Vector;

typedef struct Slider {
	SDL_Rect zone;
	int min;
	int value;
	int max;
} Slider;

typedef struct ColorPicker {
	SDL_Rect picker;
	SDL_Rect swatch;
	Color color;
} ColorPicker;

// typedef enum {
//   particlesColor,
//   backgroundColor,
//   fieldSize,
//   particles,
//   lifespan,
//   speed,
//   inertia,
//   opacity,
//   none,
// } actionType;



SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *sidebarTexture = NULL;
SDL_Event event;
int seed = 0;

// actionType action = none;

ColorPicker bgPicker = 	 {{810, 117, 180, 100}, {810, 227, 180, 30}, {255, 255, 255, 255}};
ColorPicker partPicker = {{810, 572, 180, 100}, {810, 682, 180, 30}, {0, 0, 0, 255}};

Slider fieldSize = {{820, 71, 160, 22}, 10, 20, 200};
Slider particles = {{815, 341, 160, 22}, 5000, 10000, 50000000};
Slider lifespan = {{815, 384, 160, 22}, 5, 25, 50};
Slider speed = {{815, 431, 160, 22}, 2, 5, 50};
Slider inertia = {{815, 474, 160, 22}, 0, 0, 100};
Slider opacity = {{815, 521, 160, 22}, 0, 255, 255};


Vector vectorField[(SCREEN_WIDTH / GRID_SIZE + 1) * (SCREEN_HEIGHT / GRID_SIZE + 1)];


int setupWindow() {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	// Create window
	window = SDL_CreateWindow("Perlin Noise Field", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH + 200, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	// Create renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	// Load sidebar texture
	SDL_Surface *image = SDL_LoadBMP("Sidebar.bmp");
	sidebarTexture = SDL_CreateTextureFromSurface(renderer, image);

	return 0;
}


int clamp(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}

void drawAlphaRect(Rect r) {
	SDL_SetRenderDrawColor(renderer, r.color.r, r.color.g, r.color.b, r.color.a);
	SDL_Rect rect1 = {r.x ,r.y ,r.w ,r.h};
	SDL_RenderFillRect(renderer, &rect1);
}

int randomInt(int min, int max) {
	return rand() % (max + 1 - min) + min;
}

double randomFloat(double min, double max) {
	return ((double)rand() / (double)(RAND_MAX)) * (max - min) + min;
}


void createVectorField() {
	for (int x = 0; x <= SCREEN_WIDTH; x += GRID_SIZE) {
		for (int y = 0; y <= SCREEN_HEIGHT; y += GRID_SIZE) {
			double angle = randomFloat(0, 2 * PI);
			Vector v = {x, y, cos(angle), sin(angle)};
			vectorField[(y / GRID_SIZE) * (SCREEN_HEIGHT / GRID_SIZE + 1) + x / GRID_SIZE] = v;
		}

	}
}

double dotProduct(int x ,int y ,Vector vector) {
	double dx = (x - vector.x) / (double) GRID_SIZE;
	double dy = (y - vector.y) / (double) GRID_SIZE;
	return dx * vector.vx + dy * vector.vy;
}

double interpolate(double a ,double b ,double w) {
	return (a - b) * ((w * (w * 6 - 15) + 10) * w * w * w) + b;
}


double getValue(int x, int y) {
	if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
		return 0;
	}

	int x0 = x / GRID_SIZE;
	int y0 = y / GRID_SIZE;
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	double dx = (double) (x % GRID_SIZE) / GRID_SIZE;
	double dy = (double) (y % GRID_SIZE) / GRID_SIZE;

	// printf("dx: %f, dy: %f\n", dx, dy);

	double d0 = dotProduct(x, y, vectorField[x0 + y0 * (SCREEN_HEIGHT / GRID_SIZE + 1)]);
	double d1 = dotProduct(x, y, vectorField[x1 + y0 * (SCREEN_HEIGHT / GRID_SIZE + 1)]);
	double d2 = dotProduct(x, y, vectorField[x0 + y1 * (SCREEN_HEIGHT / GRID_SIZE + 1)]);
	double d3 = dotProduct(x, y, vectorField[x1 + y1 * (SCREEN_HEIGHT / GRID_SIZE + 1)]);

	double i0 = interpolate(d1, d0, dx);
	double i1 = interpolate(d3, d2, dx);

	return interpolate(i1, i0, dy);
}

void createFlowField(int particles, int lifespan, int speed) {
	srand(seed);
	speed = speed * (randomInt(0, 1) * 2 - 1);
	for (int i = 0; i < particles; ++i)
	{
		int baseX = randomInt(0, SCREEN_WIDTH);
		int baseY = randomInt(0, SCREEN_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 0, randomInt(0, 255), randomInt(0, 255), 20);
		// SDL_SetRenderDrawColor(renderer, partPicker.color.r, partPicker.color.g, partPicker.color.b, partPicker.color.a);

		for (int j = 0; j < lifespan; ++j)
		{
			double value = (getValue(baseX, baseY) + 0.5) * 2 * PI;
			int vx = (int) (cos(value) * speed);
			int vy = (int) (sin(value) * speed);
			SDL_RenderDrawLine(renderer, baseX, baseY, baseX + vx, baseY + vy);
			baseX += vx;
			baseY += vy;
			if (baseX < 0 || baseX > SCREEN_WIDTH || baseY < 0 || baseY > SCREEN_HEIGHT) {
				break;
			}
		}

	}
}



Color HsvaToRgba(int h, int s, int v, int a) {
    Color rgb = {0, 0, 0, a};

    unsigned char region, remainder, p, q, t;

    if (s == 0)
    {
        rgb.r = v;
        rgb.g = v;
        rgb.b = v;
        return rgb;
    }

    region = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = v;
            break;
        default:
            rgb.r = v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

void createColorPickers() {
	// SDL_RenderCopy(renderer, sidebarTexture, NULL, NULL);
	Rect r1 = {810, 227, 180, 30, bgPicker.color};
	Rect r2 = {810, 682, 180, 30, partPicker.color};
	drawAlphaRect(r1);
	drawAlphaRect(r2);

	Color c;
	for (int x = 0; x < 180; x++) {
		for (int y = 0; y < 100; y++) {
			c = HsvaToRgba(x * 1.41, (10000 - y * y) / 39.21, y * 2.55, 255);
			SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
			SDL_RenderDrawPoint(renderer, 810 + x, 117 + y);
			SDL_RenderDrawPoint(renderer, 810 + x, 572 + y);
		}
	}
}


void createSliders() {
	Rect cursor = {0, 0, 5, 22, {170, 125, 30 ,255}};
	Slider sliders[] = {fieldSize, particles, lifespan, speed, inertia, opacity};
	Slider currentSlider;

	for (int i = 0; i < sizeof(sliders) / sizeof(sliders[0]); i++) {
		currentSlider = sliders[i];
		cursor.x = currentSlider.zone.x + (currentSlider.value - currentSlider.min) / (float) (currentSlider.max - currentSlider.min) * currentSlider.zone.w;
		cursor.y = currentSlider.zone.y;
		drawAlphaRect(cursor);
	}
}


void cleanScreen() {
	SDL_SetRenderDrawColor(renderer, bgPicker.color.r, bgPicker.color.g, bgPicker.color.b, bgPicker.color.a);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, sidebarTexture, NULL, NULL);
	createColorPickers();
	createSliders();
	createFlowField(10000, 25, 5);
	SDL_RenderPresent(renderer);
}



int main() {

	if (setupWindow() == 1) {
		return 1;
	}

	// Create background
	SDL_SetRenderDrawColor(renderer, partPicker.color.r, partPicker.color.g, partPicker.color.b, partPicker.color.a);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	SDL_RenderPresent(renderer);
	SDL_RenderCopy(renderer, sidebarTexture, NULL, NULL);
	createColorPickers();
	createSliders();

	// Create initial flow field
	createVectorField();
	createFlowField(10000, 25, 5);
	SDL_RenderPresent(renderer);


	// Main loop
	int quit = 0;
	while (!quit) {

		// Handle inputs
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			else if (event.type == SDL_MOUSEBUTTONUP) {
				// action = none;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {

				if (event.button.x < 810 || event.button.x > 990){
					continue;
				}
				// Background picker
				if (event.button.y > 117 && event.button.y < 217) {
					printf("Background picker\n");
					int y = event.button.y - 117;
					Color c = HsvaToRgba((event.button.x - 810) * 1.41, (10000 - y * y) / 39.21, y * 2.55, 255);
					bgPicker.color = c;
					cleanScreen();

				}
				// Particle picker
				else if (event.button.y > 572 && event.button.y < 672) {
					printf("Particles picker\n");
					int y = event.button.y - 572;
					Color c = HsvaToRgba((event.button.x - 810) * 1.41, (10000 - y * y) / 39.21, y * 2.55, 255);
					partPicker.color = c;
					cleanScreen();
				}
				// New seed
				else if (event.button.y > 735 && event.button.y < 785) {
					printf("New seed\n");
					seed = randomInt(0, 255);
					createVectorField();
					cleanScreen();
				}
				// Noise scale slider
				else if (event.button.y > 73 && event.button.y < 93) {
					printf("Scale slider\n");
				}

				// createColorPickers();
			}
			else if (event.type == SDL_MOUSEMOTION) {
				// switch (action) {
				// 	case fieldSize:
				// 		break;
				// 	case particles:
				// 		break;
				// 	case lifespan:
				// 		break;
				// 	case speed:
				// 		break;
				// 	case inertia:
				// 		break;
				// 	case opacity:
				// 		break;
				// 	default:
				// 		break;
				// }
			}
		}

		// Perlin visualisation:
		// SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		// for (int i = 0; i < (SCREEN_WIDTH / GRID_SIZE + 1) * (SCREEN_HEIGHT / GRID_SIZE + 1); i += 1) {
		// 	Vector v = vectorField[i];
		// 	SDL_RenderDrawLine(renderer, v.x, v.y, v.x + v.vx * 20, v.y + v.vy * 20);
		// }
		// for (int x = 0; x <= SCREEN_WIDTH; x += 1) {
		// 	for (int y = 0; y <= SCREEN_HEIGHT; y += 1) {
		// 		double value = getValue(x, y);
		// 		if (value < -10 || value > 10) {
		// 			printf("value: %f\n", value);
		// 		}
		// 		SDL_SetRenderDrawColor(renderer, 0, 0, 0, clamp((value + 0.5) * 255, 0, 255));
		// 		SDL_RenderDrawPoint(renderer, x, y);
		// 	}
		// }

		// Update the display
		// SDL_UpdateTexture(screen_texture, NULL, screen->pixels, screen->w * sizeof (Uint32));
		// SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
		// SDL_RenderPresent(renderer);
	}

	// Cleanup
	SDL_DestroyTexture(sidebarTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
