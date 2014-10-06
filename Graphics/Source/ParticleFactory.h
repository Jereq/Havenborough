#pragma once
#include "ParticleInstance.h"
#include "ResourceProxy.h"
#include "ResourceTranslator.h"

#include <map>
class ParticleFactory
{
public:
	typedef uint32_t ResId;

	/**
	* Callback for loading a texture to a particle.
	*
	* @param p_ResourceName the resource name of the texture
	* @param p_FilePath path to where the texture is located
	* @param p_UserData user defined data
	*/
	typedef void (*loadParticleTextureCallBack)(const char *p_ResourceName, ResId p_Res, void *p_Userdata);

private:
	std::map<std::string, std::pair<ResId, ID3D11ShaderResourceView*>> *m_TextureList;
	std::map<std::string, Shader*> *m_ShaderList;
	ID3D11SamplerState* m_Sampler;

	loadParticleTextureCallBack m_LoadParticleTexture;
	void *m_LoadParticleTextureUserdata;

	ResourceProxy* m_ResProxy;

public:
	~ParticleFactory();

	/**
	* Initialize the factory.
	*
	* @param p_TextureList pointer to the texture map with the available textures
	* @param p_ShaderList pointer to the shader map with the available shaders
	* @param p_Device pointer to the device
	*/
	void initialize(std::map<std::string, std::pair<ResId, ID3D11ShaderResourceView*>> *p_TextureList, 
		std::map<std::string, Shader*> *p_ShaderList, ID3D11Device *p_Device, ResourceProxy* p_ResProxy);

	/**
	* Creates a list of static particle systems with buffers and connects the textures to it.
	*
	* @param p_Res the resource id of the file where the definitions are stored
	* @return list of definitions
	*/
	virtual std::vector<ParticleEffectDefinition::ptr> createParticleEffectDefinition(ResId p_Res);

	/**
	 * Create an instance of a particle effect from an already loaded definition.
	 *
	 * @param p_Effect a pointer to what definition to use
	 * @return a pointer to the new particle instance created
	 */
	virtual ParticleInstance::ptr createParticleInstance(ParticleEffectDefinition::ptr p_Effect);
	
	/**
	* Set the function to load a texture to a particle.
	*
	* @param p_LoadParticleTexture the function to be called whenever a texture is to be loaded.
	* @param p_UserData user defined data
	*/
	void setLoadParticleTextureCallBack(loadParticleTextureCallBack p_LoadParticleTexture, void *p_Userdata);

private:
	std::shared_ptr<Buffer> createParticleBuffer(unsigned int p_MaxParticles);
	std::shared_ptr<Buffer> createConstBuffer();

	ID3D11ShaderResourceView *loadTexture(ResId p_Res);

	ID3D11ShaderResourceView *getTextureFromList(std::string p_Identifier);
	void createSampler(ID3D11Device* p_Device);
};