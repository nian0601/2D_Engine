#pragma once

class FW_IGame
{
public:
	virtual bool Run() = 0;
	virtual void OnShutdown() {};
	virtual void RunEditorLogic() {};

protected:
	FW_IGame() {};
};
