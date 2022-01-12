#pragma once
#include <shared_mutex>

struct FW_Mutex
{
	void ReadLock()
	{
		myMutex.lock_shared();
	}
	void ReadUnlock()
	{
		myMutex.unlock_shared();
	}

	void ReadWriteLock()
	{
		myMutex.lock();
	}

	void ReadWriteUnlock()
	{
		myMutex.unlock();
	}

	std::shared_mutex myMutex;
};

struct FW_ReadLock
{
	FW_ReadLock(FW_Mutex& aMutex)
		: myMutex(aMutex)
	{
		myMutex.ReadLock();
	}

	~FW_ReadLock()
	{
		myMutex.ReadUnlock();
	}

	FW_Mutex& myMutex;
};

struct FW_ReadWriteLock
{
	FW_ReadWriteLock(FW_Mutex& aMutex)
		: myMutex(aMutex)
	{
		myMutex.ReadWriteLock();
	}

	~FW_ReadWriteLock()
	{
		myMutex.ReadWriteUnlock();
	}

	FW_Mutex& myMutex;
};
