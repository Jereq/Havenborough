#pragma once
#include "IPhysics.h"
#include "../Utilities/Util.h"


class Player
{
private:
	Vector3 m_Position;
	Vector3 m_LookDirection;
	float m_ViewRotation[2];
	BodyHandle m_PlayerBody;
	IPhysics* m_Physics;
	float m_JumpTime;
	bool m_IsJumping;
	float m_JumpForceTime;
	float m_JumpForce;
	float maxSpeed;
	float accConstant;
	Vector4 m_PrevForce;
	float m_DirectionX;
	float m_DirectionZ;

public:
	Player(void);
	~Player(void);
	void initialize(IPhysics *p_Physics, Vector3 p_Position, Vector3 p_LookDirection);

	
	void update(float dt);


	BodyHandle getBody() const;
	void setJump();
	void setDirectionX(float p_DirectionX);
	void setDirectionZ(float p_DirectionZ);
private:
	void jump(float dt);
	void move();
};

