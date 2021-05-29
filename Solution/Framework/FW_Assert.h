#pragma once

#define FW_DEBUG_BREAK __debugbreak();
#define FW_ASSERT_ALWAYS \
{\
	int* assertPtr = nullptr;\
	*assertPtr = 1;\
}\


#define FW_ASSERT(anExpression, ...) \
{\
	if(!(anExpression)) \
	{\
		FW_DEBUG_BREAK;\
		FW_ASSERT_ALWAYS;\
	}\
}