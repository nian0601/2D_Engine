#pragma once

#include "FW_String.h"
#include "FW_GrowingArray.h"

class FW_FileParser
{
public:
	FW_FileParser(const char* aFile);
	~FW_FileParser();

	bool ReadLine(FW_String& outLine);

	void TrimBeginAndEnd(FW_String& aLine) const;

	void SplitLineOnSpace(const FW_String& aLine, FW_GrowingArray<FW_String>& outWords) const;
	void SplitLine(const FW_String& aLine, const char* aSeperator, FW_GrowingArray<FW_String>& outWords) const;

	float GetFloat(const FW_String& aWord) const;
	int GetInt(const FW_String& aWord) const;

	const FW_String& GetFilePath() const { return myFilePath; }

private:
	FILE* myFile;
	FW_String myFilePath;
};

