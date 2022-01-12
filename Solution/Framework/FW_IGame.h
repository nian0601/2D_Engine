#pragma once

class FW_IGame
{
public:
	virtual void OnStartup() {};
	virtual bool Run() = 0;
	virtual void OnShutdown() {};
	virtual void RunEditorLogic() {};
	virtual void BuildGameImguiEditor(unsigned int aGameOffscreenBufferTextureID) { aGameOffscreenBufferTextureID; };

	virtual const char* GetGameName() { return "Generic Game"; }
	virtual const char* GetDataFolderName() { return "Generic Folder"; }

protected:
	FW_IGame() {};
};
