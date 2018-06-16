#include "SkyDebugger.h"
#include "SkyConsole.h"
#include "ProcessManager.h"
#include "Thread.h"
#include "SkyModuleManager.h"
#include "Exception.h"
#include "SkyMockInterface.h"
#include "MapFile.h"
#include "StorageManager.h"
#include "stdio.h"
#include "kheap.h"
#include "SkyAPI.h"
#include "Process.h"

SkyDebugger* SkyDebugger::m_pDebugger = nullptr;

typedef I_MapFileReader*(*PGetDebugEngineDLL)();

SKY_FILE_Interface g_FileInterface =
{
	fread,
	fopen,
	fwrite,
	fclose,
	feof,
	ferror,
	fflush,
	freopen,
	fseek,
	ftell,
	fgetc,
	fgets,
};

u32int sky_kmalloc(u32int sz)
{
	return kmalloc(sz);
}

void sky_kfree(void *p)
{
	kfree(p);
}

SKY_ALLOC_Interface g_allocInterface =
{
	sky_kmalloc,
	sky_kfree,
};


void sky_printf(const char* str, ...)
{
	char *p = new char[128];
	va_list ap;

	va_start(ap, str);
	(void)vsprintf(p, str, ap);
	va_end(ap);

	SkyConsole::Print(p);
	delete p;
}


SKY_Print_Interface g_printInterface =
{
	sky_printf,
	0,
	0,
	0,
};


SkyDebugger::SkyDebugger()
{
	m_pMapReader = nullptr;
	m_symbolInit = false;
}


SkyDebugger::~SkyDebugger()
{
}

void SkyDebugger::TraceStackWithSymbol(unsigned int maxFrames)
{
	//���� ��Ȳ
	//  ù��° �Ķ���� maxFrames
	//  TraceStackWithSymbol �Լ��� �����Ų ȣ���Լ� �����ּ�
	//  ȣ���Լ��� EBP(���� EBP �������Ͱ� �̰��� ����Ű�� �ִ�)
	unsigned int* ebp = &maxFrames - 2;
	SkyConsole::Print("Stack trace:\n");
	//���� �½�ũ�� ����.
	Thread* pTask = ProcessManager::GetInstance()->GetCurrentTask();

	int lineNumber = 0;
	DWORD resultAddress = 0;
	char function[256];
	char objFileName[256];
	char filename[256];
	char undecorateName[256];

	for (unsigned int frame = 0; frame < maxFrames; ++frame)
	{
		unsigned int eip = ebp[1];
		if (eip == 0)
			//�Լ� �����ּҰ� 0�̸� �ݽ��� ����� ������.
			break;
		// ���� ȣ���Լ��� �������������� �̵��Ѵ�.
		ebp = reinterpret_cast<unsigned int *>(ebp[0]);
		unsigned int * arguments = &ebp[2];

		if (m_symbolInit == true && m_pMapReader != nullptr)
		{
			// �ɹ��������� ���� �ش��ּ��� �Լ��̸� ���� ���� ���´�.
			memset(function, 0, 256);
			memset(objFileName, 0, 256);
			memset(filename, 0, 256);
			memset(undecorateName, 0, 256);

			bool result = m_pMapReader->getAddressInfo(eip, objFileName, filename, lineNumber, function, resultAddress);

			if (result == true)
			{
				result = m_pMapReader->SimpleUndecorateCPP(function, undecorateName, 256);
				if (result == true)
				{
					SkyConsole::Print("  %s + 0x%x, %s", undecorateName, eip - resultAddress, objFileName);
				}
				else
				{
					SkyConsole::Print("  %s + 0x%x, %s", function, eip - resultAddress, objFileName);
				}
				
			}
		}
		else
		{
			SkyConsole::Print("  0x{%x}", eip);
		}		
	}
}

//������� ������ ���� ���������� ȣ��� �Լ��� ǥ����� �ʴ´�.
//������ TraceStackWithSymbol �޼ҵ�� �����غ��� �� �� �ִ�.
//�ذ�å
//1. Ÿ�� ���μ����� ������ ���丮�� ��ü�� �� EIP �������͸� �����Ѵ�.
//2. ������ ���丮�� ������� �����ѵ� EIP �ּҿ� �ش��ϴ� �ɹ��� ��´�.

