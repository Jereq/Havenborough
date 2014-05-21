#include "EventManager.h"
#include "EventData.h"
#include <DirectXMath.h>

class Tool
{
public:
	enum class Type
	{
		TRANSLATE,
		ROTATE,
		RESIZE,
		COPY,
		PASTE,
		CAMERA,
		EYE,
		SELECT,
		NONE
	};

protected:
	EventManager *m_EventManager;
	DirectX::XMFLOAT2 m_MousePos;
	DirectX::XMFLOAT2 m_WindowSize;
	Tool::Type m_Type;
public:
	Tool() :
		m_EventManager(nullptr), m_Type(Tool::Type::NONE)
	{}
	virtual ~Tool(){}
	virtual void initialize(EventManager *p_EventManager) = 0;
	virtual void onPress(){}
	virtual void onMove(){}
	virtual void onRelease(){}
	Tool::Type getToolType()
	{
		return m_Type; 
	}

	void updateScreenSize(const DirectX::XMFLOAT2 &p_WindowsSize)
	{
		m_WindowSize = p_WindowsSize;
	}
	void updateMousePos(const DirectX::XMFLOAT2 &p_MousePos)
	{
		m_MousePos = p_MousePos;
	}
};

class SelectTool : public Tool
{
private:


public:
	SelectTool(){}
	~SelectTool(){}
	void initialize(EventManager *p_EventManager) override
	{
		m_EventManager = p_EventManager;
		m_Type = Tool::Type::SELECT;
	}
	void onPress() override
	{
		m_EventManager->triggerTriggerEvent(IEventData::Ptr(new CreateRayEventData(DirectX::XMFLOAT2(m_MousePos.x, m_MousePos.y),
																					DirectX::XMFLOAT2(m_WindowSize.x, m_WindowSize.y))));
	}
};


class TranslateTool : public Tool
{
private:


public:
	TranslateTool(){}
	~TranslateTool(){}
	void initialize(EventManager *p_EventManager) override
	{
		m_EventManager = p_EventManager;
		m_Type = Tool::Type::TRANSLATE;
	}
};