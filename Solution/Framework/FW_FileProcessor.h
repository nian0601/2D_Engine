#pragma once
#include <mbstring.h>

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
};

template <typename T>
void FW_FileProcessor::Process(T& someData)
{
	assert(IsOpen() == true && "Tried to process an unopened file");
	
	if (IsWriting())
		fwrite(&someData, sizeof(someData), 1, myFile);
	else if (IsReading())
		fread(&someData, sizeof(someData), 1, myFile);
}