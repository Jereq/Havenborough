#pragma once

#include "Joint.h"
#include "LoaderStructs.h"
#include "WrapperFactory.h"

#include <memory>
#include <vector>

struct ModelDefinition
{
	std::unique_ptr<Buffer> vertexBuffer;
	std::vector<std::pair<int,int>> drawInterval;
	Shader						*shader;
	std::vector<std::pair< std::string, ID3D11ShaderResourceView*>> diffuseTexture;
	std::vector<std::pair< std::string, ID3D11ShaderResourceView*>> normalTexture;
	std::vector<std::pair< std::string, ID3D11ShaderResourceView*>> specularTexture;
	unsigned int				numOfMaterials;
	bool						m_IsAnimated;

	std::vector<Joint>					m_Joints;

	ModelDefinition(ModelDefinition&& p_Other)
		:	vertexBuffer(std::move(p_Other.vertexBuffer)),
			drawInterval(p_Other.drawInterval),
			shader(p_Other.shader),
			diffuseTexture(p_Other.diffuseTexture),
			normalTexture(p_Other.normalTexture),
			specularTexture(p_Other.specularTexture),
			numOfMaterials(p_Other.numOfMaterials),
			m_IsAnimated(p_Other.m_IsAnimated),
			m_Joints(std::move(p_Other.m_Joints))
	{}

	ModelDefinition& operator=(ModelDefinition&& p_Other)
	{
		std::swap(vertexBuffer, p_Other.vertexBuffer);
		std::swap(drawInterval, p_Other.drawInterval);
		std::swap(shader, p_Other.shader);
		std::swap(diffuseTexture, p_Other.diffuseTexture);
		std::swap(normalTexture, p_Other.normalTexture);
		std::swap(specularTexture, p_Other.specularTexture);
		std::swap(numOfMaterials, p_Other.numOfMaterials);
		std::swap(m_IsAnimated, p_Other.m_IsAnimated);
		std::swap(m_Joints, p_Other.m_Joints);

		return *this;
	}

private:
	ModelDefinition(const ModelDefinition&);

public:
	ModelDefinition(){}
	~ModelDefinition(){}
};