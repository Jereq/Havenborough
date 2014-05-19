#include "MyDX11Widget.h"

#include <ResourceManager.h>

MyDX11Widget::MyDX11Widget(QWidget* parent, Qt::WindowFlags flags)
	: DXWidget(parent, flags),
	m_Graphics(nullptr)
{
}

MyDX11Widget::~MyDX11Widget()
{
	uninitialize();
}

void MyDX11Widget::initialize(EventManager* p_EventManager, ResourceManager* p_ResourceManager, IGraphics* p_Graphics)
{
	m_EventManager = p_EventManager;
	m_ResourceManager = p_ResourceManager;
	m_Graphics = p_Graphics;

	m_ResourceIDs.push_back(m_ResourceManager->loadResource("texture","SKYBOXDDS"));
	m_Graphics->createSkydome("SKYBOXDDS", 500000.f);

	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::addLight), LightEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::removeLight), RemoveLightEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::createMesh), CreateMeshEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::removeMesh), RemoveMeshEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateModelPosition), UpdateModelPositionEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateModelRotation), UpdateModelRotationEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateModelScale), UpdateModelScaleEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateAnimation), UpdateAnimationEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::changeColorTone), ChangeColorToneEvent::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::createParticleEffect), CreateParticleEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::removeParticleEffectInstance), RemoveParticleEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateParticlePosition), UpdateParticlePositionEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateParticleRotation), UpdateParticleRotationEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateParticleBaseColor), UpdateParticleBaseColorEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::activatePowerPie), MouseEventDataPie::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::selectPie), PowerPieSelectEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::pick), CreateRayEventData::sk_EventType);

	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateLightColor), UpdateLightColorEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateLightDirection), UpdateLightDirectionEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateLightPosition), UpdateLightPositionEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateLightAngle), UpdateLightAngleEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateLightRange), UpdateLightRangeEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &MyDX11Widget::updateLightIntensity), UpdateLightIntensityEventData::sk_EventType);


	m_ResourceIDs.push_back(m_ResourceManager->loadResource("particleSystem", "TestParticle"));

	preLoadModels();

	m_PowerPie = PowerPie();
}

void MyDX11Widget::uninitialize()
{
	for (int res : m_ResourceIDs)
	{
		m_ResourceManager->releaseResource(res);
	}
	m_ResourceIDs.clear();
}

