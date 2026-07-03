//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d
{
	namespace render
	{
		class D3D12ResourceManager;

		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/**
		 * Base class for all D3D12 GPU resources that need to be tracked for synchronization purposes. Inherits the
		 * cross-backend lifetime state machine (notify/destroy/deferred-destroy) from IGpuResource and adds
		 * D3D12-specific state-tracking on top.
		 */
		class D3D12Resource : public IGpuResource
		{
		public:
			/** Constructs a manager-owned resource. @p owner must be non-null. */
			D3D12Resource(D3D12ResourceManager* owner, const StringView& name = "");

			/** Returns the D3D12 resource. */
			virtual ID3D12Resource* GetD3D12Resource() const = 0;

			/** Returns the current resource state. */
			D3D12_RESOURCE_STATES GetCurrentState() const { return mCurrentState; }

			/** Sets the current resource state. */
			void SetCurrentState(D3D12_RESOURCE_STATES state) { mCurrentState = state; }

			/**
			 * Returns a pointer to the stored current-state field, so external state-tracking code (e.g. a framebuffer
			 * attachment) can read and update the resource's tracked state in place.
			 *
			 * @note Resource state tracking assumes single-threaded command recording per resource (render thread +
			 *       internal work context) during bring-up; there is no cross-command-buffer locking on this field.
			 */
			D3D12_RESOURCE_STATES* GetCurrentStatePtr() { return &mCurrentState; }

		protected:
			/**
			 * Constructs an unmanaged resource with no owning D3D12ResourceManager. Reserved for subclasses that
			 * take responsibility for their own lifetime rather than being allocated through
			 * D3D12ResourceManager::Create — in particular the multiple-inheritance render resources
			 * (D3D12Texture, D3D12GpuBuffer) that derive both a core resource type and this class, and manage
			 * their native D3D12 objects directly. Chains to IGpuResource's protected unmanaged constructor.
			 */
			D3D12Resource() = default;

			D3D12_RESOURCE_STATES mCurrentState = D3D12_RESOURCE_STATE_COMMON;
		};

		/** @} */
	} // namespace render
} // namespace b3d
