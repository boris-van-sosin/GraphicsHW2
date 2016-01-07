#pragma once

#include <atlimage.h>
#include "Drawing.h"

// CChooseColorDlg dialog

class Choose_color_param_t {
public:
	COLORREF model_color;
	bool model_force_color;
	COLORREF normal_color;
	COLORREF model_bbox_color;
	COLORREF subObj_bbox_color;
	COLORREF background_color;
	CImage background_image;
	ZBufferImage::BGImageMode bg_image_mode;

};

class CChooseColorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChooseColorDlg)

public:
	CChooseColorDlg(Choose_color_param_t* _param, CWnd* pParent = NULL);   // standard constructor
	virtual ~CChooseColorDlg();

// Dialog Data
	enum { IDD = IDD_COLOR_DIALOG };
	int m_bgimagetype;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	Choose_color_param_t local_param;
	Choose_color_param_t* input_param;

	void OnClrModel();
	void OnClrNormal();
	void OnClrToFile();
	void OnClrModelBBox();
	void OnClrSubObjBBox();
	void OnClrBackground();
	void OnOk();
	void OnBGImage();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
