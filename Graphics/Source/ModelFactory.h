#pragma once
#include "WrapperFactory.h"
#include "ModelDefinition.h"
#include "ResourceProxy.h"
#include "ShaderStructs.h"
#include "Utilities/XMFloatUtil.h"

#include <d3d11.h>
#include <map>
#include <vector>
#include <string>

class ModelFactory
{
public:
	typedef uint32_t ResId;
	typedef std::map<std::string, std::pair<ResId, ID3D11ShaderResourceView*>> TextureMap;

	/**
	* Callback for loading a texture to a model.
	* @param p_ResourceName the resource name of the texture
	* @param p_Res resource id for the data
	* @param p_UserData user defined data
	*/
	typedef void (*loadModelTextureCallBack)(const char *p_ResourceName, ResId p_Res, void *p_Userdata);

private:
	static ModelFactory *m_Instance;
	TextureMap* m_TextureList;
	std::map<std::string, Shader*> *m_ShaderList;

	loadModelTextureCallBack m_LoadModelTexture;
	void *m_LoadModelTextureUserdata;

	ResourceProxy* m_ResProxy;

public:
	/**
	* Gets an instance of the model factory.
	* @return a pointer to the instance
	*/
	static ModelFactory *getInstance(void);

	/**
	* Initialize the factory.
	* @param p_TextureList pointer to the texture map with the available textures
	* @param p_ShaderList pointer to the shader map with the available shaders
	*/
	void initialize(TextureMap* p_TextureList,
		std::map<std::string, Shader*> *p_ShaderList,
		ResourceProxy* p_ResProxy);

	/**
	* Shuts down the factory and releases the memory allocated. Nulls all pointers.
	*/
	virtual void shutdown(void);

	/**
	* Creates a model with buffers and default shader and connects the textures to it.
	* @param p_Res the model file to read
	* @return copy of the created model
	*/
	virtual ModelDefinition createModel(ResId p_Res);

	/**
	* Creates a quad model with with a texture attached to it.
	* @param p_HalfSize the size from the center point to the xy-edges
	* @param p_TextureId the texture to be used
	* @return copy of the created quad
	*/
	virtual ModelDefinition *create2D_Model(Vector2 p_HalfSize, const char *p_TextureId);

	/**
	* Creates a quad model from a texture with the texture attached to it.
	* @param p_Texture the texture to be used
	* @return copy of the created quad
	*/
	virtual ModelDefinition *create2D_Model(ID3D11ShaderResourceView *p_Texture);

	/**
	* Set the function to load a texture to a model.
	* @param p_LoadModelTexture the function to be called whenever a texture is to be loaded.
	* @param p_UserData user defined data
	*/
	void setLoadModelTextureCallBack(loadModelTextureCallBack p_LoadModelTexture, void *p_Userdata);

protected:
	ModelFactory(void);
	~ModelFactory(void);
	
private:
	template<class T>
	Buffer::Description createBufferDescription(const std::vector<T> &p_VertexData, Buffer::Usage p_Usage);
	void create2D_VertexBuffer(ModelDefinition *p_Model, Vector2 p_HalfSize);

	void loadTextures(ModelDefinition &model, unsigned int p_NumOfMaterials,
		const std::vector<Material> &p_Materials, const char *p_Style);
	void load2D_Texture(ModelDefinition &model, const char *p_TextureId);
	ID3D11ShaderResourceView *getTextureFromList(std::string p_Identifier);
};