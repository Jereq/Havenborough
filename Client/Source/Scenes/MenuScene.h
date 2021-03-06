#pragma once
#include "IScene.h"

class MenuScene : public IScene
{
private:
	int m_SceneID;
	bool m_Visible;
	int  m_NewSceneID;
	bool m_ChangeScene;
	bool m_ChangeList;
	int m_LoadScreenResourceID;
	IGraphics::Object2D_Id m_LoadScreenGraphicsID;

	IGraphics *m_Graphics;
	GameLogic *m_GameLogic;
	EventManager *m_EventManager;
	ResourceManager *m_ResourceManager;

public: 
	MenuScene();
	~MenuScene();

	bool init(unsigned int p_SceneID, IGraphics *p_Graphics, ResourceManager *p_ResourceManager,
		Input *p_InputQueue, GameLogic *p_GameLogic, EventManager *p_EventManager) override;

	void destroy() override;

	void onFrame(float p_DeltaTime, int* p_IsCurrentScene) override;
	void onFocus() override;

	void render() override;
		 
	bool getIsVisible() override;
		 
	void setIsVisible(bool p_SetVisible) override;
		 
	void registeredInput(std::string p_Action, float p_Value, float p_PrevValue) override;
	
	/*########## TEST FUNCTIONS ##########*/

	int	getID() override;
};