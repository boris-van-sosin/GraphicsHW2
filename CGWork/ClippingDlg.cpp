// ClippingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CGWork.h"
#include "ClippingDlg.h"
#include "afxdialogex.h"


// CClippingDlg dialog

IMPLEMENT_DYNAMIC(CClippingDlg, CDialogEx)

CClippingDlg::CClippingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClippingDlg::IDD, pParent)
{

}

CClippingDlg::~CClippingDlg()
{
}

void CClippingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CClippingDlg, CDialogEx)
	ON_COMMAND(IDOK, OnOk)
END_MESSAGE_MAP()


// CClippingDlg message handlers
void CClippingDlg::OnOk() {
	CString s_near;
	SetDlgItemText(IDC_EDIT1, s_near);
}