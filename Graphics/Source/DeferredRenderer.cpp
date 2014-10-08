#include "DeferredRenderer.h"

#include "ModelBinaryLoader.h"
#include "Utilities/MemoryUtil.h"
#include "GraphicsExceptions.h"
#include "VRAMInfo.h"
#include "WrapperFactory.h"

#include <TweakSettings.h>

#include <algorithm>	// std::sort
#include <iterator>     // std::back_inserter
//#include "GraphicsLogger.h"

using std::vector;
using namespace DirectX;
//
//#define TIMER_START(x) { x->Start(); }
//#define TIMER_STOP(x) { x->Stop(); double time = 0; x->GetAverageTime(time); if(time > 0) GraphicsLogger::log(GraphicsLogger::Level::INFO, std::to_string(time)); }
//#define TIMER(x){m_Timer->Start(); x; m_Timer->Stop(); double time = 0; m_Timer->GetAverageTime(time); if(time > 0) GraphicsLogger::log(GraphicsLogger::Level::INFO, std::to_string(time));}
DeferredRenderer::DeferredRenderer()
{
	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_DepthStencilView = nullptr;
	m_SpotLights = nullptr;
	m_PointLights = nullptr;
	m_DirectionalLights = nullptr;

	m_RT[IGraphics::RenderTarget::DIFFUSE] = nullptr;
	m_RT[IGraphics::RenderTarget::NORMAL] = nullptr;
	m_RT[IGraphics::RenderTarget::W_POSITION] = nullptr;
	m_RT[IGraphics::RenderTarget::SSAO] = nullptr;
	m_RT[IGraphics::RenderTarget::SSAOPing] = nullptr;
	m_RT[IGraphics::RenderTarget::LIGHT] = nullptr;
	m_RT[IGraphics::RenderTarget::CSM] = nullptr;

	m_SRV["Diffuse"] = nullptr;
	m_SRV["Normal"] = nullptr;
	m_SRV["Light"] = nullptr;
	m_SRV["WPosition"] = nullptr;
	m_SRV["SSAO"] = nullptr;
	m_SRV["CSM"] = nullptr;
	m_SRV["SSAOPing"] = nullptr;
	m_SRV["SSAO_RandomVec"] = nullptr;

	m_Sampler["Default"] = nullptr;
	m_Sampler["ShadowMap"] = nullptr;
	m_Shader["PointLight"] = nullptr;
	m_Shader["SpotLight"] = nullptr;
	m_Shader["DirectionalLight"] = nullptr;
	m_Shader["SSAO"] = nullptr;
	m_Shader["SSAO_Blur"] = nullptr;

	m_Buffer["PointLightModel"] = nullptr;
	m_Buffer["SpotLightModel"] = nullptr;
	m_Buffer["DirectionalLightModel"] = nullptr;
	m_Buffer["DefaultConstant"] = nullptr;
	m_Buffer["ObjectConstant"] = nullptr;
	m_Buffer["AnimatedConstant"] = nullptr;
	m_Buffer["WorldInstance"] = nullptr;
	m_Shader["IGeometry"] = nullptr;
	m_Shader["ShadowMapGeometry"] = nullptr;

	m_Buffer["LightConstant"] = nullptr;
	m_Buffer["LightViewProj"] = nullptr;
	m_Buffer["SSAOConstant"] = nullptr;
	m_Buffer["SSAOConstant_Blur"] = nullptr;

	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;
	m_RasterState = nullptr;
	m_DepthState = nullptr;
	m_BlendState = nullptr;
	m_BlendState2 = nullptr;
	m_SkyDome = nullptr;
	m_SSAO = false;
	m_DepthMapDSV = nullptr;

	m_FOVIsUpdated = false;
}

DeferredRenderer::~DeferredRenderer(void)
{
	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_DepthStencilView = nullptr;
	m_SpotLights = nullptr;
	m_PointLights = nullptr;
	m_DirectionalLights = nullptr;
	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;

	for(auto &rt : m_RT)
		SAFE_RELEASE(rt.second);
	m_RT.clear();

	for(auto &srv : m_SRV)
		SAFE_RELEASE(srv.second);
	m_SRV.clear();

	for(auto &sampler : m_Sampler)
		SAFE_RELEASE(sampler.second);
	m_Sampler.clear();

	for(auto &shader : m_Shader)
		SAFE_DELETE(shader.second);
	m_Shader.clear();

	for(auto &buffer : m_Buffer)
		SAFE_DELETE(buffer.second);
	m_Buffer.clear();

	SAFE_RELEASE(m_BlendState);
	SAFE_RELEASE(m_BlendState2);
	SAFE_RELEASE(m_RasterState);
	SAFE_RELEASE(m_DepthState);
	SAFE_DELETE(m_SkyDome);
	SAFE_RELEASE(m_DepthMapDSV);
}

void DeferredRenderer::initialize(ID3D11Device* p_Device, ID3D11DeviceContext* p_DeviceContext,
	ID3D11DepthStencilView *p_DepthStencilView, unsigned int p_ScreenWidth, unsigned int p_ScreenHeight,
	DirectX::XMFLOAT3 p_CameraPosition, DirectX::XMFLOAT4X4 *p_ViewMatrix,	DirectX::XMFLOAT4X4 *p_ProjectionMatrix, int p_ShadowMapResolution,
	std::vector<Light> *p_SpotLights, std::vector<Light> *p_PointLights, std::vector<Light> *p_DirectionalLights, Light *p_ShadowMappedLight,
	float *p_FOV, float p_FarZ, ResourceProxy* p_ResProxy)
{
	m_Device			= p_Device;
	m_DeviceContext		= p_DeviceContext;
	m_DepthStencilView	= p_DepthStencilView;
	m_ShadowMappedLight = p_ShadowMappedLight;
	m_CameraPosition	= p_CameraPosition;
	m_ViewMatrix		= p_ViewMatrix;
	m_ProjectionMatrix	= p_ProjectionMatrix;
	m_ShadowMapResolution = p_ShadowMapResolution;
	m_SpotLights = p_SpotLights;
	m_PointLights = p_PointLights;
	m_DirectionalLights = p_DirectionalLights;
	m_RenderSkyDome = false;
	m_FOV = p_FOV;
	m_FarZ = p_FarZ;
	m_ScreenWidth = (float)p_ScreenWidth;
	m_ScreenHeight = (float)p_ScreenHeight;

	m_ResProxy = p_ResProxy;

	if(!m_Device || !m_DeviceContext)
		throw DeferredRenderException("Failed to initialize deferred renderer, nullpointers not allowed",
		__LINE__, __FILE__);

	createAllRenderTargets();
	
	createRandomTexture(256);
	
	createShaders();

	loadLightModels();

	// Create sampler state and blend state for Alpha rendering.
	createSamplerState();

	createBlendStates();
	createLightStates();
	createBuffers();

	registerTweakSettings();
}

void DeferredRenderer::resize(unsigned int p_ScreenWidth, unsigned int p_ScreenHeight, ID3D11DepthStencilView *p_DepthStencilView)
{
	m_DepthStencilView = p_DepthStencilView;
	m_ScreenWidth = (float)p_ScreenWidth;
	m_ScreenHeight = (float)p_ScreenHeight;

	createAllRenderTargets();
}

void DeferredRenderer::initializeShadowMap(UINT width, UINT height)
{
	//use typeless format because the DSV is going to interpret the
	//bits as DXGI_FORMAT_D24_UNORM_S8_UNIT, whereas the SRV is going
	//to interpret the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
	D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width     = width;
    texDesc.Height    = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format    = DXGI_FORMAT_R32_TYPELESS;
    texDesc.SampleDesc.Count   = 1;  
    texDesc.SampleDesc.Quality = 0;  
    texDesc.Usage          = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0; 
    texDesc.MiscFlags      = 0;

	ID3D11Texture2D* depthMap = 0;
	HRESULT hr = m_Device->CreateTexture2D(&texDesc, 0, &depthMap);
	int val = VRAMInfo::getInstance()->calculateFormatUsage(texDesc.Format, width, height);
	VRAMInfo::getInstance()->updateUsage(val);
	//render from light
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
	SAFE_RELEASE(m_DepthMapDSV);
    hr = m_Device->CreateDepthStencilView(depthMap, &dsvDesc, &m_DepthMapDSV);
	
	//render to backbuffert from the camera
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
	SAFE_RELEASE(m_SRV["CSM"]);
	hr =  m_Device->CreateShaderResourceView(depthMap, &srvDesc, &m_SRV["CSM"]);

	//viewport that  matches the shadow map dimensions.
	m_LightViewport.Width = (float)width;
	m_LightViewport.Height = (float)height;
	m_LightViewport.MinDepth = 0.0f;
	m_LightViewport.MaxDepth = 1.0f;
	m_LightViewport.TopLeftX = 0.0f;
	m_LightViewport.TopLeftY = 0.0f;
	
    // View saves a reference to the texture so we can release our reference.
    SAFE_RELEASE(depthMap);
}

