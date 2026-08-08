//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuResourceManager.h"

namespace b3d
{
	namespace render
	{
		class D3D12ResourceManager;

		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/**
		 * Wraps a native D3D12 object, adding a device accessor on top of a generic GPU resource base (@p TBase).
		 * Use counting, lifetime state and deferred destruction all come from the base.
		 *
		 * @note Thread safe
		 */
		template<class TBase>
		class TD3D12Resource : public TBase
		{
		public:
			template<class... TBaseArgs>
			TD3D12Resource(D3D12ResourceManager* owner, TBaseArgs&&... baseArgs)
				: TBase(owner, std::forward<TBaseArgs>(baseArgs)...)
			{}

			/** Returns the device this resource is created on. */
			D3D12GpuDevice& GetDevice() const;
		};

		/** Standard D3D12 resource with no specialized generic role. */
		using D3D12Resource = TD3D12Resource<IGpuResource>;

		/** Tracked lifetime wrapper for a native D3D12 COM object referenced by command-list commands. */
		template<class TInterface>
		class TD3D12GpuObject : public D3D12Resource
		{
		public:
			TD3D12GpuObject(D3D12ResourceManager* owner, ComPtr<TInterface>&& object, const StringView& name = "")
				: D3D12Resource(owner, name), mObject(std::move(object))
			{}

			/** Returns the wrapped native object. */
			TInterface* Get() const { return mObject.Get(); }

		private:
			ComPtr<TInterface> mObject;
		};

		/** Tracked D3D12 pipeline state object. */
		using D3D12Pipeline = TD3D12GpuObject<ID3D12PipelineState>;

		/** Tracked D3D12 root signature. */
		using D3D12RootSignature = TD3D12GpuObject<ID3D12RootSignature>;

		/** @} */
	} // namespace render
} // namespace b3d