void MyDX11Widget::render()
{
	m_Graphics->updateCamera(m_Camera.getPosition(), m_Camera.getForward(), m_Camera.getUp());

	if(m_PowerPie.isActive)
	{
		m_Graphics->render2D_Object(m_GUI["PowerPie"]);
		m_Graphics->render2D_Object(m_GUI["PiePiece"]);
	}



	for (auto& mesh : m_Models)
	{
		m_Graphics->renderModel(mesh.modelId);
	}

	bool usingDirectional = false;
	for(auto &light : m_Lights)
	{
		switch(light.type)
		{
		case LightClass::Type::DIRECTIONAL:
			{
				m_Graphics->useFrameDirectionalLight(light.color, light.direction, light.intensity);
				usingDirectional = true;
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

	if (!usingDirectional)
	{
		m_Graphics->useFrameDirectionalLight(Vector3(1.f, 1.f, 1.f), Vector3(0.1f, -0.8f, 0.2f), 0.1f);
	}

	m_Graphics->renderSkydome();

	present();
}

void MyDX11Widget::present()
{
	if (m_Graphics)
	{
		m_Graphics->drawFrame();
	}
}

void MyDX11Widget::onFrame(float p_DeltaTime)
{
	DXWidget::onFrame(p_DeltaTime);
	m_Graphics->updateParticles(p_DeltaTime);
}

void MyDX11Widget::onResize(unsigned int nWidth, unsigned int nHeight)
{
	if (m_Graphics)
	{
		m_Graphics->resize(nWidth, nHeight);

		render();
	}
}
	
void MyDX11Widget::addLight(IEventData::Ptr p_Data)
{
	std::shared_ptr<LightEventData> lightData = std::static_pointer_cast<LightEventData>(p_Data);
	LightClass light = lightData->getLight();
	m_Lights.push_back(light);
}

void MyDX11Widget::removeLight(IEventData::Ptr p_Data)
{
	std::shared_ptr<RemoveLightEventData> lightData = std::static_pointer_cast<RemoveLightEventData>(p_Data);

	auto remIt = std::remove_if(m_Lights.begin(), m_Lights.end(),
		[&lightData] (LightClass& p_Light)
		{
			return p_Light.id == lightData->getId();
		});
	m_Lights.erase(remIt, m_Lights.end());
}

void MyDX11Widget::createMesh(IEventData::Ptr p_Data)
{
	std::shared_ptr<CreateMeshEventData> meshData = std::static_pointer_cast<CreateMeshEventData>(p_Data);

	int resource = m_ResourceManager->loadResource("model", meshData->getMeshName());
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
}

void MyDX11Widget::removeMesh(IEventData::Ptr p_Data)
{
	std::shared_ptr<RemoveMeshEventData> meshData = std::static_pointer_cast<RemoveMeshEventData>(p_Data);

	for (auto& model : m_Models)
	{
		if (model.meshId == meshData->getId())
		{
			m_ResourceManager->releaseResource(model.resourceId);
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

void MyDX11Widget::updateModelPosition(IEventData::Ptr p_Data)
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

void MyDX11Widget::updateModelRotation(IEventData::Ptr p_Data)
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

void MyDX11Widget::updateModelScale(IEventData::Ptr p_Data)
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

void MyDX11Widget::updateAnimation(IEventData::Ptr p_Data)
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

void MyDX11Widget::changeColorTone(IEventData::Ptr p_Data)
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

void MyDX11Widget::createParticleEffect(IEventData::Ptr p_Data)
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

void MyDX11Widget::removeParticleEffectInstance(IEventData::Ptr p_Data)
{
	std::shared_ptr<RemoveParticleEventData> data = std::static_pointer_cast<RemoveParticleEventData>(p_Data);

	auto it = m_Particles.find(data->getId());

	if (it != m_Particles.end())
	{
		m_Graphics->releaseParticleEffectInstance(it->second.instance);
		m_Particles.erase(it);
	}
}

void MyDX11Widget::updateParticlePosition(IEventData::Ptr p_Data)
{
	std::shared_ptr<UpdateParticlePositionEventData> data = std::static_pointer_cast<UpdateParticlePositionEventData>(p_Data);

	auto it = m_Particles.find(data->getId());

	if (it != m_Particles.end())
	{
		m_Graphics->setParticleEffectPosition(it->second.instance, data->getPosition());
	}
}

void MyDX11Widget::updateParticleRotation(IEventData::Ptr p_Data)
{
	std::shared_ptr<UpdateParticleRotationEventData> data = std::static_pointer_cast<UpdateParticleRotationEventData>(p_Data);

	auto it = m_Particles.find(data->getId());

	if (it != m_Particles.end())
	{
		m_Graphics->setParticleEffectRotation(it->second.instance, data->getRotation());
	}
}

void MyDX11Widget::updateParticleBaseColor(IEventData::Ptr p_Data)
{
	std::shared_ptr<UpdateParticleBaseColorEventData> data = std::static_pointer_cast<UpdateParticleBaseColorEventData>(p_Data);

	auto it = m_Particles.find(data->getId());

	if (it != m_Particles.end())
	{
		m_Graphics->setParticleEffectBaseColor(it->second.instance, data->getBaseColor());
	}
}

void MyDX11Widget::pick(IEventData::Ptr p_Data)
{
	std::shared_ptr<CreateRayEventData> data = std::static_pointer_cast<CreateRayEventData>(p_Data);

	DirectX::XMFLOAT4X4 fView = m_Graphics->getView();
	DirectX::XMFLOAT4X4 fProj = m_Graphics->getProj();
	DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX mView = DirectX::XMLoadFloat4x4(&fView);
	DirectX::XMMATRIX mProj = DirectX::XMLoadFloat4x4(&fProj);
	mView = XMMatrixTranspose(mView);
	mProj = XMMatrixTranspose(mProj);

	DirectX::XMVECTOR cursorScreenSpace = DirectX::XMVectorSet(data->getMousePos().x, data->getMousePos().y, 0.f, 1.f);
	DirectX::XMVECTOR unprojectedCursor = DirectX::XMVector3Unproject(cursorScreenSpace, 0.f, 0.f, data->getResolution().x, data->getResolution().y, 0.f, 1.f, mProj, mView, mWorld);

	DirectX::XMMATRIX invView = XMMatrixInverse(nullptr, mView);
	DirectX::XMVECTOR vRayOrigin =  invView.r[3];

	using DirectX::operator-;
	DirectX::XMVECTOR direction = unprojectedCursor - vRayOrigin;
	
	direction = DirectX::XMVector3Normalize(direction);

 	DirectX::XMFLOAT4 fRayDir, fRayOrigin;
	DirectX::XMStoreFloat4(&fRayDir, direction);
	DirectX::XMStoreFloat4(&fRayOrigin, vRayOrigin);

	m_EventManager->triggerTriggerEvent(IEventData::Ptr(new CreatePickingEventData(fRayDir, fRayOrigin)));
}

void MyDX11Widget::selectPie(IEventData::Ptr p_Data)
{
	std::shared_ptr<PowerPieSelectEventData> pie = std::static_pointer_cast<PowerPieSelectEventData>(p_Data);

	m_Graphics->set2D_ObjectPosition(m_GUI["PiePiece"], Vector3(m_PowerPie.position.x, m_PowerPie.position.y, 5.f));
	Vector4 color(0.9101f, 0.f, 0.f, 1.f);
	m_Graphics->set2D_ObjectColor(m_GUI["PiePiece"], color);

	m_Graphics->set2D_ObjectRotationZ(m_GUI["PiePiece"], -0.785398163f * pie->getIndex());


}

void MyDX11Widget::activatePowerPie(IEventData::Ptr p_Data)
{
	std::shared_ptr<MouseEventDataPie> pie = std::static_pointer_cast<MouseEventDataPie>(p_Data);

	Vector2 pos = pie->getMousePos();

	m_Graphics->set2D_ObjectPosition(m_GUI["PowerPie"], Vector3(pos.x, pos.y, 0.f));

	Vector4 color(0.9101f, 0.8632f, 0.0937f, 0.f);
	m_Graphics->set2D_ObjectColor(m_GUI["PiePiece"], color);

	m_PowerPie.position = pos;
	m_PowerPie.isActive = pie->getPieStatus();
}

void MyDX11Widget::createPowerPieElement()
{
	Vector4 color(0.9101f, 0.8632f, 0.0937f, 1.f);
	Vector3 position(0.f, 0.f, 0.f);
	Vector3 scale(1.f, 1.f, 1.f);

	m_GUI.insert(std::pair<std::string, int>("PowerPie", m_Graphics->create2D_Object(position, Vector2(128.f, 128.f), scale, 0.f, "PowerPie")));
	m_Graphics->set2D_ObjectColor(m_GUI["PowerPie"], color);

	position = Vector3(0.f, 0.f, 5.f);
	color = Vector4(0.9101f, 0.9632f, 0.f, 0.f);

	m_GUI.insert(std::pair<std::string, int>("PiePiece", m_Graphics->create2D_Object(position, Vector2(128.f, 128.f), scale, 0.f, "PiePiece")));
	m_Graphics->set2D_ObjectColor(m_GUI["PiePiece"], color);
}

void MyDX11Widget::preLoadModels()
{
	static const std::string preloadedTextures[] =
	{
		"PowerPie",
		"PiePiece",
	};
	for (const std::string &texture : preloadedTextures)
	{
		m_ResourceIDs.push_back(m_ResourceManager->loadResource("texture", texture));
	}

	createPowerPieElement();
}


void MyDX11Widget::updateLightColor(IEventData::Ptr p_Data)
{
	std::shared_ptr<UpdateLightColorEventData> data = std::static_pointer_cast<UpdateLightColorEventData>(p_Data);
	LightClass* light = findLight(data->getId());

	if(light)
	{
		light->color = data->getColor();
	}
}

void MyDX11Widget::updateLightDirection(IEventData::Ptr p_Data)
{
	std::shared_ptr<UpdateLightDirectionEventData> data = std::static_pointer_cast<UpdateLightDirectionEventData>(p_Data);
	LightClass* light = findLight(data->getId());
	
	if(light)
	{
		DirectX::XMStoreFloat3(&light->direction, DirectX::XMVector3Normalize(DirectX::XMVectorSet(data->getDirection().x,data->getDirection().y,data->getDirection().z, 0.f)));
		//light->direction = data->getDirection();
	}
}

void MyDX11Widget::updateLightPosition(IEventData::Ptr p_Data)
{
	std::shared_ptr<UpdateLightPositionEventData> data = std::static_pointer_cast<UpdateLightPositionEventData>(p_Data);
	LightClass* light = findLight(data->getId());

	if(light)
	{
		light->position = data->getPosition();
	}
}

void MyDX11Widget::updateLightAngle(IEventData::Ptr p_Data)
{
	std::shared_ptr<UpdateLightAngleEventData> data = std::static_pointer_cast<UpdateLightAngleEventData>(p_Data);
	LightClass* light = findLight(data->getId());

	if(light)
	{
		light->spotlightAngles = data->getAngle();
	}
}

void MyDX11Widget::updateLightRange(IEventData::Ptr p_Data)
{
	std::shared_ptr<UpdateLightRangeEventData> data = std::static_pointer_cast<UpdateLightRangeEventData>(p_Data);
	LightClass* light = findLight(data->getId());

	if(light)
	{
		light->range = data->getRange();
	}
}

void MyDX11Widget::updateLightIntensity(IEventData::Ptr p_Data)
{
	std::shared_ptr<UpdateLightIntensityEventData> data = std::static_pointer_cast<UpdateLightIntensityEventData>(p_Data);
	LightClass* light = findLight(data->getId());

	if(light)
	{
		light->intensity = data->getIntensity();
	}
}

LightClass* MyDX11Widget::findLight(int p_Id)
{
	for(auto &light : m_Lights)
	{
		if(light.id == p_Id)
			return &light;
	}
	return nullptr;
}