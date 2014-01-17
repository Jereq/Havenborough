#include "SceneManager.h"
#include "ClientExceptions.h"

SceneManager::SceneManager()
{
	//m_MenuSceneList.clear();
	//m_RunSceneList.clear();
	m_NowShowing = 0;
	m_IsMenuState = true;
	m_Graphics = nullptr;
	m_ResourceManager = nullptr;
	m_InputQueue = nullptr;
}

SceneManager::~SceneManager()
{
	m_Graphics = nullptr;
	m_ResourceManager = nullptr;
	m_InputQueue = nullptr;
}

void SceneManager::init(IGraphics *p_Graphics, ResourceManager *p_ResourceManager,
	Input *p_InputQueue, GameLogic *p_GameLogic, EventManager *p_EventManager, ISound *p_Sound)
{
	m_Graphics = p_Graphics;
	m_ResourceManager = p_ResourceManager;
	m_InputQueue = p_InputQueue;
	m_GameLogic = p_GameLogic;
	m_Sound	= p_Sound;

	m_RunGame = false;

	m_MenuSceneList.resize(2);
	m_RunSceneList.resize(3);

	m_MenuSceneList[0] = IScene::ptr(new MenuScene);
	m_MenuSceneList[1] = IScene::ptr(new MenuScene);

	m_RunSceneList[0] = IScene::ptr(new GameScene);
	m_RunSceneList[1] = IScene::ptr(new MenuScene);
	m_RunSceneList[2] = IScene::ptr(new PostGameScene);

	m_NumberOfMenuScene = m_MenuSceneList.size();
	m_NumberOfRunScene = m_RunSceneList.size();

	bool sceneFail = false;
	unsigned int i;
	for(i = 0; i < m_NumberOfMenuScene; i++)
	{
		if(!m_MenuSceneList[i]->init(i, m_Graphics, m_ResourceManager, m_InputQueue, m_GameLogic, p_EventManager))
		{
			sceneFail = true;
		}
	}
	for(i = 0; i < m_NumberOfRunScene; i++)
	{
		if(!m_RunSceneList[i]->init(i, m_Graphics, m_ResourceManager, m_InputQueue, m_GameLogic, p_EventManager))
		{
			sceneFail = true;
		}
	}
	m_MenuSceneList[0]->setIsVisible(true);

	//((GameScene*)m_RunSceneList.at(0).get())->setGameLogic(m_GameLogic);


	if(sceneFail)
	{
		throw SceneManagerException("Failed to init all scenes", __LINE__,__FILE__);
	}
	//// ################################ Remove this override later, used for skipping the menu!! #####################################################
	//m_IsMenuState = false;

	//// ################################ Sound is being loaded here for demo 4 only!! Should be removed after! #####################################################
	m_SoundId = 0;
	m_SoundId = m_ResourceManager->loadResource("sound", "Background");
}

void SceneManager::destroy()
{
	unsigned int i;
	m_ResourceManager->releaseResource(m_SoundId);
	for(i = 0; i < m_NumberOfMenuScene; i++)
	{
		m_MenuSceneList[i]->destroy();
	}
	for(i = 0; i < m_NumberOfRunScene; i++)
	{
		m_RunSceneList[i]->destroy();
	}
	m_MenuSceneList.clear();
	m_RunSceneList.clear();
}

void SceneManager::onFrame( float p_DeltaTime )
{
	std::vector<IScene::ptr>* activeList = nullptr;
	int nrScenes = 0;

	if(m_IsMenuState)
	{
		activeList = &m_MenuSceneList;
		nrScenes = m_NumberOfMenuScene;
	}
	else
	{
		activeList = &m_RunSceneList;
		nrScenes = m_NumberOfRunScene;
	}

	for(int i = 0; i < nrScenes; i++)
	{
		if(activeList->at(i)->getIsVisible())
		{
			activeList->at(i)->onFrame(p_DeltaTime,&m_NowShowing);
			if(i != m_NowShowing)
			{
				i = nrScenes;
			}
		}
	}

	if(m_NowShowing != -1)
	{
		changeScene(p_DeltaTime, m_NowShowing);
	}
	else
	{
		if(m_IsMenuState)
		{
			startRun();
		}
		else
		{
			startMenu();
		}
	}
	activeList = nullptr;
}

