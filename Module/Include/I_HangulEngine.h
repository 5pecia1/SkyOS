#pragma once

class I_HangulEngine
{
public:
	virtual bool SwitchMode() = 0; //�ѿ���ȯ
	virtual void Reset() = 0; //�� ���ڿ��� ���� �غ� �Ѵ�.
	virtual bool InputAscii(unsigned char letter) = 0; //�ƽ�Ű ���ڸ� ����ִ´�.
	virtual int GetString(char* buffer) = 0; //���� ���ڿ��� ��´�.	
};