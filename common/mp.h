#pragma once
#include <ostream>
#include <vector>

class MemoryPool
{
private:
	MemoryPool(){}
public:
	MemoryPool(const MemoryPool&) = delete;
	MemoryPool(MemoryPool&&) = delete;
	MemoryPool& operator=(MemoryPool&) = delete;

	~MemoryPool();

	static MemoryPool* GetInstance();

	void Init(int block_count);

	void* Allocate(uint32_t size);

	void DeAllocate(void* ptr);

	int GetAllocatedCount() const;

	std::string ToString() const;
private:
	struct MemoryBlock
	{
		bool is_allocated;
		char* data;
	};

	std::vector<MemoryBlock> pool_;
};