void DeferredRenderer::renderDeferred()
{
	// Clear render targets.
	clearRenderTargets();
		
	if(m_FOVIsUpdated)
	{
		updateSSAO_VarConstantBuffer();
		m_FOVIsUpdated = false;
	}
	updateConstantBuffer(*m_ViewMatrix, *m_ProjectionMatrix);

	// Update constant buffer and render
	if(m_Objects.size() > 0)
	{
		//reset values and render other
		unsigned int nrRT = 3;
		ID3D11RenderTargetView *rtv[] = {
		m_RT[IGraphics::RenderTarget::DIFFUSE], m_RT[IGraphics::RenderTarget::NORMAL], m_RT[IGraphics::RenderTarget::W_POSITION]
		};

		std::vector<std::vector<Renderable>> instancedModels;
		std::vector<Renderable> animatedOrSingle;

		SortRenderables(animatedOrSingle, instancedModels);
		renderGeometry(m_DepthStencilView, nrRT, rtv, instancedModels, animatedOrSingle, m_Shader["IGeometry"]);
		if(m_SSAO)
		{
			D3D11_VIEWPORT vp, oldVP;
			unsigned int nrVP = 1;
			m_DeviceContext->RSGetViewports(&nrVP, &oldVP);
			vp = oldVP;
			vp.Height = m_ScreenHeight * m_SSAO_ResolutionScale;
			vp.Width =  m_ScreenWidth * m_SSAO_ResolutionScale;
			m_DeviceContext->RSSetViewports(1, &vp);

			renderSSAO();
			blurSSAO();

			m_DeviceContext->RSSetViewports(1, &oldVP);
		}
		renderLighting(instancedModels, animatedOrSingle);

		m_Objects.clear();
	}
	
	if (m_SkyDome && m_RenderSkyDome)
	{
		renderSkyDomeImpl();
		m_RenderSkyDome = false;
	}
}

void DeferredRenderer::renderGeometry(ID3D11DepthStencilView* p_DepthStencilView, unsigned int nrRT, ID3D11RenderTargetView* rtv[],
									  const std::vector<std::vector<Renderable>> &p_InstancedModels, 
									  const std::vector<Renderable> &p_AnimatedOrSingle, Shader* p_Shader)
{
	// Set the render targets.
	m_DeviceContext->OMSetRenderTargets(nrRT, rtv, p_DepthStencilView);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// The textures will be needed to be grabbed from the model later.
	m_Buffer["DefaultConstant"]->setBuffer(0);
	m_DeviceContext->PSSetSamplers(0,1,&m_Sampler["Default"]);

	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0};
	m_Buffer["ObjectConstant"]->setBuffer(1);
	m_Buffer["AnimatedConstant"]->setBuffer(2);

	for(const auto& animation : p_AnimatedOrSingle )
		renderObject(animation);

	m_DeviceContext->PSSetShaderResources(0, 3, nullsrvs);
	m_Buffer["AnimatedConstant"]->unsetBuffer(2);
	m_Buffer["ObjectConstant"]->unsetBuffer(1);

	for(const auto& k : p_InstancedModels)
		RenderObjectsInstanced(k,p_Shader);
	
	ID3D11SamplerState* const nullSamplerState = nullptr;
	m_DeviceContext->PSSetSamplers(0, 1, &nullSamplerState);
	m_Buffer["DefaultConstant"]->unsetBuffer(0);

	// Unset render targets.
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
}

void DeferredRenderer::renderSSAO(void)
{
	m_DeviceContext->OMSetRenderTargets(1, &m_RT[IGraphics::RenderTarget::SSAO], 0);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_Shader["SSAO"]->setShader();
	m_Buffer["DefaultConstant"]->setBuffer(0);
	m_Buffer["SSAOConstant"]->setBuffer(1);

	ID3D11SamplerState *samplers[] =
	{
		m_Sampler["SSAO_NormalDepth"],
		m_Sampler["SSAO_RandomVec"]
	};
	m_DeviceContext->PSSetSamplers(0, 2,  samplers);

	ID3D11ShaderResourceView *srvs[] =
	{
		m_SRV["Normal"],
		m_SRV["SSAO_RandomVec"]
	};
	m_DeviceContext->PSSetShaderResources(0, 2, srvs);

	m_DeviceContext->Draw(6, 0);

	ID3D11ShaderResourceView *nullSrvs[] = { 0, 0 };
	m_DeviceContext->PSSetShaderResources(0, 2, nullSrvs);

	ID3D11SamplerState *nullSamplers[] = { 0, 0	};
	m_DeviceContext->PSSetSamplers(0, 2, nullSamplers);

	m_Buffer["SSAOConstant"]->unsetBuffer(1);
	m_Buffer["DefaultConstant"]->unsetBuffer(0);
	m_Shader["SSAO"]->unSetShader();
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
}

void DeferredRenderer::blurSSAO(void)
{
	m_Shader["SSAO_Blur"]->setShader();
	m_Buffer["SSAOConstant_Blur"]->setBuffer(0);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_DeviceContext->PSSetSamplers(0, 1, &m_Sampler["SSAO_Blur"]);

	for(int i = 0; i < 4; i++) //TODO: Should be 4 passes
	{
		SSAO_PingPong(m_SRV["SSAO"], m_RT[IGraphics::RenderTarget::SSAOPing], false);
		SSAO_PingPong(m_SRV["SSAOPing"], m_RT[IGraphics::RenderTarget::SSAO], true);
	}

	ID3D11SamplerState *nullSamplers[] = { 0 };
	m_DeviceContext->PSSetSamplers(0, 1, nullSamplers);

	m_Buffer["SSAOConstant_Blur"]->unsetBuffer(0);
	m_Shader["SSAO_Blur"]->unSetShader();
}

void DeferredRenderer::SSAO_PingPong(ID3D11ShaderResourceView *p_InputSRV, ID3D11RenderTargetView *p_OutputTarget,
	bool p_HorizontalBlur)
{
	updateSSAO_BlurConstantBuffer(p_HorizontalBlur);

	m_DeviceContext->OMSetRenderTargets(1, &p_OutputTarget, 0);
	ID3D11ShaderResourceView *srvs[] =
	{
		m_SRV["Normal"],
		p_InputSRV
	};

	m_DeviceContext->PSSetShaderResources(0, 2, srvs);
	m_DeviceContext->Draw(6, 0);

	ID3D11ShaderResourceView *nullSrvs[] = { 0, 0 };
	m_DeviceContext->PSSetShaderResources(0, 2, nullSrvs);
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
}

void DeferredRenderer::updateSSAO_BlurConstantBuffer(bool p_HorizontalBlur)
{
	cSSAO_BlurBuffer blurBuffer;
	blurBuffer.horizontalBlur = p_HorizontalBlur;
	blurBuffer.texelWidth = 1.0f / m_ScreenWidth;
	blurBuffer.texelHeight = 1.0f / m_ScreenHeight;
	m_DeviceContext->UpdateSubresource(m_Buffer["SSAOConstant_Blur"]->getBufferPointer(), NULL, nullptr, &blurBuffer,
		NULL, NULL);
}

void DeferredRenderer::updateSSAO_VarConstantBuffer()
{
	cSSAO_Buffer ssaoBuffer;
	float aspect = m_ScreenWidth / m_ScreenHeight;
	float halfHeight = m_FarZ * std::tanf(0.5f * (*m_FOV));
	float halfWidth = aspect * halfHeight;

	ssaoBuffer.corners[0] = DirectX::XMFLOAT4(-halfWidth, -halfHeight, m_FarZ, 0);
	ssaoBuffer.corners[1] = DirectX::XMFLOAT4(-halfWidth, +halfHeight, m_FarZ, 0);
	ssaoBuffer.corners[2] = DirectX::XMFLOAT4(+halfWidth, +halfHeight, m_FarZ, 0);
	ssaoBuffer.corners[3] = DirectX::XMFLOAT4(+halfWidth, -halfHeight, m_FarZ, 0);
	buildSSAO_OffsetVectors(ssaoBuffer);

	ssaoBuffer.occlusionRadius	= 15.0f;
	ssaoBuffer.surfaceEpsilon	= 4.0f;
	ssaoBuffer.occlusionFadeEnd	= 10.0f;
	ssaoBuffer.occlusionFadeStart = 3.0f;

	TweakSettings* settings = TweakSettings::getInstance();
	settings->querySetting("ssao.radius", ssaoBuffer.occlusionRadius);
	settings->querySetting("ssao.epsilon", ssaoBuffer.surfaceEpsilon);
	settings->querySetting("ssao.fadeEnd", ssaoBuffer.occlusionFadeEnd);
	settings->querySetting("ssao.fadeStart", ssaoBuffer.occlusionFadeStart);

	m_DeviceContext->UpdateSubresource(m_Buffer["SSAOConstant"]->getBufferPointer(),
		NULL, nullptr, &ssaoBuffer, NULL, NULL);
}