void SceneManager::render()
{
	std::vector<IScene::ptr>* activeList = nullptr;
	unsigned int nrScenes = 0;

	if(m_IsMenuState)
	{
		activeList = &m_MenuSceneList;
		nrScenes = m_NumberOfMenuScene;
	}
	else
	{
		activeList = &m_RunSceneList;
		nrScenes = m_NumberOfRunScene;
	}

	for(unsigned int i = 0; i < nrScenes; i++)
	{
		if(activeList->at(i)->getIsVisible())
		{
			activeList->at(i)->render();
		}
	}
	activeList = nullptr;
}

void SceneManager::setPause()
{
	if(!m_IsMenuState)
	{
		bool currentState;
		currentState = m_RunSceneList[(int)RunScenes::GAMEPAUSE]->getIsVisible();
		m_RunSceneList[(int)RunScenes::GAMEPAUSE]->setIsVisible(!currentState);
	}
}

void SceneManager::changeScene(float p_DeltaTime, int p_NowShowing)
{
	if(m_IsMenuState)
	{
		m_MenuSceneList[p_NowShowing]->setIsVisible(true);
		m_MenuSceneList[p_NowShowing]->onFrame(p_DeltaTime,&m_NowShowing);
	}
	else
	{
		m_RunSceneList[p_NowShowing]->setIsVisible(true);
	}
}

void SceneManager::startRun()
{
	m_IsMenuState = false;
	m_RunSceneList[0]->setIsVisible(true);
	for(unsigned int i = 1; i < m_NumberOfRunScene; i++)
	{
		m_RunSceneList[i]->setIsVisible(false);
	}
	m_NowShowing = 0;
	//// ################################ Sound is being played here for demo 4 only!! Should be removed after! #####################################################
	m_Sound->playSound("Background");
	m_Sound->setSoundVolume("Background", 0.5f);
}

void SceneManager::startMenu()
{
	m_IsMenuState = true;
	m_MenuSceneList[0]->setIsVisible(true);
	for(unsigned int i = 1; i < m_NumberOfMenuScene; i++)
	{
		m_MenuSceneList[i]->setIsVisible(false);
	}
	m_NowShowing = 0;
	//// ################################ Sound is being played here for demo 4 only!! Should be removed after! #####################################################
	m_Sound->stopSound("Background");
}

void SceneManager::registeredInput(std::string p_Action, float p_Value, float p_PrevValue)
{
	if(p_Action == "pauseScene" && p_Value == 1)
	{
		setPause();
	}
	//Change scene
	else// if((p_Action == "changeSceneN"  && p_Value == 1) || (p_Action == "changeSceneP" && p_Value == 1))
	{
		passInput(p_Action, p_Value, p_PrevValue);
	}
}

void SceneManager::passInput(std::string p_Action, float p_Value, float p_PrevValue)
{
	std::vector<IScene::ptr>* activeList = nullptr;
	unsigned int nrScenes = 0;

	if(m_IsMenuState)
	{
		activeList = &m_MenuSceneList;
		nrScenes = m_NumberOfMenuScene;
	}
	else
	{
		activeList = &m_RunSceneList;
		nrScenes = m_NumberOfRunScene;
	}

	for(unsigned int i = 0; i < nrScenes; i++)
	{
		if(activeList->at(i)->getIsVisible())
		{
			activeList->at(i)->registeredInput(p_Action, p_Value, p_PrevValue);
			i = nrScenes;
		}
	}
	activeList = nullptr;
}

/*########## TEST FUNCTIONS ##########*/

std::vector<IScene::ptr> SceneManager::getScene()
{
	std::vector<IScene::ptr> temp;
	if(m_IsMenuState)
	{
		for(unsigned int i = 0; i < m_NumberOfMenuScene; i++)
		{
			if(m_MenuSceneList[i]->getIsVisible() == true)
			{
				temp.push_back(m_MenuSceneList[i]);
			}
		}
	}
	else
	{
		for(unsigned int i = 0; i < m_NumberOfRunScene; i++)
		{
			if(m_RunSceneList[i]->getIsVisible() == true)
			{
				temp.push_back(m_RunSceneList[i]);
			}
		}
	}
	return temp;
}