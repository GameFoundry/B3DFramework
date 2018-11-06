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
	}

	CParticleSystem::CParticleSystem(const HSceneObject& parent)
		: Component(parent)
	{
		setName("ParticleSystem");
	}

	void CParticleSystem::setSettings(const ParticleSystemSettings& settings)
	{
		mSettings = settings;

		if(mInternal)
			mInternal->setSettings(settings);
	}

	void CParticleSystem::setGpuSimulationSettings(const ParticleGpuSimulationSettings& settings)
	{
		mGpuSimulationSettings = settings;

		if(mInternal)
			mInternal->setGpuSimulationSettings(settings);
	}

	ParticleSystemEmitters& CParticleSystem::getEmitters()
	{
		if(mInternal)
			return mInternal->getEmitters();

		return mEmitters;
	}

	ParticleSystemEvolvers& CParticleSystem::getEvolvers()
	{
		if(mInternal)
			return mInternal->getEvolvers();

		return mEvolvers;
	}

	void CParticleSystem::setLayer(UINT64 layer)
	{
		mLayer = layer;

		if(mInternal)
			mInternal->setLayer(layer);
	}

	void CParticleSystem::onDestroyed()
	{
		destroyInternal();
	}

	void CParticleSystem::onDisabled()
	{
		destroyInternal();
	}

	void CParticleSystem::onEnabled()
	{
		restoreInternal();

		mInternal->play();
	}

	void CParticleSystem::restoreInternal()
	{
		if (mInternal == nullptr)
		{
			mInternal = ParticleSystem::create();
			gSceneManager()._bindActor(mInternal, sceneObject());
		}

		mInternal->setSettings(mSettings);
		mInternal->setGpuSimulationSettings(mGpuSimulationSettings);
		mInternal->getEmitters() = mEmitters;
		mInternal->getEvolvers() = mEvolvers;
		mInternal->setLayer(mLayer);
	}

	void CParticleSystem::destroyInternal()
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

	RTTITypeBase* CParticleSystem::getRTTIStatic()
	{
		return CParticleSystemRTTI::instance();
	}

	RTTITypeBase* CParticleSystem::getRTTI() const
	{
		return CParticleSystem::getRTTIStatic();
	}
}