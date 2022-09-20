#pragma once

#include "FW_Hashing.h"
#include "FW_GrowingArray.h"

template<typename Key, typename Value>
struct FW_MapIterator
{
	FW_MapIterator(int aFirst, int aSecond, Key* aKey, Value* aValue)
		: myFirstIndex(aFirst)
		, mySecondIndex(aSecond)
		, myKey(aKey)
		, myValue(aValue)
	{}

	bool operator==(const FW_MapIterator &aIterator) const;
	bool operator!=(const FW_MapIterator &aIterator) const;

	Key& First() { return *myKey; }
	Value& Second() { return *myValue; }

	int myFirstIndex;
	int mySecondIndex;

private:
	Key* myKey;
	Value* myValue;
};

template<typename Key, typename Value>
bool FW_MapIterator<Key, Value>::operator==(const FW_MapIterator &aIterator) const
{
	return myFirstIndex == aIterator.myFirstIndex && mySecondIndex == aIterator.mySecondIndex;
}

template<typename Key, typename Value>
bool FW_MapIterator<Key, Value>::operator!=(const FW_MapIterator &aIterator) const
{
	return (*this == aIterator) == false;
}

template<typename Key, typename Value, int StartSize = 67, int BucketSize = 3>
class FW_Hashmap
{
public:
	FW_Hashmap();

	void Insert(const Key &aKey, const Value &aValue);
	Value& Get(const Key &aKey);
	const Value& Get(const Key &aKey) const;
	const Value* GetIfExists(const Key &aKey) const;
	void Remove(const Key &aKey);
	bool KeyExists(const Key &aKey) const;
	Value& operator[](const Key &aKey);

	FW_MapIterator<Key, Value> Begin();
	FW_MapIterator<Key, Value> Next(const FW_MapIterator<Key, Value>& aCurrent);
	FW_MapIterator<Key, Value> End();

	void Clear();

private:
	struct KeyValuePair
	{
		Key myKey;
		Value myValue;
	};

	int FindKeyInBucket(int aBucketIndex, const Key& aKey) const;

	int OwnHash(const Key &aKey) const;
	FW_GrowingArray<FW_GrowingArray<KeyValuePair>> myBuckets;
};


template<typename Key, typename Value, int StartSize, int BucketSize>
FW_Hashmap<Key, Value, StartSize, BucketSize>::FW_Hashmap()
{
	myBuckets.Reserve(StartSize);

	for (int i = 0; i < myBuckets.Count(); ++i)
	{
		myBuckets[i].Respace(BucketSize);
	}
}


template<typename Key, typename Value, int StartSize, int BucketSize>
void FW_Hashmap<Key, Value, StartSize, BucketSize>::Insert(const Key &aKey, const Value &aValue)
{
	if (KeyExists(aKey) == true)
	{
		operator[](aKey) = aValue;
		return;
	}

	int index = OwnHash(aKey);

	KeyValuePair pair;
	pair.myKey = aKey;
	pair.myValue = aValue;

	myBuckets[index].Add(pair);
}


template<typename Key, typename Value, int StartSize, int BucketSize>
Value& FW_Hashmap<Key, Value, StartSize, BucketSize>::Get(const Key &aKey)
{
	int index = OwnHash(aKey);
	int keyIndex = FindKeyInBucket(index, aKey);
	FW_ASSERT(keyIndex != -1, "[FW_Hashmap]: Tried to get an nonexisting Key.");

	return myBuckets[index][keyIndex].myValue;
}

template<typename Key, typename Value, int StartSize, int BucketSize>
const Value& FW_Hashmap<Key, Value, StartSize, BucketSize>::Get(const Key &aKey) const
{
	int index = OwnHash(aKey);
	int keyIndex = FindKeyInBucket(index, aKey);
	FW_ASSERT(keyIndex != -1, "[FW_Hashmap]: Tried to get an nonexisting Key.");

	return myBuckets[index][keyIndex].myValue;
}

template<typename Key, typename Value, int StartSize, int BucketSize>
const Value* FW_Hashmap<Key, Value, StartSize, BucketSize>::GetIfExists(const Key &aKey) const
{
	if (!KeyExists(aKey))
		return nullptr;

	return &Get(aKey);
}

