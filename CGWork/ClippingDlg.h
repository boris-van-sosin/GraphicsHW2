#pragma once


// CClippingDlg dialog

struct GeneralSettings
{
	int _polygonFineness;
	double _nearClippingPlane;
	double _farClippingPlane;
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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	GeneralSettings& _refSettings;
};
