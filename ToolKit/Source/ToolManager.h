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
	std::vector<std::string> m_Order;

public:
	ToolManager();
	~ToolManager();

	void initialize(EventManager *p_EventManager, std::vector<std::string> p_Order);
	void updateToolOrder(std::vector<std::string> p_Order);
	void onPress();
	void onMove();
	void onRelease();
	void changeTool(int p_Index);
	void changeToPreviusTool();
	void updateMousePos(float x, float y);
	void updateScreenSize(int x, int y);
	Tool::Type getCurrentToolType();
};