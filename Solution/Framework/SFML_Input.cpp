#include <vcruntime_string.h>
#include <assert.h>
#include "FW_Input.h"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace SFML_Input
{
	enum State
	{
		KEYSTATE_NONE,
		KEYSTATE_PRESSED,
		KEYSTATE_DOWN,
		KEYSTATE_RELEASED,
	};

	static State ourStates[FW_Input::KeyCode::__COUNT + 1];
	static State ourMouseStates[FW_Input::MouseButton::___COUNT];
	static Vector2i ourMousePosition;
	static Vector2f ourMousePositionf;
	static bool ourWindowHasFocus = true;

	void UpdateState(State& aState)
	{
		switch (aState)
		{
		case KEYSTATE_PRESSED:
			aState = KEYSTATE_DOWN;
			break;
		case KEYSTATE_RELEASED:
			aState = KEYSTATE_NONE;
			break;
		case KEYSTATE_DOWN:
		case KEYSTATE_NONE:
			// Dont do anything
			break;
		}
	}

	sf::Keyboard::Key GetSFMLKey(FW_Input::KeyCode aEngineKeyCode)
	{
		switch (aEngineKeyCode)
		{
		case FW_Input::LEFT: return sf::Keyboard::Left;
		case FW_Input::RIGHT: return sf::Keyboard::Right;
		case FW_Input::UP: return sf::Keyboard::Up;
		case FW_Input::DOWN: return sf::Keyboard::Down;
		case FW_Input::W: return sf::Keyboard::W;
		case FW_Input::S: return sf::Keyboard::S;
		case FW_Input::A: return sf::Keyboard::A;
		case FW_Input::D: return sf::Keyboard::D;
		case FW_Input::Q: return sf::Keyboard::Q;
		case FW_Input::E: return sf::Keyboard::E;
		case FW_Input::SPACE: return sf::Keyboard::Space;
		case FW_Input::ESC: return sf::Keyboard::Escape;
		case FW_Input::DEL: return sf::Keyboard::Delete;
		default:
			break;
		}

		return sf::Keyboard::Escape;
	}

	sf::Mouse::Button GetSFMLMouseButton(FW_Input::MouseButton aEngineMouseButton)
	{
		switch (aEngineMouseButton)
		{
		case FW_Input::LEFTMB: return sf::Mouse::Left;
		case FW_Input::MIDDLEMB: return sf::Mouse::Middle;
		case FW_Input::RIGHTMB: return sf::Mouse::Right;
		default:
			break;
		}

		return sf::Mouse::Left;
	}

	void UpdateKeyboard()
	{
		for (int i = 0; i < FW_Input::KeyCode::__COUNT; ++i)
			UpdateState(ourStates[i]);

		for (int i = 0; i < FW_Input::KeyCode::__COUNT; ++i)
		{
			sf::Keyboard::Key sfmlKey = GetSFMLKey(FW_Input::KeyCode(i));

			State prevState = ourStates[i];
			bool isCurrentlyPressed = sf::Keyboard::isKeyPressed(sfmlKey);

			if (isCurrentlyPressed && prevState == KEYSTATE_NONE)
				ourStates[i] = KEYSTATE_PRESSED;
			else if (!isCurrentlyPressed && prevState == KEYSTATE_DOWN)
				ourStates[i] = KEYSTATE_RELEASED;
		}
	}

	void UpdateMouse(const sf::RenderWindow& aWindow)
	{
		for (int i = 0; i < FW_Input::MouseButton::___COUNT; ++i)
			UpdateState(ourMouseStates[i]);

		for (int i = 0; i < FW_Input::MouseButton::___COUNT; ++i)
		{
			sf::Mouse::Button sfmlKey = GetSFMLMouseButton(FW_Input::MouseButton(i));

			State prevState = ourMouseStates[i];
			bool isCurrentlyPressed = sf::Mouse::isButtonPressed(sfmlKey);

			if (isCurrentlyPressed && prevState == KEYSTATE_NONE)
				ourMouseStates[i] = KEYSTATE_PRESSED;
			else if (!isCurrentlyPressed && prevState == KEYSTATE_DOWN)
				ourMouseStates[i] = KEYSTATE_RELEASED;
		}

		sf::Vector2i sfmlPosition = sf::Mouse::getPosition(aWindow);
		ourMousePosition.x = sfmlPosition.x;
		ourMousePosition.y = sfmlPosition.y;

		ourMousePositionf.x = static_cast<float>(sfmlPosition.x);
		ourMousePositionf.y = static_cast<float>(sfmlPosition.y);
	}

	void ClearInput()
	{
		for (int i = 0; i < FW_Input::KeyCode::__COUNT; ++i)
			ourStates[i] = State::KEYSTATE_NONE;

		for (int i = 0; i < FW_Input::MouseButton::___COUNT; ++i)
			ourMouseStates[i] = State::KEYSTATE_NONE;
	}

	void FlushInput(const sf::RenderWindow& aWindow)
	{
		if (ourWindowHasFocus)
		{
			UpdateKeyboard();
			UpdateMouse(aWindow);
		}
		else
		{
			ClearInput();
		}
	}

	void SetWindowHasFocus(bool aStatus)
	{
		ourWindowHasFocus = aStatus;
	}

}

namespace FW_Input
{
	bool FW_Input::WasKeyPressed(KeyCode aKey)
	{
		return SFML_Input::ourStates[aKey] == SFML_Input::KEYSTATE_PRESSED;
	}

	bool WasKeyReleased(KeyCode aKey)
	{
		return SFML_Input::ourStates[aKey] == SFML_Input::KEYSTATE_RELEASED;
	}

	bool IsKeyDown(KeyCode aKey)
	{
		return SFML_Input::ourStates[aKey] == SFML_Input::KEYSTATE_DOWN;
	}

	bool WasMousePressed(MouseButton aMouseButton)
	{
		return SFML_Input::ourMouseStates[aMouseButton] == SFML_Input::KEYSTATE_PRESSED;
	}

	bool WasMouseReleased(MouseButton aMouseButton)
	{
		return SFML_Input::ourMouseStates[aMouseButton] == SFML_Input::KEYSTATE_RELEASED;
	}

	bool IsMouseDown(MouseButton aMouseButton)
	{
		return SFML_Input::ourMouseStates[aMouseButton] == SFML_Input::KEYSTATE_DOWN;
	}

	const Vector2i& GetMousePosition()
	{
		return SFML_Input::ourMousePosition;
	}

	const Vector2f& GetMousePositionf()
	{
		return SFML_Input::ourMousePositionf;
	}
}