// PerModel.cpp : implementation file
//

#include "stdafx.h"
#include "CGWork.h"
#include "PerModel.h"
#include "afxdialogex.h"


// CPerModel dialog

IMPLEMENT_DYNAMIC(CPerModel, CDialogEx)

CPerModel::CPerModel(CPerModelParam& param, CWnd* pParent /*=NULL*/)
: CDialogEx(CPerModel::IDD, pParent), m_orig_param(param), m_tmp_param(param)
{
	/*
	m_shading_type = 0;
	m_orig_param = param;
	m_tmp_param = param;
	*/
	m_shading_type = (int)param.Shading;
	m_remove_back_face = (int)param.removeBackFace;
	m_silluete = (int)param.silluete;
	m_boundry = (int)param.boundry;
	m_wireframe_solid = (int)param.is_wireframe;
	m_cast_shadow = (int)param.cast_shadow;
	m_force_opacity = (int)(param.forceOpacity);
}

CPerModel::~CPerModel()
{
}

void CPerModel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_shading_type);
	DDX_Radio(pDX, IDC_RADIO5, m_remove_back_face);
	DDX_Radio(pDX, IDC_RADIO7, m_silluete);
	DDX_Radio(pDX, IDC_RADIO9, m_boundry);
	DDX_Radio(pDX, IDC_RADIO11, m_wireframe_solid);
	DDX_Radio(pDX, IDC_RADIO13, m_cast_shadow);
	DDX_Radio(pDX, IDC_RADIO15, m_force_opacity);
}


BEGIN_MESSAGE_MAP(CPerModel, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CPerModel::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDOK, &CPerModel::OnBnClickedOk)
END_MESSAGE_MAP()


// CPerModel message handlers


void CPerModel::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	//CWnd *combobox = GetDlgItem(IDC_COMBO1);
	//GetDlg

}

BOOL CPerModel::OnInitDialog() {
	BOOL res = CDialogEx::OnInitDialog();
	CString str;
	CPerModelParam &param = m_tmp_param;
	str.Format(_T("%f"), param.AmbientCoefficient);
	SetDlgItemText(IDC_EDIT1, str);
	str.Format(_T("%f"), param.DiffuseCoefficient);
	SetDlgItemText(IDC_EDIT2, str);
	str.Format(_T("%f"), param.SpecularCoefficient);
	SetDlgItemText(IDC_EDIT3, str);
	str.Format(_T("%d"), param.SpecularPower);
	SetDlgItemText(IDC_EDIT4, str);
	str.Format(_T("%f"), param.AmbientIntensity);
	SetDlgItemText(IDC_EDIT5, str);
	str.Format(_T("%d"), param.shadow_wireframe_light_src);
	SetDlgItemText(IDC_EDIT6, str);
	str.Format(_T("%f"), param.opacity);
	SetDlgItemText(IDC_EDIT7, str);
	str.Format(_T("%d"), param.v_texture);
	SetDlgItemText(IDC_EDIT8, str);
	str.Format(_T("%f"), param.a);
	SetDlgItemText(IDC_EDIT9, str);
	str.Format(_T("%f"), param.turb_power);
	SetDlgItemText(IDC_EDIT10, str);
	return res;
}

void CPerModel::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	//now m_shading_type is updated
	m_tmp_param.Shading = (ShadingMode)m_shading_type;
	m_tmp_param.removeBackFace = (bool)m_remove_back_face;
	m_tmp_param.silluete = bool(m_silluete);
	m_tmp_param.boundry = (bool)m_boundry;
	m_tmp_param.is_wireframe = (bool)m_wireframe_solid;
	m_tmp_param.cast_shadow = (bool)m_cast_shadow;
	m_tmp_param.forceOpacity = (bool)m_force_opacity;

	CString str1, str2, str3, str4, str5, str6, str7, str8, str9, str10;
	GetDlgItemText(IDC_EDIT1, str1);
	GetDlgItemText(IDC_EDIT2, str2);
	GetDlgItemText(IDC_EDIT3, str3);
	GetDlgItemText(IDC_EDIT4, str4);
	GetDlgItemText(IDC_EDIT5, str5);
	GetDlgItemText(IDC_EDIT6, str6);
	GetDlgItemText(IDC_EDIT7, str7);
	GetDlgItemText(IDC_EDIT8, str8);
	GetDlgItemText(IDC_EDIT9, str9);
	GetDlgItemText(IDC_EDIT10, str10);

	m_tmp_param.AmbientCoefficient = (double)_ttof(str1);
	m_tmp_param.DiffuseCoefficient = (double)_ttof(str2);
	m_tmp_param.SpecularCoefficient = (double)_ttof(str3);
	m_tmp_param.SpecularPower = (int)_ttof(str4);
	m_tmp_param.AmbientIntensity = (double)_ttof(str5);
	m_tmp_param.shadow_wireframe_light_src = (int)_ttof(str6);
	m_tmp_param.opacity = (double)_ttof(str7);
	m_tmp_param.v_texture = (int)_ttof(str8);
	m_tmp_param.a = (double)_ttof(str9);
	m_tmp_param.turb_power = (double)_ttof(str10);
	if (m_tmp_param.opacity > 1)
	{
		m_tmp_param.opacity = 1.0;
	}

	m_orig_param = m_tmp_param;

	CDialogEx::OnOK();
}
