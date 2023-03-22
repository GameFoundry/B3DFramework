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
	 * Contains information about layout of vertices in a buffer.
	 *
	 * @note Thread safe (Immutable).
	 */
	class B3D_CORE_EXPORT VertexDataDesc : public IReflectable
	{
	public:
		VertexDataDesc(const SmallVector<VertexElement, 8>& elements);

		/**	Query if we have vertex data for the specified semantic. */
		bool HasElement(VertexElementSemantic semantic, u32 semanticIdx = 0, u32 streamIdx = 0) const;

		/**	Returns the size in bytes of the vertex element with the specified semantic. */
		u32 GetElementSize(VertexElementSemantic semantic, u32 semanticIdx = 0, u32 streamIdx = 0) const;

		/**	Returns offset of the vertex from start of the stream in bytes. */
		u32 GetElementOffsetFromStream(VertexElementSemantic semantic, u32 semanticIdx = 0, u32 streamIdx = 0) const;

		/**	Gets vertex stride in bytes (offset from one vertex to another) in the specified stream. */
		u32 GetVertexStride(u32 streamIdx) const;

		/**	Gets vertex stride in bytes (offset from one vertex to another) in all the streams. */
		u32 GetVertexStride() const;

		/**	Gets offset in bytes from the start of the internal buffer to the start of the specified stream. */
		u32 GetStreamOffset(u32 streamIdx) const;

		/**	Returns the number of vertex elements. */
		u32 GetNumElements() const { return (u32)mVertexElements.size(); }

		/**	Returns the vertex element at the specified index. */
		const VertexElement& GetElement(u32 index) const { return mVertexElements[index]; }

		/**	Returns the vertex element with the specified semantic. */
		const VertexElement* GetElement(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0) const;

		/** Returns all the elements of the definition. */
		const SmallVector<VertexElement, 8>& GetElements() const { return mVertexElements; }

	private:
		friend class Mesh;
		friend class ct::Mesh;

		VertexDataDesc() = default;

		/**	Returns the largest stream index of all the stored vertex elements. */
		u32 GetLargestStreamIndex() const;

		/**	Checks if any of the vertex elements use the specified stream index. */
		bool HasStream(u32 streamIndex) const;

		/** Calculates offsets of each vertex element, at which they will be stored in the vertex buffer. */
		void CalculateOffsets();

	private:
		SmallVector<VertexElement, 8> mVertexElements;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class VertexDataDescRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;
	};

	/** @} */
} // namespace bs
