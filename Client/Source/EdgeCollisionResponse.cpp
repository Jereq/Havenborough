#include "EdgeCollisionResponse.h"
#include "ClientExceptions.h"

EdgeCollisionResponse::EdgeCollisionResponse(void)
{
}

EdgeCollisionResponse::~EdgeCollisionResponse(void)
{
}

bool EdgeCollisionResponse::checkCollision(HitData &p_Hit, Vector4 p_EdgePosition, float p_EdgeSizeY, Player *p_Player)
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

void EdgeCollisionResponse::handleCollision(Player *p_Player, Vector4 p_EdgePosition, XMVECTOR p_VictimNormal,
	float p_EdgeOffsetY)
{
	XMFLOAT3 playerPos = p_Player->getGroundPosition();
	XMVECTOR playerStartPos = XMLoadFloat3(&playerPos);
	XMFLOAT4 collisionBodyPos = Vector4ToXMFLOAT4(&p_EdgePosition);

	XMFLOAT3 collisionPosition = XMFLOAT3(collisionBodyPos.x, collisionBodyPos.y, collisionBodyPos.z);
	XMVECTOR boundingVolumeCenter = XMLoadFloat3(&collisionPosition);
	
	float playerPositionY = playerPos.y;

	if(playerPositionY < collisionBodyPos.y)
	{
		p_VictimNormal = XMVector3Normalize(p_VictimNormal);
		
		XMVECTOR playerEndPos =  calculateEndPosition(p_VictimNormal, boundingVolumeCenter - playerStartPos,
			boundingVolumeCenter, p_EdgeOffsetY, p_Player->getHeight() * 0.5f);

		p_Player->forceMove(playerStartPos, playerEndPos);
	}
}

XMVECTOR EdgeCollisionResponse::calculateEndPosition(XMVECTOR p_Normal, XMVECTOR p_PlayerToCenter,
	XMVECTOR p_BodyCenter, float p_EdgeOffsetY, float p_HeightOffset)
{
	XMFLOAT3 up = XMFLOAT3(0,1,0);
	XMVECTOR upV = XMLoadFloat3(&up);

	XMVECTOR playerToCenterLength = XMVector3Length(p_PlayerToCenter);
	p_PlayerToCenter = XMVector3Normalize(p_PlayerToCenter);
	p_PlayerToCenter = XMVector3Reflect(p_PlayerToCenter, p_Normal);
	p_PlayerToCenter = XMVector3Reflect(-p_PlayerToCenter, upV);

	XMVECTOR playerFinal = p_BodyCenter + (playerToCenterLength * 1.1f) * p_PlayerToCenter;
	XMFLOAT3 offset = XMFLOAT3(0.0f, p_EdgeOffsetY + p_HeightOffset, 0.0f);
	XMVECTOR offsetV = XMLoadFloat3(&offset);
	
	return playerFinal + offsetV;
}