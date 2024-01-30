#pragma once

#include <vector>

/*
* 观察者模式
*/
class IDisplay {
public:
	virtual void UpdateDate(bool need_update, void* data) = 0;
	virtual void ClearData() = 0;
};

class DataCenter {
public:
	void Attach(IDisplay* d) {
		data_list_.push_back(d);
	}

	void Detach(IDisplay* d) {
		const auto it_find = std::find(data_list_.begin(), data_list_.end(), d);
		if (it_find != data_list_.end()) {
			data_list_.erase(it_find);
		}
	}

	void UpdateDate(bool need_update, void* data) const {
		for (const auto& it : data_list_) {
			it->UpdateDate(need_update, data);
		}
	}

	void ClearData() const {
		for (const auto& it : data_list_) {
			it->ClearData();
		}
	}
private:
	std::vector<IDisplay*> data_list_;
};