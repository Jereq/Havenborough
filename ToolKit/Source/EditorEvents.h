#pragma once

#include <IEventData.h>

class Actor;

class SelectObjectEventData : public BaseEventData
{
private: 
	std::weak_ptr<Actor> m_Actor;

public:
	static const IEventData::Type sk_EventType = Type(0xfb2599fa);

	explicit SelectObjectEventData(std::weak_ptr<Actor> p_Actor)
		: m_Actor(p_Actor)
	{
	}

	virtual const IEventData::Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new SelectObjectEventData(m_Actor));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
		p_Out << "[Actor]";
	}

	virtual const char *getName(void) const override
	{
		return "SelectObjectEvent";
	}

	std::weak_ptr<Actor> getActor(void) const
	{
		return m_Actor;
	}
};
