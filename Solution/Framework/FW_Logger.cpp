#include "FW_Logger.h"

namespace FW_Logger
{
	static FW_CircularArray<FW_String, 128> ourMessages;

	const FW_CircularArray<FW_String, 128>& FW_Logger::GetAllMessages()
	{
		return ourMessages;
	}

	void FW_Logger::AddMessage(const char* aMessage)
	{
		ourMessages.Add(FW_String(aMessage));
	}
}