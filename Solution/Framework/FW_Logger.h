#pragma once

#include "FW_CircularArray.h"
#include "FW_String.h"

namespace FW_Logger
{
	const FW_CircularArray<FW_String, 128>& GetAllMessages();

	void AddMessage(const char* aMessage);
}
