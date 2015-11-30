// ChooseColorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CGWork.h"
#include "ChooseColorDlg.h"
#include "afxdialogex.h"


// CChooseColorDlg dialog

IMPLEMENT_DYNAMIC(CChooseColorDlg, CDialogEx)

CChooseColorDlg::CChooseColorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChooseColorDlg::IDD, pParent)
{

}

CChooseColorDlg::~CChooseColorDlg()
{
}

void CChooseColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChooseColorDlg, CDialogEx)
END_MESSAGE_MAP()


// CChooseColorDlg message handlers
