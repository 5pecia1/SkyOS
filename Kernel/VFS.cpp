#include <VFS.h>
#include <string.h>

//������ġ�� �ִ� 26��
#define DEVICE_MAX 26

PFILESYSTEM _FileSystems[DEVICE_MAX];

//���� ����
FILE volOpenFile (const char* fname) {

	if (fname) {

		//�⺻ ���� ����̺�� a
		unsigned char device = 'a';
		
		char* filename = (char*) fname;

		// a: b: c: ����̺긦 �����ؼ� ������ ������ Ȯ���Ѵ�.
		if (fname[1]==':') {

			device = fname[0];
			filename += 3; //strip it from pathname
		}

		//! call filesystem
		if (_FileSystems [device - 'a']) 
		{			
			//������ ����� �� ������ ����̽� ���̵� �����ϰ� �����Ѵ�.
			FILE file = _FileSystems[device - 'a']->Open (filename);
			file.deviceID = device;
			return file;
		}
	}

	//��ȿ���� ����
	FILE file;
	file.flags = FS_INVALID;
	return file;
}

//���� �б�
void volReadFile (PFILE file, unsigned char* Buffer, unsigned int Length) {

	if (file)
		if (_FileSystems [file->deviceID - 'a'])
			_FileSystems[file->deviceID - 'a']->Read (file,Buffer,Length);
}

//���� �ݱ�
void volCloseFile (PFILE file) {

	if (file)
		if (_FileSystems [file->deviceID - 'a'])
			_FileSystems[file->deviceID - 'a']->Close (file);
}


//���� �ý��� ���
void volRegisterFileSystem (PFILESYSTEM fsys, unsigned int deviceID) {

	static int i=0;

	if (i < DEVICE_MAX)
		if (fsys) {

			_FileSystems[ deviceID ] = fsys;
			i++;
		}
}

//���� �ý��ۿ��� ����
void volUnregisterFileSystem (PFILESYSTEM fsys) {

	for (int i=0;i < DEVICE_MAX; i++)
		if (_FileSystems[i]==fsys)
			_FileSystems[i]=0;
}

//����̽� ���̵�� ���� �ý��ۿ��� ����
void volUnregisterFileSystemByID (unsigned int deviceID) {

	if (deviceID < DEVICE_MAX)
		_FileSystems [deviceID] = 0;
}