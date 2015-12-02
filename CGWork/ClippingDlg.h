#pragma once


// CClippingDlg dialog

class CClippingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClippingDlg)

public:
	CClippingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CClippingDlg();

// Dialog Data
	enum { IDD = IDD_CLIPPING_DLG };

	void OnOk();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
