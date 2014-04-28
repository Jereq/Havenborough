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
#include <TweakCommand.h>
#include <D3D11.h>

class MyDX11Widget : public DXWidget
{
private:
	IGraphics* m_Graphics;

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

		m_Graphics->setClearColor(Vector4(1.f, 0.f, 1.f, 1.f));
		m_Graphics->setRenderTarget((IGraphics::RenderTarget)7);
	}

	void uninitialize() override
	{
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
		//m_Graphics->reInitialize((HWND)winId(), nWidth, nHeight, false);

		render();
	}
};
