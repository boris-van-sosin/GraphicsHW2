// ClippingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CGWork.h"
#include "ClippingDlg.h"
#include "afxdialogex.h"


// CClippingDlg dialog

IMPLEMENT_DYNAMIC(CClippingDlg, CDialogEx)

CClippingDlg::CClippingDlg(GeneralSettings& s, CWnd* pParent /*=NULL*/)
	: _refSettings(s), CDialogEx(CClippingDlg::IDD, pParent)
{

}

CClippingDlg::~CClippingDlg()
{
}

void CClippingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	m_load_model_normals_status = (int)_refSettings.load_normals_from_file;
	DDX_Radio(pDX, IDC_RADIO1, m_load_model_normals_status);
}

BEGIN_MESSAGE_MAP(CClippingDlg, CDialogEx)
	ON_COMMAND(IDOK, OnOk)
END_MESSAGE_MAP()

bool IsValidFloat(const CString& text)
{
	LPCTSTR ptr = (LPCTSTR)text;
	LPTSTR endptr;
	float value = _tcstof(ptr, &endptr);
	return (*ptr && endptr - ptr == text.GetLength());
}
bool IsValidInt(const CString& text)
{
	LPCTSTR ptr = (LPCTSTR)text;
	LPTSTR endptr;
	int value = _tcstod(ptr, &endptr);
	return (*ptr && endptr - ptr == text.GetLength());
}
// CClippingDlg message handlers
void CClippingDlg::OnOk() {
	CString nearStr, farStr, finenessStr, sensitivityStr;
	GetDlgItemText(IDC_EDIT1, nearStr);
	GetDlgItemText(IDC_EDIT2, farStr);
	GetDlgItemText(IDC_EDIT3, finenessStr);
	GetDlgItemText(IDC_EDIT4, sensitivityStr);

	double nearNum, farNum;
	int finenessNum;
	double sensitivityNum;

	if (nearStr != "" && IsValidFloat(nearStr))
	{
		nearNum = (double)_ttof(nearStr);
	}
	if (farStr != "" && IsValidFloat(farStr))
	{
		farNum = (double)_ttof(farStr);
	}
	if (finenessStr != "" && IsValidInt(finenessStr))
	{
		finenessNum = (double)_ttoi(finenessStr);
	}
	if (sensitivityStr != "" && IsValidFloat(sensitivityStr))
	{
		sensitivityNum = (double)_ttof(sensitivityStr);
	}

	if (nearNum > 0 && farNum > nearNum && finenessNum >= 2)
	{
		_refSettings._nearClippingPlane = nearNum;
		_refSettings._farClippingPlane = farNum;
		_refSettings._polygonFineness = finenessNum;
		_refSettings._sensitivity = sensitivityNum;

		UpdateData(TRUE);
		_refSettings.load_normals_from_file = (Normals::NormalsGeneration)m_load_model_normals_status;

		CDialog::OnOK();
	}

}

BOOL CClippingDlg::OnInitDialog()
{
	BOOL res = CDialogEx::OnInitDialog();
	if (!res)
	{
		return res;
	}
	CString str;
	str.Format(_T("%f"), _refSettings._nearClippingPlane);
	SetDlgItemText(IDC_EDIT1, str); //near
	str.Format(_T("%f"), _refSettings._farClippingPlane);
	SetDlgItemText(IDC_EDIT2, str); //far
	str.Format(_T("%d"), _refSettings._polygonFineness);
	SetDlgItemText(IDC_EDIT3, str); //polygon fineness
	str.Format(_T("%f"), _refSettings._sensitivity);
	SetDlgItemText(IDC_EDIT4, str); //sensitivity

	return res;
}
