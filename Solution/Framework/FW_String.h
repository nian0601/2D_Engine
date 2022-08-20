#pragma once

#include <cassert>
#include <stdlib.h>
#include <stdio.h>
#include "FW_Murmur.h"
#include <string.h>
#include "FW_Assert.h"

#define FW_STRING_START_SIZE 127
#define FW_STRING_MAX_NUMBER_CHARACTERS 128

#define FW_STRING_ASSERTS 1

class FW_String
{
	friend class FileStream;
public:
	FW_String();
	~FW_String();
	FW_String(const FW_String &aString);
	FW_String(const char* aString);
	FW_String(const char aString);

	FW_String& operator+=(const FW_String &aString);
	FW_String& operator+=(const float aFloat);
	FW_String& operator+=(const double aDouble);
	FW_String& operator+=(const int aInt);
	FW_String& operator+=(const char* aString);
	FW_String& operator+=(const char aString);

	bool operator==(const FW_String &aString) const;
	bool operator==(const char* aString) const;
	bool operator!=(const FW_String &aString) const;
	bool operator!=(const char* aString) const;
	void operator=(const FW_String &aString);
	void operator=(const char* aString);

	int Find(const char* aPattern, const int aStartIndex = 0) const;
	int Find(const FW_String &aPattern, const int aStartIndex = 0) const;
	int RFind(const char* aPattern) const;
	int RFind(const FW_String &aPattern) const;
	static const int NotFound = -1;
	static const unsigned char NullTermination = 0;

	FW_String SubStr(const int aStart, const int aEnd) const;
	const char* GetBuffer() const;
	char* GetRawBuffer();

	int Length() const;
	bool Empty() const;

	const char& operator[](const int aIndex) const;
	char& operator[](const int aIndex);

	void Resize(int aNewSize);
	unsigned int GetHash() const;

	void Clear();
	void RemoveOne();

private:
	void UpdateHash();

	int myCurrentSize;
	int myMaxSize;
	unsigned int myHash;
	char* myData;
};

inline FW_String::FW_String()
	: myCurrentSize(0)
	, myMaxSize(FW_STRING_START_SIZE)
	, myData(new char[FW_STRING_START_SIZE])
{
	myHash = 0;
}

inline FW_String::~FW_String()
{
	delete[] myData;
}

inline FW_String::FW_String(const FW_String &aString)
	: myCurrentSize(0)
	, myMaxSize(FW_STRING_START_SIZE)
	, myData(new char[FW_STRING_START_SIZE])
{
	*this = aString;
}

inline FW_String::FW_String(const char* aString)
	: myCurrentSize(0)
	, myMaxSize(FW_STRING_START_SIZE)
	, myData(new char[FW_STRING_START_SIZE])
{
	*this = aString;
}

inline FW_String::FW_String(const char aChar)
	: myCurrentSize(0)
	, myMaxSize(FW_STRING_START_SIZE)
	, myData(new char[FW_STRING_START_SIZE])
{
	myHash = 0;
	myData[myCurrentSize] = aChar;
	++myCurrentSize;
	UpdateHash();
}

inline FW_String& FW_String::operator+=(const FW_String &aString)
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(myCurrentSize < myMaxSize, "[String] Tried to add a String to an already full string.")
	FW_ASSERT(myCurrentSize + aString.Length() < myMaxSize, "[String]: Tried to add to an string that would result in an overflow.");
#endif

	if (myCurrentSize + aString.Length() + 1 >= myMaxSize)
		Resize(myCurrentSize + aString.Length() + 1);

	for (int i = 0; i < aString.myCurrentSize; ++i)
	{
		myData[myCurrentSize] = aString[i];
		++myCurrentSize;
	}

	if (myData[myCurrentSize] != NullTermination)
	{
		myData[myCurrentSize] = NullTermination;
	}

	UpdateHash();

	return *this;
}

inline FW_String& FW_String::operator+=(const float aFloat)
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(myCurrentSize < myMaxSize, "[String] Tried to add a Float to an already full string.");
#endif

	if (myCurrentSize >= myMaxSize)
		Resize(myCurrentSize * 2);

	char buffer[FW_STRING_MAX_NUMBER_CHARACTERS];
	_snprintf_s(buffer, FW_STRING_MAX_NUMBER_CHARACTERS, "%.3f", aFloat);

	FW_String newString(buffer);
	*this += newString;

	return *this;
}

inline FW_String& FW_String::operator+=(const double aDouble)
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(myCurrentSize < myMaxSize, "[String] Tried to add a Double to an already full string.");
#endif

	if (myCurrentSize >= myMaxSize)
		Resize(myCurrentSize * 2);

	char buffer[FW_STRING_MAX_NUMBER_CHARACTERS];
	_snprintf_s(buffer, FW_STRING_MAX_NUMBER_CHARACTERS, "%f", aDouble);

	FW_String newString(buffer);
	*this += newString;

	return *this;
}

