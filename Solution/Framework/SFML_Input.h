#pragma once
#include "FW_Vector2.h"

namespace sf
{
	class RenderWindow;
}
namespace SFML_Input
{
	void FlushInput(const sf::RenderWindow& aWindow);
	void ClearInput();

	void SetWindowHasFocus(bool aStatus);
}
