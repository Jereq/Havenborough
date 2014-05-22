#include "ToolManager.h"


ToolManager::ToolManager() :
	m_EventManager(nullptr), m_CurrentTool(nullptr),
	m_PreviusTool(nullptr)
{}

ToolManager::~ToolManager()
{
		
}

void ToolManager::initialize(EventManager *p_EventManager, std::vector<std::string> p_Order)
{
	m_EventManager = p_EventManager;
	updateToolOrder(p_Order);

	m_ToolMap.insert(std::pair<std::string, Tool::Type>("Translate", Tool::Type::TRANSLATE));
	m_ToolMap.insert(std::pair<std::string, Tool::Type>("Paste", Tool::Type::PASTE));
	m_ToolMap.insert(std::pair<std::string, Tool::Type>("Copy", Tool::Type::COPY));
	m_ToolMap.insert(std::pair<std::string, Tool::Type>("Rotate", Tool::Type::ROTATE));
	m_ToolMap.insert(std::pair<std::string, Tool::Type>("Resize", Tool::Type::RESIZE));
	m_ToolMap.insert(std::pair<std::string, Tool::Type>("Camera", Tool::Type::CAMERA));
	m_ToolMap.insert(std::pair<std::string, Tool::Type>("Select", Tool::Type::SELECT));
	m_ToolMap.insert(std::pair<std::string, Tool::Type>("Eye", Tool::Type::EYE));

	m_ToolList.push_back(new SelectTool);
	m_ToolList.push_back(new TranslateTool);

	for(auto &a : m_ToolList)
	{
		a->initialize(m_EventManager);
	}

	for(unsigned int i = 0; i < m_ToolList.size(); i++)
	{
		if(m_ToolList[i]->getToolType() == Tool::Type::SELECT)
		{
			m_PreviusTool = m_CurrentTool;
			m_CurrentTool = m_ToolList[i];
			break;
		}
	}

	m_PreviusTool = m_CurrentTool;
}

void ToolManager::updateToolOrder(std::vector<std::string> p_Order)
{
	m_Order = p_Order;
}

void ToolManager::onPress()
{
	m_CurrentTool->onPress();
}

void ToolManager::onMove()
{
	m_CurrentTool->onMove();
}

void ToolManager::onRelease()
{
	m_CurrentTool->onRelease();
}

void ToolManager::changeTool(int p_Index)
{
	if(p_Index < 0)
		return;

	std::string tool = m_Order.at(p_Index);
	Tool::Type type;
	if(m_ToolMap.count(tool) > 0)
	{
		type = m_ToolMap.at(tool);
	}

	if(type == m_CurrentTool->getToolType())
		return;

	for(unsigned int i = 0; i < m_ToolList.size(); i++)
	{
		if(m_ToolList[i]->getToolType() == type)
		{
			m_PreviusTool = m_CurrentTool;
			m_CurrentTool = m_ToolList[i];
			break;
		}
	}
}

void ToolManager::changeToPreviusTool()
{
	m_CurrentTool = m_PreviusTool;
}

Tool::Type ToolManager::getCurrentToolType()
{
	return m_CurrentTool->getToolType();
}

void ToolManager::updateMousePos(float x, float y)
{
	m_CurrentTool->updateMousePos(DirectX::XMFLOAT2(x,y));
}

void ToolManager::updateScreenSize(int x, int y)
{
	m_CurrentTool->updateScreenSize(DirectX::XMFLOAT2((float)x,(float)y));
}
