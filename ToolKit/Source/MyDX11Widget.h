#pragma once
#ifdef _DEBUG
#pragma comment(lib, "Graphicsd.lib")
#pragma comment(lib, "Commond.lib")
#pragma comment(lib, "Physicsd.lib")
#else
#pragma comment(lib, "Graphics.lib")
#pragma comment(lib, "Common.lib")
#pragma comment(lib, "Physics.lib")
#endif
#pragma comment(lib, "d3d11.lib")

#include "DXWidget.h"

#include <EventData.h>
#include <IGraphics.h>
#include <IPhysics.h>
#include <ResourceManager.h>
#include <TweakCommand.h>

class MyDX11Widget : public DXWidget
{
	Q_OBJECT
private:
	IGraphics* m_Graphics;
	IPhysics* m_Physics;
	int m_SkyboxID;
	
	std::vector<int> m_ResourceIDs;
	std::vector<LightClass> m_Lights;

	struct ReachIK
	{
		std::string group;
		Vector3 target;
	};

	struct MeshBinding
	{
		unsigned int meshId;
		int resourceId;
		IGraphics::InstanceId modelId;
		std::vector<ReachIK> activeIKs;
	};

	std::vector<MeshBinding> m_Models;	struct ParticleBinding
	{
		std::string effectName;
		IGraphics::InstanceId instance;
	};
	std::map<unsigned int, ParticleBinding> m_Particles;

public:
	explicit MyDX11Widget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0)
		: DXWidget(parent, flags),
		m_Graphics(nullptr),
		m_Physics(nullptr)
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
		m_Graphics->setShadowMapResolution(1024);
		m_Graphics->enableShadowMap(true);
		m_Graphics->initialize((HWND)winId(), width(), height(), false, 60.f);

		m_Graphics->setLoadModelTextureCallBack(&ResourceManager::loadModelTexture, &m_ResourceManager);
		m_Graphics->setReleaseModelTextureCallBack(&ResourceManager::releaseModelTexture, &m_ResourceManager);

		m_Physics = IPhysics::createPhysics();
		m_Physics->initialize(false, 1.f / 60.f);

		using std::placeholders::_1;
		using std::placeholders::_2;
		m_ResourceManager.registerFunction("model",
			std::bind(&IGraphics::createModel, m_Graphics, _1, _2),
			std::bind(&IGraphics::releaseModel, m_Graphics, _1) );
		m_ResourceManager.registerFunction("texture",
			std::bind(&IGraphics::createTexture, m_Graphics, _1, _2),
			std::bind(&IGraphics::releaseTexture, m_Graphics, _1));
		m_ResourceManager.registerFunction("volume",
			std::bind(&IPhysics::createBV, m_Physics, _1, _2),
			std::bind(&IPhysics::releaseBV, m_Physics, _1));
		m_ResourceManager.registerFunction("particleSystem",
			std::bind(&IGraphics::createParticleEffectDefinition, m_Graphics, _1, _2),
			std::bind(&IGraphics::releaseParticleEffectDefinition, m_Graphics, _1));
		m_ResourceManager.loadDataFromFile("assets/Resources.xml");

		ActorFactory::ptr actorFactory(new ActorFactory(0));
		actorFactory->setPhysics(m_Physics);
		actorFactory->setEventManager(&m_EventManager);
		actorFactory->setResourceManager(&m_ResourceManager);
		m_ObjectManager.reset(new ObjectManager(actorFactory, &m_EventManager, &m_ResourceManager));

		m_SkyboxID = m_ResourceManager.loadResource("texture","SKYBOXDDS");
		m_Graphics->createSkydome("SKYBOXDDS", 500000.f);

		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::addLight), LightEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::removeLight), RemoveLightEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::createMesh), CreateMeshEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::removeMesh), RemoveMeshEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::updateModelPosition), UpdateModelPositionEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::updateModelRotation), UpdateModelRotationEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::updateModelScale), UpdateModelScaleEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::updateAnimation), UpdateAnimationEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::changeColorTone), ChangeColorToneEvent::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::createParticleEffect), CreateParticleEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::removeParticleEffectInstance), RemoveParticleEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::updateParticlePosition), UpdateParticlePositionEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::updateParticleRotation), UpdateParticleRotationEventData::sk_EventType);
		m_EventManager.addListener(EventListenerDelegate(this, &MyDX11Widget::updateParticleBaseColor), UpdateParticleBaseColorEventData::sk_EventType);

		m_ResourceIDs.push_back(m_ResourceManager.loadResource("particleSystem", "TestParticle"));
	}

	void uninitialize() override
	{
		m_ResourceManager.setReleaseImmediately(true);

		m_ObjectManager.reset();

		for (int res : m_ResourceIDs)
		{
			m_ResourceManager.releaseResource(res);
		}
		m_ResourceIDs.clear();
		m_ResourceManager.releaseResource(m_SkyboxID);

		m_ResourceManager.unregisterResourceType("model");
		m_ResourceManager.unregisterResourceType("texture");

		if (m_Graphics)
		{
			invalidateDeviceObjects();

			IGraphics::deleteGraphics(m_Graphics);
			m_Graphics = nullptr;
		}

		if (m_Physics)
		{
			IPhysics::deletePhysics(m_Physics);
			m_Physics = nullptr;
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

		for (auto& mesh : m_Models)
		{
			m_Graphics->renderModel(mesh.modelId);
		}

		for(auto &light : m_Lights)
		{
			switch(light.type)
			{
			case LightClass::Type::DIRECTIONAL:
				{
					m_Graphics->useFrameDirectionalLight(light.color, light.direction, light.intensity);
					break;
				}
			case LightClass::Type::POINT:
				{
					m_Graphics->useFramePointLight(light.position, light.color, light.range);

					break;
				}
			case LightClass::Type::SPOT:
				{
					m_Graphics->useFrameSpotLight(light.position, light.color, light.direction,
						light.spotlightAngles, light.range);

					break;
				}
			}
		}

		m_Graphics->renderSkydome();

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

	void updateStep(float p_DeltaTime) override
	{
		DXWidget::updateStep(p_DeltaTime);
		m_Graphics->updateParticles(p_DeltaTime);
	}

	void onResize(unsigned int nWidth, unsigned int nHeight) override
	{
		m_Graphics->resize(nWidth, nHeight);

		render();
	}
	
	void addLight(IEventData::Ptr p_Data)
	{
		std::shared_ptr<LightEventData> lightData = std::static_pointer_cast<LightEventData>(p_Data);
		LightClass light = lightData->getLight();
		m_Lights.push_back(light);
	}

	void removeLight(IEventData::Ptr p_Data)
	{
		std::shared_ptr<RemoveLightEventData> lightData = std::static_pointer_cast<RemoveLightEventData>(p_Data);

		auto remIt = std::remove_if(m_Lights.begin(), m_Lights.end(),
			[&lightData] (LightClass& p_Light)
			{
				return p_Light.id == lightData->getId();
			});
		m_Lights.erase(remIt, m_Lights.end());
	}

	void createMesh(IEventData::Ptr p_Data)
	{
		std::shared_ptr<CreateMeshEventData> meshData = std::static_pointer_cast<CreateMeshEventData>(p_Data);

		int resource = m_ResourceManager.loadResource("model", meshData->getMeshName());
		m_ResourceIDs.push_back(resource);

		MeshBinding mesh =
		{
			meshData->getId(),
			resource,
			m_Graphics->createModelInstance(meshData->getMeshName().c_str())
		};
		m_Graphics->setModelScale(mesh.modelId, meshData->getScale());
		m_Graphics->setModelColorTone(mesh.modelId, meshData->getColorTone());
		m_Graphics->setModelStyle(mesh.modelId, meshData->getStyle().c_str());
	
		m_Models.push_back(mesh);

		//emit meshCreated(meshData->getMeshName());
	}

	void removeMesh(IEventData::Ptr p_Data)
	{
		std::shared_ptr<RemoveMeshEventData> meshData = std::static_pointer_cast<RemoveMeshEventData>(p_Data);

		for (auto& model : m_Models)
		{
			if (model.meshId == meshData->getId())
			{
				m_ResourceManager.releaseResource(model.resourceId);
				auto it = std::find(m_ResourceIDs.begin(), m_ResourceIDs.end(), model.resourceId);
				if (it != m_ResourceIDs.end())
				{
					m_ResourceIDs.erase(it);
				}

				m_Graphics->eraseModelInstance(model.modelId);

				std::swap(model, m_Models.back());
				m_Models.pop_back();
				return;
			}
		}
	}

	void updateModelPosition(IEventData::Ptr p_Data)
	{
		std::shared_ptr<UpdateModelPositionEventData> positionData = std::static_pointer_cast<UpdateModelPositionEventData>(p_Data);
		for(auto &model : m_Models)
		{
			if(model.meshId == positionData->getId())
			{
				m_Graphics->setModelPosition(model.modelId, positionData->getPosition());
			}
		}
	}

	void updateModelRotation(IEventData::Ptr p_Data)
	{
		std::shared_ptr<UpdateModelRotationEventData> rotationData = std::static_pointer_cast<UpdateModelRotationEventData>(p_Data);
		for(auto &model : m_Models)
		{
			if(model.meshId == rotationData->getId())
			{
				m_Graphics->setModelRotation(model.modelId, rotationData->getRotation());
			}
		}
	}

	void updateModelScale(IEventData::Ptr p_Data)
	{
		std::shared_ptr<UpdateModelScaleEventData> scaleData = std::static_pointer_cast<UpdateModelScaleEventData>(p_Data);
		for(auto &model : m_Models)
		{
			if(model.meshId == scaleData->getId())
			{
				m_Graphics->setModelScale(model.modelId, scaleData->getScale());
			}
		}
	}

	void updateAnimation(IEventData::Ptr p_Data)
	{
		std::shared_ptr<UpdateAnimationEventData> animationData = std::static_pointer_cast<UpdateAnimationEventData>(p_Data);
		for(auto &model : m_Models)
		{
			if(model.meshId == animationData->getId())
			{
				const std::vector<DirectX::XMFLOAT4X4>& animation = animationData->getAnimationData();
				m_Graphics->animationPose(model.modelId, animation.data(), animation.size());
			}
		}
	}

	void changeColorTone(IEventData::Ptr p_Data)
	{
		std::shared_ptr<ChangeColorToneEvent> data = std::static_pointer_cast<ChangeColorToneEvent>(p_Data);

		for (auto& model : m_Models)
		{
			if (model.meshId == data->getMeshId())
			{
				m_Graphics->setModelColorTone(model.modelId, data->getColorTone());
				return;
			}
		}
	}

	void createParticleEffect(IEventData::Ptr p_Data)
	{
		std::shared_ptr<CreateParticleEventData> data = std::static_pointer_cast<CreateParticleEventData>(p_Data);
	
		ParticleBinding particle =
		{
			data->getEffectName(),
			m_Graphics->createParticleEffectInstance(data->getEffectName().c_str())
		};

		m_Graphics->setParticleEffectPosition(particle.instance, data->getPosition());

		m_Particles[data->getId()] = particle;
	}

	void removeParticleEffectInstance(IEventData::Ptr p_Data)
	{
		std::shared_ptr<RemoveParticleEventData> data = std::static_pointer_cast<RemoveParticleEventData>(p_Data);

		auto it = m_Particles.find(data->getId());

		if (it != m_Particles.end())
		{
			m_Graphics->releaseParticleEffectInstance(it->second.instance);
			m_Particles.erase(it);
		}
	}

	void updateParticlePosition(IEventData::Ptr p_Data)
	{
		std::shared_ptr<UpdateParticlePositionEventData> data = std::static_pointer_cast<UpdateParticlePositionEventData>(p_Data);

		auto it = m_Particles.find(data->getId());

		if (it != m_Particles.end())
		{
			m_Graphics->setParticleEffectPosition(it->second.instance, data->getPosition());
		}
	}

	void updateParticleRotation(IEventData::Ptr p_Data)
	{
		std::shared_ptr<UpdateParticleRotationEventData> data = std::static_pointer_cast<UpdateParticleRotationEventData>(p_Data);

		auto it = m_Particles.find(data->getId());

		if (it != m_Particles.end())
		{
			m_Graphics->setParticleEffectRotation(it->second.instance, data->getRotation());
		}
	}

	void updateParticleBaseColor(IEventData::Ptr p_Data)
	{
		std::shared_ptr<UpdateParticleBaseColorEventData> data = std::static_pointer_cast<UpdateParticleBaseColorEventData>(p_Data);

		auto it = m_Particles.find(data->getId());

		if (it != m_Particles.end())
		{
			m_Graphics->setParticleEffectBaseColor(it->second.instance, data->getBaseColor());
		}
	}

	ObjectManager* const getObjectManager() const
	{
		return m_ObjectManager.get();
	}
};
