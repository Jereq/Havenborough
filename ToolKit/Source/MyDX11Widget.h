#pragma once
#ifdef _DEBUG
#pragma comment(lib, "Graphicsd.lib")
#pragma comment(lib, "Commond.lib")
#else
#pragma comment(lib, "Graphics.lib")
#pragma comment(lib, "Common.lib")
#endif
#pragma comment(lib, "d3d11.lib")

#include "DXWidget.h"

#include <IGraphics.h>
#include <ResourceManager.h>
#include <TweakCommand.h>

class MyDX11Widget : public DXWidget
{
private:
	IGraphics* m_Graphics;
	ResourceManager m_ResourceManager;
	int m_SkyboxID;
	int m_HouseRes;
	IGraphics::InstanceId m_HouseInst;

public:
	explicit MyDX11Widget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0)
		: DXWidget(parent, flags),
		m_Graphics(nullptr)
	{
		initialize();
	}

	virtual ~MyDX11Widget()
	{
		uninitialize();
	}

	void initialize() override
	{
		TweakSettings::initializeMaster();


		m_Graphics = IGraphics::createGraphics();
		m_Graphics->setTweaker(TweakSettings::getInstance());
		m_Graphics->initialize((HWND)winId(), width(), height(), false, 60.f);

		m_Graphics->setLoadModelTextureCallBack(&ResourceManager::loadModelTexture, &m_ResourceManager);
		m_Graphics->setReleaseModelTextureCallBack(&ResourceManager::releaseModelTexture, &m_ResourceManager);

		using std::placeholders::_1;
		using std::placeholders::_2;
		m_ResourceManager.registerFunction("model",
			std::bind(&IGraphics::createModel, m_Graphics, _1, _2),
			std::bind(&IGraphics::releaseModel, m_Graphics, _1) );
		m_ResourceManager.registerFunction("texture",
			std::bind(&IGraphics::createTexture, m_Graphics, _1, _2),
			std::bind(&IGraphics::releaseTexture, m_Graphics, _1));
		m_ResourceManager.loadDataFromFile("assets/Resources.xml");

		m_Graphics->setClearColor(Vector4(1.f, 0.f, 1.f, 1.f));
		//m_Graphics->setRenderTarget((IGraphics::RenderTarget)7);

		m_SkyboxID = m_ResourceManager.loadResource("texture","SKYBOXDDS");
		m_Graphics->createSkydome("SKYBOXDDS", 10000.f);

		m_HouseRes = m_ResourceManager.loadResource("model", "House10");
		m_HouseInst = m_Graphics->createModelInstance("House10");
		m_Graphics->setModelPosition(m_HouseInst, Vector3(0.f, 0.f, 2000.f));
	}

	void uninitialize() override
	{
		m_ResourceManager.setReleaseImmediately(true);
		
		m_ResourceManager.releaseResource(m_SkyboxID);
		m_Graphics->eraseModelInstance(m_HouseInst);
		m_ResourceManager.releaseResource(m_HouseRes);

		m_ResourceManager.unregisterResourceType("model");
		m_ResourceManager.unregisterResourceType("texture");

		if (m_Graphics)
		{
			invalidateDeviceObjects();

			IGraphics::deleteGraphics(m_Graphics);
			m_Graphics = nullptr;
		}
	}

	void restoreDeviceObjects() override
	{
	}

	void invalidateDeviceObjects() override
	{
	}

	void render() override
	{
		//if (m_StandBy)
		//{
		//	present();
		//	return;
		//}
		
		m_Graphics->updateCamera(m_Camera.getPosition(), m_Camera.getForward(), m_Camera.getUp());
		m_Graphics->renderSkydome();

		m_Graphics->renderModel(m_HouseInst);

		m_Graphics->useFrameDirectionalLight(Vector3(1.f, 1.f, 1.f), Vector3(0.3f, -0.8f, 0.f), 1.f);

		present();
	}

	void present() override
	{
		if (m_Graphics)
		{
			m_Graphics->drawFrame();
		}

		//m_StandBy =  (hr == DXGI_STATUS_OCCLUDED);

		//if (hr == DXGI_ERROR_DEVICE_RESET ||
		//	hr == DXGI_ERROR_DEVICE_REMOVED)
		//{
		//	uninitialize();
		//	hr = initialize();
		//}
	}

	void onResize(unsigned int nWidth, unsigned int nHeight) override
	{
		m_Graphics->resize(nWidth, nHeight);

		render();
	}
};
