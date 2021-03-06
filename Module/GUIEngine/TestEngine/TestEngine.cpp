// TestEngine.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <stdio.h> 
#include <windows.h> 
#include "SkyMockInterface.h"
#include "I_GUIEngine.h"

#include "SDL.h"
#include <iostream>

#define MOUSE_LBUTTONDOWN   0x01
#define MOUSE_RBUTTONDOWN   0x02
#define MOUSE_MBUTTONDOWN   0x04

#pragma comment(lib, "SDL2.LIB")

FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }

extern SKY_FILE_Interface g_FileInterface;
extern SKY_ALLOC_Interface g_allocInterface;
extern SKY_Print_Interface g_printInterface;
extern SKY_PROCESS_INTERFACE g_processInterface;

typedef void (*PSetSkyMockInterface)(SKY_ALLOC_Interface, SKY_FILE_Interface, SKY_Print_Interface);
typedef I_GUIEngine* (*PGUIEngine)();


void* GetModuleFunction(HINSTANCE handle, const char* func_name)
{
	return (void*)GetProcAddress(handle, func_name);
}

#undef main

int screen_w;
int screen_h;
SDL_Surface *screen;

int main()
{
	HINSTANCE dllHandle = NULL;
	
	//디버그엔진 모듈을 로드한다.
	dllHandle = LoadLibrary("GUIEngine.dll");	
	
	//디버그엔진 모듈이 익스포트하는 SetSkyMockInterface 함수와 GetDebugEngineDLL 함수를 얻어낸다.
	PSetSkyMockInterface SetSkyMockInterface = (PSetSkyMockInterface)GetModuleFunction(dllHandle, "SetSkyMockInterface");
	PSetSkyProcessInterface SetSkyProcessInterface = (PSetSkyProcessInterface)GetModuleFunction(dllHandle, "SetSkyProcessInterface");

	PGUIEngine GUIEngine = (PGUIEngine)GetModuleFunction(dllHandle, "GetGUIEngine");

	//SetSkyMockInterface 함수를 사용해서 디버그엔진 모듈에 파일인터페이스와 입출력, 화면출력 인터페이스를 제공한다.
	SetSkyMockInterface(g_allocInterface, g_FileInterface, g_printInterface);
	SetSkyProcessInterface(g_processInterface);
	I_GUIEngine* pEngine = GUIEngine();

	//윈도우와 렌더러를 생성
	SDL_Window *pWindow;
	SDL_Renderer *pRenderer;
#ifndef WIN32 
	if (SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN, &pWindow, &pRenderer) < 0)
#else
	if (SDL_CreateWindowAndRenderer(1024, 768, 0, &pWindow, &pRenderer) < 0)