void DeferredRenderer::renderLighting(const std::vector<std::vector<Renderable>> &p_InstancedModels, const std::vector<Renderable> &p_AnimatedOrSingle)
{
	// Store previous States to be set when we exit the method.
	ID3D11RasterizerState *previousRasterState;
	ID3D11DepthStencilState *previousDepthState;
	m_DeviceContext->RSGetState(&previousRasterState);
	m_DeviceContext->OMGetDepthStencilState(&previousDepthState,0);

	m_DeviceContext->RSSetState(m_RasterState);

	// Collect the shader resources in an array and create a clear array.
	ID3D11ShaderResourceView *srvs[] = {m_SRV["WPosition"], m_SRV["Normal"], m_SRV["Diffuse"], m_SRV["SSAO"], m_SRV["CSM"]};
	ID3D11ShaderResourceView *distanceFog_srvs[] = { m_SRV["WPosition"], m_SRV["Light"] };
	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0,0,0};

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	UINT sampleMask = 0xffffffff;
	if(m_ShadowMap)
	{
		renderShadowMap(*m_ShadowMappedLight, p_InstancedModels, p_AnimatedOrSingle);
	}
	//Set constant data
	m_Buffer["DefaultConstant"]->setBuffer(0);

	// Set texture sampler.
	m_DeviceContext->PSSetShaderResources(0, 5, srvs);
	m_DeviceContext->PSSetSamplers(0, 1, &m_Sampler["ShadowMap"]);

	//Select the final rendertarget
	m_DeviceContext->OMSetRenderTargets(1, &m_RT[IGraphics::RenderTarget::LIGHT], m_DepthStencilView); 
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_DeviceContext->OMSetDepthStencilState(m_DepthState,0);
	m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, sampleMask);

	//Render PointLights
	renderLight(m_Shader["PointLight"], m_Buffer["PointLightModel"], m_PointLights);
	//Render SpotLights
	renderLight(m_Shader["SpotLight"], m_Buffer["SpotLightModel"], m_SpotLights);
	//DirectionalLights except number one
	m_DeviceContext->OMSetRenderTargets(1, &m_RT[IGraphics::RenderTarget::LIGHT], 0); 
	renderLight(m_Shader["DirectionalLight"], m_Buffer["DirectionalLightModel"], m_DirectionalLights);

	renderAmbientLight(m_Buffer["DirectionalLightModel"]);

	m_DeviceContext->PSSetShaderResources(0, 5, nullsrvs);

	m_Shader["DistanceFog"]->setShader();
	m_DeviceContext->OMSetRenderTargets(1, &m_RT[IGraphics::RenderTarget::DIFFUSE], 0);
	m_DeviceContext->PSSetShaderResources(0, 2, distanceFog_srvs);
	m_DeviceContext->OMSetBlendState(m_BlendState2, blendFactor, sampleMask);
	m_Buffer["DirectionalLightModel"]->setBuffer(0);

	m_DeviceContext->Draw(m_Buffer["DirectionalLightModel"]->getNumOfElements(), 0);

	m_Buffer["DefaultConstant"]->unsetBuffer(0);

	ID3D11SamplerState* const nullSamplerState = nullptr;
	m_DeviceContext->PSSetSamplers(0, 1, &nullSamplerState);

	m_DeviceContext->PSSetShaderResources(0, 5, nullsrvs);
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
	m_DeviceContext->RSSetState(previousRasterState);
	m_DeviceContext->OMSetDepthStencilState(previousDepthState,0);
	m_DeviceContext->OMSetBlendState(0, blendFactor, sampleMask);
	SAFE_RELEASE(previousRasterState);
	SAFE_RELEASE(previousDepthState);
}

void DeferredRenderer::renderSkyDomeImpl()
{
	ID3D11RasterizerState *previousRasterState;
	ID3D11DepthStencilState *previousDepthState;
	m_DeviceContext->RSGetState(&previousRasterState);
	m_DeviceContext->OMGetDepthStencilState(&previousDepthState,0);

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	UINT sampleMask = 0xffffffff;
	m_DeviceContext->OMSetBlendState(m_BlendState2, blendFactor, sampleMask);

	m_SkyDome->RenderSkyDome(m_RT[IGraphics::RenderTarget::DIFFUSE], m_DepthStencilView, m_Buffer["DefaultConstant"]);

	ID3D11SamplerState* const nullSamplerState = nullptr;
	m_DeviceContext->PSSetSamplers(0, 1, &nullSamplerState);
	
	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0,0,0};
	m_DeviceContext->PSSetShaderResources(0, 5, nullsrvs);
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
	m_DeviceContext->RSSetState(previousRasterState);
	m_DeviceContext->OMSetDepthStencilState(previousDepthState,0);
	m_DeviceContext->OMSetBlendState(0, blendFactor, sampleMask);
	SAFE_RELEASE(previousRasterState);
	SAFE_RELEASE(previousDepthState);
}

void DeferredRenderer::addRenderable(Renderable p_renderable)
{
	m_Objects.push_back(p_renderable);
}

void DeferredRenderer::createSkyDome(ID3D11ShaderResourceView* p_Texture, float p_Radius)
{
	if(m_SkyDome)
		SAFE_DELETE(m_SkyDome);

	m_SkyDome = new SkyDome();
	m_SkyDome->init(m_Device, m_DeviceContext, p_Texture, p_Radius, m_ResProxy);
}

void DeferredRenderer::renderSkyDome()
{
	m_RenderSkyDome = true;
}

ID3D11ShaderResourceView* DeferredRenderer::getRT(IGraphics::RenderTarget i)
{
	switch(i)
	{
	case IGraphics::RenderTarget::DIFFUSE: return m_SRV["Diffuse"];
	case IGraphics::RenderTarget::NORMAL: return m_SRV["Normal"];
	case IGraphics::RenderTarget::W_POSITION: return m_SRV["WPosition"];
	case IGraphics::RenderTarget::SSAO: return m_SRV["SSAO"];
	case IGraphics::RenderTarget::CSM: return m_SRV["CSM"];
	case IGraphics::RenderTarget::LIGHT: return m_SRV["Light"];
	default: return nullptr;
	}
}

void DeferredRenderer::updateCamera(DirectX::XMFLOAT3 p_Position)
{
	m_CameraPosition = p_Position;
}

void DeferredRenderer::enableSSAO(bool p_State)
{
	m_SSAO = p_State;
}

void DeferredRenderer::enableShadowMap(bool p_State)
{
	m_ShadowMap = p_State;
}

void DeferredRenderer::FOVIsUpdated()
{
	m_FOVIsUpdated = true;
}

void DeferredRenderer::updateConstantBuffer(DirectX::XMFLOAT4X4 p_ViewMatrix, DirectX::XMFLOAT4X4 p_ProjMatrix)
{
	m_CurrentView = p_ViewMatrix;
	m_CurrentProjection = p_ProjMatrix;

	cBuffer cb;
	cb.view = p_ViewMatrix;
	cb.proj = p_ProjMatrix;
	cb.campos = m_CameraPosition;
	cb.ssaoScale = m_SSAO_ResolutionScale;
	m_DeviceContext->UpdateSubresource(m_Buffer["DefaultConstant"]->getBufferPointer(), NULL,NULL, &cb,NULL,NULL);
}

void DeferredRenderer::updateLightBuffer(bool p_Big, bool p_ShadowMapped)
{
	cLightBuffer cb;
	cb.view = m_LightView;
	cb.projection = m_LightProjection;
	cb.big = p_Big; 
	cb.shadowMapped = p_ShadowMapped;
	m_DeviceContext->UpdateSubresource(m_Buffer["LightViewProj"]->getBufferPointer(), NULL, NULL, &cb, NULL, NULL);
}

