#include "BillboardRenderer.h"
#include "ConstantBuffers.h"
#include "WrapperFactory.h"
#include "VRAMInfo.h"
#include "Utilities/MemoryUtil.h"
#include <algorithm>
#include <functional>

BillboardRenderer::BillboardRenderer(void)
{
	m_Device = nullptr;
	m_DeviceContext = nullptr;

	m_DepthStencilView = nullptr;
	m_RenderTarget = nullptr;
	m_Sampler = nullptr;
	m_RasterState = nullptr;
	m_DepthStencilState = nullptr;

	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;

	m_ConstantBuffer = nullptr;
	m_ObjectConstantBuffer = nullptr;
	m_Shader = nullptr;

	m_TransparencyAdditiveBlend = nullptr;
}


BillboardRenderer::~BillboardRenderer(void)
{
	m_Device = nullptr;
	m_DeviceContext = nullptr;

	m_DepthStencilView = nullptr;
	m_RenderTarget = nullptr;
	SAFE_RELEASE(m_Sampler);
	SAFE_RELEASE(m_RasterState);
	SAFE_RELEASE(m_DepthStencilState);
	
	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;

	SAFE_DELETE(m_ConstantBuffer);
	SAFE_DELETE(m_ObjectConstantBuffer);
	SAFE_DELETE(m_Shader);

	SAFE_RELEASE(m_TransparencyAdditiveBlend);
}

void BillboardRenderer::init(ID3D11Device *p_Device, ID3D11DeviceContext *p_DeviceContext,
							DirectX::XMFLOAT3 p_CameraPosition, DirectX::XMFLOAT4X4 *p_ViewMatrix,
							DirectX::XMFLOAT4X4 *p_ProjectionMatrix, ID3D11DepthStencilView* p_DepthStencilView,
							ID3D11RenderTargetView *p_RenderTarget)
{
	m_Device = p_Device;
	m_DeviceContext = p_DeviceContext;

	m_DepthStencilView = p_DepthStencilView;
	m_RenderTarget = p_RenderTarget;

	m_CameraPosition = p_CameraPosition;
	m_ViewMatrix = p_ViewMatrix;
	m_ProjectionMatrix = p_ProjectionMatrix;

	createBlendStates();
	createForwardBuffers();
	createSampler();
	createRasterState();
	createDepthStencilState();

	m_Shader = WrapperFactory::getInstance()->createShader(L"assets/shaders/BillboardShader.hlsl", "VS,PS,GS", "5_0",
		ShaderType::VERTEX_SHADER | ShaderType::GEOMETRY_SHADER | ShaderType::PIXEL_SHADER);
}

void BillboardRenderer::resize(ID3D11DepthStencilView* p_DepthStencilView, ID3D11RenderTargetView *p_RenderTarget)
{
	m_DepthStencilView = p_DepthStencilView;
	m_RenderTarget = p_RenderTarget;
}

void BillboardRenderer::addRenderable(BillboardRenderable p_Renderable)
{
	m_Objects.push_back(p_Renderable);
}

void BillboardRenderer::createBlendStates()
{
	D3D11_BLEND_DESC bd;
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend =  D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_Device->CreateBlendState(&bd, &m_TransparencyAdditiveBlend);
}

void BillboardRenderer::createForwardBuffers()
{
	cBuffer cb;
	cb.view = *m_ViewMatrix;
	cb.proj = *m_ProjectionMatrix;
	cb.campos = m_CameraPosition;

	Buffer::Description cbdesc;
	cbdesc.initData = &cb;
	cbdesc.numOfElements = 1;
	cbdesc.sizeOfElement = sizeof(cBuffer);
	cbdesc.type = Buffer::Type::CONSTANT_BUFFER_ALL;
	cbdesc.usage = Buffer::Usage::DEFAULT;

	m_ConstantBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cBuffer));

	cbdesc.initData = nullptr;
	cbdesc.sizeOfElement = sizeof(cObjectTransform);
	cbdesc.type = Buffer::Type::VERTEX_BUFFER;
	m_ObjectConstantBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cObjectTransform));
}
void BillboardRenderer::createSampler()
{
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc	= D3D11_COMPARISON_NEVER;
	sd.MinLOD			= 0;
	sd.MaxLOD           = D3D11_FLOAT32_MAX;

	m_Sampler = nullptr;
	m_Device->CreateSamplerState( &sd, &m_Sampler );
}

