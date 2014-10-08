#include "ModelFactory.h"
#include "GraphicsExceptions.h"
#include "ModelBinaryLoader.h"
#include "Utilities/MemoryUtil.h"
#include "..\..\Common\Source\AnimationLoader.h"

using std::string;
using std::vector;
using std::map;
using std::pair;
using namespace DirectX;

ModelFactory *ModelFactory::m_Instance = nullptr;

ModelFactory *ModelFactory::getInstance(void)
{
	if(!m_Instance)
		m_Instance = new ModelFactory();

	return m_Instance;
}

void ModelFactory::initialize(TextureMap* p_TextureList, map<string, Shader*> *p_ShaderList,
							  ResourceProxy* p_ResProxy)
{
	if(!m_Instance)
		throw ModelFactoryException("Error when initializing ModelFactory, no instance exists", __LINE__, __FILE__);

	m_TextureList = p_TextureList;
	m_ShaderList = p_ShaderList;
	m_ResProxy = p_ResProxy;
}

void ModelFactory::shutdown(void)
{
	SAFE_DELETE(m_Instance);
}

ModelDefinition ModelFactory::createModel(ResId p_Res)
{
	ModelBinaryLoader modelLoader;
	ResourceProxy::Buff buff = m_ResProxy->getData(p_Res);
	modelLoader.loadBinaryFromMemory(buff.data, buff.size);

	ModelDefinition model;
	Buffer::Description bufferDescription;
	const vector<Material> &materialData = modelLoader.getMaterial();
	const vector<MaterialBuffer> &materialBufferData = modelLoader.getMaterialBuffer();
	
	model.isAnimated = modelLoader.getAnimated();
	model.isTransparent = modelLoader.getTransparent();

	if(!model.isAnimated)
	{
		const vector<StaticVertex> &vertexData = modelLoader.getStaticVertexBuffer();
		bufferDescription = createBufferDescription(vertexData, Buffer::Usage::USAGE_IMMUTABLE); //Change to default when needed to change data.
		if(model.isTransparent)
			model.shader = m_ShaderList->at("DefaultForwardShader");
		else
			model.shader = m_ShaderList->at("DefaultDeferredShader");
	}
	else
	{
		const vector<AnimatedVertex> &vertexData = modelLoader.getAnimatedVertexBuffer();
		bufferDescription = createBufferDescription(vertexData, Buffer::Usage::USAGE_IMMUTABLE); //Change to default when needed to change data.
		model.shader = m_ShaderList->at("DefaultAnimatedShader");
	}
	std::unique_ptr<Buffer> vertexBuffer(WrapperFactory::getInstance()->createBuffer(bufferDescription));

	if (model.isAnimated)
	{
		static const char* styles[4] =
		{
			"Green",
			"Red",
			"Blue",
			"Black",
		};

		for (unsigned int styleId = 0; styleId < 4; ++styleId)
		{
			unsigned int startIndex = model.diffuseTexture.size();

			vector<ModelDefinition::Material> tempInterval(materialBufferData.size());
			for(unsigned int i = 0; i < materialBufferData.size(); i++)
			{
				tempInterval.at(i).vertexStart = materialBufferData.at(i).start;
				tempInterval.at(i).numOfVertices = materialBufferData.at(i).length;
				tempInterval.at(i).textureIndex = startIndex + i;
			}
			model.materialSets.push_back(std::make_pair(styles[styleId], tempInterval));

			loadTextures(model, materialData.size(), materialData, styles[styleId]);
		}
	}
	else
	{
		vector<ModelDefinition::Material> tempInterval(materialBufferData.size());
		for(unsigned int i = 0; i < materialBufferData.size(); i++)
		{
			tempInterval.at(i).vertexStart = materialBufferData.at(i).start;
			tempInterval.at(i).numOfVertices = materialBufferData.at(i).length;
			tempInterval.at(i).textureIndex = i;
		}
		model.materialSets.push_back(std::make_pair("default", tempInterval));

		loadTextures(model, materialData.size(), materialData, nullptr);
	}

	model.vertexBuffer.swap(vertexBuffer);
	model.boundingVolume = modelLoader.getBoundingVolume();

	modelLoader.clear();
	
	return model;
}

ModelDefinition *ModelFactory::create2D_Model(Vector2 p_HalfSize, const char *p_TextureId)
{
	ModelDefinition *model = new ModelDefinition();

	create2D_VertexBuffer(model, p_HalfSize);
	
	model->diffuseTexture.push_back(make_pair(std::string(p_TextureId), getTextureFromList(p_TextureId)));
	model->materialSets.resize(1);
	model->materialSets[0].first = "default";
	model->materialSets[0].second.push_back(ModelDefinition::Material(0, 6, 0));
	model->is2D = true;
	model->isAnimated = false;

	return model;
}

ModelDefinition *ModelFactory::create2D_Model(ID3D11ShaderResourceView *p_Texture)
{
	ModelDefinition *model = new ModelDefinition();
	ID3D11Resource *resource;
	ID3D11Texture2D *texture;
	D3D11_TEXTURE2D_DESC textureDesc;

	p_Texture->GetResource(&resource);
	resource->QueryInterface(&texture);
	texture->GetDesc(&textureDesc);
	Vector2 halfSize(textureDesc.Width * 0.5f, textureDesc.Height * 0.5f);
	SAFE_RELEASE(texture);
	SAFE_RELEASE(resource);
	create2D_VertexBuffer(model, halfSize);

	model->diffuseTexture.push_back(make_pair(std::string("Text"), p_Texture));
	model->materialSets.resize(1);
	model->materialSets[0].first = "default";
	model->materialSets[0].second.push_back(ModelDefinition::Material(0, 6, 0));
	model->is2D = true;
	model->isAnimated = false;

	return model;
}

