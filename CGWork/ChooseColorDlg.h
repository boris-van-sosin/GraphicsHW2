#pragma once


// CChooseColorDlg dialog

class CChooseColorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChooseColorDlg)

public:
	CChooseColorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChooseColorDlg();

// Dialog Data
	enum { IDD = IDD_COLOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
