#include "EventManager.h"
#include "EventData.h"
#include <DirectXMath.h>

class Tool
{
protected:
	EventManager *m_EventManager;
public:
	Tool() :
		m_EventManager(nullptr)
	{}
	virtual ~Tool(){}
	virtual void initialize(EventManager *p_EventManager) = 0;
	virtual void action() = 0;
};

class SelectTool : public Tool
{
private:
	DirectX::XMFLOAT2 m_MousePos;
	DirectX::XMFLOAT2 m_WindowSize;
public:
	SelectTool(){}
	~SelectTool(){}
	void initialize(EventManager *p_EventManager) override
	{
		m_EventManager = p_EventManager;
	}
	void action() override
	{
		m_EventManager->triggerTriggerEvent(IEventData::Ptr(new CreateRayEventData(DirectX::XMFLOAT2(m_MousePos.x, m_MousePos.y),
																					DirectX::XMFLOAT2(m_WindowSize.x, m_WindowSize.y))));
	}

	//replace these functions with events? event overlap with createRayEventData....
	//Needs to be updated through events?
	//Where should they be updated?
	void updateScreenSize(const DirectX::XMFLOAT2 &p_WindowsSize)
	{
		m_WindowSize = p_WindowsSize;
	}
	void updateMousePos(const DirectX::XMFLOAT2 &p_MousePos)
	{
		m_MousePos = p_MousePos;
	}
};