template<typename Key, typename Value, int StartSize, int BucketSize>
void FW_Hashmap<Key, Value, StartSize, BucketSize>::Remove(const Key &aKey)
{
	FW_ASSERT(KeyExists(aKey) == true, "[FW_Hashmap]: Tried to delete an nonexisting Key.");

	int index = OwnHash(aKey);

	for (int i = 0; i < myBuckets[index].Count(); ++i)
	{
		if (myBuckets[index][i].myKey == aKey)
		{
			myBuckets[index].RemoveCyclicAtIndex(i);
			return;
		}
	}
}

template<typename Key, typename Value, int StartSize, int BucketSize>
bool FW_Hashmap<Key, Value, StartSize, BucketSize>::KeyExists(const Key &aKey) const
{
	int index = OwnHash(aKey);
	return FindKeyInBucket(index, aKey) != -1;
}

template<typename Key, typename Value, int StartSize, int BucketSize>
Value& FW_Hashmap<Key, Value, StartSize, BucketSize>::operator[](const Key &aKey)
{
	int index = OwnHash(aKey);
	if (FindKeyInBucket(index, aKey) == -1)
	{
		KeyValuePair pair;
		pair.myKey = aKey;
		pair.myValue = Value();

		FW_GrowingArray<KeyValuePair>& bucket = myBuckets[index];
		bucket.Add(pair);
		return bucket.GetLast().myValue;
	}

	return Get(aKey);
}

template<typename Key, typename Value, int StartSize, int BucketSize>
FW_MapIterator<Key, Value> FW_Hashmap<Key, Value, StartSize, BucketSize>::Begin()
{
	int first = -1;
	int second = -1;
	Key* key = nullptr;
	Value* value = nullptr;

	for (int i = 0; i < myBuckets.Count(); ++i)
	{
		if (myBuckets[i].Count() > 0)
		{
			first = i;
			second = 0;

			key = &myBuckets[first][second].myKey;
			value = &myBuckets[first][second].myValue;

			return FW_MapIterator<Key, Value>(first, second, key, value);
		}
	}

	return End();
}

#pragma warning(push)
#pragma warning(disable: 4702)
template<typename Key, typename Value, int StartSize, int BucketSize>
FW_MapIterator<Key, Value> FW_Hashmap<Key, Value, StartSize, BucketSize>::Next(const FW_MapIterator<Key, Value>& aCurrent)
{
	int first = -1;
	int second = -1;
	Key* key = nullptr;
	Value* value = nullptr;

	int innerIndex = aCurrent.mySecondIndex + 1;

	for (int i = aCurrent.myFirstIndex; i < myBuckets.Count(); ++i)
	{
		const FW_GrowingArray<KeyValuePair>& bucket = myBuckets[i];
		for (int j = innerIndex; j < bucket.Count(); ++j)
		{
			first = i;
			second = j;

			key = &myBuckets[first][second].myKey;
			value = &myBuckets[first][second].myValue;

			return FW_MapIterator<Key, Value>(first, second, key, value);
		}

		innerIndex = 0;
	}

	return End();
}
#pragma warning(pop)

template<typename Key, typename Value, int StartSize, int BucketSize>
FW_MapIterator<Key, Value> FW_Hashmap<Key, Value, StartSize, BucketSize>::End()
{
	return FW_MapIterator<Key, Value>(-1, -1, nullptr, nullptr);
}

template<typename Key, typename Value, int StartSize, int BucketSize>
void FW_Hashmap<Key, Value, StartSize, BucketSize>::Clear()
{
	for (int i = 0; i < myBuckets.Count(); ++i)
	{
		myBuckets[i].RemoveAll();
	}
}


template<typename Key, typename Value, int StartSize, int BucketSize>
int FW_Hashmap<Key, Value, StartSize, BucketSize>::FindKeyInBucket(int aBucketIndex, const Key& aKey) const
{
	const FW_GrowingArray<KeyValuePair>& bucket = myBuckets[aBucketIndex];

	for (int i = 0; i < bucket.Count(); ++i)
	{
		if (bucket[i].myKey == aKey)
			return i;
	}

	return -1;
}

template<typename Key, typename Value, int StartSize, int BucketSize>
int FW_Hashmap<Key, Value, StartSize, BucketSize>::OwnHash(const Key &aKey) const
{
	return Murmur(aKey) % StartSize;
}