//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DGpuResourceData.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "GpuBackend/B3DSubMesh.h"
#include "Math/B3DBounds.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Mesh
	 *  @{
	 */

	/** Iterator that allows you to easily populate or read vertex elements in MeshData. */
	template <class T>
	class VertexElemIterator
	{
	public:
		VertexElemIterator()
			: mData(nullptr), mEnd(nullptr), mByteStride(0), mNumElements(0)
		{
		}

		VertexElemIterator(u8* data, u32 byteStride, u32 numElements)
			: mData(data), mByteStride(byteStride), mNumElements(numElements)
		{
			mEnd = mData + byteStride * numElements;
		}

		/**
		 * Adds a new value to the iterators current position and advances the iterator. Returns true if there is more room
		 * in the container.
		 */
		bool AddValue(const T& value)
		{
			SetValue(value);
			return MoveNext();
		}

		/**	Sets a new value at the iterators current position. */
		void SetValue(const T& value)
		{
			memcpy(mData, &value, sizeof(T));
		}

		/**	Returns the value at the iterators current position. */
		T& GetValue()
		{
			return *((T*)mData);
		}

		/**	Moves the iterator to the next position. Returns true if there are more elements. */
		bool MoveNext()
		{
#ifdef B3D_DEBUG
			if(mData >= mEnd)
			{
				B3D_ASSERT(false);
				return true;
			}
#endif

			mData += mByteStride;

			return mData <= mEnd;
		}

		/**	Returns the number of elements this iterator can iterate over. */
		u32 GetNumElements() const { return mNumElements; }

	private:
		u8* mData;
		u8* mEnd;
		u32 mByteStride;
		u32 mNumElements;
	};

	/** Contains per-vertex bone weights and indexes used for skinning, for up to four bones. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Utility)) BoneWeight
	{
		int Index0;
		int Index1;
		int Index2;
		int Index3;

		float Weight0;
		float Weight1;
		float Weight2;
		float Weight3;
	};

	/** Contains mesh vertex and index data used for initializing, updating and reading mesh data from Mesh. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() MeshData : public GpuResourceData, public IScriptExportable
	{
	public:
		/**
		 * Constructs a new object that can hold number of vertices described by the provided vertex data description. As
		 * well as a number of indices of the provided type.
		 */
		MeshData(u32 vertexCount, u32 indexCount, const TShared<VertexDescription>& vertexDescription, IndexType indexType = IT_32BIT);
		~MeshData();

		/**
		 * Copies data from @p data parameter into the internal buffer for the specified semantic.
		 *
		 * @param	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param	data			Vertex data, containing at least @p size bytes.
		 * @param	size			The size of the data. Must be the size of the vertex element type * number of
		 *								vertices.
		 * @param	semanticIndex 	(optional) If there are multiple semantics with the same name, use different index
		 *								to differentiate between them.
		 * @param	streamIndex   	(optional) Zero-based index of the stream. Each stream will internally be
		 *								represented as a single vertex buffer.
		 */
		void SetVertexData(VertexElementSemantic semantic, void* data, u32 size, u32 semanticIndex = 0, u32 streamIndex = 0);

		/**
		 * Copies data from the internal buffer to the pre-allocated buffer for the specified semantic.
		 *
		 * @param	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param	data			Buffer that will receive vertex data, of at least @p size bytes.
		 * @param	size			The size of the data. Must be the size of the vertex element type * number of
		 *								vertices.
		 * @param	semanticIndex 	(optional) If there are multiple semantics with the same name, use different index
		 *								to differentiate between them.
		 * @param	streamIndex   	(optional) Zero-based index of the stream. Each stream will internally be
		 *								represented as a single vertex buffer.
		 */
		void GetVertexData(VertexElementSemantic semantic, void* data, u32 size, u32 semanticIndex = 0, u32 streamIndex = 0);

		/**
		 * Returns an iterator you can use for easily retrieving or setting Vector2 vertex elements. This is the preferred
		 * method of assigning or reading vertex data.
		 *
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIterator<Vector2> GetVec2DataIter(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0);

		/**
		 * Returns an iterator you can use for easily retrieving or setting Vector3 vertex elements. This is the preferred
		 * method of assigning or reading vertex data.
		 *
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIterator<Vector3> GetVec3DataIter(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0);

		/**
		 * Returns an iterator you can use for easily retrieving or setting Vector4 vertex elements. This is the preferred
		 * method of assigning or reading vertex data.
		 *
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIterator<Vector4> GetVec4DataIter(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0);

		/**
		 * Returns an iterator you can use for easily retrieving or setting DWORD vertex elements. This is the preferred
		 * method of assigning or reading vertex data.
		 *
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIterator<u32> GetDwordDataIter(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0);

		/**
		 * Returns an iterator you can use for easily retrieving or setting vertex elements. This is the preferred
		 * method of assigning or reading vertex data.
		 *
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		template<class T>
		VertexElemIterator<T> GetVertexIterator(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0)
		{
				u8* data;
				u32 vertexStride;
				GetDataForIterator(semantic, semanticIndex, streamIndex, data, vertexStride);

				return VertexElemIterator<T>(data, vertexStride, mVertexCount);
		}

		/** Returns the total number of vertices this object can hold. */
		u32 GetVertexCount() const { return mVertexCount; }

		/** Returns the total number of indices this object can hold. */
		u32 GetIndexCount() const { return mIndexCount; }

		/**	Returns a 16-bit pointer to the start of the internal index buffer. */
		u16* GetIndices16() const;

		/**	Returns a 32-bit pointer to the start of the internal index buffer. */
		u32* GetIndices32() const;

		/**	Returns the size of an index element in bytes. */
		u32 GetIndexElementSize() const { return mIndexType == IT_32BIT ? sizeof(u32) : sizeof(u16); }

		/**	Returns the type of an index element. */
		IndexType GetIndexType() const { return mIndexType; }

		/**
		 * Returns the pointer to the first element of the specified type. If you want to iterate over all elements you
		 * need to call getVertexStride() to get the number	of bytes you need to advance between each element.
		 *
		 * @param	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param	semanticIndex 	(optional) If there are multiple semantics with the same name, use different index
		 *								to differentiate between them.
		 * @param	streamIndex   	(optional) Zero-based index of the stream. Each stream will internally be
		 *								represented as a single vertex buffer.
		 * @return						null if it fails, else the element data.
		 */
		u8* GetElementData(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0) const;

		/**
		 * Returns an offset into the internal buffer where this element with the provided semantic starts. Offset is
		 * provided in number of bytes.
		 *
		 * @param	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param	semanticIndex 	(optional) If there are multiple semantics with the same name, use different index
		 *								to differentiate between them.
		 * @param	streamIndex   	(optional) Zero-based index of the stream. Each stream will internally be
		 *								represented as a single vertex buffer.
		 */
		u32 GetElementOffset(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0) const;

		/**	Returns a pointer to the start of the index buffer. */
		u8* GetIndexData() const { return GetData(); }

		/**	Returns a pointer to the start of the specified vertex stream. */
		u8* GetStreamData(u32 streamIndex) const;

		/**	Returns the size of the specified stream in bytes. */
		u32 GetStreamSize(u32 streamIndex) const;

		/**	Returns the size of all the streams in bytes. */
		u32 GetStreamSize() const;

		/** Returns an object that describes data contained in a single vertex. */
		const TShared<VertexDescription>& GetVertexDescription() const { return mVertexDescription; }

		/**	Return the size (in bytes) of the entire buffer. */
		u32 GetSize() const { return GetInternalBufferSize(); }

		/**	Calculates the bounds of all vertices stored in the internal buffer. */
		Bounds CalculateBounds() const;

		/**
		 * Combines a number of submeshes and their mesh data into one large mesh data buffer.
		 *
		 * @param	elements		Data containing vertices and indices referenced by the submeshes. Number of elements
		 *								must be the same as number of submeshes.
		 * @param	allSubMeshes	Submeshes representing vertex and index range to take from mesh data and combine.
		 *								Number of submeshes must match the number of provided MeshData elements.
		 * @param	subMeshes		Outputs all combined sub-meshes with their new index and vertex offsets referencing
		 *								the newly created MeshData.
		 * @return						Combined mesh data containing all vertices and indexes references by the provided
		 *								sub-meshes.
		 */
		static TShared<MeshData> Combine(const Vector<TShared<MeshData>>& elements, const Vector<Vector<SubMesh>>& allSubMeshes, Vector<SubMesh>& subMeshes);

		/**
		 * Constructs a new object that can hold number of vertices described by the provided vertex data description. As
		 * well as a number of indices of the provided type.
		 */
		static TShared<MeshData> Create(u32 vertexCount, u32 indexCount, const TShared<VertexDescription>& vertexDescription, IndexType indexType = IT_32BIT)
		{
			return B3DMakeShared<MeshData>(vertexCount, indexCount, vertexDescription, indexType);
		}

	protected:
		/**	Returns the size of the internal buffer in bytes. */
		u32 GetInternalBufferSize() const override;

	private:
		/**	Returns an offset in bytes to the start of the index buffer from the start of the internal buffer. */
		u32 GetIndexBufferOffset() const;

		/**	Returns an offset in bytes to the start of the stream from the start of the internal buffer. */
		u32 GetStreamOffset(u32 streamIdx = 0) const;

		/**	Returns the size of the index buffer in bytes. */
		u32 GetIndexBufferSize() const;

		/**
		 * Returns the data needed for iterating over the requested vertex element.
		 *
		 * @param	semantic   		Semantic of the element we are looking for.
		 * @param	semanticIndex 	If there are multiple semantics with the same name, use different index to
		 *								differentiate between them.
		 * @param	streamIndex   	Zero-based index of the stream the element resides in.
		 * @param	data			Pointer to the start of this elements data.
		 * @param	stride			Number of bytes between vertex elements of this type.
		 */
		void GetDataForIterator(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex, u8*& data, u32& stride) const;

	private:
		friend class Mesh;
		friend class render::Mesh;

		u32 mDescBuilding;

		u32 mVertexCount;
		u32 mIndexCount;
		IndexType mIndexType;

		TShared<VertexDescription> mVertexDescription;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	private:
		MeshData(); // Serialization only

	public:
		friend class MeshDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
