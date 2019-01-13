//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "RenderAPI/BsVertexBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Container class consisting of a set of vertex buffers and their declaration.
	 *
	 * @note	Used just for more easily passing around vertex information.
	 */
	class BS_CORE_EXPORT VertexData
	{
    public:
        VertexData() = default;
        ~VertexData() = default;

		/**	Assigns a new vertex buffer to the specified index. */
		void setBuffer(UINT32 index, SPtr<VertexBuffer> buffer);

		/**	Retrieves a vertex buffer from the specified index. */
		SPtr<VertexBuffer> getBuffer(UINT32 index) const;

		/**	Returns a list of all bound vertex buffers. */
		const UnorderedMap<UINT32, SPtr<VertexBuffer>>& getBuffers() const { return mVertexBuffers; }

		/**	Checks if there is a buffer at the specified index. */
		bool isBufferBound(UINT32 index) const;

		/**	Gets total number of bound buffers. */
		UINT32 getBufferCount() const { return (UINT32)mVertexBuffers.size(); }

		/**	Returns the maximum index of all bound buffers. */
		UINT32 getMaxBufferIndex() const { return mMaxBufferIdx; }

		/**	Declaration used for the contained vertex buffers. */
		SPtr<VertexDeclaration> vertexDeclaration;

		/**	Number of vertices to use. */
		UINT32 vertexCount = 0;

	private:
		void recalculateMaxIndex();

		UnorderedMap<UINT32, SPtr<VertexBuffer>> mVertexBuffers;
		UINT32 mMaxBufferIdx = 0;
	};

	/** @} */
}}