void DeferredRenderer::createAllRenderTargets()
{
	//Create render targets with the size of screen width and screen height
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof(desc) );
	desc.Width				= (UINT)m_ScreenWidth;
	desc.Height				= (UINT)m_ScreenHeight;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;
	desc.Format				= DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.SampleDesc.Count	= 1;
	desc.Usage				= D3D11_USAGE_DEFAULT;
	desc.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	
	//DXGI_FORMAT_R32G32B32A32_FLOAT 
	SAFE_RELEASE(m_RT[IGraphics::RenderTarget::W_POSITION]);
	m_RT[IGraphics::RenderTarget::W_POSITION] = createRenderTarget(desc);

	SAFE_RELEASE(m_SRV["WPosition"]);
	m_SRV["WPosition"] = createShaderResourceView(desc, m_RT[IGraphics::RenderTarget::W_POSITION]);
	//Shadow map
	m_ShadowBigSize = 20000.0f;
	m_ShadowSmallSize = 3000.0f;
	float percentage = m_ShadowSmallSize/m_ShadowBigSize;
	percentage = percentage * 0.5f;
	percentage = 0.5f - percentage;
	m_ShadowMapBorder = percentage;
	UINT resolution = m_ShadowMapResolution; //size of Shadow Map
	if(m_ShadowMap)
		initializeShadowMap(resolution, resolution);	

	//DXGI_FORMAT_R16G16B16A16_FLOAT
	desc.Format	= DXGI_FORMAT_R16G16B16A16_FLOAT;
	SAFE_RELEASE(m_RT[IGraphics::RenderTarget::DIFFUSE]);
	m_RT[IGraphics::RenderTarget::DIFFUSE] = createRenderTarget(desc);
	SAFE_RELEASE(m_SRV["Diffuse"]);
	m_SRV["Diffuse"] = createShaderResourceView(desc, m_RT[IGraphics::RenderTarget::DIFFUSE]);
	
	SAFE_RELEASE(m_RT[IGraphics::RenderTarget::NORMAL]);
	m_RT[IGraphics::RenderTarget::NORMAL] = createRenderTarget(desc);
	SAFE_RELEASE(m_SRV["Normal"]);
	m_SRV["Normal"] = createShaderResourceView(desc, m_RT[IGraphics::RenderTarget::NORMAL]);
	
	SAFE_RELEASE(m_RT[IGraphics::RenderTarget::LIGHT]);
	m_RT[IGraphics::RenderTarget::LIGHT] = createRenderTarget(desc);
	SAFE_RELEASE(m_SRV["Light"]);
	m_SRV["Light"] = createShaderResourceView(desc, m_RT[IGraphics::RenderTarget::LIGHT]);

	m_SSAO_ResolutionScale = 1.0f;

	desc.Width	= (UINT)(m_ScreenWidth * m_SSAO_ResolutionScale);
	desc.Height	= (UINT)(m_ScreenHeight * m_SSAO_ResolutionScale);
	
	SAFE_RELEASE(m_RT[IGraphics::RenderTarget::SSAO]);
	m_RT[IGraphics::RenderTarget::SSAO] = createRenderTarget(desc);
	SAFE_RELEASE(m_SRV["SSAO"]);
	m_SRV["SSAO"] = createShaderResourceView(desc, m_RT[IGraphics::RenderTarget::SSAO]);
	SAFE_RELEASE(m_RT[IGraphics::RenderTarget::SSAOPing]);
	m_RT[IGraphics::RenderTarget::SSAOPing] = createRenderTarget(desc);
	SAFE_RELEASE(m_SRV["SSAOPing"]);
	m_SRV["SSAOPing"] = createShaderResourceView(desc, m_RT[IGraphics::RenderTarget::SSAOPing]);
}

ID3D11RenderTargetView *DeferredRenderer::createRenderTarget(D3D11_TEXTURE2D_DESC &desc)
{
	// Create the render target texture
	HRESULT result = S_FALSE;
	ID3D11RenderTargetView* ret = nullptr;
	//Create the render targets
	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = desc.Format;
	rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;

	ID3D11Texture2D *temp;
	result = m_Device->CreateTexture2D(&desc, nullptr, &temp);
	if(FAILED(result))
		throw GraphicsException("Error creating Texture2D while creating a render target.", __LINE__, __FILE__);

	result = m_Device->CreateRenderTargetView(temp, &rtDesc, &ret);
	SAFE_RELEASE(temp);

	if(FAILED(result))
		throw GraphicsException("Error creating a render target.", __LINE__, __FILE__);

	VRAMInfo::getInstance()->updateUsage(VRAMInfo::getInstance()->calculateFormatUsage(desc.Format,
		desc.Width, desc.Height));

	return ret;
}

ID3D11ShaderResourceView *DeferredRenderer::createShaderResourceView( D3D11_TEXTURE2D_DESC &desc, ID3D11RenderTargetView *p_Rendertarget)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC dssrvdesc;
	dssrvdesc.Format = desc.Format;
	dssrvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dssrvdesc.Texture2D.MipLevels = 1;
	dssrvdesc.Texture2D.MostDetailedMip = 0;

	ID3D11ShaderResourceView* ret = nullptr;
	ID3D11Resource* tt;
	p_Rendertarget->GetResource(&tt);
	HRESULT result = m_Device->CreateShaderResourceView(tt, &dssrvdesc, &ret);
	SAFE_RELEASE(tt);
	if(FAILED(result))
		throw GraphicsException("Error when creating shader resource view from render target.", __LINE__, __FILE__);
	return ret;
}

void DeferredRenderer::createBuffers()
{
	cBuffer cb;
	cb.view = *m_ViewMatrix;
	cb.proj = *m_ProjectionMatrix;
	cb.campos = m_CameraPosition;
	cb.ssaoScale = m_SSAO_ResolutionScale;

	Buffer::Description cbdesc;
	cbdesc.initData = &cb;
	cbdesc.numOfElements = 1;
	cbdesc.sizeOfElement = sizeof(cBuffer);
	cbdesc.type = Buffer::Type::CONSTANT_BUFFER_ALL;
	cbdesc.usage = Buffer::Usage::DEFAULT;
	m_Buffer["DefaultConstant"] = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cBuffer));

	cbdesc.initData = nullptr;
	cbdesc.sizeOfElement = sizeof(cObjectBuffer);
	m_Buffer["ObjectConstant"] = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cObjectBuffer));

	cbdesc.sizeOfElement = sizeof(cAnimatedObjectBuffer);
	m_Buffer["AnimatedConstant"] = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cAnimatedObjectBuffer));	

	Buffer::Description adesc;
	adesc.initData = nullptr;
	adesc.numOfElements = 1; //Minimum
	adesc.sizeOfElement = sizeof(Light);
	adesc.type = Buffer::Type::VERTEX_BUFFER;
	adesc.usage = Buffer::Usage::CPU_WRITE_DISCARD;
	m_Buffer["LightConstant"] = WrapperFactory::getInstance()->createBuffer(adesc);
	VRAMInfo::getInstance()->updateUsage(adesc.sizeOfElement);

	Buffer::Description lightDesc;
	lightDesc.initData = nullptr;
	lightDesc.numOfElements = 1;
	lightDesc.sizeOfElement = sizeof(cLightBuffer);
	lightDesc.type = Buffer::Type::CONSTANT_BUFFER_PS;
	lightDesc.usage = Buffer::Usage::DEFAULT;
	m_Buffer["LightViewProj"] = WrapperFactory::getInstance()->createBuffer(lightDesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cLightBuffer));

	Buffer::Description instanceWorldDesc;
	instanceWorldDesc.initData = nullptr;
	instanceWorldDesc.numOfElements = 1; //Minimum
	instanceWorldDesc.sizeOfElement = sizeof(cInstanceBuffer);
	instanceWorldDesc.type = Buffer::Type::VERTEX_BUFFER;
	instanceWorldDesc.usage = Buffer::Usage::CPU_WRITE_DISCARD;
	m_Buffer["WorldInstance"] = WrapperFactory::getInstance()->createBuffer(instanceWorldDesc);
	VRAMInfo::getInstance()->updateUsage(instanceWorldDesc.sizeOfElement);

	cbdesc.sizeOfElement = sizeof(cSSAO_Buffer);
	cbdesc.initData = nullptr;
	cbdesc.type = Buffer::Type::CONSTANT_BUFFER_ALL;
	cbdesc.usage = Buffer::Usage::DEFAULT;

	m_Buffer["SSAOConstant"] = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cSSAO_Buffer));

	updateSSAO_VarConstantBuffer();

	cSSAO_BlurBuffer ssaoBlurBuffer;
	ssaoBlurBuffer.horizontalBlur = true;
	ssaoBlurBuffer.texelWidth = 1.0f / m_ScreenWidth;
	ssaoBlurBuffer.texelHeight = 1.0f / m_ScreenHeight;

	cbdesc.sizeOfElement = sizeof(cSSAO_BlurBuffer);
	cbdesc.initData = &ssaoBlurBuffer;
	cbdesc.type = Buffer::Type::CONSTANT_BUFFER_ALL;
	cbdesc.usage = Buffer::Usage::DEFAULT;
	m_Buffer["SSAOConstant_Blur"] = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cSSAO_BlurBuffer));

	cbdesc.sizeOfElement = sizeof(DirectX::XMFLOAT3);
	m_ColorShadingConstantBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(DirectX::XMFLOAT3));
}

