//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "RenderAPI/BsSamplerState.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Handles creation of various render states. */
	class B3D_CORE_EXPORT RenderStateManager : public Module<RenderStateManager>
	{
	public:
		/**
		 * Creates and initializes a new SamplerState.
		 *
		 * @param[in]	desc	Object describing the sampler state to create.
		 */
		SPtr<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& desc) const;

		/** Creates an uninitialized sampler state. Requires manual initialization after creation. */
		SPtr<SamplerState> CreateSamplerStatePtrInternal(const SamplerStateCreateInformation& desc) const;

		/** Gets a sampler state initialized with default options. */
		const SPtr<SamplerState>& GetDefaultSamplerState() const;

	private:
		friend class SamplerState;

		mutable SPtr<SamplerState> mDefaultSamplerState;
	};

	namespace ct
	{
		/**	Handles creation of various render states. */
		class B3D_CORE_EXPORT RenderStateManager : public Module<RenderStateManager>
		{
		private:
		public:
			RenderStateManager() = default;

			/**
			 * @copydoc bs::RenderStateManager::CreateSamplerState
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& desc) const;

			/** Creates an uninitialized sampler state. Requires manual initialization after creation. */
			SPtr<SamplerState> CreateSamplerStateInternal(const SamplerStateCreateInformation& desc) const;

			/** Gets a sampler state initialized with default options. */
			const SPtr<SamplerState>& GetDefaultSamplerState() const;

		protected:
			friend class bs::SamplerState;
			friend class SamplerState;

			void OnShutDown() override;

			/** @copydoc CreateSamplerState */
			virtual SPtr<SamplerState> CreateSamplerStateInternalInternal(const SamplerStateCreateInformation& desc) const;

		private:
			/**	Triggered when a new sampler state is created. */
			void NotifySamplerStateCreated(const SamplerStateCreateInformation& desc, const SPtr<SamplerState>& state) const;

			/**
			 * Triggered when the last reference to a specific sampler state is destroyed, which means we must clear our cached
			 * version as well.
			 */
			void NotifySamplerStateDestroyed(const SamplerStateInformation& desc) const;

			/**
			 * Attempts to find a cached sampler state corresponding to the provided descriptor. Returns null if one doesn't
			 * exist.
			 */
			SPtr<SamplerState> FindCachedState(const SamplerStateInformation& desc) const;

			mutable SPtr<SamplerState> mDefaultSamplerState;

			mutable UnorderedMap<SamplerStateInformation, std::weak_ptr<SamplerState>> mCachedSamplerStates;

			mutable Mutex mMutex;
		};
	} // namespace ct

	/** @} */
} // namespace bs