void ModelFactory::setLoadModelTextureCallBack(loadModelTextureCallBack p_LoadModelTexture, void* p_Userdata)
{
	m_LoadModelTexture = p_LoadModelTexture;
	m_LoadModelTextureUserdata = p_Userdata;
}

ModelFactory::ModelFactory(void)
{
}

ModelFactory::~ModelFactory(void)
{
}

template<class T>
Buffer::Description ModelFactory::createBufferDescription(const vector<T> &p_VertexData, Buffer::Usage p_Usage)
{
	Buffer::Description bufferDescription;
	bufferDescription.initData = p_VertexData.data();
	bufferDescription.numOfElements = p_VertexData.size();
	bufferDescription.sizeOfElement = sizeof(T);
	bufferDescription.type = Buffer::Type::VERTEX_BUFFER;
	bufferDescription.usage = p_Usage;

	return bufferDescription;
}

void ModelFactory::create2D_VertexBuffer(ModelDefinition *p_Model, Vector2 p_HalfSize)
{
	vector<StaticVertex> initData;
	StaticVertex vertex;
	vertex.m_Normal = vertex.m_Binormal = vertex.m_Tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);

	vertex.m_Position = XMFLOAT4(-p_HalfSize.x, p_HalfSize.y, 0.0f, 1.0f);
	vertex.m_UV = XMFLOAT2(0.0f, 0.0f);
	initData.push_back(vertex);

	vertex.m_Position = XMFLOAT4(p_HalfSize.x, p_HalfSize.y, 0.0f, 1.0f);
	vertex.m_UV = XMFLOAT2(1.0f, 0.0f);
	initData.push_back(vertex);

	vertex.m_Position = XMFLOAT4(p_HalfSize.x, -p_HalfSize.y, 0.0f, 1.0f);
	vertex.m_UV = XMFLOAT2(1.0f, 1.0f);
	initData.push_back(vertex);

	vertex.m_Position = XMFLOAT4(p_HalfSize.x, -p_HalfSize.y, 0.0f, 1.0f);
	vertex.m_UV = XMFLOAT2(1.0f, 1.0f);
	initData.push_back(vertex);

	vertex.m_Position = XMFLOAT4(-p_HalfSize.x, -p_HalfSize.y, 0.0f, 1.0f);
	vertex.m_UV = XMFLOAT2(0.0f, 1.0f);
	initData.push_back(vertex);

	vertex.m_Position = XMFLOAT4(-p_HalfSize.x, p_HalfSize.y, 0.0f, 1.0f);
	vertex.m_UV = XMFLOAT2(0.0f, 0.0f);
	initData.push_back(vertex);

	Buffer::Description bufferDescription = createBufferDescription(initData, Buffer::Usage::USAGE_IMMUTABLE);
	std::unique_ptr<Buffer> vertexBuffer(WrapperFactory::getInstance()->createBuffer(bufferDescription));

	p_Model->vertexBuffer.swap(vertexBuffer);
}

void ModelFactory::loadTextures(ModelDefinition &p_Model, unsigned int p_NumOfMaterials,
	const vector<Material> &p_Materials, const char *p_Style)
{
	using std::pair;

	string styleOfDoom;
	if(p_Style)
		styleOfDoom = p_Style;
	for(unsigned int i = 0; i < p_NumOfMaterials; i++)
	{
		const Material &material = p_Materials.at(i);

		ResId diffId = m_ResProxy->findResourceId(("assets/textures/" + material.m_DiffuseMap).c_str());
		ResId normId = m_ResProxy->findResourceId(("assets/textures/" + material.m_NormalMap).c_str());
		ResId specId = m_ResProxy->findResourceId(("assets/textures/" + material.m_SpecularMap).c_str());
			
		m_LoadModelTexture(material.m_DiffuseMap.c_str(), diffId, m_LoadModelTextureUserdata);
		m_LoadModelTexture(material.m_NormalMap.c_str(), normId, m_LoadModelTextureUserdata);
		m_LoadModelTexture(material.m_SpecularMap.c_str(), specId, m_LoadModelTextureUserdata);

		p_Model.diffuseTexture.push_back(std::make_pair(material.m_DiffuseMap, getTextureFromList(material.m_DiffuseMap)));
		p_Model.normalTexture.push_back(std::make_pair(material.m_NormalMap, getTextureFromList(material.m_NormalMap)));
		p_Model.specularTexture.push_back(std::make_pair(material.m_SpecularMap, getTextureFromList(material.m_SpecularMap)));
	}
}

ID3D11ShaderResourceView *ModelFactory::getTextureFromList(string p_Identifier)
{
	if(m_TextureList->count(p_Identifier) > 0)
		return m_TextureList->at(p_Identifier).second;
	else
		throw GraphicsException("Texture was not found. The " + p_Identifier + " identifier does not exist.", __LINE__, __FILE__);
}