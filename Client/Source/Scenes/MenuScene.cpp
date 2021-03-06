#include "MenuScene.h"

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
	m_GameLogic = nullptr;
	m_ResourceManager = nullptr;
}

bool MenuScene::init(unsigned int p_SceneID, IGraphics *p_Graphics, ResourceManager *p_ResourceManager,
	Input *p_InputQueue, GameLogic *p_GameLogic, EventManager *p_EventManager)
{
	m_SceneID = p_SceneID;

	m_Graphics = p_Graphics;
	m_GameLogic = p_GameLogic;
	m_EventManager = p_EventManager;
	m_ResourceManager = p_ResourceManager;

	m_LoadScreenResourceID = m_ResourceManager->loadResource("texture", "LoadingScreen");
	m_LoadScreenGraphicsID = m_Graphics->create2D_Object(Vector3(0.f, 0.f, 0.f), Vector2(640, 360), Vector3(1.f, 1.f, 1.f), 0.f, "LoadingScreen");

	return true;
}

void MenuScene::destroy()
{
	m_Graphics->release2D_Model(m_LoadScreenGraphicsID);
	m_ResourceManager->releaseResource(m_LoadScreenResourceID);
}

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

void MenuScene::onFocus()
{
	std::shared_ptr<MouseEventDataShow> showMouse(new MouseEventDataShow(true));
	m_EventManager->queueEvent(showMouse);
	std::shared_ptr<MouseEventDataLock> lockMouse(new MouseEventDataLock(false));
	m_EventManager->queueEvent(lockMouse);
}

void MenuScene::render()
{
	m_Graphics->setClearColor(Vector4(0, 1, 0, 1));
	m_Graphics->setRenderTarget(IGraphics::RenderTarget::FINAL);
	m_Graphics->render2D_Object(m_LoadScreenGraphicsID);
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
	// On "KeyDown"
	if (p_Value >= 0.5f && p_PrevValue < 0.5f)
	{
		if(p_Action == "changeSceneN")
		{
			if(m_SceneID == 0)
			{
				//m_NewSceneID = (int)MenuScenes::OPTION;
			}
			else if(m_SceneID == 1)
			{
				m_NewSceneID = (int)MenuScenes::MAIN; 
			}
			m_ChangeScene = true;
		}
		else if(p_Action == "changeSceneP")
		{
			m_ChangeList = true;
		}
		else if (p_Action == "leaveGame")
		{
			m_EventManager->queueEvent(IEventData::Ptr(new QuitGameEventData));
		}
	}
}

/*########## TEST FUNCTIONS ##########*/

int MenuScene::getID()
{
	return m_SceneID;
}
