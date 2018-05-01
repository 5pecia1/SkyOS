#include "SkyGUI.h"
#include "memory.h"
#include "FontData.h"
#include "ProcessManager.h"
#include "sprintf.h"
#include "SkyAPI.h"
#include "SkyMouse.h"
#include "fifo.h"
#include "PhysicalMemoryManager.h"
#include "SkySheetController8.h"
#include "SkySheetController32.h"
#include "SkyRenderer.h"
#include "SkySimpleGUI.h"
#include "SkyRenderer8.h"
#include "SkyRenderer32.h"
#include "Scheduler.h"
#include "KernelProcedure.h"
#include "ConsoleIOListener.h"

extern void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
extern void inthandler2c();
extern void inthandler21();

__declspec(naked) void kSkyMouseHandler()
{

	_asm {
		cli
		pushad
	}

	_asm
	{
		call inthandler2c
	}

	_asm {
		mov al, 0x20
		out 0x20, al
		popad
		sti
		iretd
	}
}
static char keytable0[0x80] = {
	0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0,   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',   0,   '\\', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};
static char keytable1[0x80] = {
	0,   0,   '!', '@', '#', '$', '%', '&', '^', '*', '(', ')', '_', '+', 0,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,   '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

__declspec(naked) void kSkyKeyboardHandler()
{

	_asm {
		cli
		pushad
	}

	_asm
	{
		call inthandler21
	}

	_asm {
		mov al, 0x20
		out 0x20, al
		popad
		sti
		iretd
	}
}

extern char hankaku[4096];

SkyGUI::SkyGUI()
{
	m_pPressedSheet = nullptr;
	m_RButtonPressed = false;
}


SkyGUI::~SkyGUI()
{
}

bool SkyGUI::Initialize(void* pVideoRamPtr, int width, int height, int bpp)
{
	m_pVideoRamPtr = (ULONG*)pVideoRamPtr;
	m_width = width;
	m_height = height;
	m_bpp = bpp;

	LoadFontFromMemory();

	//�ʱ�ȭ�� ����
	MakeInitScreen();
	//����� �ý��� �籸��
	MakeIOSystem();
	
	return true;
}

bool SkyGUI::MakeInitScreen()
{
	m_pVideoRamPtr = (ULONG*)0xFD000000;
	m_width = 1024;
	m_height = 768;
	m_bpp = 8;

	m_pRenderer = new SkyRenderer8();
	m_pRenderer->Initialize();

	m_mainSheet = new SkySheetController8();
	m_mainSheet->Initialize((unsigned char*)m_pVideoRamPtr, m_width, m_height);;

	//��׶��� ��Ʈ�� ����
	sht_back = m_mainSheet->Alloc();
	unsigned char* buf_back = m_mainSheet->AllocBuffer(m_width, m_height);
	sht_back->SetBuf(buf_back, m_width, m_height, -1);
	sht_back->m_movable = false;
	m_pRenderer->InitScreen((unsigned char *)buf_back, m_width, m_height);

	//���콺 ��Ʈ�� ����
	sht_mouse = m_mainSheet->Alloc();
	unsigned char* buf_mouse = m_mainSheet->AllocBuffer(16, 16);
	sht_mouse->SetBuf(buf_mouse, 16, 16, 99);
	sht_mouse->m_movable = false;
	m_pRenderer->InitMouseCursor((char *)buf_mouse, 99);

	
	mx = (m_width - 16) / 2;
	my = (m_height - 28 - 16) / 2;

	sht_back->Slide(0, 0);
	sht_mouse->Slide(mx, my);

	sht_back->Updown(0);
	sht_mouse->Updown(3);

	return true;
}

bool SkyGUI::LoadFontFromMemory()
{
	unsigned char* buffer = (unsigned char*)hankaku;
	int bufferIndex = 0;
	int charIndex = 0;

	memset(buffer, 0, 4096);

	int readIndex = 0;

	while (readIndex < 32768) {


		for (int i = 0; i < 512; i++)
		{
			if (fontData[readIndex + i] == '*')
			{

				if (charIndex < 8)
				{
					char a = (char)(1 << (8 - 1 - charIndex));
					buffer[bufferIndex] |= a;
				}

			}

			if ((fontData[readIndex + i] == '*') || (fontData[readIndex + i] == '.'))
			{
				charIndex++;

				if (charIndex >= 8)
				{
					bufferIndex++;
					charIndex = 0;
				}
			}
		}

		readIndex += 512;

	}

	return true;
}
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47
bool SkyGUI::MakeIOSystem()
{
	//Ű����, ���콺 �ڵ鷯 ����
	kEnterCriticalSection();

	fifo32_init(&fifo, 128, fifobuf);

	fifo32_init(&keycmd, 32, keycmd_buf);

	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	setvect(0x21, kSkyKeyboardHandler);
	setvect(0x2c, kSkyMouseHandler);

	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	kLeaveCriticalSection();

	
	wait_KBC_sendready();
	OutPortByte(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	OutPortByte(PORT_KEYDAT, KBC_MODE);

	

	/* ���콺 ��ȿ */
	wait_KBC_sendready();
	OutPortByte(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	OutPortByte(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* �ߵǸ� ACK(0xfa)�� �۽ŵǾ� �´� */
	mdec.phase = 0; /* ���콺�� 0xfa�� ��ٸ��� �ִ� �ܰ� */

	OutPortByte(PIC0_IMR, 0xf8); /* PIT�� PIC1�� Ű���带 �㰡(11111000) */
	OutPortByte(PIC1_IMR, 0xef); /* ���콺�� �㰡(11101111) */

	//kEnterCriticalSection();
	
	//kLeaveCriticalSection();	
	
	return true;
}

bool SkyGUI::kGetMessage(LPSKY_MSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	lpMsg->_msgId = SKY_MSG_NO_MESSAGE;
	lpMsg->_extra = 0;

	kEnterCriticalSection();
	if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) 
	{
		/* Ű���� ��Ʈ�ѷ��� ���� �����Ͱ� ������, ������ */
		keycmd_wait = fifo32_get(&keycmd);
		kLeaveCriticalSection();
		wait_KBC_sendready();
		OutPortByte(PORT_KEYDAT, keycmd_wait);
	}	
	else
		kLeaveCriticalSection();

	kEnterCriticalSection();
	if (fifo32_status(&fifo) != 0)
	{
		lpMsg->_msgId = SKY_MSG_MESSAGE;
		lpMsg->_extra = fifo32_get(&fifo);
	}
	else
	{
		//Process* pProcess = ProcessManager::GetInstance()->GetCurrentTask()->m_pParent;
		//if(pProcess != nullptr)
		//	Scheduler::GetInstance()->Yield(pProcess->GetProcessId());
	}
	
	kLeaveCriticalSection();

	return true;
}

bool SkyGUI::kTranslateAccelerator(HWND hWnd, HANDLE hAccTable, LPSKY_MSG lpMsg)
{
	return lpMsg->_msgId != SKY_MSG_MESSAGE;	
}

bool SkyGUI::Run()
{
	SKY_MSG msg;
	while (kGetMessage(&msg, nullptr, 0, 0))
	{			
		//if (!kTranslateAccelerator(msg._hwnd, nullptr, &msg))
		{			
			kTranslateMessage(&msg);
			kDispatchMessage(&msg);
		}			
	}

	return msg._msgId == SKY_MSG_EXIT;
}

bool SkyGUI::kTranslateMessage(const LPSKY_MSG lpMsg)
{
	
	if (256 <= lpMsg->_extra && lpMsg->_extra <= 511) //Ű���� ������
	{
		lpMsg->_msgId = SKY_MSG_KEYBOARD;
	}
	else if (512 <= lpMsg->_extra && lpMsg->_extra <= 767) //���콺 ������ ó��
	{

		lpMsg->_msgId = SKY_MSG_MOUSE;
	}

	return true;
}

bool SkyGUI::kDispatchMessage(const LPSKY_MSG lpMsg)
{
	return kWndProc(lpMsg->_hwnd, lpMsg->_msgId, lpMsg->_extra, 0);
}

bool CALLBACK SkyGUI::kWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case SKY_MSG_KEYBOARD:
	{
		//ProcessKeyboard(wParam);
	}
	break;
	case SKY_MSG_MOUSE:
	{
		
		ProcessMouse(wParam);
	}
	break;
	case SKY_MSG_EXIT:
		//Not Implemented
		break;
	//default:
		//return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void SkyGUI::ProcessKeyboard(int value)
{
	if (value < 0x80 + 256) { /* Ű�ڵ带 ���� �ڵ�� ��ȯ */
		if (key_shift == 0) {
			s[0] = keytable0[value - 256];
		}
		else {
			s[0] = keytable1[value - 256];
		}
	}
	else {
		s[0] = 0;
	}

	if ('A' <= s[0] && s[0] <= 'Z') {	/* �Է� ���ڰ� ���ĺ� */
		if (((key_leds & 4) == 0 && key_shift == 0) ||
			((key_leds & 4) != 0 && key_shift != 0)) {
			s[0] += 0x20;	/* �빮�ڸ� �ҹ��ڿ� ��ȯ */
		}
	}
	kEnterCriticalSection();
	if (s[0] != 0) 
	{ 
		SendToMessage(m_pFocusSheet, SKY_MSG_KEYBOARD, s[0] + 256);
	}
	if (value == 256 + 0x0e) // �� �����̽�
	{	
		SendToMessage(m_pFocusSheet, SKY_MSG_KEYBOARD, 8 + 256);
	}
	if (value == 256 + 0x1c)// Enter 
	{	
		SendToMessage(m_pFocusSheet, SKY_MSG_KEYBOARD, 10 + 256);
	}
	if (value == 256 + 0x2a) {	/* ���� ����Ʈ ON */
		key_shift |= 1;
	}
	if (value == 256 + 0x36) {	/* ������ ����Ʈ ON */
		key_shift |= 2;
	}
	if (value == 256 + 0xaa) {	/* ���� ����Ʈ OFF */
		key_shift &= ~1;
	}
	if (value == 256 + 0xb6) {	/* ������ ����Ʈ OFF */
		key_shift &= ~2;
	}
	if (value == 256 + 0x3a) {	/* CapsLock */
		key_leds ^= 4;
		fifo32_put(&keycmd, KEYCMD_LED);
		fifo32_put(&keycmd, key_leds);
	}
	if (value == 256 + 0x45) {	/* NumLock */
		key_leds ^= 2;
		fifo32_put(&keycmd, KEYCMD_LED);
		fifo32_put(&keycmd, key_leds);
	}
	if (value == 256 + 0x46) {	/* ScrollLock */
		key_leds ^= 1;
		fifo32_put(&keycmd, KEYCMD_LED);
		fifo32_put(&keycmd, key_leds);
	}

	if (value == 256 + 0xfa) {	/* Ű���尡 �����͸� �����ϰ� �޾Ҵ� */
		keycmd_wait = -1;
	}
	kLeaveCriticalSection();

	if (value == 256 + 0xfe) {	/* Ű���尡 �����͸� �����ϰ� ���� �� ������ */
		wait_KBC_sendready();
		OutPortByte(PORT_KEYDAT, keycmd_wait);
	}


}

void SkyGUI::ProcessMouse(int value)
{
	if (mouse_decode(&mdec, value - 512) != 0) 
	{
		/* ���콺 Ŀ���� �̵� */
		mx += mdec.x;
		my += mdec.y;
		if (mx < 0) {
			mx = 0;
		}
		if (my < 0) {
			my = 0;
		}
		if (mx > m_width - 1) {
			mx = m_width - 1;
		}
		if (my > m_height - 1) {
			my = m_height - 1;
		}

		kEnterCriticalSection();

		sht_mouse->Slide(mx, my);
		//sht_mouse->Updown(100);
		
		/*if ((mdec.btn & 0x01) != 0)  //���� ��ư�� �����ٸ� ���콺 �ٷ� �Ʒ��� �����츦 �巡�� ó���Ѵ�.
		{
			if(m_pPressedSheet == nullptr)
				ProcessMouseLButton(mx,  my);
		}
		else
		{
			if (m_pPressedSheet)
			{
				m_pPressedSheet->Slide(mx, my);
				
			}
			m_pPressedSheet = nullptr;
		}
	
		if((mdec.btn & 0x02) != 0) //������ ��ư�� �����ٸ� �ܼ� ���μ����� �����Ѵ�.
		{
			if (m_RButtonPressed == false)
			{				
				m_RButtonPressed = true;
				CreateGUIConsoleProcess();
			}
			//SkySimpleGUI::FillRect8(100, 100, 100, 100, COL8_C6C6C6, 1024, 768);
		}
		else
		{
			m_RButtonPressed = false;
		}*/

		kLeaveCriticalSection();
		
	}
}

void SkyGUI::ProcessMouseLButton(int x, int y)
{
	SkySheet* pSheet = m_mainSheet->FindSheet(x, y);
	if (pSheet == nullptr)
	{
		m_pFocusSheet = nullptr;
		m_pPressedSheet = nullptr;
		return;
	}

	SendToMessage(pSheet, SKY_MSG_MOUSE, 0);

	m_pPressedSheet = pSheet;
	m_pFocusSheet = pSheet;
}

bool SkyGUI::SendToMessage(SkySheet* pSheet, int message, int value)
{
	if (pSheet == nullptr)
		return false;

	map<int, ConsoleIOListener*>::iterator iter = m_mapIOListener.find(pSheet->m_ownerProcess);

	if (iter == m_mapIOListener.end())
		return false;

	//pSheet->Slide(x, y);

	ConsoleIOListener* listener = (*iter).second;
	listener->PushMessage(message, value);

	return true;
}

void SkyGUI::CreateGUIConsoleProcess()
{
	Process* pProcess = nullptr;

	kEnterCriticalSection();
	pProcess = ProcessManager::GetInstance()->CreateProcessFromMemory("GUIConsole", ConsoleGUIProc, this, PROCESS_KERNEL);
	if (pProcess != nullptr)
	{
		//�ܼ� �½�ũ ��Ʈ�� ����
		SkySheet* console = m_mainSheet->Alloc();
		unsigned char* buf = m_mainSheet->AllocBuffer(256, 165);
		console->SetBuf(buf, 256, 165, -1);
		m_pRenderer->MakeWindow(buf, 256, 165, "Sky Console", 0);
		m_pRenderer->MakeTextBox(console, 8, 28, 240, 128, COL8_000000);
		console->Slide(32, 4);
		console->Updown(4);
		console->m_ownerProcess = pProcess->GetProcessId();
	}
	kLeaveCriticalSection();
}

void SkyGUI::RegisterIOListener(int processID, ConsoleIOListener* listener)
{
	kEnterCriticalSection();

	m_mapIOListener[processID] = listener;

	kLeaveCriticalSection();
}

SkySheet* SkyGUI::FindSheetByID(int processId)
{
	return m_mainSheet->FindSheetById(processId);
}





//�Է�â �½�ũ(task_a) ����
/*sht_win = shtctl->Alloc();
buf_win = new unsigned char[160 * 52];
sht_win->SetBuf(buf_win, 144, 52, -1);
SkyRenderer::MakeWindow8(buf_win, 144, 52, "task_a", 1);
SkyRenderer::MakeTextBox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
*/

//sht_win->Slide(64, 56);
//sht_win->Updown(2);
