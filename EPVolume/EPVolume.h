
// EPVolume.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CEPVolumeApp: 
// �йش����ʵ�֣������ EPVolume.cpp
//

class CEPVolumeApp : public CWinApp
{
public:
	CEPVolumeApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CEPVolumeApp theApp;