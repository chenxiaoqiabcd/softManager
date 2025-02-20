#ifndef __UIOPTIONLAYOUT_H__
#define __UIOPTIONLAYOUT_H__

#pragma once

namespace DuiLib
{
	class UILIB_API COptionLayoutUI : public CButtonLayoutUI
	{
		DECLARE_DUICONTROL(COptionLayoutUI)
	public:
		COptionLayoutUI();
		~COptionLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		bool Activate();
		void SetEnabled(bool bEnable = true);

		virtual void SwitchControlVisible();

		LPCTSTR GetSelectedImage();
		void SetSelectedImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedHotImage();
		void SetSelectedHotImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedPushedImage();
		void SetSelectedPushedImage(LPCTSTR pStrImage);

		void SetSelectedTextColor(DWORD dwTextColor);
		DWORD GetSelectedTextColor();

		void SetSelectedBkColor(DWORD dwBkColor);
		DWORD GetSelectBkColor();

		LPCTSTR GetSelectedForedImage();
		void SetSelectedForedImage(LPCTSTR pStrImage);

		void SetSelectedStateCount(int nCount);
		int GetSelectedStateCount() const;
		virtual LPCTSTR GetSelectedStateImage();
		virtual void SetSelectedStateImage(LPCTSTR pStrImage);

		void SetSelectedFont(int index);
		int GetSelectedFont() const;

		void SetSelectedBorderColor(DWORD dwColor) { m_dwSelectedBorderColor = dwColor; }	//add by liqs99
		DWORD GetSelectedBorderColor() const { return m_dwSelectedBorderColor; }			//add by liqs99

		LPCTSTR GetGroup() const;
		void SetGroup(LPCTSTR pStrGroupName = NULL);
		virtual bool IsSelected() const;
		virtual void Selected(bool bSelected, bool bTriggerEvent=true);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void PaintBkColor(HDC hDC);
		void PaintStatusImage(HDC hDC);
		void PaintForeImage(HDC hDC);
		void PaintText(HDC hDC);
		virtual void PaintBorder(HDC hDC); //add by liqs99

	protected:
		bool			m_bSelected;
		CDuiString		m_sGroupName;

		int				m_iSelectedFont;

		DWORD			m_dwSelectedBkColor;
		DWORD			m_dwSelectedTextColor;

		CDuiString		m_sSelectedImage;
		CDuiString		m_sSelectedHotImage;
		CDuiString		m_sSelectedPushedImage;
		CDuiString		m_sSelectedForeImage;

		int m_nSelectedStateCount;
		CDuiString m_sSelectedStateImage;

		DWORD m_dwSelectedBorderColor;
	};
} // namespace DuiLib

#endif // __UIOPTION_H__