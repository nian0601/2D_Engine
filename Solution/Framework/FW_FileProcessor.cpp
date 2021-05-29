#include "FW_FileProcessor.h"
#include "FW_String.h"
#include "FW_Assert.h"
#include <stdio.h>

FW_FileProcessor::FW_FileProcessor(const char* aFile, int someFlags)
	: myFilePath(aFile)
	, myFlags(someFlags)
{
	FW_String flags;
	if ((myFlags & WRITE) > 0)
		flags += "w";
	if ((myFlags& READ) > 0)
		flags += "r";

	myStatus = fopen_s(&myFile, myFilePath, flags.GetBuffer());
}

FW_FileProcessor::~FW_FileProcessor()
{
	if (IsOpen())
		fclose(myFile);
}

void FW_FileProcessor::Process(FW_String& aString)
{
	assert(IsOpen() == true && "Tried to process an unopened file");

	if (IsWriting())
	{
		int stringLenght = aString.Lenght() + 2;
		fwrite(&stringLenght, sizeof(int), 1, myFile);
		if (ferror(myFile))
		{
			perror("Error writing StringLenght");
			FW_ASSERT_ALWAYS("Something went wrong");
		}

		fwrite(aString.GetBuffer(), sizeof(char), stringLenght, myFile);
		if (ferror(myFile))
		{
			perror("Error writing String");
			FW_ASSERT_ALWAYS("Something went wrong");
		}
	}
	else if (IsReading())
	{
		int stringLenght = 0;
		fread(&stringLenght, sizeof(int), 1, myFile);
		if (ferror(myFile))
		{
			perror("Error reading StringLenght");
			FW_ASSERT_ALWAYS("Something went wrong");
		}

		char stringBuffer[64] = { "INVALID" };
		fread(&stringBuffer, sizeof(char), stringLenght, myFile);
		if (ferror(myFile))
		{
			perror("Error reading String");
			FW_ASSERT_ALWAYS("Something went wrong");
		}

		aString = stringBuffer;
	}
}

void FW_FileProcessor::Process(const FW_String& aString)
{
	FW_ASSERT(IsOpen() == true && "Tried to process an unopened file");
	FW_ASSERT(IsWriting(), "Tried to read from disc into a const string");

	int stringLenght = aString.Lenght() + 2;
	fwrite(&stringLenght, sizeof(int), 1, myFile);
	if (ferror(myFile))
	{
		perror("Error writing StringLenght");
		FW_ASSERT_ALWAYS("Something went wrong");
	}

	fwrite(aString.GetBuffer(), sizeof(char), stringLenght, myFile);
	if (ferror(myFile))
	{
		perror("Error writing String");
		FW_ASSERT_ALWAYS("Something went wrong");
	}
}

bool FW_FileProcessor::AtEndOfFile() const
{
	return feof(myFile);
}
