#pragma once

#include "WndImpl.h"
#include "observerMode.h"

struct UpdateInfo;
class CUpdateListElementUI;
class KfString;
struct SoftInfo;

class CUpdateWnd : public CWndImpl, public IDisplay
{
public:
	~CUpdateWnd();
protected:
	LPCTSTR GetSkinFile() override;
	void Init() override;
	void Notify(DuiLib::TNotifyUI& msg) override;

	CUpdateListElementUI* CreateLine(const SoftInfo& soft_it, const UpdateInfo* info);

	void UpdateDate(bool need_update, void* data) override;

	void ClearData() override;

	void UpdateDate();
private:
	std::vector<std::tuple<KfString, uint8_t>> update_info_vec_;
};