#pragma once

#include "FW_GrowingArray.h"
#include "FW_String.h"
#include <windows.h>

namespace FW_FileSystem
{
	struct FileInfo
	{
		FW_String myFileName;
		FW_String myFileNameNoExtention;
		FW_String myFilePath;
		unsigned long myLastTimeModifiedLowbit;
		unsigned long myLastTimeModifiedHighbit;
	};

	// The Contents of the file will be freed when the FileContent
	// goes out of scope, be careful if you allocated temporary FileContents
	struct FileContent
	{
		FileContent(bool aAutoFreeContents = true) : myContents(nullptr), myAutoFreeContents(aAutoFreeContents) {}
		~FileContent() { if (myAutoFreeContents) delete myContents; }
		unsigned char* myContents;
		long myFileSize;
		bool myAutoFreeContents;
	};


	void SetDataFolder(const char* aFolderName);

	void GetRealFilePath(const FW_String& aFilePath, FW_String& aFilePathOut);

	bool GetAllFilesFromDirectory(const char* aDirectory, FW_GrowingArray<FileInfo>& someOutFilePaths);
	void GetFileName(const FW_String& aFilePath, FW_String& aNameOut);
	void RemoveFileName(const FW_String& aFilePath, FW_String& aFilePathOut);
	void GetFileExtention(const FW_String& aFilePath, FW_String& aExtentionOut);
	void RemoveFileExtention(const FW_String& aFilePath, FW_String& aNameOut);
	bool GetFileInfo(const FW_String& aFilePath, FileInfo& aFileInfoOut);

	bool UpdateFileInfo(FW_GrowingArray<FileInfo>& someFiles);

	void ReadEntireFile(const FW_String& aFilePath, FileContent& aFileContentOut);
}