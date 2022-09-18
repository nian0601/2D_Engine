#include "FW_FileSystem.h"

#include "FW_Assert.h"

namespace FW_FileSystem
{
	static FW_String ourDataFolderPath = "";
	void SetDataFolder(const char* aFolderName)
	{
		ourDataFolderPath = aFolderName;
		ourDataFolderPath += "/Data/";
	}

	void GetRealFilePath(const FW_String& aFilePath, FW_String& aFilePathOut)
	{
		aFilePathOut = ourDataFolderPath + aFilePath;
	}

	bool GetAllFilesFromDirectory(const char* aDirectory, FW_GrowingArray<FileInfo>& someOutFilePaths)
	{
		FW_ASSERT(strlen(aDirectory) + 3 < MAX_PATH, "Path to directory is too long");

		FW_String directory = ourDataFolderPath;
		directory += aDirectory;
		directory += "/*";

		WIN32_FIND_DATA data;
		HANDLE filehandle = FindFirstFile(directory.GetBuffer(), &data);

		if (filehandle == INVALID_HANDLE_VALUE)
			return false;

		do
		{
			FW_String name = data.cFileName;
			if (name == "." || name == "..")
				continue;

			FW_String fullPath = aDirectory;
			fullPath += "/";
			fullPath += name;

			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				GetAllFilesFromDirectory(fullPath.GetBuffer(), someOutFilePaths);
			}
			else
			{
				FileInfo& info = someOutFilePaths.Add();
				info.myFileName = name;
				info.myFilePath = fullPath;
				info.myLastTimeModifiedLowbit = data.ftLastWriteTime.dwLowDateTime;
				info.myLastTimeModifiedHighbit = data.ftLastWriteTime.dwHighDateTime;

				RemoveFileExtention(name, info.myFileNameNoExtention);
			}
		} while (FindNextFile(filehandle, &data) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES)
			FW_ASSERT_ALWAYS("Something went wrong...");

		FindClose(filehandle);
		return true;
	}

	void GetFileName(const FW_String& aFilePath, FW_String& aNameOut)
	{
		int findIndex = aFilePath.RFind("/");
		aNameOut = aFilePath.SubStr(findIndex + 1, aFilePath.Length());
	}

	void RemoveFileName(const FW_String& aFilePath, FW_String& aFilePathOut)
	{
		int findIndex = aFilePath.RFind("/");
		aFilePathOut = aFilePath.SubStr(0, findIndex);
	}

	void GetFileExtention(const FW_String& aFilePath, FW_String& aExtentionOut)
	{
		int findIndex = aFilePath.RFind(".");
		aExtentionOut = aFilePath.SubStr(findIndex + 1, aFilePath.Length());
	}

	void RemoveFileExtention(const FW_String& aFilePath, FW_String& aNameOut)
	{
		int findIndex = aFilePath.RFind(".");
		aNameOut = aFilePath.SubStr(0, findIndex - 1);
	}

	bool GetFileInfo(const FW_String& aFilePath, FileInfo& aFileInfoOut)
	{
		FW_ASSERT(aFilePath.Length() + 3 < MAX_PATH, "Filepath is too long");

		WIN32_FIND_DATA data;
		HANDLE filehandle = FindFirstFile(aFilePath.GetBuffer(), &data);

		if (filehandle == INVALID_HANDLE_VALUE)
			return false;

		aFileInfoOut.myFileName = data.cFileName;
		aFileInfoOut.myFilePath = aFilePath;
		aFileInfoOut.myLastTimeModifiedLowbit = data.ftLastWriteTime.dwLowDateTime;
		aFileInfoOut.myLastTimeModifiedHighbit = data.ftLastWriteTime.dwHighDateTime;

		FindClose(filehandle);
		return true;
	}

	bool UpdateFileInfo(FW_GrowingArray<FileInfo>& someFiles)
	{
		bool somethingChanged = false;
		FileInfo newInfo;
		for (FileInfo& oldInfo : someFiles)
		{
			GetFileInfo(oldInfo.myFilePath, newInfo);

			FILETIME oldTime;
			oldTime.dwLowDateTime = oldInfo.myLastTimeModifiedLowbit;
			oldTime.dwHighDateTime = oldInfo.myLastTimeModifiedHighbit;

			FILETIME newTime;
			newTime.dwLowDateTime = newInfo.myLastTimeModifiedLowbit;
			newTime.dwHighDateTime = newInfo.myLastTimeModifiedHighbit;

			if (CompareFileTime(&oldTime, &newTime) != 0)
				somethingChanged = true;

			oldInfo.myLastTimeModifiedLowbit = newInfo.myLastTimeModifiedLowbit;
			oldInfo.myLastTimeModifiedHighbit = newInfo.myLastTimeModifiedHighbit;
		}

		return somethingChanged;
	}

	void ReadEntireFile(const FW_String& aFilePath, FileContent& aFileContentOut)
	{
		FILE* file;
		fopen_s(&file, aFilePath.GetBuffer(), "rb");
		fseek(file, 0, SEEK_END);
		long fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		unsigned char* string = new unsigned char[fileSize + 1];
		fread(string, fileSize, 1, file);
		fclose(file);

		string[fileSize] = 0;

		aFileContentOut.myContents = string;
		aFileContentOut.myFileSize = fileSize;
	}
}