#pragma once
#include "assert.h"
#include "FW_Math.h"

enum eCircularArray_Errors
{
	CIRCULARARRAY_INVALID_SIZE,
	CIRCULARARRAY_LOW_INDEX,
	CIRCULARARRAY_HIGH_INDEX,
	_CIRCULARARRAY_ERROR_COUNT,
};

#ifdef CE_ARRAY_BOUNDS_CHECK
static const char* locCircularArray_ErrorStrings[_CIRCULARARRAY_ERROR_COUNT] = {
	"Invalid size of circulararray",
	"Index has to be 0 or more.",
	"a index out of bounds!"
};
#endif

template<typename ObjectType, int ElementCount>
class FW_CircularArray
{
public:
	FW_CircularArray() { myData = new ObjectType[ElementCount]; }
	~FW_CircularArray() { delete[] myData; }

	void Add(const ObjectType& anObject)
	{
		myData[myCurrentIndex] = anObject;
		++myCurrentIndex;
		myTotalCount = FW_Min(++myTotalCount, ElementCount);
		if (myCurrentIndex >= ElementCount)
		{
			myCurrentIndex = 0;
		}
	}

	void RemoveAll()
	{
		myCurrentIndex = 0;
		myTotalCount = 0;
	}

	void DeleteAll()
	{
		for (int i = 0; i < myTotalCount; ++i)
		{
			delete myData[i];
			myData[i] = nullptr;
		}

		myCurrentIndex = 0;
		myTotalCount = 0;
	}

	ObjectType& operator[](const int& aIndex)
	{
		int realIndex = (myCurrentIndex + aIndex) % myTotalCount;
		return myData[realIndex];
	}

	const ObjectType& operator[](const int& aIndex) const
	{
		int realIndex = (myCurrentIndex + aIndex) % myTotalCount;
		return myData[realIndex];
	}

	typedef ObjectType* iterator;
	typedef const ObjectType* const_iterator;
	iterator begin() { return &myData[0]; }
	const_iterator begin() const { return &myData[0]; }
	iterator end() { return &myData[myTotalCount]; }
	const_iterator end() const { return &myData[myTotalCount]; }

	ObjectType* myData = nullptr;
	int myCurrentIndex = 0;
	int myTotalCount = 0;
};