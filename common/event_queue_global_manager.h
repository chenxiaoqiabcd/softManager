#pragma once

#include "eventpp/eventqueue.h"

#include <Windows.h>

enum {
	EVENT_UPDATE_STATUS_LABEL,
	EVENT_UPDATE_SOFT_LIST,				// 更新软件列表
	EVENT_REFRESH_UPDATE_WND_SOFT_LIST,	// 更新软件列表
	EVENT_UPDATE_SOFT_DATA,
	EVENT_INSTALL_PACKAGE,				// 执行安装
};

enum class EventType {
	TypeProcess,
	TypeThread
};

// 单例模式
template<typename T>
class SingletonModel {
protected:
	SingletonModel() {}
	SingletonModel(SingletonModel&) = delete;
public:
	static T* GetInstance() {
		static T instance;
		return &instance;
	}
};

class CEventQueueGlobalManager : public SingletonModel<CEventQueueGlobalManager> {
public:
	void AppendCurrentThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM)>& callback);

	void AppendCurrentThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM, LPVOID)>& callback, LPVOID data);

	void AppendNewThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM)>& callback);

	void AppendNewThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM, LPVOID)>& callback, LPVOID data);

	bool RemoveNewThreadListener(LPVOID data);

	void AppendMainThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM)>& callback, HWND hWnd);

	void AppendMainThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM, LPVOID)>& callback, HWND hWnd, LPVOID data);

	void PostEvent(int event, WPARAM wParam = 0, LPARAM lParam = 0);

	void SendEvent(int event, WPARAM wParam = 0, LPARAM lParam = 0);

	LRESULT HandlerMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL* handler);
protected:
	static DWORD __stdcall ThreadPostEventFunc(void* pArguments);
private:
	inline static eventpp::EventQueue<int, DWORD(WPARAM, LPARAM)> current_thread_queue_;
	inline static eventpp::EventQueue<int, DWORD(WPARAM, LPARAM, LPVOID), LPVOID> current_thread_param_queue_;

	inline static std::vector<int> current_thread_event_list_;
	inline static std::map<int, LPVOID> current_thread_event_param_list_;

	inline static eventpp::EventQueue<int, DWORD(WPARAM, LPARAM)> new_thread_queue_;
	inline static eventpp::EventQueue<int, DWORD(WPARAM, LPARAM, LPVOID), LPVOID> new_thread_param_queue_;

	inline static std::vector<int> new_thread_event_list_;
	inline static std::vector<std::tuple<int, LPVOID>> new_thread_event_param_list_;

	inline static eventpp::EventQueue<int, DWORD(WPARAM, LPARAM)> main_thread_queue_;
	inline static eventpp::EventQueue<int, DWORD(WPARAM, LPARAM, LPVOID), LPVOID> main_thread_param_queue_;

	inline static std::map<int, HWND> main_thread_event_list_;
	inline static std::map<int, std::tuple<HWND, LPVOID>> main_thread_event_param_list_;
};

extern CEventQueueGlobalManager* EventQueueInstance;