void SkyDebugger::TraceStackWithProcessId(int processId)
{
	kEnterCriticalSection();

	Process* pProcess = ProcessManager::GetInstance()->FindProcess(processId);
	Thread* pTask = ProcessManager::GetInstance()->GetCurrentTask();


	if (pProcess == nullptr)
	{
		SkyConsole::Print("process not found!!\n");
	}
	else
	{
		if (pTask->m_pParent == pProcess)
		{
			kLeaveCriticalSection();
			TraceStackWithSymbol();
			return;
		}

		SkyConsole::Print("Stack trace:\n");
		Thread* pThread = pProcess->GetMainThread();
		unsigned int* ebp = (unsigned int*)pThread->m_contextSnapshot.ebp;
		

		int lineNumber = 0;
		DWORD resultAddress = 0;
		char function[256];
		char objFileName[256];
		char filename[256];
		char undecorateName[256];

		//SkyConsole::Print("  0x%x 0x%x 0x%x\n", ebp, ebp[0], ebp[1]);

		for (unsigned int frame = 0; frame < 20; ++frame)
		{
			unsigned int eip = ebp[1];
			if (eip == 0)
				//�Լ� �����ּҰ� 0�̸� �ݽ��� ����� ������.
				break;

			// ���� ȣ���Լ��� �������������� �̵��Ѵ�.
			ebp = reinterpret_cast<unsigned int *>(ebp[0]);
			unsigned int * arguments = &ebp[2];

			if (m_symbolInit == true && m_pMapReader != nullptr)
			{
				// �ɹ��������� ���� �ش��ּ��� �Լ��̸� ���� ���� ���´�.
				memset(function, 0, 256);
				memset(objFileName, 0, 256);
				memset(filename, 0, 256);
				memset(undecorateName, 0, 256);
				
				bool result = m_pMapReader->getAddressInfo(eip, objFileName, filename, lineNumber, function, resultAddress);
								

				if (result == true)
				{
					result = m_pMapReader->SimpleUndecorateCPP(function, undecorateName, 256);
					if (result == true)
					{
						SkyConsole::Print("  %s + 0x%x, %s", undecorateName, eip - resultAddress, objFileName);
					}
					else
					{
						SkyConsole::Print("  %s + 0x%x, %s", function, eip - resultAddress, objFileName);
					}

				}
			}
			else
			{
				SkyConsole::Print("  0x{%x}", eip);
			}
		}
	}

	kLeaveCriticalSection();
}

//����׿��� ����� �ε��Ѵ�.
bool SkyDebugger::LoadSymbol(const char* moduleName)
{
	
//����� ��� ������ ã�´�.
	MODULE_HANDLE hwnd = SkyModuleManager::GetInstance()->LoadModuleFromMemory(moduleName);

	if (hwnd == nullptr)
	{
		HaltSystem("Memory Module Load Fail!!");
	}

	
//����� ���� ���� ���� SetSkyMockInterface, GetDebugEngineDLL �Լ��� ���´�.
	PSetSkyMockInterface SetSkyMockInterface = (PSetSkyMockInterface)SkyModuleManager::GetInstance()->GetModuleFunction(hwnd, "SetSkyMockInterface");
	PGetDebugEngineDLL GetDebugEngineDLLInterface = (PGetDebugEngineDLL)SkyModuleManager::GetInstance()->GetModuleFunction(hwnd, "GetDebugEngineDLL");

//����� ������ �÷��� �������� �������̽��� �ѱ��.
	SetSkyMockInterface(g_allocInterface, g_FileInterface, g_printInterface);

	if (!GetDebugEngineDLLInterface)
	{
		HaltSystem("Memory Module Load Fail!!");
	}
	
	m_pMapReader = GetDebugEngineDLLInterface();
	
	if (m_pMapReader == nullptr)
	{
		HaltSystem("Map Reader Creation Fail!!");
	}

	StorageManager::GetInstance()->SetCurrentFileSystemByID('L');

	bool result = m_pMapReader->readFile("SKYOS32_MAP");
	
	if (result == false)
	{
		HaltSystem("Map Read Fail!!");
	}

	unsigned int preferredAddress = m_pMapReader->getPreferredLoadAddress();
	m_pMapReader->setLoadAddress(preferredAddress);

	m_symbolInit = true;
	return true;
}
