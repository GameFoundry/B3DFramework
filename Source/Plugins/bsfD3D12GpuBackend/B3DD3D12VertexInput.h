//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"

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
		class D3D12VertexInput : public TD3D12Resource<GpuVertexInput>
		{
		public:
			/**
			 * @param	owner		Manager that takes care of tracking and releasing of this object.
			 * @param	id			Identifier which uniquely represents this vertex input configuration.
			 * @param	layout		Vertex buffer layout resolved against the vertex shader inputs.
			 */
			D3D12VertexInput(D3D12ResourceManager* owner, u32 id, const GpuVertexInputLayout& layout);

			/** Returns the input layout descriptor for use in a pipeline state descriptor. */
			const D3D12_INPUT_LAYOUT_DESC& GetLayoutDescription() const { return mLayoutDescription; }

			/** Returns the number of vertex buffer slots that are expected to be bound as vertex input. */
			u32 GetVertexBufferBindingCount() const { return mVertexBufferBindingCount; }

		private:
			Vector<D3D12_INPUT_ELEMENT_DESC> mElements;
			D3D12_INPUT_LAYOUT_DESC mLayoutDescription = {};
			u32 mVertexBufferBindingCount = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
