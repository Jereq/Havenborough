#pragma once
#include "ResourceProxy.h"
#include "WrapperFactory.h"

#include <DirectXMath.h>
#include <vector>
#include <d3d11.h>

class SkyDome
{
private:
	ID3D11DeviceContext			*m_DeviceContext;

	Buffer						*m_SkyDomeBuffer;
	Shader						*m_SkyDomeShader;
	ID3D11ShaderResourceView	*m_SkyDomeSRV;
	ID3D11DepthStencilState		*m_SkyDomeDepthStencilState;
	ID3D11RasterizerState		*m_SkyDomeRasterizerState;
	ID3D11SamplerState			*m_SkyDomeSampler;

public:
	/**
	* Constructor.
	*/
	SkyDome();
	
	/**
	* Destructor.
	*/
	~SkyDome();
	
	/**
	* Initialize the sky dome 
	* @param p_Device, the DirectX device to be used
	* @param p_DeviceContext, the DirectX context to be used
	* @param p_Texture, the texture to use for the skydome
	* @param p_Radius the size of the sphere's radius in cm
	*/
	bool init(ID3D11Device *p_Device, ID3D11DeviceContext *p_DeviceContext, 
		ID3D11ShaderResourceView* p_Texture, float p_Radius, ResourceProxy* p_ResProxy);

	/**
	* Initialize the sky dome 
	* @param p_RenderTarget, the render target to be drawn to
	* @param p_DepthStencilView, used for depth comparison in pixel shader
	* @param p_ConstantBuffer, view, projection, camera position matrices used by the skydome shader
	*/
	void RenderSkyDome(ID3D11RenderTargetView *p_RenderTarget, ID3D11DepthStencilView *p_DepthStencilView,
		Buffer *p_ConstantBuffer);
	
private:

	std::vector<DirectX::XMFLOAT3> buildGeoSphere(unsigned int p_NumSubDivisions, float p_Radius);
	void subdivide(std::vector<DirectX::XMFLOAT3> &p_Vertices, std::vector<unsigned int> &p_InitIndices);
};