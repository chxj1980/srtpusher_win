/* ----------------------------------------------------------
 * �ļ����ƣ�DS_AudioVideoDevices.h
 *
 * ���ߣ��ؽ���
 *
 * ΢�ţ�splashcn
 *
 * ���ͣ�http://www.firstsolver.com/wordpress/
 *
 * ����������
 *      Visual Studio V2017
 *
 * �汾��ʷ��
 *		V1.1	2019��01��01��
 *				����Json���������C#/JAVA����
 *
 *		V1.0    2010��10��09��
 *				��ȡ��Ƶ��Ƶ�����豸�б�
 * ---------------------------------------------------------- */
#pragma once

#include <windows.h>
#include <vector>
#include <dshow.h>

#ifndef MACRO_GROUP_DEVICENAME
#define MACRO_GROUP_DEVICENAME

#define MAX_FRIENDLY_NAME_LENGTH	128
#define MAX_MONIKER_NAME_LENGTH		256

typedef struct _TDeviceName
{
	WCHAR Name[MAX_FRIENDLY_NAME_LENGTH];	// �豸�Ѻ���
	WCHAR Moniker[MAX_MONIKER_NAME_LENGTH];	// �豸Moniker��
} TDeviceName;
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	/*
	���ܣ�����Ԥ���ض�̬��
	����ֵ��TRUE
	*/
	BOOL WINAPI Prepare();

	/*
	���ܣ���ȡ��Ƶ��Ƶ�����豸�б�
	����˵����
		devices�����ڴ洢���ص��豸�Ѻ�����Moniker��
		guid��
			CLSID_AudioInputDeviceCategory����ȡ��Ƶ�����豸�б�
			CLSID_VideoInputDeviceCategory����ȡ��Ƶ�����豸�б�
	����ֵ��
		�������
	˵����
		����DirectShow
		�б��еĵ�һ���豸Ϊϵͳȱʡ�豸
		capGetDriverDescriptionֻ�ܻ���豸������
	*/
	HRESULT WINAPI DS_GetAudioVideoInputDevices(OUT std::vector<TDeviceName> &devices, REFGUID guid);

	/*
	���ܣ���ȡ��Ƶ��Ƶ�����豸�б�
	����˵����
		devices�������Json�ַ��������ڴ洢���ص��豸Name��Moniker
		cch���������Ч�ַ��������������ַ���������
		video��
			FALSE����ȡ��Ƶ�����豸�б�
			TRUE����ȡ��Ƶ�����豸�б�
	����ֵ��
		�������
	˵����
		����DirectShow
		�б��еĵ�һ���豸Ϊϵͳȱʡ�豸
		capGetDriverDescriptionֻ�ܻ���豸������
	*/
	HRESULT WINAPI DS_GetDeviceSources(OUT TCHAR* &devices, OUT int &cch, BOOL video);

	/*
	���ܣ��ͷ��ڴ�
	������
		p��Ҫ�ͷŵ��ڴ�ָ��
	*/
	VOID WINAPI FreeMemory(LPVOID p);

#ifdef __cplusplus
}
#endif
