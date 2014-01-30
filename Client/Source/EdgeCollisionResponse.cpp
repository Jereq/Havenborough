#include "EdgeCollisionResponse.h"
#include "ClientExceptions.h"

using namespace DirectX;

EdgeCollisionResponse::EdgeCollisionResponse(void)
{
}

EdgeCollisionResponse::~EdgeCollisionResponse(void)
{
}

bool EdgeCollisionResponse::checkCollision(HitData &p_Hit, Vector3 p_EdgePosition, float p_EdgeSizeY, Player *p_Player)
{
	if(!p_Player->getForceMove() && p_Hit.isEdge && p_Hit.collider == p_Player->getBody())
	{
		XMFLOAT3 collisionNormal = Vector4ToXMFLOAT3(&p_Hit.colNorm);
		handleCollision(p_Player, p_EdgePosition, XMLoadFloat3(&collisionNormal),
			p_EdgeSizeY);
		return true;
	}
	return false;
}

void EdgeCollisionResponse::handleCollision(Player *p_Player, Vector3 p_EdgePosition, XMVECTOR p_VictimNormal,
	float p_EdgeOffsetY)
{
	XMFLOAT3 playerPos = p_Player->getCollisionCenter();
	XMFLOAT3 playerOrigPos = p_Player->getPosition();
	XMVECTOR playerStartPos = XMLoadFloat3(&playerPos);
	XMFLOAT3 collisionBodyPos = p_EdgePosition;

	XMFLOAT3 collisionPosition = XMFLOAT3(collisionBodyPos.x, collisionBodyPos.y, collisionBodyPos.z);
	XMVECTOR boundingVolumeCenter = XMLoadFloat3(&collisionPosition);
	
	float playerPositionY = playerPos.y;

	if(playerPositionY < collisionBodyPos.y)
	{
		p_VictimNormal.m128_f32[1] = 0.f;
		p_VictimNormal = XMVector3Normalize(p_VictimNormal);
		
		XMVECTOR playerEndPos =  calculateEndPosition(p_VictimNormal, boundingVolumeCenter - playerStartPos,
			boundingVolumeCenter, p_EdgeOffsetY, 0.f);

		XMVECTOR playerOrigPosV = XMLoadFloat3(&playerOrigPos);
		XMFLOAT3 playerEndPosV;
		XMStoreFloat3(&playerEndPosV, playerEndPos);

		p_Player->forceMove(playerOrigPos, playerEndPosV);
	}
}

XMVECTOR EdgeCollisionResponse::calculateEndPosition(XMVECTOR p_Normal, XMVECTOR p_PlayerToCenter,
	XMVECTOR p_BodyCenter, float p_EdgeOffsetY, float p_HeightOffset)
{
	XMFLOAT3 up = XMFLOAT3(0,1,0);
	XMVECTOR upV = XMLoadFloat3(&up);

	XMVECTOR collisionTangent = XMVector3Cross(upV, p_Normal);
	XMVECTOR projectedPosition = p_BodyCenter + XMVector3Dot(-p_PlayerToCenter, collisionTangent) * collisionTangent;

	projectedPosition.m128_f32[1] += p_EdgeOffsetY + p_HeightOffset;

	return projectedPosition;
}