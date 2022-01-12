#pragma once
#include "assert.h"
#include <cstring>

enum eGrowingArray_Errors
{
	INVALID_SIZE,
	LOW_INDEX,
	HIGH_INDEX,
	_GROWINGARRAY_ERROR_COUNT,
};

#ifdef CE_ARRAY_BOUNDS_CHECK
static const char* locGrowingArray_ErrorStrings[_GROWINGARRAY_ERROR_COUNT] = {
	"Invalid size of growingarray",
	"Index has to be 0 or more.",
	"a index out of bounds!"
};
#endif

template<typename ObjectType>
class FW_GrowingArray
{
public:
	FW_GrowingArray();
	FW_GrowingArray(const FW_GrowingArray& aGrowingArray);
	~FW_GrowingArray();

	FW_GrowingArray& operator=(const FW_GrowingArray& aGrowingArray);


	inline void Respace(int aNewSize);
	void Reserve(int aNrOfItems);

	inline ObjectType& operator[](const int& aIndex);
	inline const ObjectType& operator[](const int& aIndex) const;

	inline void Add(const ObjectType& aObject);
	inline ObjectType& Add();
	inline bool AddUnique(const ObjectType& aObject);
	inline void DeleteCyclic(ObjectType& aObject);
	inline void DeleteCyclicAtIndex(int aItemNumber);
	inline void DeleteNonCyclicAtIndex(int aItemNumber);
	inline void RemoveCyclic(const ObjectType& aObject);
	inline void RemoveCyclicAtIndex(int aItemNumber);
	inline void RemoveNonCyclic(const ObjectType& aObject);
	inline void RemoveNonCyclicAtIndex(int aItemNumber);

	inline int Find(const ObjectType& aObject) const;

	template<typename T>
	inline int Find(const T& aObject) const;

	inline ObjectType& GetLast();
	inline const ObjectType& GetLast() const;

	static const int FoundNone = -1;

	inline void RemoveAll();
	inline void DeleteAll();

	void Optimize();
	int Count() const;
	int GetCapacity() const;
	bool IsEmpty() const;

	inline ObjectType* GetArrayAsPointer();
	inline const ObjectType* GetArrayAsPointer() const;

	typedef ObjectType* iterator;
	typedef const ObjectType* const_iterator;
	iterator begin() { return &myData[0]; }
	const_iterator begin() const { return &myData[0]; }
	iterator end() { return &myData[myCurrentSize]; }
	const_iterator end() const { return &myData[myCurrentSize]; }

	bool operator==(const FW_GrowingArray& aOther);

private:
	ObjectType* myData;
	int myCurrentSize;
	int myMaxSize;
	bool myUseSafeModeFlag;
};

template<typename ObjectType>
inline FW_GrowingArray<ObjectType>::FW_GrowingArray()
	: myData(nullptr)
	, myCurrentSize(0)
	, myMaxSize(0)
	, myUseSafeModeFlag(true)
{
	//Respace(1);
}

template<typename ObjectType>
inline FW_GrowingArray<ObjectType>::FW_GrowingArray(const FW_GrowingArray& aGrowingArray)
{
	myData = nullptr;
	operator=(aGrowingArray);
}

template<typename ObjectType>
inline FW_GrowingArray<ObjectType>::~FW_GrowingArray()
{
	delete[] myData;
}