#endif
	{
		std::cout << "SDL_CreateWindowAndRenderer Error: " << SDL_GetError() << std::endl;
		return 0;
	}
	SDL_GetWindowSize(pWindow, &screen_w, &screen_h);
	screen = SDL_CreateRGBSurface(0, screen_w, screen_h, 16,
		0,
		0,
		0,
		0);

	if (screen == 0)
	{
		std::cout << "SDL_CreateRGBSurface Error: " << SDL_GetError() << std::endl;
		return false;
	}

	SDL_Texture *pTexture = SDL_CreateTexture(pRenderer,
		SDL_PIXELFORMAT_RGB565,
		SDL_TEXTUREACCESS_STREAMING,
		screen_w, screen_h);

	if (pRenderer == 0)
	{
		std::cout << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	if (pTexture == 0)
	{
		SDL_DestroyRenderer(pRenderer);
		SDL_DestroyWindow(pWindow);
		std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
		return 0;
	}

	LinearBufferInfo info;
	info.pBuffer = (unsigned long*)screen->pixels;
	info.width = 1024;
	info.height = 768;
	info.depth = 16;
	info.type = 0;
	info.isDirectVideoBuffer = false;

	pEngine->SetLinearBuffer(info);
	pEngine->Initialize();

	/*unsigned char* pixels = (unsigned char*)screen->pixels;

	if (pBuffer->_bufferType == IAMGE_BUFFER_2D_ARRAY)
	{
		for (int y = 0; y < pBuffer->_height; y++)
		{
			for (int x = 0; x < pBuffer->_width; x++)
			{
				char* row = (char*)pBuffer->_pBuffer[y];

				char* pixel = &row[x * 3]; //SDL Surface => BGRA : PNG => RGBA
				pixels[4 * (y * screen->w + x) + 0] = pixel[0];
				pixels[4 * (y * screen->w + x) + 1] = pixel[1];
				pixels[4 * (y * screen->w + x) + 2] = pixel[2];
				pixels[4 * (y * screen->w + x) + 3] = pixel[3];
			}
		}
	}
	if (pBuffer->_bufferType == IAMGE_BUFFER_LINEAR)
	{
		for (int y = 0; y < pBuffer->_height; y++)
		{
			for (int x = 0; x < pBuffer->_width; x++)
			{
				char* pixel = (char*)pBuffer->_pBuffer;
				pixels[4 * (y * screen->w + x) + 0] = pixel[(y * pBuffer->_width + x) * 3 + 2];
				pixels[4 * (y * screen->w + x) + 1] = pixel[(y * pBuffer->_width + x) * 3 + 1];
				pixels[4 * (y * screen->w + x) + 2] = pixel[(y * pBuffer->_width + x) * 3 + 0];
				pixels[4 * (y * screen->w + x) + 3] = 255;
			}
		}
	}

	pImageInterface->SavePixelData("sample3.png", nullptr, 0);*/


	
	bool running = true;
	//루프를 돌며 화면을 그린다.
	while (running)
	{
		//이벤트를 가져온다.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{	
			

			if (event.type == SDL_MOUSEMOTION )
			{
				MOUSEDATA data;
				
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					data.bButtonStatusAndFlag = MOUSE_LBUTTONDOWN;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					data.bButtonStatusAndFlag = MOUSE_RBUTTONDOWN;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					data.bButtonStatusAndFlag = MOUSE_MBUTTONDOWN;
				}
				else
				{
					data.bButtonStatusAndFlag = 0;
				}

				data.bXMovement = event.motion.x;
				data.bYMovement = event.motion.y;
				data.bAbsoluteCoordinate = 1;
				pEngine->PutMouseQueue(&data);				
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				MOUSEDATA data;

				if (event.button.button == SDL_BUTTON_LEFT)
				{
					data.bButtonStatusAndFlag = MOUSE_LBUTTONDOWN;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					data.bButtonStatusAndFlag = MOUSE_RBUTTONDOWN;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					data.bButtonStatusAndFlag = MOUSE_MBUTTONDOWN;
				}
				else
				{
					data.bButtonStatusAndFlag = 0;
				}				
				
				data.bXMovement = event.motion.x;
				data.bYMovement = event.motion.y;
				data.bAbsoluteCoordinate = 1;
				pEngine->PutMouseQueue(&data);
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				MOUSEDATA data;
				data.bButtonStatusAndFlag = 0;
				data.bXMovement = event.motion.x;
				data.bYMovement = event.motion.y;
				data.bAbsoluteCoordinate = 1;
				pEngine->PutMouseQueue(&data);
			}
			//키보드 이벤트가 발생했다면
			else if (event.type == SDL_KEYDOWN)
			{        //ESC키를 눌렀다면 프로그램 종료
				KEYDATA data;
				data.bScanCode = event.key.keysym.scancode;
				data.bASCIICode = SDL_GetKeyFromScancode(event.key.keysym.scancode);
				data.bFlags = 0x01;
				pEngine->PutKeyboardQueue(&data);
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					running = false;
				}
			} //QUIT 메시지가 들어왔다면 프로그램 종료
			else if (event.type == SDL_KEYUP)
			{        //ESC키를 눌렀다면 프로그램 종료
				KEYDATA data;
				data.bScanCode = event.key.keysym.scancode;
				data.bASCIICode = SDL_GetKeyFromScancode(event.key.keysym.scancode);
				data.bFlags = 0x02;
				pEngine->PutKeyboardQueue(&data);
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					running = false;
				}
			} //QUIT 메시지가 들어왔다면 프로그램 종료
			else if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		pEngine->Update(5);

		//렌더러를 클리어하고 Hello World 텍스처를 렌더러에 복사한다.
		SDL_RenderClear(pRenderer);
		//SDL_RenderCopy(pRenderer, pTexture, NULL, NULL);

		SDL_UpdateTexture(pTexture, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(pRenderer, pTexture, NULL, NULL);
		//렌더러의 내용을 화면에 뿌린다.
		SDL_RenderPresent(pRenderer);
	}
	//텍스처, 렌더러, 윈도우 객체를 제거하고 SDL을 종료한다.
	SDL_DestroyTexture(pTexture);
	SDL_DestroyRenderer(pRenderer);
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
	
	
	return 0;
}

