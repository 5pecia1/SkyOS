/**
 *  file    RAMDisk.h
 *  date    2009/05/22
 *  author  kkamagui 
 *  brief   �� ��ũ�� ���õ� �Լ��� ������ ��� ����
 *          Copyright(c)2008 All rights reserved by kkamagui
 */

#ifndef __RAMDISK_H__
#define __RAMDISK_H__

#include "windef.h"

#define FILESYSTEM_SIGNATURE                0x7E38CF10
 // Ŭ�������� ũ��(���� ��), 4Kbyte
#define FILESYSTEM_SECTORSPERCLUSTER        8
 // ���� Ŭ�������� ������ ǥ��
#define FILESYSTEM_LASTCLUSTER              0xFFFFFFFF
 // �� Ŭ������ ǥ��
#define FILESYSTEM_FREECLUSTER              0x00
 // ��Ʈ ���͸��� �ִ� �ִ� ���͸� ��Ʈ���� ��
#define FILESYSTEM_MAXDIRECTORYENTRYCOUNT   ( ( FILESYSTEM_SECTORSPERCLUSTER * 512 ) / \
        sizeof( DIRECTORYENTRY ) )
 // Ŭ�������� ũ��(����Ʈ ��)
#define FILESYSTEM_CLUSTERSIZE              ( FILESYSTEM_SECTORSPERCLUSTER * 512 )

 // �ڵ��� �ִ� ����, �ִ� �½�ũ ���� 3��� ����
#define FILESYSTEM_HANDLE_MAXCOUNT          ( TASK_MAXCOUNT * 3 )

 // ���� �̸��� �ִ� ����
#define FILESYSTEM_MAXFILENAMELENGTH        24

 // �ڵ��� Ÿ���� ����
#define FILESYSTEM_TYPE_FREE                0
#define FILESYSTEM_TYPE_FILE                1
#define FILESYSTEM_TYPE_DIRECTORY           2

 // SEEK �ɼ� ����
#define FILESYSTEM_SEEK_SET                 0
#define FILESYSTEM_SEEK_CUR                 1
#define FILESYSTEM_SEEK_END                 2

#pragma pack( push, 1 )

 // ��Ƽ�� �ڷᱸ��
typedef struct kPartitionStruct
{
	// ���� ���� �÷���. 0x80�̸� ���� ������ ��Ÿ���� 0x00�� ���� �Ұ�
	BYTE bBootableFlag;
	// ��Ƽ���� ���� ��巹��. ����� ���� ������� ������ �Ʒ��� LBA ��巹���� ��� ���
	BYTE vbStartingCHSAddress[3];
	// ��Ƽ�� Ÿ��
	BYTE bPartitionType;
	// ��Ƽ���� ������ ��巹��. ����� ���� ��� �� ��
	BYTE vbEndingCHSAddress[3];
	// ��Ƽ���� ���� ��巹��. LBA ��巹���� ��Ÿ�� ��
	DWORD dwStartingLBAAddress;
	// ��Ƽ�ǿ� ���Ե� ���� ��
	DWORD dwSizeInSector;
} _PARTITION;


// MBR �ڷᱸ��
typedef struct kMBRStruct
{
	// ��Ʈ �δ� �ڵ尡 ��ġ�ϴ� ����
	BYTE vbBootCode[430];

	// ���� �ý��� �ñ׳�ó, 0x7E38CF10
	DWORD dwSignature;
	// ����� ������ ���� ��
	DWORD dwReservedSectorCount;
	// Ŭ������ ��ũ ���̺� ������ ���� ��
	DWORD dwClusterLinkSectorCount;
	// Ŭ�������� ��ü ����
	DWORD dwTotalClusterCount;

	// ��Ƽ�� ���̺�
	_PARTITION vstPartition[4];

	// ��Ʈ �δ� �ñ׳�ó, 0x55, 0xAA
	BYTE vbBootLoaderSignature[2];
} MBR;

 // HDD�� ���� ������ ��Ÿ���� ����ü
typedef struct kHDDInformationStruct
{
	// ������
	WORD wConfiguation;

	// �Ǹ��� ��
	WORD wNumberOfCylinder;
	WORD wReserved1;

	// ��� ��
	WORD wNumberOfHead;
	WORD wUnformattedBytesPerTrack;
	WORD wUnformattedBytesPerSector;

	// �Ǹ����� ���� ��
	WORD wNumberOfSectorPerCylinder;
	WORD wInterSectorGap;
	WORD wBytesInPhaseLock;
	WORD wNumberOfVendorUniqueStatusWord;

	// �ϵ� ��ũ�� �ø��� �ѹ�
	WORD vwSerialNumber[10];
	WORD wControllerType;
	WORD wBufferSize;
	WORD wNumberOfECCBytes;
	WORD vwFirmwareRevision[4];

	// �ϵ� ��ũ�� �� ��ȣ
	WORD vwModelNumber[20];
	WORD vwReserved2[13];

	// ��ũ�� �� ���� ��
	DWORD dwTotalSectors;
	WORD vwReserved3[196];
} HDDINFORMATION;

#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// �� ��ũ�� �� ���� ��. 8Mbyte ũ��� ����
#define RDD_TOTALSECTORCOUNT        ( 8 * 1024 * 1024 / 512)

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// �� ��ũ�� �ڷᱸ���� �����ϴ� ����ü
typedef struct kRDDManagerStruct
{
    // �� ��ũ������ �Ҵ� ���� �޸� ����
    BYTE* pbBuffer;
    
    // �� ���� ��
    DWORD dwTotalSectorCount;
} RDDMANAGER;

#pragma pack( pop)

////////////////////////////////////////////////////////////////////////////////
//
//  �Լ�
//
////////////////////////////////////////////////////////////////////////////////
bool kInitializeRDD( DWORD dwTotalSectorCount );
bool kReadRDDInformation(bool bPrimary, bool bMaster,
        HDDINFORMATION* pstHDDInformation );
int kReadRDDSector(bool bPrimary, bool bMaster, DWORD dwLBA, int iSectorCount,
        char* pcBuffer );
int kWriteRDDSector(bool bPrimary, bool bMaster, DWORD dwLBA, int iSectorCount,
        char* pcBuffer );

#endif /*__RAMDISK_H__*/
