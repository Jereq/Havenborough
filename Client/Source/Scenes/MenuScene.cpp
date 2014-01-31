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
}

bool MenuScene::init(unsigned int p_SceneID, IGraphics *p_Graphics, ResourceManager *p_ResourceManager,
	Input *p_InputQueue, GameLogic *p_GameLogic, EventManager *p_EventManager)
{
	m_SceneID = p_SceneID;

	m_Graphics = p_Graphics;
	m_GameLogic = p_GameLogic;
	m_EventManager = p_EventManager;

	m_ServerAddress = "localhost";
	m_ServerPort = 31415;

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError res = doc.LoadFile("UserOptions.xml");
	if (res == tinyxml2::XML_NO_ERROR)
	{
		tinyxml2::XMLElement* options = doc.FirstChildElement("UserOptions");
		if (options)
		{
			tinyxml2::XMLElement* server = options->FirstChildElement("Server");
			if (server)
			{
				const char* address = server->Attribute("Hostname");
				if (address)
				{
					m_ServerAddress = address;
				}

				unsigned int tPort = m_ServerPort;
				server->QueryAttribute("Port", &tPort);
#undef max
				if (tPort <= std::numeric_limits<uint16_t>::max())
				{
					m_ServerPort = tPort;
				}
			}
		}
	}

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
	// On "KeyDown"
	if (p_Value >= 0.5f && p_PrevValue < 0.5f)
	{
		if(p_Action == "changeSceneN")
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
		else if(p_Action == "changeSceneP")
		{
			m_ChangeList = true;
		}
		else if (p_Action == "joinTestLevel")
		{
			m_GameLogic->joinGame("test");
		}
		else if (p_Action == "joinServerLevel")
		{
#ifdef _DEBUG
			m_GameLogic->joinGame("serverDebugLevel");
#else
			m_GameLogic->joinGame("serverLevel");
#endif
		}
		else if (p_Action == "playLocalTest")
		{
			m_GameLogic->playLocalLevel();
		}
		else if (p_Action == "connectToServer")
		{
			m_GameLogic->connectToServer(m_ServerAddress, m_ServerPort);
		}
		else if (p_Action == "back")
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
