//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsVertexDeclaration.h"
#include "BsVertexBuffer.h"

namespace bs 
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
        VertexData();
        ~VertexData();

		/**	Assigns a new vertex buffer to the specified index. */
		void setBuffer(UINT32 index, SPtr<VertexBufferCore> buffer);

		/**	Retrieves a vertex buffer from the specified index. */
		SPtr<VertexBufferCore> getBuffer(UINT32 index) const;

		/**	Returns a list of all bound vertex buffers. */
		const UnorderedMap<UINT32, SPtr<VertexBufferCore>>& getBuffers() const { return mVertexBuffers; }

		/**	Checks if there is a buffer at the specified index. */
		bool isBufferBound(UINT32 index) const;

		/**	Gets total number of bound buffers. */
		UINT32 getBufferCount() const { return (UINT32)mVertexBuffers.size(); }

		/**	Returns the maximum index of all bound buffers. */
		UINT32 getMaxBufferIndex() const { return mMaxBufferIdx; }

		/**	Declaration used for the contained vertex buffers. */
		SPtr<VertexDeclarationCore> vertexDeclaration;

		/**	Number of vertices to use. */
		UINT32 vertexCount;

	private:
		void recalculateMaxIndex();

		UnorderedMap<UINT32, SPtr<VertexBufferCore>> mVertexBuffers;
		UINT32 mMaxBufferIdx = 0;
	};

	/** @} */
}