inline FW_String& FW_String::operator+=(const int aInt)
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(myCurrentSize < myMaxSize, "[String] Tried to add a Int to an already full string.");
#endif
	if (myCurrentSize >= myMaxSize)
		Resize(myCurrentSize * 2);

	char buffer[FW_STRING_MAX_NUMBER_CHARACTERS];
	_snprintf_s(buffer, FW_STRING_MAX_NUMBER_CHARACTERS, "%i", aInt);

	FW_String newString(buffer);
	*this += newString;

	return *this;
}

inline FW_String& FW_String::operator+=(const char* aString)
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(myCurrentSize < myMaxSize, "[String] Tried to add a Char* to an already full string.");
#endif
	if (myCurrentSize >= myMaxSize)
		Resize(myCurrentSize * 2);

	FW_String newString(aString);
	*this += newString;

	return *this;
}

inline FW_String& FW_String::operator+=(const char aChar)
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(myCurrentSize < myMaxSize, "[String] Tried to add a Char to an already full string.");
#endif
	if (myCurrentSize >= myMaxSize)
		Resize(myCurrentSize * 2);

	FW_String newString(aChar);
	*this += newString;

	return *this;
}

inline bool FW_String::operator==(const FW_String &aString) const
{
	return myHash == aString.myHash;
}

inline bool FW_String::operator==(const char* aString) const
{
	return strcmp(myData, aString) == 0;
}

inline bool FW_String::operator!=(const FW_String &aString) const
{
	return !(*this == aString);
}

inline bool FW_String::operator!=(const char* aString) const
{
	return !(*this == aString);
}

inline void FW_String::operator=(const FW_String &aString)
{
	int targetLenght = aString.Length();
	if (myMaxSize <= targetLenght + 1)
		Resize(targetLenght + 1);

	myCurrentSize = 0;
	for (int i = 0; i < aString.myCurrentSize; ++i)
	{
		myData[myCurrentSize] = aString[i];
		++myCurrentSize;
	}

	if (myData[myCurrentSize] != NullTermination)
	{
		myData[myCurrentSize] = NullTermination;
	}

	UpdateHash();
}

inline void FW_String::operator=(const char* aString)
{
	int targetLenght = static_cast<int>(strlen(aString));

	if (myMaxSize <= targetLenght)
		Resize(targetLenght);

	myCurrentSize = 0;
	for (unsigned int i = 0; i < strlen(aString); ++i)
	{
		myData[myCurrentSize] = aString[i];
		++myCurrentSize;
	}

	if (myData[myCurrentSize] != NullTermination)
	{
		myData[myCurrentSize] = NullTermination;
	}

	UpdateHash();
}


inline int FW_String::Find(const char* aPattern, const int aStartIndex) const
{
	FW_String pattern(aPattern);

	return Find(pattern, aStartIndex);
}

inline int FW_String::Find(const FW_String &aPattern, const int aStartIndex) const
{
	int lenghtOfText = myCurrentSize;
	int lenghtOfPattern = aPattern.myCurrentSize;

	int jumpTable[256];
	for (int i = 0; i < 256; ++i)
	{
		jumpTable[i] = NotFound;
	}

	for (int k = 0; k < lenghtOfPattern - 1; ++k)
	{
		jumpTable[aPattern[k]] = lenghtOfPattern - k - 1;
	}


	if (lenghtOfPattern > lenghtOfText) return NotFound;

	int k = lenghtOfPattern - 1 + aStartIndex;

	while (k < lenghtOfText)
	{
		int j = lenghtOfPattern - 1;
		int i = k;
		while (j >= 0 && myData[i] == aPattern[j])
		{
			--j;
			--i;
		}
		if (j == -1)
			return i + 1;


		int jumpValue = jumpTable[myData[k]];
		if (jumpValue == NotFound)
		{
			k += lenghtOfPattern;
		}
		else
		{
			k += jumpValue;
		}
	}


	return NotFound;
}

inline int FW_String::RFind(const char* aPattern) const
{
	FW_String pattern(aPattern);

	return RFind(pattern);
}

inline int FW_String::RFind(const FW_String &aPattern) const
{
	int lenghtOfText = myCurrentSize;
	int lenghtOfPattern = aPattern.myCurrentSize;

	int jumpTable[256];
	for (int x = 0; x < 256; ++x)
	{
		jumpTable[x] = NotFound;
	}


	for (int x = lenghtOfPattern - 1; x > 0; --x)
	{
		jumpTable[aPattern[x]] = x;
	}

	if (lenghtOfPattern > lenghtOfText) return NotFound;

	int k = myCurrentSize - 1;

	while (k >= 0)
	{
		int j = 0;
		int i = k;
		while (j <= lenghtOfPattern /*- 1*/ && myData[i] == aPattern[j])
		{
			++j;
			++i;
		}
		if (j == lenghtOfPattern)
			return i - lenghtOfPattern;


		int jumpValue = jumpTable[myData[k]];
		if (jumpValue == NotFound)
		{
			k -= lenghtOfPattern;
		}
		else
		{
			k -= jumpValue;
		}
	}


	return NotFound;
}


