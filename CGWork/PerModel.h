#pragma once
#include "Drawing.h"


// CPerModel dialog

class CPerModelParam {
public:
	bool removeBackFace = true;
	double AmbientCoefficient = 1.0, DiffuseCoefficient = 1.0, SpecularCoefficient = 1.0;
	int SpecularPower = 4;
	ShadingMode Shading = SHADING_GOURAUD;
};

class CPerModel : public CDialogEx
{
	DECLARE_DYNAMIC(CPerModel)

public:
	CPerModel(CPerModelParam& param, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPerModel();

	int m_shading_type;
	int m_remove_back_face;
	CPerModelParam m_tmp_param;
	CPerModelParam& m_orig_param;
	BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_PER_MODEL_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedOk();
};
