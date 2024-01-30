#include "WndImpl.h"

#include "event_queue_global_manager.h"
#include "scheme.h"

DuiLib::CControlUI* CDialogBuilderCallbackEx::CreateControl(LPCTSTR pstrClass) {
	return nullptr;
}

void CWndImpl::Notify(DuiLib::TNotifyUI& msg) {
}

LPCTSTR CWndImpl::GetWindowClassName() const {
	return L"base dialog";
}

LRESULT CWndImpl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle) {
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS);

	m_pm.Init(m_hWnd);
	DuiLib::CDialogBuilder builder;
	CDialogBuilderCallbackEx cb;

	const DuiLib::STRINGorID& skin_file = GetSkinFile();

	DuiLib::CControlUI* pRoot = builder.Create(skin_file, nullptr, &cb, &m_pm);

	ASSERT(pRoot && "Failed to parse XML");
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);

	Init();

	scheme_ = std::make_shared<Scheme>();

	switch (scheme_->GetMode()) {
	case SchemeMode::Dark:
		scheme_->SetDarkMode(&m_pm);
		break;
	case SchemeMode::Light:
		scheme_->SetLightMode(&m_pm);
		break;
	default:
		scheme_->SetDefaultMode(&m_pm);
		break;
	}

	bHandle = FALSE;
	return 0;
}

LRESULT CWndImpl::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle) {
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	RECT rcCaption = m_pm.GetCaptionRect();
	if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
		DuiLib::CControlUI* pControl = static_cast<DuiLib::CControlUI*>(m_pm.FindControl(pt));
		if (pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 &&
			_tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
			_tcscmp(pControl->GetClass(), _T("TextUI")) != 0 &&
			_tcscmp(pControl->GetClass(), _T("EditUI")))
			return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CWndImpl::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT lResult = 0;
	BOOL bHandle = TRUE;

	lResult = EventQueueInstance->HandlerMessage(uMsg, wParam, lParam, &bHandle);
	if(bHandle) {
		return lResult;
	}

	bHandle = TRUE;

	switch (uMsg) {
	case WM_CREATE: lResult = OnCreate(uMsg, wParam, lParam, bHandle); break;
	case WM_NCHITTEST: lResult = OnNcHitTest(uMsg, wParam, lParam, bHandle); break;
	default: bHandle = FALSE; break;
	}

	if (bHandle) return lResult;
	if (m_pm.MessageHandler(uMsg, wParam, lParam, lResult)) return lResult;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}
