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
}

CPerModel::~CPerModel()
{
}

void CPerModel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_shading_type);
}


BEGIN_MESSAGE_MAP(CPerModel, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CPerModel::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDOK, &CPerModel::OnBnClickedOk)
END_MESSAGE_MAP()


// CPerModel message handlers


void CPerModel::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	CWnd *combobox = GetDlgItem(IDC_COMBO1);
	//GetDlg

}



void CPerModel::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	//now m_shading_type is updated
	m_tmp_param.Shading = (ShadingMode)m_shading_type;
	m_orig_param = m_tmp_param;

	CDialogEx::OnOK();
}
