#pragma once


// CFitComboBox

class CFitComboBox : public CComboBox
{
	DECLARE_DYNAMIC(CFitComboBox)

public:
	CFitComboBox();
	virtual ~CFitComboBox();

protected:
	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnCbnDropdown();
	afx_msg void OnCbnDropdown();
};


