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
		_1,
		_2,
		_3,
		_4,
		_5,
		_6,
		_7,
		_8,
		_9,
		_0,
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
