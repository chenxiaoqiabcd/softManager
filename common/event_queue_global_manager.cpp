#include "event_queue_global_manager.h"

#include <algorithm>

#include <UIlib.h>

#include "define.h"
#include "kf_log.h"

struct EventInfo {
	int id;
	WPARAM wParam;
	LPARAM lParam;
};

void CEventQueueGlobalManager::AppendCurrentThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM)>& callback) {
	current_thread_queue_.appendListener(event, callback);
	current_thread_event_list_.emplace_back(event);
}

void CEventQueueGlobalManager::AppendCurrentThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM, LPVOID)>& callback, LPVOID data) {
	current_thread_param_queue_.appendListener(event, callback);
	current_thread_event_param_list_[event] = data;
}

void CEventQueueGlobalManager::AppendNewThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM)>& callback) {
	new_thread_queue_.appendListener(event, callback);
	new_thread_event_list_.emplace_back(event);
}

void CEventQueueGlobalManager::AppendNewThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM, LPVOID)>& callback, LPVOID data) {
	new_thread_param_queue_.appendListener(event, callback);
	new_thread_event_param_list_[event] = data;
}

void CEventQueueGlobalManager::AppendMainThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM)>& callback, HWND hWnd) {
	main_thread_queue_.appendListener(event, callback);
	main_thread_event_list_[event] = hWnd;
}

void CEventQueueGlobalManager::AppendMainThreadListener(int event, const std::function<DWORD(WPARAM, LPARAM, LPVOID)>& callback, HWND hWnd, LPVOID data) {
	main_thread_param_queue_.appendListener(event, callback);
	main_thread_event_param_list_[event] = std::make_tuple(hWnd, data);
}

void CEventQueueGlobalManager::PostEvent(int event, WPARAM wParam, LPARAM lParam) {
	const auto found = std::any_of(new_thread_event_list_.begin(), new_thread_event_list_.end(),
								[=](int value) { return event == value; });
	if(found) {
		auto info = new EventInfo;

		info->id = event;
		info->wParam = wParam;
		info->lParam = lParam;

		HANDLE thread_handle = CreateThread(nullptr, 0, ThreadPostEventFunc, info, 0, nullptr);
		CloseHandle(thread_handle);
		return;
	} 

	auto it_find = new_thread_event_param_list_.find(event);

	if(it_find != new_thread_event_param_list_.end()) {
		auto info = new EventInfo;

		info->id = event;
		info->wParam = wParam;
		info->lParam = lParam;

		HANDLE thread_handle = CreateThread(nullptr, 0, ThreadPostEventFunc, info, 0, nullptr);
		CloseHandle(thread_handle);
		return;
	}

	KF_WARN("请使用AppendNewThreadListener注册此消息, id: {}", event);
}

void CEventQueueGlobalManager::SendEvent(int event, WPARAM wParam, LPARAM lParam) {
	const auto it_current_thread_event_find = std::find_if(current_thread_event_list_.begin(),
														   current_thread_event_list_.end(),
														   [=](int value) { return event == value; });
	if (it_current_thread_event_find != current_thread_event_list_.end()) {
		current_thread_queue_.dispatch(event, wParam, lParam);
		return;
	}

	const auto it_current_thread_event_param_find = current_thread_event_param_list_.find(event);
	if(it_current_thread_event_param_find != current_thread_event_param_list_.end()) {
		current_thread_param_queue_.dispatch(event, wParam, lParam, it_current_thread_event_param_find->second);
		return;
	}

	const auto it_main_thread_event_find = main_thread_event_list_.find(event);
	if (it_main_thread_event_find != main_thread_event_list_.end()) {
		auto info = new EventInfo;

		info->id = event;
		info->wParam = wParam;
		info->lParam = lParam;

		::PostMessage(it_main_thread_event_find->second, WM_USER_EVENT_MSG, reinterpret_cast<WPARAM>(info), 0);
		return;
	}

	const auto it_main_thread_event_param_find = main_thread_event_param_list_.find(event);
	if (it_main_thread_event_param_find != main_thread_event_param_list_.end()) {
		auto info = new EventInfo;

		info->id = event;
		info->wParam = wParam;
		info->lParam = lParam;

		::PostMessage(std::get<0>(it_main_thread_event_param_find->second), WM_USER_EVENT_MSG,
					  reinterpret_cast<WPARAM>(info), 0);
		return;
	}

	KF_WARN("请使用AppendCurrentThreadListener或者AppendMainThreadListener注册此消息, id: {}", event);
}

LRESULT CEventQueueGlobalManager::HandlerMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL* handler) {
	if (WM_USER_EVENT_MSG == msg) {
		auto event_info = reinterpret_cast<EventInfo*>(wParam);

		const auto it_main_thread_event_find = main_thread_event_list_.find(event_info->id);
		if (it_main_thread_event_find != main_thread_event_list_.end()) {
			main_thread_queue_.dispatch(event_info->id, event_info->wParam, event_info->lParam);

			delete event_info;
			event_info = nullptr;

			return 0;
		}

		const auto it_main_thread_event_param_find = main_thread_event_param_list_.find(event_info->id);
		if (it_main_thread_event_param_find != main_thread_event_param_list_.end()) {
			main_thread_param_queue_.dispatch(event_info->id, event_info->wParam, event_info->lParam,
											  std::get<1>(it_main_thread_event_param_find->second));

			delete event_info;
			event_info = nullptr;

			return 0;
		}

		delete event_info;
		event_info = nullptr;

		*handler = true;
	}

	*handler = FALSE;
	return 0;
}

DWORD CEventQueueGlobalManager::ThreadPostEventFunc(void* pArguments) {
	auto event_info = (EventInfo*)pArguments;

	const auto found = std::any_of(new_thread_event_list_.begin(), new_thread_event_list_.end(),
							 [=](int value) {return event_info->id == value; });
	if(found) {
		new_thread_queue_.dispatch(event_info->id, event_info->wParam, event_info->lParam);
		return 0;
	}

	const auto it_find = new_thread_event_param_list_.find(event_info->id);
	if(it_find != new_thread_event_param_list_.end()) {
		new_thread_param_queue_.dispatch(event_info->id, event_info->wParam, event_info->lParam,
										 it_find->second);
	}

	return 0;
}

CEventQueueGlobalManager* EventQueueInstance = CEventQueueGlobalManager::GetInstance();