void DeferredRenderer::buildSSAO_OffsetVectors(cSSAO_Buffer &p_Buffer)
{
	p_Buffer.offsetVectors[0] = XMFLOAT4(1, 1, 1, 0);
	p_Buffer.offsetVectors[1] = XMFLOAT4(-1, -1, -1, 0);

	p_Buffer.offsetVectors[2] = XMFLOAT4(-1, +1, +1, 0);
	p_Buffer.offsetVectors[3] = XMFLOAT4(+1, -1, -1, 0);

	p_Buffer.offsetVectors[4] = XMFLOAT4(+1, +1, -1, 0);
	p_Buffer.offsetVectors[5] = XMFLOAT4(-1, -1, +1, 0);

	p_Buffer.offsetVectors[6] = XMFLOAT4(-1, +1, -1, 0);
	p_Buffer.offsetVectors[7] = XMFLOAT4(+1, -1, +1, 0);

	p_Buffer.offsetVectors[8] = XMFLOAT4(-1, 0, 0, 0);
	p_Buffer.offsetVectors[9] = XMFLOAT4(+1, 0, 0, 0);

	p_Buffer.offsetVectors[10] = XMFLOAT4(0, -1, 0, 0);
	p_Buffer.offsetVectors[11] = XMFLOAT4(0, +1, 0, 0);

	p_Buffer.offsetVectors[12] = XMFLOAT4(0, 0, -1, 0);
	p_Buffer.offsetVectors[13] = XMFLOAT4(0, 0, +1, 0);

	std::default_random_engine randomizer;
	std::uniform_real_distribution<float> distribution(0.25f, 1.0f);

	for(int i = 0; i < 14; i++)
	{
		float s = distribution(randomizer);
		XMVECTOR v = XMLoadFloat4(&p_Buffer.offsetVectors[i]);
		v = s * XMVector4Normalize(v);
		XMStoreFloat4(&p_Buffer.offsetVectors[i], v);
	}
}

void DeferredRenderer::clearRenderTargets()
{
	float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_DeviceContext->ClearRenderTargetView(m_RT[IGraphics::RenderTarget::DIFFUSE], color);

	color[0] = color[1] = color[2] = 0.5f;
	m_DeviceContext->ClearRenderTargetView(m_RT[IGraphics::RenderTarget::NORMAL], color);

	color[0] = color[1] = color[2] = 1.0f;
	m_DeviceContext->ClearRenderTargetView(m_RT[IGraphics::RenderTarget::W_POSITION], color);

	color[0] = color[1] = color[2] = 1.0f;
	color[3] = 1.0f;
	m_DeviceContext->ClearRenderTargetView(m_RT[IGraphics::RenderTarget::SSAO], color);
	m_DeviceContext->ClearRenderTargetView(m_RT[IGraphics::RenderTarget::SSAOPing], color);

	color[0] = color[1] = color[2] = color[3] = 0.0f;
	m_DeviceContext->ClearRenderTargetView(m_RT[IGraphics::RenderTarget::LIGHT], color);
}

void DeferredRenderer::createSamplerState()
{
	// Create texture sampler.
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter			= D3D11_FILTER_ANISOTROPIC;
	sd.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc   = D3D11_COMPARISON_NEVER;
	sd.MinLOD			= 0;
	sd.MaxLOD			= D3D11_FLOAT32_MAX;
	m_Device->CreateSamplerState( &sd, &m_Sampler["Default"] );

	// Create SSAO random vector texture sampler.
	sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;//D3D11_FILTER_MIN_MAG_MIP_POINT; //Should be D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_Device->CreateSamplerState(&sd, &m_Sampler["SSAO_RandomVec"]);

	// Create SSAO normal depth texture sampler.
	sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.BorderColor[0] = sd.BorderColor[1] = sd.BorderColor[2] = 0.0f;
	sd.BorderColor[3] = 1e5f;
	m_Device->CreateSamplerState(&sd, &m_Sampler["SSAO_NormalDepth"]);

	// Create SSAO blur texture sampler
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	m_Device->CreateSamplerState(&sd, &m_Sampler["SSAO_Blur"]);

	// Create Shadow map texture sampler
	sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.BorderColor[0] = 1.f;
	sd.BorderColor[1] = sd.BorderColor[2] = sd.BorderColor[3] = 0.f;
	sd.ComparisonFunc = D3D11_COMPARISON_LESS;;

	m_Device->CreateSamplerState(&sd, &m_Sampler["ShadowMap"]);
}

void DeferredRenderer::createBlendStates()
{
	D3D11_BLEND_DESC bd;
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlend =  D3D11_BLEND_ONE;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_Device->CreateBlendState(&bd, &m_BlendState);

	for(int i = 0; i < 4; i++)
	{
		bd.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		bd.RenderTarget[i].DestBlend =  D3D11_BLEND_ZERO;
		bd.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		bd.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	}

	m_Device->CreateBlendState(&bd, &m_BlendState2);
}

