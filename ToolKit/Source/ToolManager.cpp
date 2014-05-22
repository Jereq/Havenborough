#include "ToolManager.h"

ToolManager::ToolManager() :
	m_EventManager(nullptr), m_CurrentTool(nullptr),
	m_PreviusTool(nullptr)
{}

ToolManager::~ToolManager()
{
		
}

void ToolManager::initialize(EventManager *p_EventManager)
{
	m_EventManager = p_EventManager;

	m_ToolList.push_back(new SelectTool);

	for(auto &a : m_ToolList)
	{
		a->initialize(m_EventManager);
	}

	changeTool(Tool::Type::SELECT);
	m_PreviusTool = m_CurrentTool;
}

void ToolManager::OnPress()
{
	m_CurrentTool->onPress();
}

void ToolManager::OnMove()
{
	m_CurrentTool->onMove();
}

void ToolManager::OnRelease()
{
	m_CurrentTool->onRelease();
}

void ToolManager::changeTool(Tool::Type p_Type)
{
	for(unsigned int i = 0; i < m_ToolList.size(); i++)
	{
		if(m_ToolList[i]->getToolType() == p_Type)
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