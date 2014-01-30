#include "CheckpointSystem.h"
#include "Components.h"

using std::vector;

CheckpointSystem::CheckpointSystem(Vector3 p_DefaultTone, Vector3 p_CurrentTone, Vector3 p_FinishTone) :
	m_DefaultColorTone(p_DefaultTone),
	m_CurrentColorTone(p_CurrentTone),
	m_FinishColorTone(p_FinishTone)
{
}

CheckpointSystem::~CheckpointSystem(void)
{
}

void CheckpointSystem::addCheckpoint(const std::weak_ptr<Actor> p_Checkpoint)
{
	std::weak_ptr<ModelInterface> mI = p_Checkpoint.lock()->getComponent<ModelInterface>(ModelInterface::m_ComponentId);

	if(m_Checkpoints.empty())
	{
		mI.lock()->setColorTone(m_FinishColorTone);
	}
	else if(m_Checkpoints.size() > 1)
	{
		mI.lock()->setColorTone(m_CurrentColorTone);
		mI = m_Checkpoints.back().lock()->getComponent<ModelInterface>(ModelInterface::m_ComponentId);
		mI.lock()->setColorTone(m_DefaultColorTone);
	}

	m_Checkpoints.push_back(p_Checkpoint);
}

BodyHandle CheckpointSystem::getCurrentCheckpointBodyHandle(void)
{
	return m_Checkpoints.back().lock()->getBodyHandles().back();
}

bool CheckpointSystem::reachedFinishLine(void)
{
	return m_Checkpoints.empty();
}

Actor::ptr CheckpointSystem::getCurrentCheckpoint(void)
{
	if(m_Checkpoints.empty())
	{
		return Actor::ptr();
	}
	return m_Checkpoints.back().lock();
}

void CheckpointSystem::changeCheckpoint()
{
	m_Checkpoints.pop_back();
}	

Vector3 CheckpointSystem::getCurrentCheckpointColor()
{
	if(m_Checkpoints.size() > 1)
	{
		return m_CurrentColorTone;	
	}
	else
	{
		return Vector3(0.2f, 0.9f, 0.2f);
	}
}