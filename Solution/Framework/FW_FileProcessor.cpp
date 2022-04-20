#include "FW_FileProcessor.h"
#include "FW_String.h"
#include "FW_Assert.h"
#include <stdio.h>
#include "FW_FileSystem.h"

FW_FileProcessor::FW_FileProcessor(const char* aFile, int someFlags)
	: myFilePath(aFile)
	, myFlags(someFlags)
{
	FW_String flags;
	if ((myFlags & WRITE) > 0)
	{
		flags += "wb";

		myDataSize = 1024 * 1024 * 1024;
		myData = new unsigned char[myDataSize];
		myCursorPosition = 0;
	}
	if ((myFlags& READ) > 0)
	{
		flags += "rb";

		FW_FileSystem::FileContent entireFile(false);
		FW_FileSystem::ReadEntireFile(aFile, entireFile);

		myData = entireFile.myContents;
		myDataSize = entireFile.myFileSize;
		myCursorPosition = 0;
	}

	myStatus = fopen_s(&myFile, myFilePath, flags.GetBuffer());
}

FW_FileProcessor::~FW_FileProcessor()
{
	if (IsOpen())
	{
		if (IsWriting())
		{
			size_t numWritten = fwrite(myData, sizeof(char), myCursorPosition, myFile);
			if (numWritten != unsigned int(myCursorPosition))
				perror(nullptr);
		}

		fclose(myFile);
	}

	delete myData;
}

void FW_FileProcessor::Process(FW_String& aString)
{
	assert(IsOpen() == true && "Tried to process an unopened file");

	if (IsWriting())
	{
#if USING_RAM_STORAGE
		if (aString.Empty())
		{
			int stringLenght = 0;
			memcpy(&myData[myCursorPosition], &stringLenght, sizeof(stringLenght));
			myCursorPosition += sizeof(stringLenght);
		}
		else
		{
			int stringLenght = aString.Length() + 2;
			memcpy(&myData[myCursorPosition], &stringLenght, sizeof(stringLenght));
			myCursorPosition += sizeof(stringLenght);

			memcpy(&myData[myCursorPosition], aString.GetRawBuffer(), sizeof(char) * stringLenght);
			myCursorPosition += sizeof(char) * stringLenght;
		}
#else
		if (aString.Empty())
		{
			int stringLenght = 0;
			fwrite(&stringLenght, sizeof(int), 1, myFile);
			if (ferror(myFile))
			{
				perror("Error writing StringLenght");
				FW_ASSERT_ALWAYS("Something went wrong");
			}
		}
		else
		{
			int stringLenght = aString.Length() + 2;
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
#endif
	}
	else if (IsReading())
	{
#if USING_RAM_STORAGE
		int stringLenght = 0;
		memcpy(&stringLenght, &myData[myCursorPosition], sizeof(stringLenght));
		myCursorPosition += sizeof(stringLenght);

		if (stringLenght > 100)
		{
			int apa = 5;
			++apa;
		}

		if (stringLenght > 0)
		{
			char stringBuffer[64] = { "INVALID" };

			memcpy(&stringBuffer, &myData[myCursorPosition], sizeof(char) * stringLenght);
			myCursorPosition += sizeof(char) * stringLenght;

			aString = stringBuffer;
		}
#else
		int stringLenght = 0;
		fread(&stringLenght, sizeof(int), 1, myFile);
		if (ferror(myFile))
		{
			perror("Error reading StringLenght");
			FW_ASSERT_ALWAYS("Something went wrong");
		}
		
		if (stringLenght > 0)
		{
			char stringBuffer[64] = { "INVALID" };
			fread(&stringBuffer, sizeof(char), stringLenght, myFile);
			if (ferror(myFile))
			{
				perror("Error reading String");
				FW_ASSERT_ALWAYS("Something went wrong");
			}
			aString = stringBuffer;
		}
#endif
	}
}

void FW_FileProcessor::Process(const FW_String& aString)
{
	FW_ASSERT(IsOpen() == true && "Tried to process an unopened file");
	FW_ASSERT(IsWriting(), "Tried to read from disc into a const string");

	int stringLenght = aString.Length() + 2;
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
