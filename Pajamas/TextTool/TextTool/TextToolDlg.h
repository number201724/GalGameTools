// TextToolDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CTextToolDlg 对话框
class CTextToolDlg : public CDialog
{
// 构造
public:
	CTextToolDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TEXTTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void GetTextData();
	void SetTextData();
	void OnOK();
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_OrigText;
	CString m_NewText;
	CSpinButtonCtrl m_Spin_Count;
	CString m_IndexEdit;
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnBnClickedButton5();
	CString m_MaxCount;
	afx_msg void OnBnClickedButton1();
	CEdit m_CountEdit;
	afx_msg void OnEnChangeEdit1();
	CString m_CountValue;
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton6();
};
