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
	switch (_param->bg_image_mode)
	{
	case ZBufferImage::BGImageMode::STRECH:
		m_bgimagetype = 0;
		break;
	case ZBufferImage::BGImageMode::REPEAT:
		m_bgimagetype = 1;
		break;
	default:
		ASSERT(0);	// maybe deletes assert in release mode
		m_bgimagetype = 0;
	}
}

CChooseColorDlg::~CChooseColorDlg()
{
}

void CChooseColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_bgimagetype);
}


BEGIN_MESSAGE_MAP(CChooseColorDlg, CDialogEx)
	ON_COMMAND(IDC_CLR_MODEL, OnClrModel)
	ON_COMMAND(IDC_CLR_NORMAL, OnClrNormal)
	ON_COMMAND(IDC_CLR_RESET_TO_FILE, OnClrToFile)
	ON_COMMAND(IDC_CLR_MODEL_BBOX, OnClrModelBBox)
	ON_COMMAND(IDC_CLR_SUB_BBOX, OnClrSubObjBBox)
	ON_COMMAND(IDC_CLR_BG, OnClrBackground)
	ON_COMMAND(IDOK, OnOk)
	ON_COMMAND(IDC_BGIMAGE, OnBGImage)
	ON_BN_CLICKED(IDOK, &CChooseColorDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChooseColorDlg message handlers
void CChooseColorDlg::OnClrModel() {
	CColorDialog dlg(local_param.model_color);
	if (dlg.DoModal() == IDOK)
	{
		COLORREF clr = dlg.GetColor();
		local_param.model_color = clr;
		local_param.model_force_color = true;
	}
}


void CChooseColorDlg::OnClrNormal() {
	CColorDialog dlg(local_param.normal_color);
	if (dlg.DoModal() == IDOK)
	{
		COLORREF clr = dlg.GetColor();
		local_param.normal_color = clr;
	}
}

void CChooseColorDlg::OnClrToFile() {
	local_param.model_force_color = false;
}

void CChooseColorDlg::OnOk() {
	UpdateData(TRUE);
	switch (m_bgimagetype)
	{
	case 0:
		local_param.bg_image_mode = ZBufferImage::BGImageMode::STRECH;
		break;
	case 1:
		local_param.bg_image_mode = ZBufferImage::BGImageMode::REPEAT;
		break;
	default:
		ASSERT(0);
		local_param.bg_image_mode = ZBufferImage::BGImageMode::STRECH;
		break;
	}

	*input_param = local_param;
	CDialog::OnOK();
}

void CChooseColorDlg::OnClrModelBBox()
{
	CColorDialog dlg(local_param.model_bbox_color);
	if (dlg.DoModal() == IDOK)
	{
		COLORREF clr = dlg.GetColor();
		local_param.model_bbox_color = clr;
	}
}

void CChooseColorDlg::OnClrSubObjBBox()
{
	CColorDialog dlg(local_param.subObj_bbox_color);
	if (dlg.DoModal() == IDOK)
	{
		COLORREF clr = dlg.GetColor();
		local_param.subObj_bbox_color = clr;
	}
}

void CChooseColorDlg::OnClrBackground()
{
	CColorDialog dlg(local_param.background_color);
	if (dlg.DoModal() == IDOK)
	{
		COLORREF clr = dlg.GetColor();
		local_param.background_color = clr;
	}
}

void CChooseColorDlg::OnBGImage()
{
	TCHAR szFilters[] = _T("PNG images (*.itd)|*.itd|BMP images (*.bmp)|*.bmp|JPG images (*.jpg)|*.jpg|JPG images (*.jpeg)|*.jpeg|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("png"), _T("*.png"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

	if (dlg.DoModal() == IDOK)
	{
		if (!local_param.background_image.IsNull())
		{
			local_param.background_image.Destroy();
		}
		local_param.background_image.Load(dlg.GetPathName());
	}
}


void CChooseColorDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}
