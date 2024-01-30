#include "mp.h"

#include <cassert>
#include <tuple>

#include "kf_log.h"

MemoryPool::~MemoryPool() {
	for(auto& it : pool_) {
		assert(it.data == nullptr);
	}
}

MemoryPool* MemoryPool::GetInstance() {
	static MemoryPool instance;
	return &instance;
}

void MemoryPool::Init(int block_count) {
	pool_.resize(block_count);

	for (int n = 0; n < block_count; ++n) {
		pool_[n].data = nullptr;
		pool_[n].is_allocated = false;
	}
}

void* MemoryPool::Allocate(uint32_t size) { 
	for(auto& it : pool_) {
		if (!it.is_allocated) {
			it.is_allocated = true;
			it.data = static_cast<char*>(malloc(size));
			// KF_INFO("allocate success, info: %s", ToString().c_str());
			return it.data;
		}
	}

	// KF_WARN("�ڴ���Ѵ����ޣ�ִ��malloc�����ڴ�");
	return malloc(size);
}

void MemoryPool::DeAllocate(void* ptr) {
	for(auto& it : pool_) {
		if(!it.is_allocated) {
			continue;
		}

		if(it.data == ptr) {
			it.is_allocated = false;
			free(it.data);
			it.data = nullptr;

			// KF_INFO("deallocate success, info %s", ToString().c_str());
			return;
		}
	}

	// KF_WARN("�ͷŵ��ڴ治�����ڴ�أ�ʹ��free�ͷ�");
	free(ptr);
}

int MemoryPool::GetAllocatedCount() const {
	int count = 0;

	for(auto& it : pool_) {
		if(it.is_allocated) {
			count++;
		}
	}

	return count;
}

std::string MemoryPool::ToString() const {
	const auto allocated_count = GetAllocatedCount();

	char buffer[515];
	memset(buffer, 0, sizeof(buffer));
	std::ignore = sprintf_s(buffer, "pool count: %zd, used count: %d, free count: %zd",
							pool_.size(), allocated_count, pool_.size() - allocated_count);

	return buffer;
}
