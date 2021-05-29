#include <shared_mutex>

struct Mutex
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

struct ReadLock
{
	ReadLock(Mutex& aMutex)
		: myMutex(aMutex)
	{
		myMutex.ReadLock();
	}

	~ReadLock()
	{
		myMutex.ReadUnlock();
	}

	Mutex& myMutex;
};

struct ReadWriteLock
{
	ReadWriteLock(Mutex& aMutex)
		: myMutex(aMutex)
	{
		myMutex.ReadWriteLock();
	}

	~ReadWriteLock()
	{
		myMutex.ReadWriteUnlock();
	}

	Mutex& myMutex;
};