#include "MenuScene.h"

#include "../EventData.h"

MenuScene::MenuScene()
{
	m_SceneID = 0;
	m_Visible = false;
	m_NewSceneID = 0;
	m_ChangeScene = false;
	m_ChangeList = false;

	m_Graphics = nullptr;
}

MenuScene::~MenuScene()
{
	m_Graphics = nullptr;
	m_EventManager = nullptr;
}

bool MenuScene::init(unsigned int p_SceneID, IGraphics *p_Graphics, ResourceManager *p_ResourceManager,
	Input *p_InputQueue, GameLogic *p_GameLogic, EventManager *p_EventManager)
{
	m_SceneID = p_SceneID;

	m_Graphics = p_Graphics;
	m_GameLogic = p_GameLogic;
	m_EventManager = p_EventManager;

	return true;
}

void MenuScene::destroy(){}

void MenuScene::onFrame(float p_Dt, int* p_IsCurrentScene)
{
	if(m_ChangeScene)
	{
		*p_IsCurrentScene = m_NewSceneID;
		m_Visible = false;
		m_ChangeScene = false;
	}
	else if(m_ChangeList)
	{
		*p_IsCurrentScene = -1;
		m_ChangeList = false;
	}

	
}

void MenuScene::render()
{
	m_Graphics->setClearColor(Vector4(0, 1, 0, 1));
	m_Graphics->setRenderTarget(-1);
}

bool MenuScene::getIsVisible()
{
	return m_Visible;
}

void MenuScene::setIsVisible(bool p_SetVisible)
{
	m_ChangeList = false;

	m_Visible = p_SetVisible;
}

void MenuScene::registeredInput(std::string p_Action, float p_Value, float p_PrevValue)
{
	if(p_Action == "changeSceneN" && p_Value == 1 && p_PrevValue == 0)
	{
		if(m_SceneID == 0)
		{
			m_NewSceneID = (int)MenuScenes::OPTION;
		}
		else if(m_SceneID == 1)
		{
			m_NewSceneID = (int)MenuScenes::MAIN; 
		}
		m_ChangeScene = true;
	}
	else if(p_Action == "changeSceneP" && p_Value == 1 && p_PrevValue == 0)
	{
		m_ChangeList = true;
	}
	else if (p_Action == "joinTestLevel" && p_Value == 1.f)
	{
		m_GameLogic->joinGame("test");
	}
	else if (p_Action == "playLocalTest" && p_Value == 1.f)
	{
		m_GameLogic->playLocalLevel();
	}
	else if (p_Action == "connectToServer" && p_Value == 1.f)
	{
		m_GameLogic->connectToServer("localhost", 31415); //Note: IP to server if running: 194.47.150.5
	}
}

/*########## TEST FUNCTIONS ##########*/

int MenuScene::getID()
{
	return m_SceneID;
}
