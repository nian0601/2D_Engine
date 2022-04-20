#pragma once
#include <mbstring.h>

#define USING_RAM_STORAGE 0

class FW_String;
class FW_FileProcessor
{
public:
	enum Flags
	{
		BINARY = 1 << 0,
		READ = 1 << 1,
		WRITE = 1 << 2,
	};

	FW_FileProcessor(const char* aFile, int someFlags);
	~FW_FileProcessor();

	template <typename T>
	void Process(T& someData);

	void Process(FW_String& aString);
	void Process(const FW_String& aString);

	bool IsWriting() const { return (myFlags & WRITE) > 0; }
	bool IsReading() const { return (myFlags & READ) > 0; }

	bool AtEndOfFile() const;

private:
	bool IsOpen() const { return myStatus == 0; }
	FILE* myFile;
	int myFlags;
	int myStatus;
	const char* myFilePath; // Just for debugging

	unsigned char* myData;
	int myDataSize;
	int myCursorPosition;
};

template <typename T>
void FW_FileProcessor::Process(T& someData)
{
	assert(IsOpen() == true && "Tried to process an unopened file");
	
	if (IsWriting())
	{
#if USING_RAM_STORAGE
		memcpy(&myData[myCursorPosition], &someData, sizeof(someData));
		myCursorPosition += sizeof(someData);
#else
		fwrite(&someData, sizeof(someData), 1, myFile);
		if (ferror(myFile))
		{
			perror("Error writing generic data");
			FW_ASSERT_ALWAYS("Something went wrong");
		}
#endif
	}
	else if (IsReading())
	{
#if USING_RAM_STORAGE
		memcpy(&someData, &myData[myCursorPosition], sizeof(someData));
		myCursorPosition += sizeof(someData);
#else
		fread(&someData, sizeof(someData), 1, myFile);
		if (ferror(myFile))
		{
			perror("Error reading generic data");
			FW_ASSERT_ALWAYS("Something went wrong");
		}
#endif
	}
}