inline FW_String FW_String::SubStr(const int aStart, const int aEnd) const
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(aStart < myMaxSize, "[String]: Tried to create a SubString starting at an Index greater than the lenght of the main-string.");
	FW_ASSERT(aStart >= 0, "[String]: Tried to create a SubString starting at an Index greater than the lenght of the main-string.");
	FW_ASSERT(aEnd <= Length(), "[String]: Tried to create a SubString thats longer than the main-string.");
#endif

	FW_String newString;
	for (int i = aStart; i <= aEnd; ++i)
	{
		newString += static_cast<char>(operator[](i));
	}


	if (newString[newString.myCurrentSize] != NullTermination)
	{
		newString[newString.myCurrentSize] = NullTermination;
	}

	return newString;
}

inline const char* FW_String::GetBuffer() const
{
	return myData;
}

inline char* FW_String::GetRawBuffer()
{
	return myData;
}

inline int FW_String::Length() const
{
	return myCurrentSize - 1;
}

inline bool FW_String::Empty() const
{
	return myCurrentSize == 0;
}

inline const char& FW_String::operator[](const int aIndex) const
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(aIndex < myCurrentSize + 1, "[String]: Tried to use Subscript with an index greater than the lenght.");
	FW_ASSERT(aIndex >= 0, "[Sting]: Tried to use Subscript with an index smaller than 0");
#endif
	return myData[aIndex];
}

inline char& FW_String::operator[](const int aIndex)
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(aIndex < myCurrentSize + 1, "[String]: Tried to use Subscript with an index greater than the lenght.");
	FW_ASSERT(aIndex >= 0, "[Sting]: Tried to use Subscript with an index smaller than 0");
#endif

	return myData[aIndex];
}

inline void FW_String::Resize(int aNewSize)
{
#ifdef FW_STRING_ASSERTS
	FW_ASSERT(aNewSize > 0, "Can't make String smaller than 1.");
#endif
	myMaxSize = aNewSize + 1;
	char* newData = new char[myMaxSize];
	//memcpy(newData, myData, sizeof(char) * myCurrentSize);
	for (int i = 0; i < myCurrentSize; ++i)
	{
		newData[i] = myData[i];
	}

	newData[myCurrentSize] = NullTermination;

	delete[] myData;
	myData = newData;
}

inline unsigned int FW_String::GetHash() const
{
	return myHash;
}

inline void FW_String::Clear()
{
	for (int i = 0; i < myCurrentSize; ++i)
		myData[i] = NullTermination;

	myCurrentSize = 0;
}

inline void FW_String::RemoveOne()
{
	if (myCurrentSize <= 0)
		return;

	myCurrentSize -= 1;
}

inline void FW_String::UpdateHash()
{
	MurmurHash3_x86_32(myData, myCurrentSize, 1000, &myHash);
}

//
//
//

inline FW_String operator+(const FW_String &aFirst, const FW_String &aSecond)
{
	FW_String newString(aFirst);
	newString += aSecond;

	return newString;
}

inline FW_String operator+(const FW_String &aString, const float aFloat)
{
	FW_String newString(aString);
	newString += aFloat;

	return newString;
}

inline FW_String operator+(const FW_String &aString, const double aDouble)
{
	FW_String newString(aString);
	newString += aDouble;

	return newString;
}

inline FW_String operator+(const FW_String &aString, const int aInt)
{
	FW_String newString(aString);
	newString += aInt;

	return newString;
}

inline FW_String operator+(const FW_String &aFirst, const char* aSecond)
{
	FW_String newString(aFirst);
	newString += aSecond;

	return newString;
}

inline FW_String operator+(const FW_String &aString, const char aChar)
{
	FW_String newString(aString);
	newString += aChar;

	return newString;
}

//

inline FW_String operator+(const float aFloat, const FW_String &aString)
{
	FW_String newString(aString);
	newString += aFloat;

	return newString;
}

inline FW_String operator+(const double aDouble, const FW_String &aString)
{
	FW_String newString(aString);
	newString += aDouble;

	return newString;
}

inline FW_String operator+(const int aInt, const FW_String &aString)
{
	FW_String newString(aString);
	newString += aInt;

	return newString;
}

inline FW_String operator+(const char* aSecond, const FW_String &aFirst)
{
	FW_String newString(aFirst);
	newString += aSecond;

	return newString;
}

inline FW_String operator+(const char aChar, const FW_String &aString)
{
	FW_String newString(aString);
	newString += aChar;

	return newString;
}