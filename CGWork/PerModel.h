#pragma once
#include "Drawing.h"


// CPerModel dialog

class CPerModelParam {
public:
	bool removeBackFace = true;
	double AmbientCoefficient = 1.0, DiffuseCoefficient = 1.0, SpecularCoefficient = 1.0;
	int SpecularPower = 4;
	ShadingMode Shading = SHADING_GOURAUD;
	bool silluete = true, boundry = true;
	double AmbientIntensity = 1;
	bool is_wireframe = false;
	bool cast_shadow = false;
	int shadow_wireframe_light_src = -1;
	double opacity;
	bool forceOpacity;
	int v_texture = 0;
	double a, turb_power;
};

class CPerModel : public CDialogEx
{
	DECLARE_DYNAMIC(CPerModel)

public:
	CPerModel(CPerModelParam& param, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPerModel();

	int m_shading_type;
	int m_remove_back_face;
	int m_silluete;
	int m_boundry;
	int m_wireframe_solid;
	int m_cast_shadow;
	int m_force_opacity;
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
