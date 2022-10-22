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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Particles),ExportName(ParticleSystem)) CParticleSystem : public Component
	{
	public:
		CParticleSystem(const HSceneObject& parent);
		virtual ~CParticleSystem() = default;
		
		/** @copydoc ParticleSystem::setSettings */
		BS_SCRIPT_EXPORT(Property(Setter),ExportName(Settings),PassByCopy(true),inline)
		void SetSettings(const ParticleSystemSettings& settings);

		/** @copydoc ParticleSystem::getSettings */
		BS_SCRIPT_EXPORT(Property(Getter),ExportName(Settings),PassByCopy(true))
		const ParticleSystemSettings& GetSettings() const { return mSettings; }

		/** @copydoc ParticleSystem::setGpuSimulationSettings */
		BS_SCRIPT_EXPORT(Property(Setter),ExportName(GpuSimulationSettings),PassByCopy(true))
		void SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings);

		/** @copydoc ParticleSystem::getGpuSimulationSettings */
		BS_SCRIPT_EXPORT(Property(Getter),ExportName(GpuSimulationSettings),PassByCopy(true))
		const ParticleGpuSimulationSettings& GetGpuSimulationSettings() const { return mGpuSimulationSettings; }

		/** @copydoc ParticleSystem::setEmitters */
		BS_SCRIPT_EXPORT(Property(Setter),ExportName(Emitters))
		void SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters);

		/** @copydoc ParticleSystem::getEmitters */
		BS_SCRIPT_EXPORT(Property(Getter),ExportName(Emitters))
		const Vector<SPtr<ParticleEmitter>>& GetEmitters() const { return mEmitters; }

		/** @copydoc ParticleSystem::setEvolvers */
		BS_SCRIPT_EXPORT(Property(Setter),ExportName(Evolvers))
		void SetEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers);

		/** @copydoc ParticleSystem::getEvolvers */
		BS_SCRIPT_EXPORT(Property(Getter),ExportName(Evolvers))
		const Vector<SPtr<ParticleEvolver>>& GetEvolvers() const { return mEvolvers; }

		/** @copydoc ParticleSystem::setLayer() */
		BS_SCRIPT_EXPORT(Property(Setter),ExportName(Layer),layerMask)
		void SetLayer(u64 layer);

		/** @copydoc ParticleSystem::getLayer() */
		BS_SCRIPT_EXPORT(Property(Getter),ExportName(Layer),layerMask)
		u64 GetLayer() const { return mLayer; }

		/** @name Internal
		 *  @{
		 */

		/**
		 * Enables or disabled preview mode. Preview mode allows the particle system to play while the game is not running,
		 * primarily for preview purposes in the editor. Returns true if the preview mode was enabled, false if it was
		 * disabled or enabling preview failed.
		 */
		BS_SCRIPT_EXPORT(ExportName(TogglePreviewMode),Visibility(Internal))
		bool TogglePreviewModeInternal(bool enabled);

		/** Returns the ParticleSystem implementation wrapped by this component. */
		ParticleSystem* GetInternalInternal() const { return mInternal.get(); }

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
		using Component::DestroyInternal;

		/** Creates the internal representation of the ParticleSystem and restores the values saved by the Component. */
		void RestoreInternal();

		/** Destroys the internal ParticleSystem representation. */
		void DestroyInternal();

		SPtr<ParticleSystem> mInternal;

		ParticleSystemSettings mSettings;
		ParticleGpuSimulationSettings mGpuSimulationSettings;
		Vector<SPtr<ParticleEmitter>> mEmitters;
		Vector<SPtr<ParticleEvolver>> mEvolvers;
		u64 mLayer = 1;

		bool mPreviewMode = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CParticleSystemRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CParticleSystem(); // Serialization only
	 };

	 /** @} */
}
