#include "StdAfx.h"
#include "UIDynamicLayout.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CDynamicLayoutUI)
	CDynamicLayoutUI::CDynamicLayoutUI() : m_iSepWidth(0), m_iSepHeight(0), m_bImmMode(false)
	{
		m_ptLastMouse.x = m_ptLastMouse.y = 0;
		::ZeroMemory(&m_rcNewPos, sizeof(m_rcNewPos));

		m_pCalcControl = NULL;
		::ZeroMemory(&m_rcCalcChild, sizeof(RECT));
		m_bCalcResult = false;

		m_eLayout = Layout_HorizontalLayout;
		m_eSepAction = eSepNull;
	}

	LPCTSTR CDynamicLayoutUI::GetClass() const
	{
		return _T("DynamicLayoutUI");
	}

	LPVOID CDynamicLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_DYNAMICLAYOUT) == 0 ) return static_cast<CDynamicLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CDynamicLayoutUI::GetControlFlags() const
	{
		if( IsEnabled() && m_iSepWidth != 0 ) return UIFLAG_SETCURSOR;
		else return 0;
	}

	void CDynamicLayoutUI::SetLayout(emLayoutType eMode) 
	{ 
		if(m_eLayout == eMode) return;
		m_eLayout = eMode;
		NeedUpdate();
	}

	emLayoutType CDynamicLayoutUI::GetLayout() const { return m_eLayout; }

	SIZE CDynamicLayoutUI::EstimateSize(SIZE szAvailable)
	{
		return __super::EstimateSize(szAvailable);
	}

	void CDynamicLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		if(GetLayout() == Layout_HorizontalLayout)
		{
			SetPosHorizontalLayout(rc, bNeedInvalidate);
		}
		else if(GetLayout() == Layout_VerticalLayout)
		{
			SetPosVerticalLayout(rc, bNeedInvalidate);
		}
	}

	void CDynamicLayoutUI::SetPosHorizontalLayout(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		// Adjust for inset
		RECT rcInset = GetInset();
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		if( m_items.GetSize() == 0) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		//if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
		//	szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
			szAvailable.cy += m_pVerticalScrollBar->GetScrollRange();

		//第一次循环，计算哪些控件需要自动布局，以及剩余可自动布局的空间。

		int cyNeeded = 0;
		int nAdjustables = 0;	//需要自动布局的控件数量
		int cxFixed = 0;
		int nEstimateNum = 0;
		SIZE szControlAvailable;
		int iControlMaxWidth = 0;
		int iControlMaxHeight = 0;
		for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;
			szControlAvailable = szAvailable;
			RECT rcPadding = pControl->GetPadding();
			szControlAvailable.cy -= rcPadding.top + rcPadding.bottom;
			iControlMaxWidth = pControl->GetFixedWidth();
			iControlMaxHeight = pControl->GetFixedHeight();
			if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth(); 
			if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
			if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
			if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
			SIZE sz = pControl->EstimateSize(szControlAvailable);
			if(sz.cx == 0 && pControl->GetFixedWidthPercent() > 0)
			{
				sz.cx = szAvailable.cx * pControl->GetFixedWidthPercent() / 100;
			}

			if( sz.cx == 0 ) //控件的宽度为0，需要自动布局
			{ 
				nAdjustables++;			
			}
			else {
				if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
				if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			}
			cxFixed += sz.cx + pControl->GetPadding().left + pControl->GetPadding().right;

			sz.cy = MAX(sz.cy, 0);
			if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			cyNeeded = MAX(cyNeeded, sz.cy + rcPadding.top + rcPadding.bottom);
			nEstimateNum++;
		}
		cxFixed += (nEstimateNum - 1) * m_iChildPadding;
		// Place elements
		int cxNeeded = 0;
		int cxExpand = 0;	//需要自动调整控件的宽度，也就是 width==0 的子控件
		if( nAdjustables > 0 ) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosX = rc.left;

		// 滚动条
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		else {
			// 子控件横向对其方式
			if(nAdjustables <= 0) {
				UINT iChildAlign = GetChildAlign(); 
				if (iChildAlign == DT_CENTER) {
					iPosX += (szAvailable.cx -cxFixed) / 2;
				}
				else if (iChildAlign == DT_RIGHT) {
					iPosX += (szAvailable.cx - cxFixed);
				}
			}
		}

		//第二次循环，剩余可自动布局的空间均分给需要自动布局的子控件。

		int iEstimate = 0;
		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;
		for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it2);
				continue;
			}

			iEstimate += 1;
			RECT rcPadding = pControl->GetPadding();
			szRemaining.cx -= rcPadding.left;

			szControlAvailable = szRemaining;
			szControlAvailable.cy -= rcPadding.top + rcPadding.bottom;
			iControlMaxWidth = pControl->GetFixedWidth();
			iControlMaxHeight = pControl->GetFixedHeight();
			if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth(); 
			if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
			if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
			if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
			cxFixedRemaining = cxFixedRemaining - (rcPadding.left + rcPadding.right);
			if (iEstimate > 1) cxFixedRemaining = cxFixedRemaining - m_iChildPadding;
			SIZE sz = pControl->EstimateSize(szControlAvailable);
			if(sz.cx == 0 && pControl->GetFixedWidthPercent() > 0)
			{
				sz.cx = szAvailable.cx * pControl->GetFixedWidthPercent() / 100;
			}

			if( sz.cx == 0 ) {
				iAdjustable++;
				sz.cx = cxExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if( iAdjustable == nAdjustables ) {
					sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
				} 
				if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
				if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			}
			else {
				if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
				if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
				cxFixedRemaining -= sz.cx;
			}

			if(sz.cy == 0 && pControl->GetFixedHeight() == 0 && pControl->GetFixedHeightPercent() > 0) 
				sz.cy = szAvailable.cy * pControl->GetFixedHeightPercent() / 100;
			if( sz.cy == 0 ) sz.cy = szAvailable.cy - rcPadding.top - rcPadding.bottom;
			if( sz.cy < 0 ) sz.cy = 0;
			if( sz.cy > szControlAvailable.cy ) sz.cy = szControlAvailable.cy;
			if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();

			UINT iChildAlign = GetChildVAlign(); 
			if (iChildAlign == DT_VCENTER) {
				int iPosY = (rc.bottom + rc.top) / 2;
				if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
					iPosY += m_pVerticalScrollBar->GetScrollRange() / 2;
					iPosY -= m_pVerticalScrollBar->GetScrollPos();
				}
				RECT rcCtrl = { iPosX + rcPadding.left, iPosY - sz.cy/2, iPosX + sz.cx + rcPadding.left, iPosY + sz.cy - sz.cy/2 };
				if(m_pCalcControl)
				{
					if(m_pCalcControl == pControl) { m_rcCalcChild = rcCtrl; m_bCalcResult = true; return; }
				}
				else
					pControl->SetPos(rcCtrl, false);
			}
			else if (iChildAlign == DT_BOTTOM) {
				int iPosY = rc.bottom;
				if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
					iPosY += m_pVerticalScrollBar->GetScrollRange();
					iPosY -= m_pVerticalScrollBar->GetScrollPos();
				}
				RECT rcCtrl = { iPosX + rcPadding.left, iPosY - rcPadding.bottom - sz.cy, iPosX + sz.cx + rcPadding.left, iPosY - rcPadding.bottom };
				if(m_pCalcControl)
				{
					if(m_pCalcControl == pControl) { m_rcCalcChild = rcCtrl; m_bCalcResult = true; return; }
				}
				else
					pControl->SetPos(rcCtrl, false);
			}
			else {
				int iPosY = rc.top;
				if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
					iPosY -= m_pVerticalScrollBar->GetScrollPos();
				}
				RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + sz.cx + rcPadding.left, iPosY + sz.cy + rcPadding.top };
				if(m_pCalcControl)
				{
					if(m_pCalcControl == pControl) { m_rcCalcChild = rcCtrl; m_bCalcResult = true; return; }
				}
				else
					pControl->SetPos(rcCtrl, false);
			}

			iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
			cxNeeded += sz.cx + rcPadding.left + rcPadding.right;
			szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
		}
		cxNeeded += (nEstimateNum - 1) * m_iChildPadding;

		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}

	void CDynamicLayoutUI::SetPosVerticalLayout(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		// Adjust for inset
		RECT rcInset = GetInset();
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		if( m_items.GetSize() == 0) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
			szAvailable.cy += m_pVerticalScrollBar->GetScrollRange();

		int cxNeeded = 0;
		int nAdjustables = 0;
		int cyFixed = 0;
		int nEstimateNum = 0;
		SIZE szControlAvailable;
		int iControlMaxWidth = 0;
		int iControlMaxHeight = 0;
		for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;
			szControlAvailable = szAvailable;
			RECT rcPadding = pControl->GetPadding();
			szControlAvailable.cx -= rcPadding.left + rcPadding.right;
			iControlMaxWidth = pControl->GetFixedWidth();
			iControlMaxHeight = pControl->GetFixedHeight();
			if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth(); 
			if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
			if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
			if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
			SIZE sz = pControl->EstimateSize(szControlAvailable);
			if(sz.cy == 0 && pControl->GetFixedHeightPercent() > 0)
			{
				sz.cy = szAvailable.cy * pControl->GetFixedHeightPercent() / 100;
			}

			if( sz.cy == 0 ) {
				nAdjustables++;
			}
			else {
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			}
			cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

			sz.cx = MAX(sz.cx, 0);
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			cxNeeded = MAX(cxNeeded, sz.cx + rcPadding.left + rcPadding.right);
			nEstimateNum++;
		}
		cyFixed += (nEstimateNum - 1) * m_iChildPadding;

		// Place elements
		int cyNeeded = 0;
		int cyExpand = 0;
		if( nAdjustables > 0 ) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosY = rc.top;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			iPosY -= m_pVerticalScrollBar->GetScrollPos();
		}
		else {
			// 子控件垂直对其方式
			if(nAdjustables <= 0) {
				UINT iChildAlign = GetChildVAlign(); 
				if (iChildAlign == DT_VCENTER) {
					iPosY += (szAvailable.cy -cyFixed) / 2;
				}
				else if (iChildAlign == DT_BOTTOM) {
					iPosY += (szAvailable.cy - cyFixed);
				}
			}
		}
		int iEstimate = 0;
		int iAdjustable = 0;
		int cyFixedRemaining = cyFixed;
		for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it2);
				continue;
			}

			iEstimate += 1;
			RECT rcPadding = pControl->GetPadding();
			szRemaining.cy -= rcPadding.top;

			szControlAvailable = szRemaining;
			szControlAvailable.cx -= rcPadding.left + rcPadding.right;
			iControlMaxWidth = pControl->GetFixedWidth();
			iControlMaxHeight = pControl->GetFixedHeight();
			if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth(); 
			if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
			if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
			if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
			cyFixedRemaining = cyFixedRemaining - (rcPadding.top + rcPadding.bottom);
			if (iEstimate > 1) cyFixedRemaining = cyFixedRemaining - m_iChildPadding;
			SIZE sz = pControl->EstimateSize(szControlAvailable);
			if(sz.cy == 0 && pControl->GetFixedHeightPercent() > 0)
			{
				sz.cy = szAvailable.cy * pControl->GetFixedHeightPercent() / 100;
			}

			if( sz.cy == 0 ) {
				iAdjustable++;
				sz.cy = cyExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if( iAdjustable == nAdjustables ) {
					sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
				} 
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			}
			else {
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
				cyFixedRemaining -= sz.cy;
			}

			sz.cx = MAX(sz.cx, 0);
			if(sz.cx == 0 && pControl->GetFixedWidth() == 0 && pControl->GetFixedWidthPercent() > 0) 
				sz.cx = szAvailable.cx * pControl->GetFixedWidthPercent() / 100;
			if( sz.cx == 0 ) sz.cx = szAvailable.cx - rcPadding.left - rcPadding.right;
			if( sz.cx > szControlAvailable.cx ) sz.cx = szControlAvailable.cx;
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();

			UINT iChildAlign = GetChildAlign(); 
			if (iChildAlign == DT_CENTER) {
				int iPosX = (rc.right + rc.left) / 2;
				if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
					iPosX += m_pHorizontalScrollBar->GetScrollRange() / 2;
					iPosX -= m_pHorizontalScrollBar->GetScrollPos();
				}
				RECT rcCtrl = { iPosX - sz.cx/2, iPosY + rcPadding.top, iPosX + sz.cx - sz.cx/2, iPosY + sz.cy + rcPadding.top };
				if(m_pCalcControl)
				{
					if(m_pCalcControl == pControl) { m_rcCalcChild = rcCtrl; m_bCalcResult = true; return; }
				}
				else
					pControl->SetPos(rcCtrl, false);
			}
			else if (iChildAlign == DT_RIGHT) {
				int iPosX = rc.right;
				if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
					iPosX += m_pHorizontalScrollBar->GetScrollRange();
					iPosX -= m_pHorizontalScrollBar->GetScrollPos();
				}
				RECT rcCtrl = { iPosX - rcPadding.right - sz.cx, iPosY + rcPadding.top, iPosX - rcPadding.right, iPosY + sz.cy + rcPadding.top };
				if(m_pCalcControl)
				{
					if(m_pCalcControl == pControl) { m_rcCalcChild = rcCtrl; m_bCalcResult = true; return; }
				}
				else
					pControl->SetPos(rcCtrl, false);
			}
			else {
				int iPosX = rc.left;
				if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
					iPosX -= m_pHorizontalScrollBar->GetScrollPos();
				}
				RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top };
				if(m_pCalcControl)
				{
					if(m_pCalcControl == pControl) { m_rcCalcChild = rcCtrl; m_bCalcResult = true; return; }
				}
				else
					pControl->SetPos(rcCtrl, false);
			}

			iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
			cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
			szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
		}
		cyNeeded += (nEstimateNum - 1) * m_iChildPadding;

		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}

	void CDynamicLayoutUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode ) {
			RECT rcSeparator = GetThumbRect(true);
			CRenderEngine::DrawColor(hDC, rcSeparator, 0xAA000000);
		}
	}

	void CDynamicLayoutUI::SetSepHeight(int iHeight)
	{
		m_iSepHeight = iHeight;
	}

	int CDynamicLayoutUI::GetSepHeight() const
	{
		return m_iSepHeight;
	}

	void CDynamicLayoutUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	int CDynamicLayoutUI::GetSepWidth() const
	{
		return m_iSepWidth;
	}

	void CDynamicLayoutUI::SetSepImmMode(bool bImmediately)
	{
		if( m_bImmMode == bImmediately ) return;
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode && m_pManager != NULL ) {
			m_pManager->RemovePostPaint(this);
		}

		m_bImmMode = bImmediately;
	}

	bool CDynamicLayoutUI::IsSepImmMode() const
	{
		return m_bImmMode;
	}

	void CDynamicLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("layout")) == 0 )
		{
			if(_tcsicmp(pstrValue, _T("VerticalLayout")) == 0) SetLayout(Layout_VerticalLayout);
			else SetLayout(Layout_HorizontalLayout);
		}
		else if( _tcsicmp(pstrName, _T("sepwidth")) == 0 ) SetSepWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("sepheight")) == 0 ) SetSepHeight(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("sepimm")) == 0 ) SetSepImmMode(_tcsicmp(pstrValue, _T("true")) == 0);
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CDynamicLayoutUI::DoEvent(TEventUI& event)
	{
		if(m_iSepWidth !=0 || m_iSepHeight != 0)
		{
			if( event.Type == UIEVENT_SETCURSOR )
			{
				m_eSepAction = eSepWidth;
				RECT rcSeparator = GetThumbRect(false);
				if( IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
					return;
				}

				m_eSepAction = eSepHeight;
				rcSeparator = GetThumbRect(false);
				if( IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
					return;
				}

				m_eSepAction = eSepNull;
			}
		}

		if( m_eSepAction == eSepWidth ) {
			if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
					m_uButtonState |= UISTATE_CAPTURED;
					m_ptLastMouse = event.ptMouse;
					m_rcNewPos = m_rcItem;
					if( !m_bImmMode && m_pManager ) m_pManager->AddPostPaint(this);
					return;
				}
			}
			if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~UISTATE_CAPTURED;
					m_rcItem = m_rcNewPos;
					if( !m_bImmMode && m_pManager ) m_pManager->RemovePostPaint(this);
					NeedParentUpdate();
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					LONG cx = event.ptMouse.x - m_ptLastMouse.x;
					m_ptLastMouse = event.ptMouse;
					RECT rc = m_rcNewPos;
					if( m_iSepWidth >= 0 ) {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.right - m_iSepWidth ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.right ) return;
						rc.right += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.right = rc.left + GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.right = rc.left + GetMaxWidth();
						}
					}
					else {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.left ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.left - m_iSepWidth ) return;
						rc.left += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.left = rc.right - GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.left = rc.right - GetMaxWidth();
						}
					}

					CDuiRect rcInvalidate = GetThumbRect(true);
					m_rcNewPos = rc;
					m_cxyFixed.cx = m_rcNewPos.right - m_rcNewPos.left;

					if( m_bImmMode ) {
						m_rcItem = m_rcNewPos;
						NeedParentUpdate();
					}
					else {
						rcInvalidate.Join(GetThumbRect(true));
						rcInvalidate.Join(GetThumbRect(false));
						if( m_pManager ) m_pManager->Invalidate(rcInvalidate);
					}
					return;
				}
			}		
		}

		if( m_eSepAction == eSepHeight ) {
			if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
					m_uButtonState |= UISTATE_CAPTURED;
					m_ptLastMouse = event.ptMouse;
					m_rcNewPos = m_rcItem;
					if( !m_bImmMode && m_pManager ) m_pManager->AddPostPaint(this);
					return;
				}
			}
			if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~UISTATE_CAPTURED;
					m_rcItem = m_rcNewPos;
					if( !m_bImmMode && m_pManager ) m_pManager->RemovePostPaint(this);
					NeedParentUpdate();
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					LONG cy = event.ptMouse.y - m_ptLastMouse.y;
					m_ptLastMouse = event.ptMouse;
					RECT rc = m_rcNewPos;
					if( m_iSepHeight >= 0 ) {
						if( cy > 0 && event.ptMouse.y < m_rcNewPos.bottom + m_iSepHeight ) return;
						if( cy < 0 && event.ptMouse.y > m_rcNewPos.bottom ) return;
						rc.bottom += cy;
						if( rc.bottom - rc.top <= GetMinHeight() ) {
							if( m_rcNewPos.bottom - m_rcNewPos.top <= GetMinHeight() ) return;
							rc.bottom = rc.top + GetMinHeight();
						}
						if( rc.bottom - rc.top >= GetMaxHeight() ) {
							if( m_rcNewPos.bottom - m_rcNewPos.top >= GetMaxHeight() ) return;
							rc.bottom = rc.top + GetMaxHeight();
						}
					}
					else {
						if( cy > 0 && event.ptMouse.y < m_rcNewPos.top ) return;
						if( cy < 0 && event.ptMouse.y > m_rcNewPos.top + m_iSepHeight ) return;
						rc.top += cy;
						if( rc.bottom - rc.top <= GetMinHeight() ) {
							if( m_rcNewPos.bottom - m_rcNewPos.top <= GetMinHeight() ) return;
							rc.top = rc.bottom - GetMinHeight();
						}
						if( rc.bottom - rc.top >= GetMaxHeight() ) {
							if( m_rcNewPos.bottom - m_rcNewPos.top >= GetMaxHeight() ) return;
							rc.top = rc.bottom - GetMaxHeight();
						}
					}

					CDuiRect rcInvalidate = GetThumbRect(true);
					m_rcNewPos = rc;
					m_cxyFixed.cy = GetManager()->GetDPIObj()->Scale(m_rcNewPos.bottom - m_rcNewPos.top);

					if( m_bImmMode ) {
						m_rcItem = m_rcNewPos;
						NeedParentUpdate();
					}
					else {
						rcInvalidate.Join(GetThumbRect(true));
						rcInvalidate.Join(GetThumbRect(false));
						if( m_pManager ) m_pManager->Invalidate(rcInvalidate);
					}
					return;
				}
			}
			if( event.Type == UIEVENT_SETCURSOR )
			{
				m_eSepAction = eSepHeight;
				RECT rcSeparator = GetThumbRect(false);
				if( IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
					return;
				}
				m_eSepAction = eSepNull;
			}
		}

		CContainerUI::DoEvent(event);
	}

	RECT CDynamicLayoutUI::GetThumbRect(bool bUseNew) const
	{
		if(m_eSepAction == eSepWidth)
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew) 
			{
				if( m_iSepWidth >= 0 ) return CDuiRect(m_rcNewPos.right - m_iSepWidth, m_rcNewPos.top, m_rcNewPos.right, m_rcNewPos.bottom);
				else return CDuiRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.left - m_iSepWidth, m_rcNewPos.bottom);
			}
			else 
			{
				if( m_iSepWidth >= 0 ) return CDuiRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
				else return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
			}
		}
		else if(m_eSepAction == eSepHeight)
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew) 
			{
				if( m_iSepHeight >= 0 ) 
					return CDuiRect(m_rcNewPos.left, MAX(m_rcNewPos.bottom - m_iSepHeight, m_rcNewPos.top), 
					m_rcNewPos.right, m_rcNewPos.bottom);
				else 
					return CDuiRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.right, 
					MIN(m_rcNewPos.top - m_iSepHeight, m_rcNewPos.bottom));
			}
			else 
			{
				if( m_iSepHeight >= 0 ) 
					return CDuiRect(m_rcItem.left, MAX(m_rcItem.bottom - m_iSepHeight, m_rcItem.top), m_rcItem.right, 
					m_rcItem.bottom);
				else 
					return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.right, 
					MIN(m_rcItem.top - m_iSepHeight, m_rcItem.bottom));

			}
		}
		return CDuiRect(0,0,0,0);
	}

	bool CDynamicLayoutUI::CalcPos(CControlUI *pChildControl, RECT &rcChild)
	{
		m_pCalcControl = pChildControl;
		m_bCalcResult = false;

		SetPos(GetPos(), false);

		if(m_bCalcResult)
		{
			rcChild = m_rcCalcChild;
			m_pCalcControl = NULL;
			return true;
		}
		m_pCalcControl = NULL;
		return false;
	}
}
