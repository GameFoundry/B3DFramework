//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCParticleSystem.h"
#include "Scene/BsSceneObject.h"
#include "Utility/BsTime.h"
#include "Private/RTTI/BsCParticleSystemRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace std::placeholders;

using namespace bs;

CParticleSystem::CParticleSystem()
{
	SetName("ParticleSystem");
	SetFlag(ComponentFlag::AlwaysRun, true);
}

CParticleSystem::CParticleSystem(const HSceneObject& parent)
	: Component(parent)
{
	SetName("ParticleSystem");
	SetFlag(ComponentFlag::AlwaysRun, true);
}

void CParticleSystem::SetSettings(const ParticleSystemSettings& settings)
{
	mSettings = settings;

	if(mInternal)
		mInternal->SetSettings(settings);
}

void CParticleSystem::SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings)
{
	mGpuSimulationSettings = settings;

	if(mInternal)
		mInternal->SetGpuSimulationSettings(settings);
}

void CParticleSystem::SetEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers)
{
	mEvolvers = evolvers;

	if(mInternal)
		mInternal->SetEvolvers(evolvers);
}

void CParticleSystem::SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters)
{
	mEmitters = emitters;

	if(mInternal)
		mInternal->SetEmitters(emitters);
}

void CParticleSystem::SetLayer(u64 layer)
{
	mLayer = layer;

	if(mInternal)
		mInternal->SetLayer(layer);
}

void CParticleSystem::OnDestroyed()
{
	DestroyInternal();
}

void CParticleSystem::OnDisabled()
{
	DestroyInternal();
}

void CParticleSystem::OnEnabled()
{
	if(mPreviewMode)
	{
		DestroyInternal();
		mPreviewMode = false;
	}

	if(SceneManager::Instance().IsRunning())
	{
		RestoreInternal();
		mInternal->Play();
	}
}

void CParticleSystem::RestoreInternal()
{
	if(mInternal == nullptr)
	{
		mInternal = ParticleSystem::Create();
		GetSceneManager().BindActorInternal(mInternal, SceneObject());
	}

	mInternal->SetSettings(mSettings);
	mInternal->SetGpuSimulationSettings(mGpuSimulationSettings);
	mInternal->SetEmitters(mEmitters);
	mInternal->SetEvolvers(mEvolvers);
	mInternal->SetLayer(mLayer);
}

void CParticleSystem::DestroyInternal()
{
	if(mInternal)
	{
		mEmitters = mInternal->GetEmitters();
		mEvolvers = mInternal->GetEvolvers();

		GetSceneManager().UnbindActorInternal(mInternal);
	}

	// This should release the last reference and destroy the internal object
	mInternal = nullptr;
}

bool CParticleSystem::TogglePreviewModeInternal(bool enabled)
{
	bool isRunning = SceneManager::Instance().IsRunning();

	if(enabled)
	{
		// Cannot enable preview while running
		if(isRunning)
			return false;

		if(!mPreviewMode)
		{
			RestoreInternal();
			mInternal->Play();
			mPreviewMode = true;
		}

		return true;
	}
	else
	{
		if(!isRunning)
			DestroyInternal();

		mPreviewMode = false;
		return false;
	}
}

RTTITypeBase* CParticleSystem::GetRttiStatic()
{
	return CParticleSystemRTTI::Instance();
}

RTTITypeBase* CParticleSystem::GetRtti() const
{
	return CParticleSystem::GetRttiStatic();
}
