#pragma once
#include "windef.h"
#include "VirtualMemoryManager.h"
#include "kheap.h"
#include "DoubleLinkedList.h"

class Thread;

extern void HaltSystem(const char* errMsg);

class Process
{
public:
	Process();
	virtual ~Process();

	bool AddThread(Thread* pThread); //�����带 �߰��Ѵ�.
	Thread* GetThread(int index); //�����带 �����Ѵ�.

	void SetPDBR();

	UINT32		m_processId; //���μ��� ���̵�
	char		m_processName[256]; //���μ��� �̸�
	UINT32		m_dwRunState; //���μ��� ����
	UINT32		m_dwPriority; //���μ��� �켱����
	int			m_dwRunningTime; // ���μ��� CPU ���� �ð�
	UINT32		m_dwPageCount; //���μ����� �����ϴ� ������ ��
	UINT32		m_dwProcessType; //���μ��� Ÿ��		
		
	int			m_stackIndex; //�����尡 �����ɶ� ���� �� ������ �Ҵ��� �־�� �ϴµ� �׶� ���Ǵ� �ε���
	DoubleLinkedList m_threadList; //���μ������� ���� ������ ����Ʈ

	uint32_t	m_imageBase; //���Ϸ� ���� �ڵ带 �ε��� ��� ����. ���Ͽ� �޸𸮿� �ε�� �ּ�
	uint32_t	m_imageSize; //������ ũ��
	bool		m_IskernelProcess;

	PageDirectory* GetPageDirectory() { return m_pPageDirectory; }
	
	void SetPageDirectory(PageDirectory* pPageDirectory) 
	{
		if(pPageDirectory == 0)
			HaltSystem("PageDirectory is Null");;

		m_pPageDirectory = pPageDirectory;
	}
	void*		m_lpHeap; //���μ����� ����ϴ� ��

private:
	PageDirectory * m_pPageDirectory; //�� ���μ����� ����ϴ� ������ ���丮		
	
};

