#pragma once
#include "FW_Vector2.h"

namespace FW_Input
{
	enum KeyCode
	{
		LEFT,
		RIGHT,
		UP,
		DOWN,
		W,
		S,
		A,
		D,
		Q,
		E,
		SPACE,
		ESC,
		DEL,
		__COUNT,
	};

	enum MouseButton
	{
		LEFTMB,
		MIDDLEMB,
		RIGHTMB,
		___COUNT
	};

	bool WasKeyPressed(KeyCode aKey);
	bool WasKeyReleased(KeyCode aKey);
	bool IsKeyDown(KeyCode aKey);

	bool WasMousePressed(MouseButton aMouseButton);
	bool WasMouseReleased(MouseButton aMouseButton);
	bool IsMouseDown(MouseButton aMouseButton);

	const Vector2i& GetMousePosition();
	const Vector2f& GetMousePositionf();
}