void BillboardRenderer::createRasterState()
{
	D3D11_RASTERIZER_DESC rasterDesc;

	//Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	m_Device->CreateRasterizerState(&rasterDesc, &m_RasterState);
}

void BillboardRenderer::createDepthStencilState(void)
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

	//Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	//Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	//Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
}

void BillboardRenderer::updateConstantBuffer()
{
	cBuffer cb;
	cb.view = *m_ViewMatrix;
	cb.proj = *m_ProjectionMatrix;
	cb.campos = m_CameraPosition;
	m_DeviceContext->UpdateSubresource(m_ConstantBuffer->getBufferPointer(), NULL,NULL, &cb,NULL,NULL);
}

void BillboardRenderer::renderBillboards()
{
	if(m_Objects.size() > 0)
	{
				ID3D11RasterizerState *previousRasterState;
		ID3D11DepthStencilState *previousDepthState;
		m_DeviceContext->RSGetState(&previousRasterState);
		m_DeviceContext->OMGetDepthStencilState(&previousDepthState,0);

		m_DeviceContext->RSSetState(m_RasterState);
		m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState,0);

		//Sort objects by the range to the camera
		std::sort(m_Objects.begin(),m_Objects.end(),std::bind(&BillboardRenderer::depthSortCompareFunc, this, std::placeholders::_1, std::placeholders::_2));

		// Set the render targets.
		m_DeviceContext->OMSetRenderTargets(1, &m_RenderTarget, m_DepthStencilView);
		m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		m_DeviceContext->PSSetSamplers(0, 1, &m_Sampler);

		updateConstantBuffer();
		m_ConstantBuffer->setBuffer(0);
		m_Shader->setShader();
		m_ObjectConstantBuffer->setBuffer(0);
		float data[] = { 1.0f, 1.0f, 1.f, 1.0f};
		m_Shader->setBlendState(m_TransparencyAdditiveBlend, data);
		for(auto& object : m_Objects)
		{
			cObjectTransform trans(object.m_Position, object.m_Size, object.m_Scale, object.m_Rotation);
			m_DeviceContext->UpdateSubresource(m_ObjectConstantBuffer->getBufferPointer(), 0, nullptr, &trans, 0, 0);
			m_DeviceContext->PSSetShaderResources(0, 1, &object.m_Texture);

			m_DeviceContext->Draw(1,0);
		}
		m_Shader->setBlendState(0, data);
		m_ObjectConstantBuffer->unsetBuffer(0);
		m_Shader->unSetShader();
		m_ConstantBuffer->unsetBuffer(0);
		// Unset render targets.
		ID3D11ShaderResourceView *nullsrv[] = {NULL};
		m_DeviceContext->PSSetShaderResources(0, 1, nullsrv);

		m_DeviceContext->OMSetRenderTargets(0, 0, 0);
		m_DeviceContext->RSSetState(previousRasterState);
		m_DeviceContext->OMSetDepthStencilState(previousDepthState,0);
		float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
		UINT sampleMask = 0xffffffff;
		m_DeviceContext->OMSetBlendState(0, blendFactor, sampleMask);
		SAFE_RELEASE(previousRasterState);
		SAFE_RELEASE(previousDepthState);
		m_Objects.clear();
	}
}

void BillboardRenderer::updateCamera(const DirectX::XMFLOAT3& p_CameraPos)
{
	m_CameraPosition = p_CameraPos;
}

bool BillboardRenderer::depthSortCompareFunc(const BillboardRenderable &a, const BillboardRenderable &b)
{
	DirectX::XMVECTOR aV = DirectX::XMLoadFloat3(&a.m_Position);
	DirectX::XMVECTOR bV = DirectX::XMLoadFloat3(&b.m_Position);
	DirectX::XMVECTOR eV = DirectX::XMLoadFloat3(&m_CameraPosition);
	
	using DirectX::operator -;
	DirectX::XMVECTOR aVeVLength = DirectX::XMVector3Length(aV - eV);
	DirectX::XMVECTOR bVeVLength = DirectX::XMVector3Length(bV - eV);

	return aVeVLength.m128_f32[0] > bVeVLength.m128_f32[0];
}