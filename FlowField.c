#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

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

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *sidebarTexture = NULL;
SDL_Event event;
int seed = 10;
int renderMode = 1;

ColorPicker bgPicker = 	 {{810, 117, 180, 100}, {810, 227, 180, 30}, {14, 12, 89, 255}};
ColorPicker partPicker = {{810, 576, 180, 100}, {810, 686, 180, 30}, {7, 130, 122, 255}};
ColorPicker *pickers[] = {&bgPicker, &partPicker};

Slider fieldSize = {{815, 71, 172, 22}, 1, 8, 16};
Slider particles = {{815, 343, 172, 22}, 1000, 10000, 100000};
Slider lifespan = {{815, 388, 172, 22}, 5, 25, 50};
Slider speed = {{815, 433, 172, 22}, 1, 5, 30};
Slider opacity = {{815, 478, 172, 22}, 0, 100, 255};
Slider colorRange = {{815, 524, 172, 22}, 0, 40, 128};
Slider *sliders[] = {&fieldSize, &particles, &lifespan, &speed, &opacity, &colorRange};

Vector vectorField[400];


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
	int gridSize = (SCREEN_WIDTH / (float) fieldSize.value);

	for (int x = 0; x <= SCREEN_WIDTH; x += gridSize) {
		for (int y = 0; y <= SCREEN_HEIGHT; y += gridSize) {
			double angle = randomFloat(0, 2 * PI);
			Vector v = {x, y, cos(angle), sin(angle)};
			vectorField[(y / gridSize) * (SCREEN_HEIGHT / gridSize + 1) + x / gridSize] = v;
		}

	}
}


double dotProduct(int x ,int y ,Vector vector) {
	double dx = (x - vector.x) / (double) ((SCREEN_WIDTH / (float) fieldSize.value) + 1);
	double dy = (y - vector.y) / (double) ((SCREEN_WIDTH / (float) fieldSize.value) + 1);
	return dx * vector.vx + dy * vector.vy;
}


double interpolate(double a ,double b ,double w) {
	return (a - b) * ((w * (w * 6 - 15) + 10) * w * w * w) + b;
}


double getValue(int x, int y) {
	if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
		return 0;
	}

	int gridSize = SCREEN_WIDTH / (float) fieldSize.value;
	int x0 = x / gridSize;
	int y0 = y / gridSize;
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	double dx = (x % gridSize) / (float) gridSize;
	double dy = (y % gridSize) / (float) gridSize;

	double d0 = dotProduct(x, y, vectorField[x0 + y0 * (fieldSize.value + 1)]);
	double d1 = dotProduct(x, y, vectorField[x1 + y0 * (fieldSize.value + 1)]);
	double d2 = dotProduct(x, y, vectorField[x0 + y1 * (fieldSize.value + 1)]);
	double d3 = dotProduct(x, y, vectorField[x1 + y1 * (fieldSize.value + 1)]);

	double i0 = interpolate(d1, d0, dx);
	double i1 = interpolate(d3, d2, dx);

	return interpolate(i1, i0, dy);
}


void createFlowField() {
	if (renderMode == 0) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_Rect screeWipe = {0 ,0 ,800, 800};
		SDL_RenderFillRect(renderer, &screeWipe);

		for (int x = 0; x <= SCREEN_WIDTH; x += 1) {
			for (int y = 0; y <= SCREEN_HEIGHT; y += 1) {
				double value = getValue(x, y);
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, clamp((value + 0.5) * 255, 0, 255));
				SDL_RenderDrawPoint(renderer, x, y);
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		for (int i = 0; i < (fieldSize.value + 1) * (fieldSize.value + 1); i ++) {
			Vector v = vectorField[i];
			SDL_RenderDrawLine(renderer, v.x, v.y, v.x + v.vx * 20, v.y + v.vy * 20);
			SDL_RenderDrawLine(renderer,
				v.x + v.vx * 20,
				v.y + v.vy * 20,
				v.x + v.vx * 10 - v.vy * 5,
				v.y + v.vy * 10 + v.vx * 5
			);
			SDL_RenderDrawLine(renderer,
				v.x + v.vx * 20,
				v.y + v.vy * 20,
				v.x + v.vx * 10 + v.vy * 5,
				v.y + v.vy * 10 - v.vx * 5
			);
		}
	}

	else if (renderMode == 1) {
		srand(seed);
		for (int i = 0; i < particles.value; ++i) {
			int baseX = randomInt(0, SCREEN_WIDTH);
			int baseY = randomInt(0, SCREEN_HEIGHT);
			SDL_SetRenderDrawColor(
				renderer,
				clamp(partPicker.color.r + randomInt(-colorRange.value, colorRange.value), 0, 255),
				clamp(partPicker.color.g + randomInt(-colorRange.value, colorRange.value), 0, 255),
				clamp(partPicker.color.b + randomInt(-colorRange.value, colorRange.value), 0, 255),
				opacity.value);

			for (int j = 0; j < lifespan.value; ++j)
			{
				double value = (getValue(baseX, baseY) + 0.5) * 2 * PI;
				int vx = (int) (cos(value) * speed.value);
				int vy = (int) (sin(value) * speed.value);
				if (baseX < 0 || baseX > SCREEN_WIDTH || baseY < 0 || baseY > SCREEN_HEIGHT) {
					break;
				}
				SDL_RenderDrawLine(renderer, baseX, baseY, baseX + vx, baseY + vy);
				baseX += vx;
				baseY += vy;
			}
		}
	}
}


