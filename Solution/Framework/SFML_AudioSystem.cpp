#include "FW_AudioSystem.h"
#include "SFML_AudioSystem.h"

#include <SFML/Audio.hpp>
#include "FW_GrowingArray.h"
#include "FW_String.h"

#include <assert.h>
#include <windows.h>

namespace SFML_AudioSystem
{
	struct CachedClip
	{
		~CachedClip()
		{
			delete myBuffer;
		}

		sf::SoundBuffer* myBuffer = nullptr;
		int myClipID;
		FW_String myClipFilePath;
	};
	static FW_GrowingArray<CachedClip> ourCachedClips;
	static int ourNewClipID = 0;
	static sf::Sound* ourSound = nullptr;

	static std::string ourAssetPath;

	void ReplaceAllOccurancesInString(std::string& aString, const std::string& aSomethingToReplace, const std::string& aNewString)
	{
		// Get the first occurrence
		size_t pos = aString.find(aSomethingToReplace);

		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			aString.replace(pos, aSomethingToReplace.size(), aNewString);
			// Get the next occurrence from the current position
			pos = aString.find(aSomethingToReplace, pos + aNewString.size());
		}
	}

	void Init()
	{
		ourSound = new sf::Sound();
	}

	void Shutdown()
	{
		delete ourSound;
	}

	void SetDataFolder(const char* aFolderName)
	{
		char fileBuffer[128];
		assert(GetModuleFileNameA(NULL, fileBuffer, 128) != 0 && "Failed to get Executable path");
		ourAssetPath.append(fileBuffer);
		ourAssetPath.erase(ourAssetPath.rfind("\\"), std::string::npos);
		ReplaceAllOccurancesInString(ourAssetPath, "\\", "/");
		ourAssetPath.append("/");
		ourAssetPath.append(aFolderName);
		ourAssetPath.append("/Data/");
	}

	int LoadClip(const char* aFilePath)
	{
		for (const CachedClip& clip : ourCachedClips)
		{
			if (clip.myClipFilePath == aFilePath)
				return clip.myClipID;
		}

		std::string fullPath = ourAssetPath;
		fullPath.append(aFilePath);

		sf::SoundBuffer* buffer = new sf::SoundBuffer();
		if (buffer->loadFromFile(fullPath))
		{
			CachedClip& newClip = ourCachedClips.Add();
			newClip.myBuffer = buffer;
			newClip.myClipID = ourNewClipID++;
			newClip.myClipFilePath = aFilePath;
			return newClip.myClipID;
		}

		delete buffer;
		return -1;
	}

	void PlayClip(int aClipID)
	{
		for (const CachedClip& clip : ourCachedClips)
		{
			if (clip.myClipID == aClipID)
			{
				ourSound->setBuffer(*clip.myBuffer);
				ourSound->play();
			}
		}
	}
}

namespace FW_AudioSystem
{
	int LoadClip(const char* aFilePath)
	{
		return SFML_AudioSystem::LoadClip(aFilePath);
	}
	
	void PlayClip(int aClipID)
	{
		SFML_AudioSystem::PlayClip(aClipID);
	}
}