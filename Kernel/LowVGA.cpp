#include "LowVGA.h"
#include "SkyAPI.h"
#include "SkyConsole.h"


#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	kEnterCriticalSection(&g_criticalSection);


	for (int i = start; i <= end; i++) {
		OutPortByte(0x03c9, rgb[0] / 4);
		OutPortByte(0x03c9, rgb[1] / 4);
		OutPortByte(0x03c9, rgb[2] / 4);
		rgb += 3;
	}

	kLeaveCriticalSection(&g_criticalSection);
	
}

void InitPalette()
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:�� */
		0xff, 0x00, 0x00,	/*  1:���� ���� */
		0x00, 0xff, 0x00,	/*  2:���� �ʷ� */
		0xff, 0xff, 0x00,	/*  3:���� Ȳ�� */
		0x00, 0x00, 0xff,	/*  4:���� �Ķ� */
		0xff, 0x00, 0xff,	/*  5:���� ����� */
		0x00, 0xff, 0xff,	/*  6:���� ���� */
		0xff, 0xff, 0xff,	/*  7:��� */
		0xc6, 0xc6, 0xc6,	/*  8:���� ȸ�� */
		0x84, 0x00, 0x00,	/*  9:��ο� ���� */
		0x00, 0x84, 0x00,	/* 10:��ο� �ʷ� */
		0x84, 0x84, 0x00,	/* 11:��ο� Ȳ�� */
		0x00, 0x00, 0x84,	/* 12:��ο� �Ķ� */
		0x84, 0x00, 0x84,	/* 13:��ο� ����� */
		0x00, 0x84, 0x84,	/* 14:��ο� ���� */
		0x84, 0x84, 0x84	/* 15:��ο� ȸ�� */
	};
	set_palette(0, 15, table_rgb);
}

void StartLowModeVGA()
{
	InitPalette();

	unsigned char* video = (unsigned char*)0xa0000;

	/*for (int i = 0; i < 0xffff; i++)
	{
		video[i] = i & 0x0f;
	}*/

	boxfill8(video, 320, COL8_FF0000, 20, 20, 120, 120);
	boxfill8(video, 320, COL8_00FF00, 70, 50, 170, 150);
	boxfill8(video, 320, COL8_0000FF, 120, 80, 220, 180);

	for (;;);
}