int isInside(int x, int y, SDL_Rect zone) {
	return x > zone.x && x < zone.x + zone.w && y > zone.y && y < zone.y + zone.h;
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
	Color c;
	ColorPicker currentPicker;

	for (unsigned int i = 0; i < sizeof(pickers) / sizeof(pickers[0]); i++) {
		currentPicker = *pickers[i];

		// Swatches
		SDL_SetRenderDrawColor(renderer, currentPicker.color.r, currentPicker.color.g, currentPicker.color.b, 255);
		SDL_RenderFillRect(renderer, &(*pickers[i]).swatch);

		// Pickers
		for (int x = 0; x < currentPicker.picker.w; x++) {
			for (int y = 0; y < currentPicker.picker.h; y++) {
				c = HsvaToRgba(
					x * (255 / currentPicker.picker.w),
					(10000 - y * y) / (currentPicker.picker.h * currentPicker.picker.h / (float) 255),
					y * (255 / currentPicker.picker.h),
					255);
				SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
				SDL_RenderDrawPoint(renderer, currentPicker.picker.x + x, currentPicker.picker.y + y);
			}
		}
	}
}


void createSliders() {
	Rect cursor = {0, 0, 4, 22, {249, 226, 175 ,255}};
	Slider currentSlider;

	for (unsigned int i = 0; i < sizeof(sliders) / sizeof(sliders[0]); i++) {
		currentSlider = *sliders[i];
		cursor.x = currentSlider.zone.x + (currentSlider.value - currentSlider.min) / (float) (currentSlider.max - currentSlider.min) * currentSlider.zone.w - 2;
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
	createFlowField();
	SDL_RenderPresent(renderer);
}


int main() {
	if (setupWindow() == 1) {
		return 1;
	}

	// Create background
	SDL_SetRenderDrawColor(renderer, bgPicker.color.r, bgPicker.color.g, bgPicker.color.b, bgPicker.color.a);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	SDL_RenderPresent(renderer);
	SDL_RenderCopy(renderer, sidebarTexture, NULL, NULL);
	createColorPickers();
	createSliders();

	// Create initial flow field
	createVectorField();
	createFlowField();
	SDL_RenderPresent(renderer);


	// Main loop
	int quit = 0;
	SDL_Cursor *cursor;
	SDL_Rect newSeed = {810, 740, 180, 50};
	while (!quit) {

		// Handle inputs
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_SPACE) {
					renderMode = (renderMode + 1) % 2;
					cleanScreen();
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {

				// New seed
				if (isInside(event.button.x ,event.button.y, newSeed)) {
					printf("New seed\n");
					seed = randomInt(0, 255);
					createVectorField();
					cleanScreen();
					goto end;
				}

				// Sliders
				Slider currentSlider;
				for (unsigned int i = 0; i < sizeof(sliders) / sizeof(sliders[0]); i++) {
					currentSlider = *sliders[i];
					if (isInside(event.button.x, event.button.y, currentSlider.zone)) {
						(*sliders[i]).value = (event.button.x - currentSlider.zone.x) / (float) currentSlider.zone.w * (currentSlider.max - currentSlider.min) + currentSlider.min;
						if (sliders[i] == &fieldSize) {
							createVectorField();
						}
						cleanScreen();
						goto end;
					}
				}

				// Pickers
				ColorPicker currentPicker;
				for (unsigned int i = 0; i < sizeof(pickers) / sizeof(pickers[0]); i++) {
					currentPicker = *pickers[i];
					if (isInside(event.button.x, event.button.y, currentPicker.picker)) {
						(*pickers[i]).color = HsvaToRgba(
							(event.button.x - currentPicker.picker.x) * (255 / currentPicker.picker.w),
							(10000 - (event.button.y - currentPicker.picker.y) * (event.button.y - currentPicker.picker.y)) / (currentPicker.picker.h * currentPicker.picker.h / (float) 255),
							(event.button.y - currentPicker.picker.y) * (255 / currentPicker.picker.h),
							255);
						cleanScreen();
						goto end;
					}
				}
			}

			else if (event.type == SDL_MOUSEMOTION) {
				// Button
				if (isInside(event.button.x ,event.button.y, newSeed)) {
					cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
					SDL_SetCursor(cursor);
					continue;
				}

				// Sliders
				for (unsigned int i = 0; i < sizeof(sliders) / sizeof(sliders[0]); i++) {
					if (isInside(event.button.x, event.button.y, (*sliders[i]).zone)) {
						cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
						SDL_SetCursor(cursor);
						goto end;
					}
				}

				// Pickers
				ColorPicker currentPicker;
				for (unsigned int i = 0; i < sizeof(pickers) / sizeof(pickers[0]); i++) {
					if (isInside(event.button.x, event.button.y, (*pickers[i]).picker)) {
						cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
						SDL_SetCursor(cursor);
						goto end;
					}
				}
				cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
				SDL_SetCursor(cursor);
			}

			end: ;
		}
	}

	// Cleanup
	SDL_DestroyTexture(sidebarTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
