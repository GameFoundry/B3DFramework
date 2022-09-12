//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCParticleSystem.h"
#include "Scene/BsSceneObject.h"
#include "Utility/BsTime.h"
#include "Private/RTTI/BsCParticleSystemRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace std::placeholders;

namespace bs
{
	CParticleSystem::CParticleSystem()
	{
		setName("ParticleSystem");
		setFlag(ComponentFlag::AlwaysRun, true);
	}

	CParticleSystem::CParticleSystem(const HSceneObject& parent)
		: Component(parent)
	{
		setName("ParticleSystem");
		setFlag(ComponentFlag::AlwaysRun, true);
	}

	void CParticleSystem::SetSettings(const ParticleSystemSettings& settings)
	{
		mSettings = settings;

		if(mInternal)
			mInternal->setSettings(settings);
	}

	void CParticleSystem::SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings)
	{
		mGpuSimulationSettings = settings;

		if(mInternal)
			mInternal->setGpuSimulationSettings(settings);
	}

	void CParticleSystem::SetEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers)
	{
		mEvolvers = evolvers;

		if(mInternal)
			mInternal->setEvolvers(evolvers);
	}

	void CParticleSystem::SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters)
	{
		mEmitters = emitters;

		if(mInternal)
			mInternal->setEmitters(emitters);
	}

	void CParticleSystem::SetLayer(UINT64 layer)
	{
		mLayer = layer;

		if(mInternal)
			mInternal->setLayer(layer);
	}

	void CParticleSystem::OnDestroyed()
	{
		destroyInternal();
	}

	void CParticleSystem::OnDisabled()
	{
		destroyInternal();
	}

	void CParticleSystem::OnEnabled()
	{
		if(mPreviewMode)
		{
			destroyInternal();
			mPreviewMode = false;
		}
		
		if(SceneManager::instance().isRunning())
		{
			restoreInternal();
			mInternal->play();
		}
	}

	void CParticleSystem::RestoreInternal()
	{
		if (mInternal == nullptr)
		{
			mInternal = ParticleSystem::create();
			gSceneManager()._bindActor(mInternal, sceneObject());
		}

		mInternal->setSettings(mSettings);
		mInternal->setGpuSimulationSettings(mGpuSimulationSettings);
		mInternal->setEmitters(mEmitters);
		mInternal->setEvolvers(mEvolvers);
		mInternal->setLayer(mLayer);
	}

	void CParticleSystem::DestroyInternal()
	{
		if(mInternal)
		{
			mEmitters = mInternal->getEmitters();
			mEvolvers = mInternal->getEvolvers();

			gSceneManager()._unbindActor(mInternal);
		}

		// This should release the last reference and destroy the internal object
		mInternal = nullptr;
	}

	bool CParticleSystem::_togglePreviewMode(bool enabled)
	{
		bool isRunning = SceneManager::instance().isRunning();

		if(enabled)
		{
			// Cannot enable preview while running
			if (isRunning)
				return false;

			if(!mPreviewMode)
			{
				restoreInternal();
				mInternal->play();
				mPreviewMode = true;
			}

			return true;
		}
		else
		{
			if (!isRunning)
				destroyInternal();

			mPreviewMode = false;
			return false;
		}
	}

	RTTITypeBase* CParticleSystem::getRTTIStatic()
	{
		return CParticleSystemRTTI::Instance();
	}

	RTTITypeBase* CParticleSystem::getRTTI() const
	{
		return CParticleSystem::GetRTTIStatic();
	}
}
