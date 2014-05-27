#pragma once

#include "DXWidget.h"

#include <EventData.h>
#include <IGraphics.h>
#include <IPhysics.h>

class MyDX11Widget : public DXWidget
{
private:
	IGraphics* m_Graphics;
	IPhysics* m_Physics;
	
	std::vector<int> m_ResourceIDs;
	std::vector<LightClass> m_Lights;
	bool m_PowerPieActive;
	std::map<std::string, int> m_GUI;

	enum class DRAW
	{
		LOW,
		MEDIUM,
		HIGH
	};

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
	Actor::wPtr m_SelectedObject;

public:
	explicit MyDX11Widget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
	~MyDX11Widget() override;

	void initialize(EventManager* p_EventManager, ResourceManager* p_ResourceManager, IGraphics* p_Graphics, RotationTool* p_RotationTool, IPhysics* p_Physics) override;
	void uninitialize() override;

	void render() override;
	void present() override;
	void onFrame(float p_DeltaTime) override;
	void onResize(unsigned int nWidth, unsigned int nHeight) override;
	std::vector<std::string> getPieList();
	void updatePowerPie(std::vector<std::string> p_List);

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
	void selectActor(IEventData::Ptr p_Data);

	void createPowerPieElement();
	void reinitializePowerPie();
	void preLoadModels();


	void updateLightColor(IEventData::Ptr p_Data);
	void updateLightDirection(IEventData::Ptr p_Data);
	void updateLightPosition(IEventData::Ptr p_Data);
	void updateLightAngle(IEventData::Ptr p_Data);
	void updateLightRange(IEventData::Ptr p_Data);
	void updateLightIntensity(IEventData::Ptr p_Data);

	LightClass* findLight(int p_Id);
};
