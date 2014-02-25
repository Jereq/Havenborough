#pragma once

#include "Animation.h"
#include "AnimationLoader.h"
#include "ActorComponent.h"
#include "Components.h"
#include "EventManager.h"
#include "IPhysics.h"

class HumanAnimationComponent : public AnimationInterface
{
private:
	enum class ForwardAnimationState
	{
		IDLE,
		WALKING_FORWARD,
		RUNNING_FORWARD,
		WALKING_BACKWARD,
		RUNNING_BACKWARD,
	};

	enum class SideAnimationState
	{
		IDLE,
		WALKING_LEFT,
		RUNNING_LEFT,
		WALKING_RIGHT,
		RUNNING_RIGHT,
	};

	enum class JumpAnimationState
	{
		IDLE,
		JUMP,
		FLYING,
		FALLING,
		LIGHT_LANDING,
		HARD_LANDING,
	};

	enum class ForceMoveState
	{
		IDLE,
		CLIMB1,
		CLIMB2,
		CLIMB3,
		CLIMB4,
	};

	Animation m_Animation;
	ForwardAnimationState m_PrevForwardState;
	SideAnimationState m_PrevSideState;
	JumpAnimationState m_PrevJumpState;
	ForceMoveState m_ForceMoveState;
	float m_FallSpeed;
	bool m_ForceMove;

	std::weak_ptr<ModelComponent> m_Model;
	EventManager* m_EventManager;
	ResourceManager* m_ResourceManager;
	std::string m_AnimationName;
	int m_AnimationResource;
	AnimationLoader* m_AnimationLoader;

	DirectX::XMFLOAT3 m_CenterReachPos;
	DirectX::XMFLOAT3 m_EdgeOrientation;
	IPhysics		*m_Physics;
	std::string		m_ClimbId;

public:
	~HumanAnimationComponent()
	{
		m_ResourceManager->releaseResource(m_AnimationResource);
	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		m_FallSpeed = 0.f;
		m_PrevForwardState = ForwardAnimationState::IDLE;
		m_PrevSideState = SideAnimationState::IDLE;
		m_PrevJumpState = JumpAnimationState::IDLE;
		m_ForceMoveState = ForceMoveState::IDLE;
		m_ForceMove = false;

		const char* resourceName = p_Data->Attribute("Animation");
		if (!resourceName)
		{
			throw CommonException("Human animation component must have animation", __LINE__, __FILE__);
		}

		m_AnimationName = resourceName;
		m_AnimationResource = m_ResourceManager->loadResource("animation", m_AnimationName);
		m_Animation.setAnimationData(m_AnimationLoader->getAnimationData(m_AnimationName.c_str()));
	}

	void setPhysics(IPhysics *p_Physics) override
	{
		m_Physics = p_Physics;
	}

	void postInit() override
	{
		m_Model = m_Owner->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
		m_EventManager = m_Owner->getEventManager();
		playAnimation( "Idle", false );
	}

	void onUpdate(float p_DeltaTime) override
	{
		updateAnimation();
		m_Animation.updateAnimation(p_DeltaTime);

		Vector3 left = getJointPos("L_Ankle");
		left.y = left.y + 5.f;
		m_Physics->setBodyVolumePosition(m_Owner->getBodyHandles()[0], 2, left);
			
		Vector3 right = getJointPos("R_Ankle");
		right.y = right.y + 5.f;
		m_Physics->setBodyVolumePosition(m_Owner->getBodyHandles()[0], 3, right);

		Vector3 eye = getJointPos("Head");
		m_Physics->setBodyVolumePosition(m_Owner->getBodyHandles()[0], 4, eye);

		updateIKJoints();

		std::shared_ptr<ModelComponent> comp = m_Model.lock();
		if (comp)
		{
			m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateAnimationEventData(comp->getId(), m_Animation.getFinalTransform(), m_Animation.getAnimationData(), m_Owner->getWorldMatrix())));
		}
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("HumanAnimation");
		p_Printer.PushAttribute("Animation", m_AnimationName.c_str());
		p_Printer.CloseElement();
	}
	/**
	 * Set the resource manager for the component.
	 *
	 * @param p_ResourceManager the resource manager to use
	 */
	void setResourceManager(ResourceManager* p_ResourceManager)
	{
		m_ResourceManager = p_ResourceManager;
	}

	void setAnimationLoader(AnimationLoader* p_AnimationLoader)
	{
		m_AnimationLoader = p_AnimationLoader;
	}

	void updateAnimation();

	void playAnimation(std::string p_AnimationName, bool p_Override) override
	{
		m_Animation.playClip(p_AnimationName, p_Override);
	}

	void queueAnimation(std::string p_AnimationName) override
	{
		m_Animation.queueClip(p_AnimationName);
	}

	void changeAnimationWeight(int p_Track, float p_Weight) override
	{
		m_Animation.changeWeight(p_Track, p_Weight);
	}

	void applyIK_ReachPoint(const std::string& p_GroupName, Vector3 p_Target) override
	{
		m_Animation.applyIK_ReachPoint(p_GroupName, p_Target, m_Owner->getWorldMatrix());
		
		std::shared_ptr<ModelComponent> comp = m_Model.lock();
		if (comp)
		{
			m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateAnimationEventData(comp->getId(), m_Animation.getFinalTransform(), m_Animation.getAnimationData(), m_Owner->getWorldMatrix())));
		}
	}

	DirectX::XMFLOAT3 getJointPos(const std::string& p_JointName) override
	{
		return m_Animation.getJointPos(p_JointName, m_Owner->getWorldMatrix());
	}

	const AnimationPath getAnimationData(std::string p_AnimationId) const override
	{
		return m_Animation.getAnimationData().get()->animationPath[p_AnimationId];
	}

	void playClimbAnimation(std::string p_ClimbID) override
	{
		playAnimation(p_ClimbID, true);
		m_ForceMove = true;
		m_ClimbId = p_ClimbID;
	}

	void resetClimbState() override
	{
		m_ForceMove = false;
	}

	void updateIKData(Vector3 p_EdgeOrientation, Vector3 p_CenterReachPos) override
	{
		m_EdgeOrientation = p_EdgeOrientation;
		m_CenterReachPos = p_CenterReachPos;
	}

	void updateIKJoints();
};