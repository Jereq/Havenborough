#include "Tools.h"
#include <vector>
#include <map>


class ToolManager
{
private:
	Tool *m_CurrentTool;
	Tool *m_PreviusTool;
	std::vector<Tool*> m_ToolList;
	EventManager *m_EventManager;
	std::map<std::string, Tool::Type> m_ToolMap;
	const std::string *m_Icons;
public:
	ToolManager();
	~ToolManager();
	void initialize(EventManager *p_EventManager, const std::string *p_Icons);
	void onPress();
	void onMove();
	void onRelease();
	void changeTool(int p_Index);
	void changeToPreviusTool();
	void updateMousePos(float x, float y);
	void updateScreenSize(int x, int y);
	Tool::Type getCurrentToolType();
};