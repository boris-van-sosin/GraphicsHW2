#pragma once

#include "Geometry.h"

// CClippingDlg dialog

struct GeneralSettings
{
	int _polygonFineness;
	double _nearClippingPlane;
	double _farClippingPlane;
	double _sensitivity;
	Normals::NormalsGeneration load_normals_from_file;
};

class CClippingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClippingDlg)

public:
	CClippingDlg(GeneralSettings& s, CWnd* pParent = NULL);   // standard constructor
	virtual ~CClippingDlg();

// Dialog Data
	enum { IDD = IDD_CLIPPING_DLG };

	void OnOk();
	BOOL OnInitDialog();

	int m_load_model_normals_status = 0;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	GeneralSettings& _refSettings;
};