void DeferredRenderer::createShaders()
{
	ShaderInputElementDescription shaderDesc[] = 
	{
		{"POSITION",	0, Format::R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,	  0},
		{"LPOSITION",	0, Format::R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA,  1},
		{"COLOR",		0, Format::R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"DIRECTION",	0, Format::R32G32B32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"ANGLE",		0, Format::R32G32_FLOAT,	1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"RANGE",		0, Format::R32_FLOAT,		1, 44, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"INTENSITY",	0, Format::R32_FLOAT,		1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

	ResourceProxy::Buff buff = m_ResProxy->getData("assets/shaders/LightPassSpotLight.hlsl");
	m_Shader["SpotLight"] = WrapperFactory::getInstance()->createShader(buff.data, buff.size, nullptr,
		"SpotLightVS,SpotLightPS", "5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER, shaderDesc, 7);

	buff = m_ResProxy->getData("assets/shaders/LightPassPointLight.hlsl");
	m_Shader["PointLight"] = WrapperFactory::getInstance()->createShader(buff.data, buff.size, nullptr,
		"PointLightVS,PointLightPS", "5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER, shaderDesc, 7);
	std::string resolution = std::to_string(m_ShadowMapResolution);
	std::string border = std::to_string(m_ShadowMapBorder);
	D3D_SHADER_MACRO preDefines[3] = {{ "SHADOW_RES", resolution.c_str()}, { "SHADOW_BORDER", border.c_str()}, nullptr};

	buff = m_ResProxy->getData("assets/shaders/LightPassDirectionalLight.hlsl");
	m_Shader["DirectionalLight"] = WrapperFactory::getInstance()->createShader(buff.data, buff.size, preDefines,
		"DirectionalLightVS,DirectionalLightPS", "5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER, shaderDesc, 7);

	ShaderInputElementDescription instanceshaderDesc[] = 
	{
		{"POSITION",0, Format::R32G32B32A32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",	0, Format::R32G32B32_FLOAT, 0, 16,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COORD",	0, Format::R32G32_FLOAT, 0, 28,		D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",	0, Format::R32G32B32_FLOAT, 0, 36,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL",0, Format::R32G32B32_FLOAT, 0, 48,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WORLD", 0, Format::R32G32B32A32_FLOAT, 1, 0,	D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 1, Format::R32G32B32A32_FLOAT, 1, 16, D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 2, Format::R32G32B32A32_FLOAT, 1, 32, D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 3, Format::R32G32B32A32_FLOAT, 1, 48, D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"COLOR_TONE", 0, Format::R32G32B32_FLOAT, 1, 64, D3D10_INPUT_PER_INSTANCE_DATA, 1},
	};

	ShaderInputElementDescription instanceshaderDescSHADOWMAP[] = 
	{
		{"POSITION",0, Format::R32G32B32A32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",	0, Format::R32G32B32_FLOAT, 0, 16,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COORD",	0, Format::R32G32_FLOAT, 0, 28,		D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",	0, Format::R32G32B32_FLOAT, 0, 36,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL",0, Format::R32G32B32_FLOAT, 0, 48,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WORLD", 0, Format::R32G32B32A32_FLOAT, 1, 0,	D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 1, Format::R32G32B32A32_FLOAT, 1, 16, D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 2, Format::R32G32B32A32_FLOAT, 1, 32, D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 3, Format::R32G32B32A32_FLOAT, 1, 48, D3D10_INPUT_PER_INSTANCE_DATA, 1},
	};

	buff = m_ResProxy->getData("assets/shaders/GeoInstanceShader.hlsl");
	m_Shader["IGeometry"] = WrapperFactory::getInstance()->createShader(buff.data, buff.size, nullptr,
		"VS,PS", "5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER, instanceshaderDesc, 10);

	buff = m_ResProxy->getData("assets/shaders/ShadowMapGeometry.hlsl");
	m_Shader["ShadowMapGeometry"] = WrapperFactory::getInstance()->createShader(buff.data, buff.size, nullptr,
		"VS,PS", "5_0", ShaderType::VERTEX_SHADER| ShaderType::PIXEL_SHADER, instanceshaderDescSHADOWMAP, 9); 

	buff = m_ResProxy->getData("assets/shaders/SSAO.hlsl");
	m_Shader["SSAO"] = WrapperFactory::getInstance()->createShader(buff.data, buff.size,
		"VS,PS", "5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);

	buff = m_ResProxy->getData("assets/shaders/SSAO_Blur.hlsl");
	m_Shader["SSAO_Blur"] = WrapperFactory::getInstance()->createShader(buff.data, buff.size,
		"VS,PS", "5_0", ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);

	buff = m_ResProxy->getData("assets/shaders/LightPassAmbient.hlsl");
	D3D_SHADER_MACRO ambientDefine[2] = {{ "AMBIENT_STRENGTH", "0.315f" }, nullptr };
	m_Shader["Ambient"] = WrapperFactory::getInstance()->createShader(buff.data, buff.size,
		ambientDefine, "VS,PS", "5_0", ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);

	m_FogColor = std::string("0.15f,0.35f,0.6f");
	m_MinFogDistance = 3000.0f;
	m_MaxFogDistance = 30000.0f;
	recompileFogShader();
}

void DeferredRenderer::loadLightModels()
{
	ModelBinaryLoader modelLoader;
	Buff buff = m_ResProxy->getData("assets/LightModels/SpotLight.btx");
	modelLoader.loadBinaryFromMemory(buff.data, buff.size);
	const std::vector<StaticVertex>& vertices = modelLoader.getStaticVertexBuffer();
	std::vector<DirectX::XMFLOAT3> temp;
	for(unsigned int i = 0; i < vertices.size(); i++)
	{
		temp.push_back(DirectX::XMFLOAT3(vertices.at(i).m_Position.x,vertices.at(i).m_Position.y,vertices.at(i).m_Position.z));
	}

	Buffer::Description cbdesc;
	cbdesc.initData = temp.data();
	cbdesc.numOfElements = temp.size();
	cbdesc.sizeOfElement = sizeof(DirectX::XMFLOAT3);
	cbdesc.type = Buffer::Type::VERTEX_BUFFER;
	cbdesc.usage = Buffer::Usage::USAGE_IMMUTABLE;
	VRAMInfo::getInstance()->updateUsage(sizeof(XMFLOAT3) * temp.size());

	m_Buffer["SpotLightModel"] = WrapperFactory::getInstance()->createBuffer(cbdesc);
	temp.clear();
	buff = m_ResProxy->getData("assets/LightModels/Sphere2.btx");
	modelLoader.loadBinaryFromMemory(buff.data, buff.size);
	for(unsigned int i = 0; i < vertices.size(); i++)
	{
		temp.push_back(DirectX::XMFLOAT3(vertices.at(i).m_Position.x,vertices.at(i).m_Position.y,vertices.at(i).m_Position.z));
	}

	cbdesc.initData = temp.data();
	cbdesc.numOfElements = temp.size();
	m_Buffer["PointLightModel"] = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(XMFLOAT3) * temp.size());
	temp.clear();
	modelLoader.clear();

	//Create a quad
	temp.push_back(DirectX::XMFLOAT3(-1,1,0));
	temp.push_back(DirectX::XMFLOAT3(-1,-1,0));
	temp.push_back(DirectX::XMFLOAT3(1,1,0));
	temp.push_back(DirectX::XMFLOAT3(-1,-1,0));
	temp.push_back(DirectX::XMFLOAT3(1,-1,0));
	temp.push_back(DirectX::XMFLOAT3(1,1,0));
	cbdesc.initData = temp.data();
	cbdesc.numOfElements = 6;
	m_Buffer["DirectionalLightModel"] = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(XMFLOAT3) * temp.size());

	temp.clear();
	temp.shrink_to_fit();
}

void DeferredRenderer::createLightStates()
{
	D3D11_DEPTH_STENCIL_DESC dsdesc;
	ZeroMemory( &dsdesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
	dsdesc.DepthEnable = true;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsdesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	m_Device->CreateDepthStencilState(&dsdesc, &m_DepthState);

	D3D11_RASTERIZER_DESC rdesc;
	ZeroMemory( &rdesc, sizeof( D3D11_RASTERIZER_DESC ) );
	rdesc.FillMode = D3D11_FILL_SOLID;
	rdesc.CullMode = D3D11_CULL_FRONT;
	m_Device->CreateRasterizerState(&rdesc,&m_RasterState);
}

void DeferredRenderer::renderLight(Shader *p_Shader, Buffer* p_ModelBuffer, vector<Light> *p_Lights)
{
	if(p_Lights->size() > 0)
	{
		if(p_Lights->size() >  m_Buffer["LightConstant"]->getNumOfElements())
		{
			VRAMInfo::getInstance()->updateUsage(sizeof(DirectX::XMFLOAT4X4) * m_Buffer["LightConstant"]->getNumOfElements() * -1);
			SAFE_DELETE(m_Buffer["LightConstant"]);
			Buffer::Description instanceWorldDesc;
			instanceWorldDesc.initData = nullptr;
			instanceWorldDesc.numOfElements = p_Lights->size() + 5;
			instanceWorldDesc.sizeOfElement = sizeof(DirectX::XMFLOAT4X4);
			instanceWorldDesc.type = Buffer::Type::VERTEX_BUFFER;
			instanceWorldDesc.usage = Buffer::Usage::CPU_WRITE_DISCARD;
			m_Buffer["LightConstant"] = WrapperFactory::getInstance()->createBuffer(instanceWorldDesc);
			VRAMInfo::getInstance()->updateUsage(instanceWorldDesc.sizeOfElement * instanceWorldDesc.numOfElements);
		}

		UINT Offsets[2] = {0,0};
		ID3D11Buffer * buffers[] = {p_ModelBuffer->getBufferPointer(), m_Buffer["LightConstant"]->getBufferPointer()};
		UINT Stride[2] = {sizeof(DirectX::XMFLOAT3), sizeof(Light)};

		p_Shader->setShader();
		m_DeviceContext->IASetVertexBuffers(0,2,buffers,Stride, Offsets);
		D3D11_MAPPED_SUBRESOURCE ms;

		m_DeviceContext->Map(m_Buffer["LightConstant"]->getBufferPointer(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

		Light *ptr = (Light *)ms.pData;
		for(unsigned int i = 0; i < p_Lights->size(); i++)
			ptr[i] = p_Lights->at(i);

		m_DeviceContext->Unmap(m_Buffer["LightConstant"]->getBufferPointer(), NULL);

		m_DeviceContext->DrawInstanced(p_ModelBuffer->getNumOfElements(), p_Lights->size(),0,0);

		ID3D11Buffer* nullBuffers[] = { nullptr, nullptr };
		m_DeviceContext->IASetVertexBuffers(0, 2, nullBuffers, Stride, Offsets);
		p_Shader->unSetShader();
	}
}

void DeferredRenderer::renderAmbientLight(Buffer* p_ModelBuffer)
{
	m_Shader["Ambient"]->setShader();
	p_ModelBuffer->setBuffer(0);
	
	ID3D11ShaderResourceView *srvs[] = {m_SRV["Diffuse"]};
	m_DeviceContext->PSSetShaderResources(0, 1, srvs);

	m_DeviceContext->Draw(p_ModelBuffer->getNumOfElements(), 0);

	ID3D11ShaderResourceView* nullSrv = nullptr;
	m_DeviceContext->PSSetShaderResources(0, 1, &nullSrv);
	p_ModelBuffer->unsetBuffer(0);
	m_Shader["Ambient"]->unSetShader();
}

void DeferredRenderer::createRandomTexture(unsigned int p_Size)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	textureDesc.Width = textureDesc.Height = p_Size;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	std::default_random_engine randomizer;
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	vector<DirectX::XMFLOAT3> initData;

	for(unsigned int i = 0; i < p_Size * p_Size; i++)
	{
		XMFLOAT3 randomVec;
		XMVECTOR temp = XMVector3Normalize(XMVectorSet(distribution(randomizer),
			distribution(randomizer), distribution(randomizer), 0.0f));
		XMStoreFloat3(&randomVec, temp);

		initData.push_back(randomVec);
	}

	ID3D11Texture2D *texture;

	D3D11_SUBRESOURCE_DATA subData;
	subData.SysMemPitch = sizeof(DirectX::XMFLOAT3) * p_Size;
	subData.pSysMem = initData.data();

	m_Device->CreateTexture2D(&textureDesc, &subData, &texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC dssrvdesc = {};
	dssrvdesc.Format = textureDesc.Format;
	dssrvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dssrvdesc.Texture2D.MipLevels = 1;
	dssrvdesc.Texture2D.MostDetailedMip = 0;

	m_Device->CreateShaderResourceView(texture, &dssrvdesc, &m_SRV["SSAO_RandomVec"]);
	unsigned int size = VRAMInfo::getInstance()->calculateFormatUsage(textureDesc.Format,
		textureDesc.Width, textureDesc.Height);
	VRAMInfo::getInstance()->updateUsage(size);

	SAFE_RELEASE(texture);
}

void DeferredRenderer::renderObject(const Renderable &p_Object)
{
	if (!isVisible(p_Object))
		return;

	p_Object.model->vertexBuffer->setBuffer(0);

	if (p_Object.model->isAnimated)
	{
		cAnimatedObjectBuffer temp;
		temp.invTransposeWorld = p_Object.invTransposeWorld;

		const std::vector<DirectX::XMFLOAT4X4>* tempBones = p_Object.finalTransforms;
		for (unsigned int a = 0; a < tempBones->size(); a++)
			temp.boneTransform[a] = (*tempBones)[a];

		m_DeviceContext->UpdateSubresource(m_Buffer["AnimatedConstant"]->getBufferPointer(), NULL,NULL, &temp,NULL,NULL);
	}

	m_DeviceContext->UpdateSubresource(m_Buffer["ObjectConstant"]->getBufferPointer(), NULL,NULL, &p_Object.world,NULL,NULL);
	
	m_DeviceContext->UpdateSubresource(m_ColorShadingConstantBuffer->getBufferPointer(),
		NULL,NULL,p_Object.colorTone ,NULL,NULL);
	m_ColorShadingConstantBuffer->setBuffer(2);

	// Set shader.
	p_Object.model->shader->setShader();
	float data[] = { 1.0f, 1.0f, 1.f, 1.0f};
	p_Object.model->shader->setBlendState(m_BlendState2, data);

	const auto& materialSet = p_Object.model->materialSets[p_Object.materialSet].second;
	for(const auto& material : materialSet)
	{
		ID3D11ShaderResourceView *srvs[] =  {
			p_Object.model->diffuseTexture[material.textureIndex].second, 
			p_Object.model->normalTexture[material.textureIndex].second, 
			p_Object.model->specularTexture[material.textureIndex].second 
		};
		m_DeviceContext->PSSetShaderResources(0, 3, srvs);


		m_DeviceContext->Draw(material.numOfVertices, material.vertexStart);
	}

	p_Object.model->shader->setBlendState(0, data);
	p_Object.model->shader->unSetShader();
	p_Object.model->vertexBuffer->unsetBuffer(0);
}

void DeferredRenderer::SortRenderables( std::vector<Renderable> &animatedOrSingle, std::vector<std::vector<Renderable>> &instancedModels )
{
	std::sort(m_Objects.begin(),m_Objects.end(), [] (Renderable &a, Renderable &b)
	{ 
		return a.model > b.model;
	});

	for(unsigned int i = 0; i < m_Objects.size(); i++)
	{
		if(m_Objects.at(i).model->isAnimated)
			animatedOrSingle.push_back(std::move(m_Objects.at(i)));
		else
		{
			std::vector<Renderable> l;
			int current = i;
			int nr = 0;
			for(unsigned int j = i + 1; j < m_Objects.size();j++)
				if(m_Objects.at(current).model == m_Objects.at(j).model)
					nr++;
				else
					break;

			i += nr;

			if(nr >= 1)
			{
				l.reserve(nr);
				std::move(m_Objects.begin() + current, m_Objects.begin() + i + 1, std::back_inserter(l));
				instancedModels.push_back(l);
			}
			else
				animatedOrSingle.push_back(std::move(m_Objects.at(current)));
		}
	}

	std::sort(animatedOrSingle.begin(), animatedOrSingle.end(), [&](Renderable &a, Renderable &b)
		{
			DirectX::XMFLOAT3 aa = DirectX::XMFLOAT3(a.world._14,a.world._24,a.world._34);
			DirectX::XMFLOAT3 bb = DirectX::XMFLOAT3(b.world._14,b.world._24,b.world._34);

			DirectX::XMVECTOR aV = DirectX::XMLoadFloat3(&aa);
			DirectX::XMVECTOR bV = DirectX::XMLoadFloat3(&bb);
			DirectX::XMVECTOR eV = DirectX::XMLoadFloat3(&m_CameraPosition);

			using DirectX::operator -;
			DirectX::XMVECTOR aVeVLength = DirectX::XMVector3Length(aV - eV);
			DirectX::XMVECTOR bVeVLength = DirectX::XMVector3Length(bV - eV);

			return aVeVLength.m128_f32[0] > bVeVLength.m128_f32[0];
	});
}

void DeferredRenderer::RenderObjectsInstanced(const std::vector<Renderable> &p_Objects, Shader* p_Shader)
{
	if(p_Objects.size() >  m_Buffer["WorldInstance"]->getNumOfElements())
	{
		VRAMInfo::getInstance()->updateUsage(sizeof(cInstanceBuffer) * m_Buffer["WorldInstance"]->getNumOfElements() * -1);
		SAFE_DELETE(m_Buffer["WorldInstance"]);
		Buffer::Description instanceWorldDesc;
		instanceWorldDesc.initData = nullptr;
		instanceWorldDesc.numOfElements = p_Objects.size() + 5;
		instanceWorldDesc.sizeOfElement = sizeof(cInstanceBuffer);
		instanceWorldDesc.type = Buffer::Type::VERTEX_BUFFER;
		instanceWorldDesc.usage = Buffer::Usage::CPU_WRITE_DISCARD;
		m_Buffer["WorldInstance"] = WrapperFactory::getInstance()->createBuffer(instanceWorldDesc);
		VRAMInfo::getInstance()->updateUsage(instanceWorldDesc.sizeOfElement * instanceWorldDesc.numOfElements);
	}

	UINT Offsets[2] = {0,0};
	ID3D11Buffer * buffers[] = {p_Objects.front().model->vertexBuffer->getBufferPointer(), m_Buffer["WorldInstance"]->getBufferPointer()};
	UINT Stride[2] = {60, sizeof(cInstanceBuffer)};

	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0};

	// Set shader.
	p_Shader->setShader();
	float data[] = { 1.0f, 1.0f, 1.f, 1.0f};
	p_Shader->setBlendState(m_BlendState2, data);
	m_DeviceContext->IASetVertexBuffers(0,2,buffers,Stride, Offsets);

	D3D11_MAPPED_SUBRESOURCE ms;

	m_DeviceContext->Map(m_Buffer["WorldInstance"]->getBufferPointer(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

	cInstanceBuffer *ptr = (cInstanceBuffer *)ms.pData;
	size_t numVisible = 0;
	for(unsigned int j = 0; j < p_Objects.size(); j++)
	{
		if (!isVisible(p_Objects[j]))
			continue;

		ptr[numVisible].world = p_Objects[j].world;
		ptr[numVisible].colorTone = *p_Objects[j].colorTone;
		++numVisible;
	}

	m_DeviceContext->Unmap(m_Buffer["WorldInstance"]->getBufferPointer(), NULL);

	const auto& materialSet = p_Objects.front().model->materialSets[p_Objects.front().materialSet].second;
	for(const auto& material : materialSet)
	{
		ID3D11ShaderResourceView *srvs[] =  {
			p_Objects.front().model->diffuseTexture[material.textureIndex].second, 
			p_Objects.front().model->normalTexture[material.textureIndex].second, 
			p_Objects.front().model->specularTexture[material.textureIndex].second 
		};
		m_DeviceContext->PSSetShaderResources(0, 3, srvs);
		
		m_DeviceContext->DrawInstanced(material.numOfVertices, numVisible, material.vertexStart,0);
		
		m_DeviceContext->PSSetShaderResources(0, 3, nullsrvs);
	}

	ID3D11Buffer* nullBuffers[] = { nullptr, nullptr };
	m_DeviceContext->IASetVertexBuffers(0, 2, nullBuffers, Stride, Offsets);
	p_Shader->setBlendState(0, data);
	p_Shader->unSetShader();
}

void DeferredRenderer::updateLightView(DirectX::XMFLOAT3 p_Dir)
{
	XMVECTOR pos = XMVectorSet(m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z, 1.f);
	XMStoreFloat4x4(&m_LightView, XMMatrixTranspose(XMMatrixLookToLH(pos, XMVectorSet(p_Dir.x, p_Dir.y, p_Dir.z ,0.0f) , XMVectorSet(0, 1, 0, 0))));
}

void DeferredRenderer::updateLightProjection(float p_viewHW)
{
	m_ViewHW = p_viewHW; // size of viewHeight and viewWidth
	XMStoreFloat4x4(&m_LightProjection, XMMatrixTranspose(XMMatrixOrthographicLH(m_ViewHW, m_ViewHW, -20000.f, 20000)));
}

void DeferredRenderer::renderShadowMap(Light p_Directional, const std::vector<std::vector<Renderable>> &p_InstancedModels, const std::vector<Renderable> &p_AnimatedOrSingle)
{
	ID3D11DepthStencilState* previousDepthState;
	m_DeviceContext->OMGetDepthStencilState(&previousDepthState,0);

	//Shadow Map
	unsigned int nrRT = 0;
	ID3D11RenderTargetView *noRTV = nullptr;
	
	ID3D11ShaderResourceView *srvs[] = {m_SRV["WPosition"], m_SRV["Normal"], m_SRV["Diffuse"], m_SRV["SSAO"], m_SRV["CSM"]};
	ID3D11ShaderResourceView *nullSrvs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	UINT sampleMask = 0xffffffff;

	Light& dirLight = p_Directional;
	updateLightView(dirLight.lightDirection); //light 0

	for(int j = 0; j < 2; j++)
	{
		m_DeviceContext->ClearDepthStencilView(m_DepthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

		//create new viewport
		D3D11_VIEWPORT prevViewport;
		UINT numViewPort = 1;
		m_DeviceContext->RSGetViewports(&numViewPort, &prevViewport);
		m_DeviceContext->RSSetViewports(1, &m_LightViewport);

		if(j==0)
			updateLightProjection(m_ShadowBigSize);
		else
			updateLightProjection(m_ShadowSmallSize);

		//update and render Shadow map
		updateConstantBuffer(m_LightView, m_LightProjection);

		renderGeometry(m_DepthMapDSV, nrRT, &noRTV, p_InstancedModels, p_AnimatedOrSingle, m_Shader["ShadowMapGeometry"]);

		m_DeviceContext->RSSetViewports(1, &prevViewport);

		// Set texture sampler.
		m_DeviceContext->PSSetShaderResources(0, 5, srvs);

		m_DeviceContext->PSSetSamplers(0, 1, &m_Sampler["ShadowMap"]);

		//Select the final render target
		m_DeviceContext->OMSetRenderTargets(1, &m_RT[IGraphics::RenderTarget::LIGHT], m_DepthStencilView); 

		m_DeviceContext->OMSetDepthStencilState(m_DepthState,0);
		m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, sampleMask);

		//Render DirectionalLights
		m_DeviceContext->OMSetRenderTargets(1, &m_RT[IGraphics::RenderTarget::LIGHT], 0);

		//Render Shadow Map
		if(j == 0)
			updateLightBuffer(true,true);
		else
			updateLightBuffer(false, true);

		updateConstantBuffer(*m_ViewMatrix, *m_ProjectionMatrix);

		//Set constant data
		m_Buffer["DefaultConstant"]->setBuffer(0);
		m_Buffer["LightViewProj"]->setBuffer(1);

		m_Buffer["DirectionalLightModel"]->setBuffer(0);
		m_Buffer["LightConstant"]->setBuffer(1);

		m_Shader["DirectionalLight"]->setShader();

		D3D11_MAPPED_SUBRESOURCE ms;
		m_DeviceContext->Map(m_Buffer["LightConstant"]->getBufferPointer(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &dirLight, sizeof(Light));
		m_DeviceContext->Unmap(m_Buffer["LightConstant"]->getBufferPointer(), NULL);

		m_DeviceContext->Draw(m_Buffer["DirectionalLightModel"]->getNumOfElements(), 0);

		m_Shader["DirectionalLight"]->unSetShader();

		m_DeviceContext->PSSetShaderResources(0, 5, nullSrvs);
		m_DeviceContext->OMSetDepthStencilState(previousDepthState,0);
		m_DeviceContext->OMSetBlendState(0, blendFactor, sampleMask);
	}

	m_Buffer["LightViewProj"]->unsetBuffer(1);
	SAFE_RELEASE(previousDepthState);
}

void DeferredRenderer::registerTweakSettings()
{
	TweakSettings* settings = TweakSettings::getInstance();

	settings->setSetting("ssao.radius", 15.f);
	settings->setListener("ssao.radius", std::function<void(float)>(
		[&] (float)
	{
		updateSSAO_VarConstantBuffer();
	}
	));

	settings->setSetting("ssao.epsilon", 4.f);
	settings->setListener("ssao.epsilon", std::function<void(float)>(
		[&] (float)
	{
		updateSSAO_VarConstantBuffer();
	}
	));

	settings->setSetting("ssao.fadeEnd", 10.f);
	settings->setListener("ssao.fadeEnd", std::function<void(float)>(
		[&] (float)
	{
		updateSSAO_VarConstantBuffer();
	}
	));

	settings->setSetting("ssao.fadeStart", 3.f);
	settings->setListener("ssao.fadeStart", std::function<void(float)>(
		[&] (float)
	{
		updateSSAO_VarConstantBuffer();
	}
	));
	settings->setListener("shadows.maxDirectional", std::function<void(int)>(
		[&] (int p_Value)
	{
		if (m_MaxNumDirectionalShadows >= 0)
		{
			m_MaxNumDirectionalShadows = p_Value;
		}
	}
	));
	settings->setSetting("shadows.maxDirectional", 1);

	settings->setSetting("light.ambient", 0.315f);
	settings->setListener("light.ambient", std::function<void(float)>(
		[&] (float p_Value)
		{
			SAFE_DELETE(m_Shader["Ambient"]);

			std::string strength = std::to_string(p_Value) + 'f';
			Buff buff = m_ResProxy->getData("assets/shaders/LightPassAmbient.hlsl");
			D3D_SHADER_MACRO ambientDefine[2] = {{ "AMBIENT_STRENGTH", strength.c_str() }, nullptr };
			m_Shader["Ambient"] = WrapperFactory::getInstance()->createShader(buff.data, buff.size,
				ambientDefine, "VS,PS", "5_0", ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);
		}
	));

	settings->setSetting("fog.color", m_FogColor);
	settings->setListener("fog.color", std::function<void(const std::string&)>(
		[&] (const std::string& p_Value)
	{
		m_FogColor = p_Value;
		recompileFogShader();
	}
	));

	settings->setSetting("fog.minDistance", m_MinFogDistance);
	settings->setListener("fog.minDistance", std::function<void(float)>(
		[&] (float p_Value)
	{
		m_MinFogDistance = p_Value;
		recompileFogShader();
	}
	));
	
	settings->setSetting("fog.maxDistance", m_MaxFogDistance);
	settings->setListener("fog.maxDistance", std::function<void(float)>(
		[&] (float p_Value)
	{
		m_MaxFogDistance = p_Value;
		recompileFogShader();
	}
	));
}

void DeferredRenderer::recompileFogShader(void)
{

	std::string color = "float4(" + m_FogColor + ",1.0f)";
	std::string minDistance = std::to_string(m_MinFogDistance) + "f";
	std::string maxDistance = std::to_string(m_MaxFogDistance) + "f";
	D3D_SHADER_MACRO fogDefine[4] =
	{
		{ "FOG_COLOR", color.c_str() },
		{ "MIN_DISTANCE", minDistance.c_str() },
		{ "MAX_DISTANCE", maxDistance.c_str() },
		nullptr
	};

	Shader *tempFogShader = nullptr;
	try
	{
		Buff buff = m_ResProxy->getData("assets/shaders/DistanceFog.hlsl");
		tempFogShader = WrapperFactory::getInstance()->createShader(buff.data, buff.size,
			fogDefine, "DistanceFogVS,DistanceFogPS", "5_0", ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);
	}
	catch (...)
	{
		return;
	}

	SAFE_DELETE(m_Shader["DistanceFog"]);
	m_Shader["DistanceFog"] = tempFogShader;
}

bool DeferredRenderer::isVisible(const Renderable& p_Object) const
{
	XMMATRIX world = XMLoadFloat4x4(&p_Object.world);
	XMMATRIX view = XMLoadFloat4x4(&m_CurrentView);
	XMMATRIX proj = XMLoadFloat4x4(&m_CurrentProjection);
	XMMATRIX projWorldView = XMMatrixTranspose(proj * view * world);

	std::array<XMVECTOR, 8> corners;
	size_t i = 0;
	for (const auto& corner : p_Object.model->boundingVolume)
	{
		corners[i] = XMVector4Transform(XMVectorSetW(XMLoadFloat3(&corner), 1.f), projWorldView);
		corners[i] /= corners[i].m128_f32[3];

		++i;
	}

	static const std::tuple<int, float, float> planes[] =
	{
		std::make_tuple(0, 4.f, 1.f),
		std::make_tuple(0, 4.f, -1.f),
		std::make_tuple(1, 4.f, 1.f),
		std::make_tuple(1, 4.f, -1.f),
		std::make_tuple(2, 4.f, 1.f),
		std::make_tuple(2, 0.f, -1.f)
	};

	for (const auto& plane : planes)
	{
		bool outside = true;
		for (const auto& corner : corners)
		{
			if (corner.m128_f32[std::get<0>(plane)] * std::get<2>(plane) <= std::get<1>(plane))
			{
				outside = false;
				break;
			}
		}

		if (outside)
			return false;
	}

	return true;
}
