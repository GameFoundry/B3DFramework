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
		BS_SCRIPT_EXPORT(pr:setter,n:Settings,passByCopy,inline)
		void SetSettings(const ParticleSystemSettings& settings);

		/** @copydoc ParticleSystem::getSettings */
		BS_SCRIPT_EXPORT(pr:getter,n:Settings,passByCopy)
		const ParticleSystemSettings& GetSettings() const { return mSettings; }

		/** @copydoc ParticleSystem::setGpuSimulationSettings */
		BS_SCRIPT_EXPORT(pr:setter,n:GpuSimulationSettings,passByCopy)
		void SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings);

		/** @copydoc ParticleSystem::getGpuSimulationSettings */
		BS_SCRIPT_EXPORT(pr:getter,n:GpuSimulationSettings,passByCopy)
		const ParticleGpuSimulationSettings& GetGpuSimulationSettings() const { return mGpuSimulationSettings; }

		/** @copydoc ParticleSystem::setEmitters */
		BS_SCRIPT_EXPORT(pr:setter,n:Emitters)
		void SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters);

		/** @copydoc ParticleSystem::getEmitters */
		BS_SCRIPT_EXPORT(pr:getter,n:Emitters)
		const Vector<SPtr<ParticleEmitter>>& GetEmitters() const { return mEmitters; }

		/** @copydoc ParticleSystem::setEvolvers */
		BS_SCRIPT_EXPORT(pr:setter,n:Evolvers)
		void SetEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers);

		/** @copydoc ParticleSystem::getEvolvers */
		BS_SCRIPT_EXPORT(pr:getter,n:Evolvers)
		const Vector<SPtr<ParticleEvolver>>& GetEvolvers() const { return mEvolvers; }

		/** @copydoc ParticleSystem::setLayer() */
		BS_SCRIPT_EXPORT(pr:setter,n:Layer,layerMask)
		void SetLayer(UINT64 layer);

		/** @copydoc ParticleSystem::getLayer() */
		BS_SCRIPT_EXPORT(pr:getter,n:Layer,layerMask)
		UINT64 GetLayer() const { return mLayer; }

		/** @name Internal
		 *  @{
		 */

		/**
		 * Enables or disabled preview mode. Preview mode allows the particle system to play while the game is not running,
		 * primarily for preview purposes in the editor. Returns true if the preview mode was enabled, false if it was
		 * disabled or enabling preview failed.
		 */
		BS_SCRIPT_EXPORT(n:TogglePreviewMode,v:internal)
		bool _togglePreviewMode(bool enabled);

		/** Returns the ParticleSystem implementation wrapped by this component. */
		ParticleSystem* _getInternal() const { return mInternal.Get(); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onDestroyed() */
		void OnDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void OnDisabled() override;

		/** @copydoc Component::onEnabled() */
		void OnEnabled() override;

	protected:
		using Component::destroyInternal;

		/** Creates the internal representation of the ParticleSystem and restores the values saved by the Component. */
		void RestoreInternal();

		/** Destroys the internal ParticleSystem representation. */
		void DestroyInternal();

		SPtr<ParticleSystem> mInternal;

		ParticleSystemSettings mSettings;
		ParticleGpuSimulationSettings mGpuSimulationSettings;
		Vector<SPtr<ParticleEmitter>> mEmitters;
		Vector<SPtr<ParticleEvolver>> mEvolvers;
		UINT64 mLayer = 1;

		bool mPreviewMode = false;

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
