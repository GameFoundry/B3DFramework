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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportName(ParticleSystem)) CParticleSystem : public Component
	{
	public:
		CParticleSystem(const HSceneObject& parent);
		virtual ~CParticleSystem() = default;

		/** @copydoc ParticleSystem::SetSettings */
		BS_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), PassByCopy(true), UI(Inline))
		void SetSettings(const ParticleSystemSettings& settings);

		/** @copydoc ParticleSystem::GetSettings */
		BS_SCRIPT_EXPORT(Property(Getter), ExportName(Settings), PassByCopy(true))
		const ParticleSystemSettings& GetSettings() const { return mSettings; }

		/** @copydoc ParticleSystem::SetGpuSimulationSettings */
		BS_SCRIPT_EXPORT(Property(Setter), ExportName(GpuSimulationSettings), PassByCopy(true))
		void SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings);

		/** @copydoc ParticleSystem::GetGpuSimulationSettings */
		BS_SCRIPT_EXPORT(Property(Getter), ExportName(GpuSimulationSettings), PassByCopy(true))
		const ParticleGpuSimulationSettings& GetGpuSimulationSettings() const { return mGpuSimulationSettings; }

		/** @copydoc ParticleSystem::SetEmitters */
		BS_SCRIPT_EXPORT(Property(Setter), ExportName(Emitters))
		void SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters);

		/** @copydoc ParticleSystem::GetEmitters */
		BS_SCRIPT_EXPORT(Property(Getter), ExportName(Emitters))
		const Vector<SPtr<ParticleEmitter>>& GetEmitters() const { return mEmitters; }

		/** @copydoc ParticleSystem::SetEvolvers */
		BS_SCRIPT_EXPORT(Property(Setter), ExportName(Evolvers))
		void SetEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers);

		/** @copydoc ParticleSystem::GetEvolvers */
		BS_SCRIPT_EXPORT(Property(Getter), ExportName(Evolvers))
		const Vector<SPtr<ParticleEvolver>>& GetEvolvers() const { return mEvolvers; }

		/** @copydoc ParticleSystem::SetLayer() */
		BS_SCRIPT_EXPORT(Property(Setter), ExportName(Layer), UI(AsLayerMask))
		void SetLayer(u64 layer);

		/** @copydoc ParticleSystem::GetLayer() */
		BS_SCRIPT_EXPORT(Property(Getter), ExportName(Layer), UI(AsLayerMask))
		u64 GetLayer() const { return mLayer; }

		/** @name Internal
		 *  @{
		 */

		/**
		 * Enables or disabled preview mode. Preview mode allows the particle system to play while the game is not running,
		 * primarily for preview purposes in the editor. Returns true if the preview mode was enabled, false if it was
		 * disabled or enabling preview failed.
		 */
		BS_SCRIPT_EXPORT(ExportName(TogglePreviewMode), Visibility(Internal))
		bool TogglePreviewModeInternal(bool enabled);

		/** Returns the ParticleSystem implementation wrapped by this component. */
		ParticleSystem* GetInternalInternal() const { return mInternal.get(); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnDestroyed() override;
		void OnDisabled() override;
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
} // namespace bs
