#include "Level.h"

Level::Level(IGraphics* p_Graphics, ResourceManager* p_Resources, IPhysics* p_Physics)
{
	m_Graphics = p_Graphics;
	m_Resources = p_Resources;
	m_Physics = p_Physics;
}

Level::~Level()
{
}

void Level::releaseLevel()
{
	for(int i : m_ResourceID)
	{
		m_Resources->releaseResource(i);
	}
	for(int j : m_BVResourceID)
	{
		m_Resources->releaseResource(j);
	}
	m_ResourceID.clear();
	m_ResourceID.shrink_to_fit();
	m_BVResourceID.clear();
	m_BVResourceID.shrink_to_fit();
	m_LevelData.clear();
	m_LevelData.shrink_to_fit();
	m_LevelCollisionData.clear();
	m_LevelCollisionData.shrink_to_fit();
	m_Graphics = nullptr;
	m_Resources = nullptr;
}
bool Level::loadLevel(std::string p_LevelFilePath, std::string p_CollisionFilePath)
{
	if(!m_LevelLoader.loadBinaryFile(p_LevelFilePath))
	{
		return false;
	}
	m_Graphics->createShader("DefaultShader", L"../../Graphics/Source/DeferredShaders/GeometryPass.hlsl",
							"VS,PS","5_0", ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);
	m_LevelData = m_LevelLoader.getModelData();
	for(unsigned int i = 0; i < m_LevelData.size(); i++)
	{
		m_ResourceID.push_back(m_Resources->loadResource("model", m_LevelData.at(i).m_MeshName));
		m_Graphics->linkShaderToModel("DefaultShader", m_LevelData.at(i).m_MeshName.c_str());
		for(unsigned int j = 0; j < m_LevelData.at(i).m_Translation.size(); j++)
		{
			m_DrawID.push_back(m_Graphics->createModelInstance(m_LevelData.at(i).m_MeshName.c_str()));
			DirectX::XMFLOAT3 translation, rotation, scale;
			translation = m_LevelData.at(i).m_Translation.at(j);
			rotation = m_LevelData.at(i).m_Rotation.at(j);
			scale = m_LevelData.at(i).m_Scale.at(j);
			m_Graphics->setModelPosition(m_DrawID.back(), Vector3(translation.x, translation.y, translation.z));
			m_Graphics->setModelRotation(m_DrawID.back(), Vector3(rotation.x, rotation.y, rotation.z));
			m_Graphics->setModelScale(m_DrawID.back(), Vector3(scale.x, scale.y, scale.z));
		}
	}
	//This will be implemented at a later stage when physics has what it takes!
	if(!m_CollisionLoader.loadBinaryFile(p_CollisionFilePath))
	{
		return false;
	}
	m_LevelCollisionData = m_CollisionLoader.getModelData();
	for(unsigned int i = 0; i < m_LevelCollisionData.size(); i++)
	{
		m_BVResourceID.push_back(m_Resources->loadResource("volume", m_LevelCollisionData.at(i).m_MeshName));

		//for(unsigned int j = 0; m_LevelCollisionData.at(i).m_Translation.size(); j++)
		//{
		//	//m_DrawID.push_back(m_Graphic->createModelInstance(m_LevelData.at(i).m_MeshName.c_str()));
		//	
		//	DirectX::XMFLOAT3 translation, rotation, scale;
		//	translation = m_LevelData.at(i).m_Translation.at(j);
		//	rotation = m_LevelData.at(i).m_Rotation.at(j);
		//	scale = m_LevelData.at(i).m_Scale.at(j);
		//	m_Physics->setBVPosition(m_DrawID.back(), translation.z, translation.y, translation.x);
		//	m_Physics->setBVRotation(m_DrawID.back(), rotation.z, rotation.y, rotation.x);
		//	m_Physics->setBVScale(m_DrawID.back(), scale.z, scale.y, scale.x);
		//}
	}


	return true;
}

void Level::drawLevel()
{
	for(unsigned int i = 0; i < m_DrawID.size(); i++)
	{
		m_Graphics->renderModel(m_DrawID.at(i));
	}
}