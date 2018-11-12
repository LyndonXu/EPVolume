// FitComboBox.cpp : 实现文件
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



// CFitComboBox 消息处理程序




void CFitComboBox::OnCbnDropdown()
{
	// TODO: 在此添加控件通知处理程序代码

	CClientDC csDC(this);
	int nWitdh = 10;
	int nSaveDC = csDC.SaveDC();

	//获取字体信息，
	csDC.SelectObject(GetFont());

	//计算最大的显示长度
	for (int i = 0; i < GetCount(); i++)
	{
		CString strLable = _T("");
		GetLBText(i, strLable);

		nWitdh = max(nWitdh, csDC.GetTextExtent(strLable).cx);
	}

	//多增加的冗余宽度
	nWitdh += 16;

	//设置下拉列表宽度
	SetDroppedWidth(nWitdh);
	//恢复实际dc
	csDC.RestoreDC(nSaveDC);
}
