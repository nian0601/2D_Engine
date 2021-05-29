#include "FW_FileParser.h"

FW_FileParser::FW_FileParser(const char* aFile)
{
	int result = fopen_s(&myFile, aFile, "r");
	assert(result == 0 && "Failed to open file");
}

FW_FileParser::~FW_FileParser()
{
	fclose(myFile);
}

bool FW_FileParser::ReadLine(FW_String& outLine)
{
	char text[64];
	if (fgets(text, 64, myFile) == NULL)
		return false;

	outLine = text;
	return true;
}

void FW_FileParser::TrimBeginAndEnd(FW_String& aLine) const
{
	if (aLine.Empty() == true)
		return;

	int begin = 0;
	while (aLine[begin] == ' ' || aLine[begin] == '\t' || aLine[begin] == '\n')
		++begin;

	int end = aLine.Lenght();
	if (end > begin)
	{
		while (aLine[end] == ' ' || aLine[end] == '\t' || aLine[end] == '\n')
			--end;
	}

	if (begin != 0 || end != aLine.Lenght())
		aLine = aLine.SubStr(begin, end);
}

void FW_FileParser::SplitLine(const FW_String& aLine, FW_GrowingArray<FW_String>& outWords) const
{
	outWords.RemoveAll();

	int begin = 0;
	int end = aLine.Find(" ", begin);
	if (end == -1)
	{
		outWords.Add(aLine);
		return;
	}

	while (end != -1)
	{
		outWords.Add(aLine.SubStr(begin, end-1));

		begin = end+1;
		end = aLine.Find(" ", begin);
	}

	outWords.Add(aLine.SubStr(begin, aLine.Lenght()));
}

void FW_FileParser::SplitWord(const FW_String& aLine, const char* aSeperator, FW_GrowingArray<FW_String>& outWords) const
{
	outWords.RemoveAll();

	int begin = 0;
	int end = aLine.Find(aSeperator, begin);
	if (end == -1)
	{
		outWords.Add(aLine);
		return;
	}

	while (end != -1)
	{
		outWords.Add(aLine.SubStr(begin, end - 1));

		begin = end + 1;
		end = aLine.Find(aSeperator, begin);
	}

	outWords.Add(aLine.SubStr(begin, aLine.Lenght()));
}

float FW_FileParser::GetFloat(const FW_String& aWord) const
{
	return static_cast<float>(atof(aWord.GetBuffer()));
}

int FW_FileParser::GetInt(const FW_String& aWord) const
{
	return static_cast<int>(atoll(aWord.GetBuffer()));
}
