//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsVertexDeclaration.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**
	 * Contains information about layout of vertices in a buffer. This is very similar to VertexDeclaration but unlike
	 * VertexDeclaration it has no render API object to back it up and is very lightweight.
	 */
	class BS_CORE_EXPORT VertexDataDesc : public IReflectable
	{
	public:
		VertexDataDesc() = default;

		/**
		 * Informs the internal buffer that it needs to make room for the specified vertex element. If a vertex with same 
		 * stream and semantics already exists it will just be updated. 
		 *
		 * @param[in]	type	   			Type of the vertex element. Determines size.
		 * @param[in]	semantic			Semantic that allows the engine to connect the data to a shader input slot.
		 * @param[in]	semanticIdx			(optional) If there are multiple semantics with the same name, use different
		 *									index to differentiate between them.
		 * @param[in]	streamIdx  			(optional) Zero-based index of the stream. Each stream will internally be
		 *									represented as a single vertex buffer.
		 * @param[in]	instanceStepRate	Determines at what rate does vertex element data advance. Zero means each vertex
		 *									will advance the data pointer and receive new data (standard behaviour). Values
		 *									larger than one are relevant for instanced rendering and determine how often do
		 *									instances advance the vertex element (for example a value of 1 means each
		 *									instance will retrieve a new value for this vertex element, a value of 2 means
		 *									each second instance will, etc.).
		 */
		void addVertElem(VertexElementType type, VertexElementSemantic semantic, UINT32 semanticIdx = 0, 
			UINT32 streamIdx = 0, UINT32 instanceStepRate = 0);

		/**	Query if we have vertex data for the specified semantic. */
		bool hasElement(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

		/**	Returns the size in bytes of the vertex element with the specified semantic. */
		UINT32 getElementSize(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

		/**	Returns offset of the vertex from start of the stream in bytes. */
		UINT32 getElementOffsetFromStream(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

		/**	Gets vertex stride in bytes (offset from one vertex to another) in the specified stream. */
		UINT32 getVertexStride(UINT32 streamIdx) const;

		/**	Gets vertex stride in bytes (offset from one vertex to another) in all the streams. */
		UINT32 getVertexStride() const;

		/**	Gets offset in bytes from the start of the internal buffer to the start of the specified stream. */
		UINT32 getStreamOffset(UINT32 streamIdx) const;

		/**	Returns the number of vertex elements. */
		UINT32 getNumElements() const { return (UINT32)mVertexElements.size(); }

		/**	Returns the vertex element at the specified index. */
		const VertexElement& getElement(UINT32 idx) const { return mVertexElements[idx]; }

		/**	Returns the vertex element with the specified semantic. */
		const VertexElement* getElement(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

		/**	Creates a list of vertex elements from internal data. */
		Vector<VertexElement> createElements() const;

		/**	Creates a new empty vertex data descriptor. */
		static SPtr<VertexDataDesc> create();

	private:
		friend class Mesh;
		friend class ct::Mesh;
		friend class MeshHeap;
		friend class ct::MeshHeap;

		/**	Returns the largest stream index of all the stored vertex elements. */
		UINT32 getMaxStreamIdx() const;

		/**	Checks if any of the vertex elements use the specified stream index. */
		bool hasStream(UINT32 streamIdx) const;

		/**	Removes a vertex element of the specified type and semantics if it exists. */
		void clearIfItExists(VertexElementType type, VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx);

	private:
		Vector<VertexElement> mVertexElements;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class VertexDataDescRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}