#include "stdafx.h"

#include "Score.h"
#include <sstream>

Score::Score()
{
	Reset();
}

void Score::Render()
{
	const int screenCenter = FW_Renderer::GetScreenWidth() / 2;
	// Replace with own String-class, or add number-rendering to the RenderInterface
	std::stringstream ss;
	ss << myLeftPlayerScore;
	FW_Renderer::RenderText(ss.str().c_str(), screenCenter - 100, 50);

	ss.str("");

	ss << myRightPlayerScore;
	FW_Renderer::RenderText(ss.str().c_str(), screenCenter + 100, 50);
}