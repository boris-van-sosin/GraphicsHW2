// ChooseColorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CGWork.h"
#include "ChooseColorDlg.h"
#include "afxdialogex.h"


// CChooseColorDlg dialog

IMPLEMENT_DYNAMIC(CChooseColorDlg, CDialogEx)

CChooseColorDlg::CChooseColorDlg(Choose_color_param_t* _param, CWnd* pParent /*=NULL*/)
	: CDialogEx(CChooseColorDlg::IDD, pParent), input_param(_param)
{
	local_param = *_param;
}

CChooseColorDlg::~CChooseColorDlg()
{
}

void CChooseColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChooseColorDlg, CDialogEx)
	ON_COMMAND(IDC_CLR_MODEL, OnClrModel)
	ON_COMMAND(IDOK, OnOk)
END_MESSAGE_MAP()


// CChooseColorDlg message handlers
void CChooseColorDlg::OnClrModel() {
	CColorDialog dlg(local_param.model_color);
	dlg.DoModal();
	COLORREF clr = dlg.GetColor();
	local_param.model_color = clr;
}

void CChooseColorDlg::OnOk() {
	*input_param = local_param;
	CDialog::OnOK();
}