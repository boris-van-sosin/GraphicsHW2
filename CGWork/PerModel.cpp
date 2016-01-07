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
}

CPerModel::~CPerModel()
{
}

void CPerModel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_shading_type);
	DDX_Radio(pDX, IDC_RADIO5, m_remove_back_face);
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
	return res;
}

void CPerModel::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	//now m_shading_type is updated
	m_tmp_param.Shading = (ShadingMode)m_shading_type;
	m_tmp_param.removeBackFace = (bool)m_remove_back_face;

	CString str1, str2, str3, str4;
	GetDlgItemText(IDC_EDIT1, str1);
	GetDlgItemText(IDC_EDIT2, str2);
	GetDlgItemText(IDC_EDIT3, str3);
	GetDlgItemText(IDC_EDIT4, str4);

	m_tmp_param.AmbientCoefficient = (double)_ttof(str1);
	m_tmp_param.DiffuseCoefficient = (double)_ttof(str2);
	m_tmp_param.SpecularCoefficient = (double)_ttof(str3);
	m_tmp_param.SpecularPower = (int)_ttof(str4);


	m_orig_param = m_tmp_param;

	CDialogEx::OnOK();
}
