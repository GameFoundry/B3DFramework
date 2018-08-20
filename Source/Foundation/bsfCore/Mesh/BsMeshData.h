//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsGpuResourceData.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "RenderAPI/BsIndexBuffer.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "RenderAPI/BsSubMesh.h"
#include "Math/BsBounds.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/** Iterator that allows you to easily populate or read vertex elements in MeshData. */
	template<class T>
	class VertexElemIter
	{
	public:
		VertexElemIter()
			:mData(nullptr), mEnd(nullptr), mByteStride(0), mNumElements(0)
		{

		}

		VertexElemIter(UINT8* data, UINT32 byteStride, UINT32 numElements)
			:mData(data), mByteStride(byteStride), mNumElements(numElements)
		{
			mEnd = mData + byteStride * numElements;
		}

		/**	
		 * Adds a new value to the iterators current position and advances the iterator. Returns true if there is more room
		 * in the container.
		 */
		bool addValue(const T& value)
		{
			setValue(value);
			return moveNext();
		}

		/**	Sets a new value at the iterators current position. */
		void setValue(const T& value)
		{
			memcpy(mData, &value, sizeof(T));
		}

		/**	Returns the value at the iterators current position. */
		T& getValue()
		{
			return *((T*)mData);
		}

		/**	Moves the iterator to the next position. Returns true if there are more elements. */
		bool moveNext()
		{
#ifdef BS_DEBUG_MODE
			if(mData >= mEnd)
			{
				BS_EXCEPT(InternalErrorException, "Vertex element iterator out of buffer bounds.");
			}
#endif

			mData += mByteStride;

			return mData <= mEnd;
		}

		/**	Returns the number of elements this iterator can iterate over. */
		UINT32 getNumElements() const { return mNumElements; }

	private:
		UINT8* mData;
		UINT8* mEnd;
		UINT32 mByteStride;
		UINT32 mNumElements;
	};

	/** Contains per-vertex bone weights and indexes used for skinning, for up to four bones. */
	struct BS_SCRIPT_EXPORT(pl:true,m:Utility) BoneWeight
	{
		int index0;
		int index1;
		int index2;
		int index3;

		float weight0;
		float weight1;
		float weight2;
		float weight3;
	};

	/** Contains mesh vertex and index data used for initializing, updating and reading mesh data from Mesh. */
	class BS_CORE_EXPORT MeshData : public GpuResourceData
	{
	public:
		/**
		 * Constructs a new object that can hold number of vertices described by the provided vertex data description. As 
		 * well as a number of indices of the provided type.
		 */
		MeshData(UINT32 numVertices, UINT32 numIndexes, const SPtr<VertexDataDesc>& vertexData, IndexType indexType = IT_32BIT);
		~MeshData();

		/**
		 * Copies data from @p data parameter into the internal buffer for the specified semantic.
		 *
		 * @param[in]	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param[in]	data			Vertex data, containing at least @p size bytes.
		 * @param[in]	size			The size of the data. Must be the size of the vertex element type * number of 
		 *								vertices.
		 * @param[in]	semanticIdx 	(optional) If there are multiple semantics with the same name, use different index
		 *								to differentiate between them.
		 * @param[in]	streamIdx   	(optional) Zero-based index of the stream. Each stream will internally be 
		 *								represented as a single vertex buffer.
		 */
		void setVertexData(VertexElementSemantic semantic, void* data, UINT32 size, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * Copies data from the internal buffer to the pre-allocated buffer for the specified semantic.
		 *
		 * @param[in]	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param[in]	data			Buffer that will receive vertex data, of at least @p size bytes.
		 * @param[in]	size			The size of the data. Must be the size of the vertex element type * number of 
		 *								vertices.
		 * @param[in]	semanticIdx 	(optional) If there are multiple semantics with the same name, use different index
		 *								to differentiate between them.
		 * @param[in]	streamIdx   	(optional) Zero-based index of the stream. Each stream will internally be 
		 *								represented as a single vertex buffer.
		 */
		void getVertexData(VertexElementSemantic semantic, void* data, UINT32 size, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * Returns an iterator you can use for easily retrieving or setting Vector2 vertex elements. This is the preferred
		 * method of assigning or reading vertex data. 
		 * 			
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIter<Vector2> getVec2DataIter(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * Returns an iterator you can use for easily retrieving or setting Vector3 vertex elements. This is the preferred
		 * method of assigning or reading vertex data. 
		 * 			
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIter<Vector3> getVec3DataIter(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * Returns an iterator you can use for easily retrieving or setting Vector4 vertex elements. This is the preferred
		 * method of assigning or reading vertex data. 
		 * 			
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIter<Vector4> getVec4DataIter(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * Returns an iterator you can use for easily retrieving or setting DWORD vertex elements. This is the preferred
		 * method of assigning or reading vertex data. 
		 * 			
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIter<UINT32> getDWORDDataIter(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/** Returns the total number of vertices this object can hold. */
		UINT32 getNumVertices() const { return mNumVertices; }

		/** Returns the total number of indices this object can hold. */
		UINT32 getNumIndices() const;

		/**	Returns a 16-bit pointer to the start of the internal index buffer. */
		UINT16* getIndices16() const;

		/**	Returns a 32-bit pointer to the start of the internal index buffer. */
		UINT32* getIndices32() const;

		/**	Returns the size of an index element in bytes. */
		UINT32 getIndexElementSize() const;

		/**	Returns the type of an index element. */
		IndexType getIndexType() const { return mIndexType; }

		/**
		 * Returns the pointer to the first element of the specified type. If you want to iterate over all elements you 
		 * need to call getVertexStride() to get the number	of bytes you need to advance between each element.
		 *
		 * @param[in]	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param[in]	semanticIdx 	(optional) If there are multiple semantics with the same name, use different index 
		 *								to differentiate between them.
		 * @param[in]	streamIdx   	(optional) Zero-based index of the stream. Each stream will internally be 
		 *								represented as a single vertex buffer.
		 * @return						null if it fails, else the element data.
		 */
		UINT8* getElementData(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

		/**
		 * Returns an offset into the internal buffer where this element with the provided semantic starts. Offset is 
		 * provided in number of bytes.
		 * 
		 * @param[in]	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param[in]	semanticIdx 	(optional) If there are multiple semantics with the same name, use different index 
		 *								to differentiate between them.
		 * @param[in]	streamIdx   	(optional) Zero-based index of the stream. Each stream will internally be 
		 *								represented as a single vertex buffer.
		 */
		UINT32 getElementOffset(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

		/**	Returns a pointer to the start of the index buffer. */
		UINT8* getIndexData() const { return getData(); }

		/**	Returns a pointer to the start of the specified vertex stream. */
		UINT8* getStreamData(UINT32 streamIdx) const;

		/**	Returns the size of the specified stream in bytes. */
		UINT32 getStreamSize(UINT32 streamIdx) const;

		/**	Returns the size of all the streams in bytes. */
		UINT32 getStreamSize() const;

		/** Returns an object that describes data contained in a single vertex. */
		const SPtr<VertexDataDesc>& getVertexDesc() const { return mVertexData; }

		/**	Return the size (in bytes) of the entire buffer. */
		UINT32 getSize() const { return getInternalBufferSize(); }

		/**	Calculates the bounds of all vertices stored in the internal buffer. */
		Bounds calculateBounds() const;

		/**
		 * Combines a number of submeshes and their mesh data into one large mesh data buffer.
		 *
		 * @param[in]	elements		Data containing vertices and indices referenced by the submeshes. Number of elements
		 *								must be the same as number of submeshes.
		 * @param[in]	allSubMeshes	Submeshes representing vertex and index range to take from mesh data and combine. 
		 *								Number of submeshes must match the number of provided MeshData elements.
		 * @param[out]	subMeshes		Outputs all combined sub-meshes with their new index and vertex offsets referencing
		 *								the newly created MeshData.
		 * @return						Combined mesh data containing all vertices and indexes references by the provided 
		 *								sub-meshes.
		 */
		static SPtr<MeshData> combine(const Vector<SPtr<MeshData>>& elements, const Vector<Vector<SubMesh>>& allSubMeshes,
			Vector<SubMesh>& subMeshes);

		/**
		 * Constructs a new object that can hold number of vertices described by the provided vertex data description. As 
		 * well as a number of indices of the provided type.
		 */
		static SPtr<MeshData> create(UINT32 numVertices, UINT32 numIndexes, const SPtr<VertexDataDesc>& vertexData, 
			IndexType indexType = IT_32BIT)
		{
			return bs_shared_ptr_new<MeshData>(numVertices, numIndexes, vertexData, indexType);
		}

	protected:
		/**	Returns the size of the internal buffer in bytes. */
		UINT32 getInternalBufferSize() const override;

	private:
		/**	Returns an offset in bytes to the start of the index buffer from the start of the internal buffer. */
		UINT32 getIndexBufferOffset() const;

		/**	Returns an offset in bytes to the start of the stream from the start of the internal buffer. */
		UINT32 getStreamOffset(UINT32 streamIdx = 0) const;

		/**	Returns the size of the index buffer in bytes. */
		UINT32 getIndexBufferSize() const;

		/**
		 * Returns the data needed for iterating over the requested vertex element.
		 *
		 * @param[in]	semantic   		Semantic of the element we are looking for.
		 * @param[in]	semanticIdx 	If there are multiple semantics with the same name, use different index to 
		 *								differentiate between them.
		 * @param[in]	streamIdx   	Zero-based index of the stream the element resides in.
		 * @param[out] data				Pointer to the start of this elements data.
		 * @param[out] stride			Number of bytes between vertex elements of this type.
		 */
		void getDataForIterator(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx, UINT8*& data, UINT32& stride) const;

	private:
		friend class Mesh;
		friend class ct::Mesh;
		friend class MeshHeap;
		friend class ct::MeshHeap;

		UINT32 mDescBuilding;

		UINT32 mNumVertices;
		UINT32 mNumIndices;
		IndexType mIndexType;

		SPtr<VertexDataDesc> mVertexData;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	private:
		MeshData(); // Serialization only

	public:
		friend class MeshDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}