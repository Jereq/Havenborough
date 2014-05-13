#pragma once

#include "DXWidget.h"

#include <EventData.h>
#include <IGraphics.h>

class MyDX11Widget : public DXWidget
{
private:
	IGraphics* m_Graphics;
	
	std::vector<int> m_ResourceIDs;
	std::vector<LightClass> m_Lights;

	bool m_PowerPieActive;
	std::map<std::string, int> m_GUI;

	std::vector<Vector2> m_RelativeIconPositions;

	enum class DRAW
	{
		LOW,
		MEDIUM,
		HIGH
	};

	struct PowerPie
	{
		Vector2 position;
		bool isActive;
		Vector4 selectedColor;
		Vector4 pieColor;


		PowerPie()
		{
			position = Vector2(0.f, 0.f);
			isActive = false;
		}
	};

	PowerPie m_PowerPie;

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
	explicit MyDX11Widget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
	~MyDX11Widget() override;

	void initialize(EventManager* p_EventManager, ResourceManager* p_ResourceManager, IGraphics* p_Graphics) override;
	void uninitialize() override;

	void render() override;
	void present() override;
	void onFrame(float p_DeltaTime) override;
	void onResize(unsigned int nWidth, unsigned int nHeight) override;

private:
	void addLight(IEventData::Ptr p_Data);
	void removeLight(IEventData::Ptr p_Data);
	void createMesh(IEventData::Ptr p_Data);
	void removeMesh(IEventData::Ptr p_Data);
	void updateModelPosition(IEventData::Ptr p_Data);
	void updateModelRotation(IEventData::Ptr p_Data);
	void updateModelScale(IEventData::Ptr p_Data);
	void updateAnimation(IEventData::Ptr p_Data);
	void changeColorTone(IEventData::Ptr p_Data);
	void createParticleEffect(IEventData::Ptr p_Data);
	void removeParticleEffectInstance(IEventData::Ptr p_Data);
	void updateParticlePosition(IEventData::Ptr p_Data);
	void updateParticleRotation(IEventData::Ptr p_Data);
	void updateParticleBaseColor(IEventData::Ptr p_Data);
	void pick(IEventData::Ptr p_Data);
	void selectPie(IEventData::Ptr p_Data);
	void activatePowerPie(IEventData::Ptr p_Data);
	void createPowerPieElement();
	void preLoadModels();

};
