#include "Tools.h"
#include <vector>


class ToolManager
{
private:
	Tool *m_CurrentTool;
	Tool *m_PreviusTool;
	std::vector<Tool*> m_ToolList;
	EventManager *m_EventManager;
public:
	ToolManager();
	~ToolManager();
	void initialize(EventManager *p_EventManager);
	void OnPress();
	void OnMove();
	void OnRelease();
	void changeTool(Tool::Type p_Type);
	void changeToPreviusTool();
	void updateMousePos(float x, float y);
	void updateScreenSize(int x, int y);
	Tool::Type getCurrentToolType();
};