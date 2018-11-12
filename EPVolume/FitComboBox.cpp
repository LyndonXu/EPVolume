// FitComboBox.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EPVolume.h"
#include "FitComboBox.h"


// CFitComboBox

IMPLEMENT_DYNAMIC(CFitComboBox, CComboBox)

CFitComboBox::CFitComboBox()
{

}

CFitComboBox::~CFitComboBox()
{
}


BEGIN_MESSAGE_MAP(CFitComboBox, CComboBox)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, &CFitComboBox::OnCbnDropdown)
END_MESSAGE_MAP()



// CFitComboBox ��Ϣ�������




void CFitComboBox::OnCbnDropdown()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CClientDC csDC(this);
	int nWitdh = 10;
	int nSaveDC = csDC.SaveDC();

	//��ȡ������Ϣ��
	csDC.SelectObject(GetFont());

	//����������ʾ����
	for (int i = 0; i < GetCount(); i++)
	{
		CString strLable = _T("");
		GetLBText(i, strLable);

		nWitdh = max(nWitdh, csDC.GetTextExtent(strLable).cx);
	}

	//�����ӵ�������
	nWitdh += 16;

	//���������б���
	SetDroppedWidth(nWitdh);
	//�ָ�ʵ��dc
	csDC.RestoreDC(nSaveDC);
}
