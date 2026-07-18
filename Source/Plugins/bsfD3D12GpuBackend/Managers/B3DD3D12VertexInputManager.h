//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/**
		 * Contains data describing vertex inputs for a graphics pipeline. Owns the input element array the layout
		 * descriptor points to, so the descriptor stays valid for the lifetime of this object.
		 */
		class D3D12VertexInput
		{
		public:
			/**
			 * @param	id			Identifier which uniquely represents this vertex input configuration.
			 * @param	layout		Vertex buffer layout resolved against the vertex shader inputs.
			 */
			D3D12VertexInput(u32 id, const GpuVertexInputLayout& layout);

			/** Returns the input layout descriptor for use in a pipeline state descriptor. */
			const D3D12_INPUT_LAYOUT_DESC& GetLayoutDesc() const { return mLayoutDesc; }

			/** Returns an identifier which uniquely represents this vertex input configuration. */
			u32 GetId() const { return mId; }

			/** Returns the number of vertex buffer slots that are expected to be bound as vertex input. */
			u32 GetVertexBufferBindingCount() const { return mVertexBufferBindingCount; }

		private:
			u32 mId;

			Vector<D3D12_INPUT_ELEMENT_DESC> mElements;
			D3D12_INPUT_LAYOUT_DESC mLayoutDesc = {};
			u32 mVertexBufferBindingCount = 0;
		};

		class D3D12VertexInputManager;
		extern template class TGpuVertexInputManager<D3D12VertexInputManager, TShared<D3D12VertexInput>>;

		/**
		 * Maps vertex buffer structure and vertex shader inputs in order to create vertex input descriptions usable
		 * by DirectX 12 input layouts.
		 */
		class D3D12VertexInputManager : public Module<D3D12VertexInputManager>, public TGpuVertexInputManager<D3D12VertexInputManager, TShared<D3D12VertexInput>>
		{
		public:
			~D3D12VertexInputManager();

		private:
			friend class TGpuVertexInputManager<D3D12VertexInputManager, TShared<D3D12VertexInput>>;

			/** Creates a vertex input object for the provided resolved layout. */
			TShared<D3D12VertexInput> CreateVertexInput(const GpuVertexInputLayout& layout);

			/** Releases a vertex input object evicted from the cache. */
			void DestroyVertexInput(TShared<D3D12VertexInput>& vertexInput);

			u32 mNextId = 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
