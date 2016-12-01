// Chip8.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Chip8Emulator.h"

Chip8Emulator emu;

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
int modifier = 10;

// Window size
int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;

unsigned short ScreenData[SCREEN_HEIGHT][SCREEN_WIDTH][3];

void DisplayProc();
void UpdateTexture();
void ResizeProc(GLsizei w, GLsizei h);
void keyboardUp(unsigned char key, int x, int y);
void keyboardDown(unsigned char key, int x, int y);

int main(int argc, char **argv)
{
	srand(GetTickCount());
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(display_width, display_height);
	glutInitWindowPosition(320, 320);
	glutCreateWindow("CHIP-8 - By KaiserKyle");

	glutDisplayFunc(DisplayProc);
	glutIdleFunc(DisplayProc);
	glutReshapeFunc(ResizeProc);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);

	emu.Init();
	emu.LoadRom(L"c:\\emu\\chip8\\blinky");

	glutMainLoop();

    return 0;
}

void DisplayProc()
{
	emu.EmulateCycle();

	if (emu.ShouldDraw())
	{
		UpdateTexture();			

		// Swap buffers!
		glutSwapBuffers();
	}
}

void draw_square(float x_coord, float y_coord) {
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(x_coord, y_coord);
	glVertex2f(x_coord + 10, y_coord);
	glVertex2f(x_coord + 10, y_coord + 10);
	glVertex2f(x_coord, y_coord + 10);
	glEnd();
}

void UpdateTexture()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	int i, j;
	for (i = 0; i < 64; i++) {
		for (j = 0; j < 32; j++) {
			if (emu.gfx[(j * 64) + i] == 1) {
				draw_square((float)(i * 10), (float)(j * 10));
			}
		}
	}
}

void ResizeProc(GLsizei w, GLsizei h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, h, 0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);

	// Resize quad
	display_width = w;
	display_height = h;
}

void SetupTexture()
{
	// Clear screen
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			ScreenData[y][x][0] = ScreenData[y][x][1] = ScreenData[y][x][2] = 0;
		}
	}

	// Create a texture 
	glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)ScreenData);

	// Set up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Enable textures
	glEnable(GL_TEXTURE_2D);
}

void keyboardDown(unsigned char key, int x, int y)
{
	if (key == 27)    // esc
		exit(0);
	if (key == 'p')
		getchar();

	int pressed = 0xFF;

	if (key == '1')		pressed = 0x1;
	else if (key == '2')	pressed = 0x2;
	else if (key == '3')	pressed = 0x3;
	else if (key == '4')	pressed = 0xC;
	else if (key == 'q')	pressed = 0x4;
	else if (key == 'w')	pressed = 0x5;
	else if (key == 'e')	pressed = 0x6;
	else if (key == 'r')	pressed = 0xD;
	else if (key == 'a')	pressed = 0x7;
	else if (key == 's')	pressed = 0x8;
	else if (key == 'd')	pressed = 0x9;
	else if (key == 'f')	pressed = 0xE;
	else if (key == 'z')	pressed = 0xA;
	else if (key == 'x')	pressed = 0x0;
	else if (key == 'c')	pressed = 0xB;
	else if (key == 'v')	pressed = 0xF;

	if (0xFF != pressed)
	{
		emu.KeyPress(pressed);
	}
}

void keyboardUp(unsigned char key, int x, int y)
{
	int released = 0xFF;

	if (key == '1')		released = 0x1;
	else if (key == '2')	released = 0x2;
	else if (key == '3')	released = 0x3;
	else if (key == '4')	released = 0xC;
	else if (key == 'q')	released = 0x4;
	else if (key == 'w')	released = 0x5;
	else if (key == 'e')	released = 0x6;
	else if (key == 'r')	released = 0xD;
	else if (key == 'a')	released = 0x7;
	else if (key == 's')	released = 0x8;
	else if (key == 'd')	released = 0x9;
	else if (key == 'f')	released = 0xE;
	else if (key == 'z')	released = 0xA;
	else if (key == 'x')	released = 0x0;
	else if (key == 'c')	released = 0xB;
	else if (key == 'v')	released = 0xF;

	if (0xFF != released)
	{
		emu.KeyRelease(released);
	}
}
