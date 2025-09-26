//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DVertexDescription.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderAPI
		 *  @{
		 */

		/**
		 * Container class consisting of a set of vertex buffers and their declaration.
		 *
		 * @note	Used just for more easily passing around vertex information.
		 */
		class B3D_EXPORT VertexData
		{
		public:
			VertexData() = default;
			~VertexData() = default;

			/**	Assigns a new vertex buffer to the specified index. */
			void SetBuffer(u32 index, SPtr<GpuBuffer> buffer);

			/**	Retrieves a vertex buffer from the specified index. */
			SPtr<GpuBuffer> GetBuffer(u32 index) const;

			/**	Returns a list of all bound vertex buffers. */
			const UnorderedMap<u32, SPtr<GpuBuffer>>& GetBuffers() const { return mVertexBuffers; }

			/**	Checks if there is a buffer at the specified index. */
			bool IsBufferBound(u32 index) const;

			/**	Gets total number of bound buffers. */
			u32 GetBufferCount() const { return (u32)mVertexBuffers.size(); }

			/**	Returns the maximum index of all bound buffers. */
			u32 GetMaxBufferIndex() const { return mMaxBufferIdx; }

			/**	Declaration used for the contained vertex buffers. */
			SPtr<VertexDescription> VertexDescription;

			/**	Number of vertices to use. */
			u32 VertexCount = 0;

		private:
			void RecalculateMaxIndex();

			UnorderedMap<u32, SPtr<GpuBuffer>> mVertexBuffers;
			u32 mMaxBufferIdx = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