template<typename ObjectType>
inline FW_GrowingArray<ObjectType>& FW_GrowingArray<ObjectType>::operator=(const FW_GrowingArray& aGrowingArray)
{
	// if aGrowingArray.myMaxSize <= myMaxSize, I could just copy everything, don't need newData
	delete[] myData;
	myMaxSize = aGrowingArray.myMaxSize;
	myCurrentSize = aGrowingArray.myCurrentSize;
	myUseSafeModeFlag = aGrowingArray.myUseSafeModeFlag;

	ObjectType* newData = new ObjectType[aGrowingArray.myMaxSize];


	if (myUseSafeModeFlag == true)
	{
		for (int i = 0; i < myCurrentSize; ++i)
		{
			newData[i] = aGrowingArray.myData[i];
		}
	}
	else
	{
		memcpy(newData, aGrowingArray.myData, sizeof(ObjectType) * aGrowingArray.myCurrentSize);
	}

	myData = newData;

	return *this;
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::Respace(int aNewSize)
{
#ifdef CE_ARRAY_BOUNDS_CHECK
	CE_ASSERT(aNewSize > 0, locGrowingArray_ErrorStrings[INVALID_SIZE]);
#endif

	myMaxSize += aNewSize;
	ObjectType* newData = new ObjectType[myMaxSize];
	if (myUseSafeModeFlag == true)
	{
		for (int i = 0; i < myCurrentSize; ++i)
		{
			newData[i] = myData[i];
		}
	}
	else
	{
		memcpy(newData, myData, sizeof(ObjectType) * myCurrentSize);
	}
	delete[] myData;
	myData = newData;
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::Reserve(int aNrOfItems)
{
	Respace(aNrOfItems);
	myCurrentSize = myMaxSize;
}

template<typename ObjectType>
inline ObjectType& FW_GrowingArray<ObjectType>::operator[](const int& aIndex)
{
#ifdef CE_ARRAY_BOUNDS_CHECK
	CE_ASSERT(aIndex >= 0, locGrowingArray_ErrorStrings[LOW_INDEX]);
	CE_ASSERT(aIndex < myCurrentSize, locGrowingArray_ErrorStrings[HIGH_INDEX]);
#endif

	return myData[aIndex];
}

template<typename ObjectType>
inline const ObjectType& FW_GrowingArray<ObjectType>::operator[](const int& aIndex) const
{
#ifdef CE_ARRAY_BOUNDS_CHECK
	CE_ASSERT(aIndex >= 0, locGrowingArray_ErrorStrings[LOW_INDEX]);
	CE_ASSERT(aIndex < myCurrentSize, locGrowingArray_ErrorStrings[HIGH_INDEX]);
#endif

	return myData[aIndex];
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::Add(const ObjectType& aObject)
{
	if (myMaxSize == 0)
		Respace(8);

	if (myCurrentSize >= myMaxSize)
		Respace(myMaxSize * 2);

	myData[myCurrentSize++] = aObject;
}


template<typename ObjectType>
inline ObjectType& FW_GrowingArray<ObjectType>::Add()
{
	if (myMaxSize == 0)
		Respace(8);

	if (myCurrentSize >= myMaxSize)
		Respace(myMaxSize * 2);

	ObjectType& object = myData[myCurrentSize++];
	return object;
}

template<typename ObjectType>
inline bool FW_GrowingArray<ObjectType>::AddUnique(const ObjectType& aObject)
{
	if (Find(aObject) == -1)
	{
		Add(aObject);
		return true;
	}

	return false;
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::DeleteCyclic(ObjectType& aObject)
{
	for (int i = 0; i < myCurrentSize; ++i)
	{
		if (myData[i] == aObject)
		{
			DeleteCyclicAtIndex(i);
			return;
		}
	}
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::DeleteCyclicAtIndex(int aItemNumber)
{
#ifdef CE_ARRAY_BOUNDS_CHECK
	CE_ASSERT(aItemNumber >= 0, locGrowingArray_ErrorStrings[LOW_INDEX]);
	CE_ASSERT(aItemNumber < myCurrentSize, locGrowingArray_ErrorStrings[HIGH_INDEX]);
#endif

	delete myData[aItemNumber];
	myData[aItemNumber] = nullptr;
	myData[aItemNumber] = myData[--myCurrentSize];
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::DeleteNonCyclicAtIndex(int aItemNumber)
{
#ifdef CE_ARRAY_BOUNDS_CHECK
	CE_ASSERT(aItemNumber >= 0, locGrowingArray_ErrorStrings[LOW_INDEX]);
	CE_ASSERT(aItemNumber < myCurrentSize, locGrowingArray_ErrorStrings[HIGH_INDEX]);
#endif

	delete myData[aItemNumber];
	myData[aItemNumber] = nullptr;

	for (int i = aItemNumber; i < myCurrentSize - 1; ++i)
	{
		myData[i] = myData[i + 1];
	}
	--myCurrentSize;
}


template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::RemoveCyclic(const ObjectType& aObject)
{
	for (int i = 0; i < myCurrentSize; ++i)
	{
		if (myData[i] == aObject)
		{
			RemoveCyclicAtIndex(i);
			return;
		}
	}
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::RemoveCyclicAtIndex(int aItemNumber)
{
#ifdef CE_ARRAY_BOUNDS_CHECK
	CE_ASSERT(aItemNumber >= 0, locGrowingArray_ErrorStrings[LOW_INDEX]);
	CE_ASSERT(aItemNumber < myCurrentSize, locGrowingArray_ErrorStrings[HIGH_INDEX]);
#endif

	myData[aItemNumber] = myData[--myCurrentSize];
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::RemoveNonCyclic(const ObjectType& aObject)
{
	for (int i = 0; i < myCurrentSize; ++i)
	{
		if (myData[i] == aObject)
		{
			RemoveNonCyclicAtIndex(i);
			return;
		}
	}
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::RemoveNonCyclicAtIndex(int aItemNumber)
{
#ifdef CE_ARRAY_BOUNDS_CHECK
	CE_ASSERT(aItemNumber >= 0, locGrowingArray_ErrorStrings[LOW_INDEX]);
	CE_ASSERT(aItemNumber < myCurrentSize, locGrowingArray_ErrorStrings[HIGH_INDEX]);
#endif

	for (int i = aItemNumber; i < myCurrentSize - 1; ++i)
	{
		myData[i] = myData[i + 1];
	}
	--myCurrentSize;
}

template<typename ObjectType>
inline int FW_GrowingArray<ObjectType>::Find(const ObjectType& aObject) const
{
	for (int i = 0; i < myCurrentSize; ++i)
	{
		if (myData[i] == aObject)
		{
			return i;
		}
	}
	return FoundNone;
}

template<typename ObjectType>
template<typename T>
inline int FW_GrowingArray<ObjectType>::Find(const T& aObject) const
{
	for (int i = 0; i < myCurrentSize; ++i)
	{
		if (myData[i] == aObject)
		{
			return i;
		}
	}
	return FoundNone;
}

template<typename ObjectType>
inline ObjectType& FW_GrowingArray<ObjectType>::GetLast()
{
	return myData[myCurrentSize - 1];
}

template<typename ObjectType>
inline const ObjectType& FW_GrowingArray<ObjectType>::GetLast() const
{
	return myData[myCurrentSize - 1];
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::RemoveAll()
{
	myCurrentSize = 0;
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::DeleteAll()
{
	for (int i = 0; i < myCurrentSize; ++i)
	{
		delete myData[i];
		myData[i] = nullptr;
	}
	myCurrentSize = 0;
}

template<typename ObjectType>
inline void FW_GrowingArray<ObjectType>::Optimize()
{
	myMaxSize = myCurrentSize;
	if (myMaxSize < 1)
	{
		myMaxSize = 1;
	}
	Respace(myMaxSize);
}

template<typename ObjectType>
int FW_GrowingArray<ObjectType>::Count() const
{
	return myCurrentSize;
}

template<typename ObjectType>
int FW_GrowingArray<ObjectType>::GetCapacity() const
{
	return myMaxSize;
}

template<typename ObjectType>
bool FW_GrowingArray<ObjectType>::IsEmpty() const
{
	return myCurrentSize == 0;
}

template<typename ObjectType>
inline ObjectType* FW_GrowingArray<ObjectType>::GetArrayAsPointer()
{
	return myData;
}

template<typename ObjectType>
inline const ObjectType* FW_GrowingArray<ObjectType>::GetArrayAsPointer() const
{
	return myData;
}

template<typename ObjectType>
inline bool FW_GrowingArray<ObjectType>::operator==(const FW_GrowingArray& aOther)
{
	if (myCurrentSize != aOther.myCurrentSize) return false;

	for (int i = 0; i < myCurrentSize; ++i)
	{
		if (myData[i] != aOther.myData[i])
			return false;
	}

	return true;
}