//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Particles/BsParticleSystem.h"
#include "Scene/BsComponent.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	ParticleSystem
	 *
	 * @note Wraps ParticleSystem as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles,n:ParticleSystem) CParticleSystem : public Component
	{
	public:
		CParticleSystem(const HSceneObject& parent);
		virtual ~CParticleSystem() = default;
		
		/** @copydoc ParticleSystem::setSettings */
		BS_SCRIPT_EXPORT(pr:setter,n:Settings,passByCopy)
		void setSettings(const ParticleSystemSettings& settings);

		/** @copydoc ParticleSystem::getSettings */
		BS_SCRIPT_EXPORT(pr:getter,n:Settings,passByCopy)
		const ParticleSystemSettings& getSettings() const { return mSettings; }

		/** @copydoc ParticleSystem::setGpuSimulationSettings */
		BS_SCRIPT_EXPORT(pr:setter,n:GpuSimulationSettings,passByCopy)
		void setGpuSimulationSettings(const ParticleGpuSimulationSettings& settings);

		/** @copydoc ParticleSystem::getGpuSimulationSettings */
		BS_SCRIPT_EXPORT(pr:getter,n:GpuSimulationSettings,passByCopy)
		const ParticleGpuSimulationSettings& getGpuSimulationSettings() const { return mGpuSimulationSettings; }

		/** @copydoc ParticleSystem::setEmitters */
		BS_SCRIPT_EXPORT(pr:setter,n:Emitters)
		void setEmitters(const Vector<SPtr<ParticleEmitter>>& emitters);

		/** @copydoc ParticleSystem::getEmitters */
		BS_SCRIPT_EXPORT(pr:getter,n:Emitters)
		const Vector<SPtr<ParticleEmitter>>& getEmitters() const { return mEmitters; }

		/** @copydoc ParticleSystem::setEvolvers */
		BS_SCRIPT_EXPORT(pr:setter,n:Evolvers)
		void setEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers);

		/** @copydoc ParticleSystem::getEvolvers */
		BS_SCRIPT_EXPORT(pr:getter,n:Evolvers)
		const Vector<SPtr<ParticleEvolver>>& getEvolvers() const { return mEvolvers; }

		/** @copydoc ParticleSystem::setLayer() */
		BS_SCRIPT_EXPORT(pr:setter,n:Layer)
		void setLayer(UINT64 layer);

		/** @copydoc ParticleSystem::getLayer() */
		BS_SCRIPT_EXPORT(pr:getter,n:Layer)
		UINT64 getLayer() const { return mLayer; }

		/** @name Internal
		 *  @{
		 */

		/** Returns the ParticleSystem implementation wrapped by this component. */
		ParticleSystem* _getInternal() const { return mInternal.get(); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onDestroyed() */
		void onDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void onDisabled() override;

		/** @copydoc Component::onEnabled() */
		void onEnabled() override;

	protected:
		using Component::destroyInternal;

		/** Creates the internal representation of the ParticleSystem and restores the values saved by the Component. */
		void restoreInternal();

		/** Destroys the internal ParticleSystem representation. */
		void destroyInternal();

		SPtr<ParticleSystem> mInternal;

		ParticleSystemSettings mSettings;
		ParticleGpuSimulationSettings mGpuSimulationSettings;
		Vector<SPtr<ParticleEmitter>> mEmitters;
		Vector<SPtr<ParticleEvolver>> mEvolvers;
		UINT64 mLayer = 1;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CParticleSystemRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CParticleSystem(); // Serialization only
	 };

	 /** @} */
}