//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsMeshData.h"

namespace bs
{
	/** cond INTERNAL */
	/** @addtogroup Renderer
	 *  @{
	 */

	/**	Available vertex layouts (Combinations other than those provided are allowed). */
	enum class VertexLayout
	{
		Position = 0x01,
		Color = 0x02,
		Normal = 0x04,
		Tangent = 0x08,
		BoneWeights = 0x10,
		UV0 = 0x20,
		UV1 = 0x40,
		PC = Position | Color,
		PU = Position | UV0,
		PCU = Position | Color | UV0,
		PCN = Position | Color | Normal,
		PCNU = Position | Color | Normal | UV0,
		PCNT = Position | Color | Normal | Tangent,
		PCNTU = Position | Color | Normal | Tangent | UV0,
		PN = Position | Normal,
		PNU = Position | Normal | UV0,
		PNT = Position | Normal | Tangent,
		PNTU = Position | Normal | Tangent | UV0,
	};

	/**
	 * Wrapper around MeshData that constructs the default mesh data structure expected by the renderer and other engine 
	 * systems. Data will be compressed and uncompressed when written to and read to as needed to comply with wanted format.
	 *			
	 * @note	This is the default implementation while the Renderer plugins can override it by overriding
	 *			createMeshData() method in their Renderer implementation.
	 */
	class BS_CORE_EXPORT RendererMeshData
	{
	public:
		/**
		 * Reads the vertex positions into the provided output buffer. Data will be copied and potentially uncompressed to
		 * fit the output format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to output the position data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices * 
		 *						sizeof(Vector3)).
		 */
		void getPositions(Vector3* buffer, UINT32 size);

		/**
		 * Writes the vertex positions from the provided output buffer. Data will be copied and potentially compressed to
		 * fit the internal mesh data format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to read the position data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector3)).
		 */
		void setPositions(Vector3* buffer, UINT32 size);

		/**
		 * Reads the vertex normals into the provided output buffer. Data will be copied and potentially uncompressed to 
		 * fit the output format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to output the normal data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices * 
		 *						sizeof(Vector3)).
		 */
		void getNormals(Vector3* buffer, UINT32 size);

		/**
		 * Writes the vertex normals from the provided output buffer. Data will be copied and potentially compressed to 
		 * fit the internal mesh data format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to read the normal data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector3)).
		 */
		void setNormals(Vector3* buffer, UINT32 size);

		/**
		 * Reads the vertex tangents into the provided output buffer. Data will be copied and potentially uncompressed to 
		 * fit the output format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to output the tangent data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices * 
		 *						sizeof(Vector4)).
		 */
		void getTangents(Vector4* buffer, UINT32 size);

		/**
		 * Writes the vertex tangents from the provided output buffer. Data will be copied and potentially compressed to 
		 * fit the internal mesh data format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to read the tangent data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector4)).
		 */
		void setTangents(Vector4* buffer, UINT32 size);

		/**
		 * Reads the vertex colors into the provided output buffer. Data will be copied and potentially uncompressed to 
		 * fit the output format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to output the color data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices * 
		 *						sizeof(Color)).
		 */
		void getColors(Color* buffer, UINT32 size);

		/**
		 * Writes the vertex colors from the provided output buffer. Data will be copied and potentially compressed to 
		 * fit the internal mesh data format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to read the color data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Color)).
		 */
		void setColors(Color* buffer, UINT32 size);

		/**
		 * Writes the vertex colors from the provided output buffer. Data will be copied and potentially compressed to 
		 * fit the internal mesh data format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to read the color data from. Colors should be in RGBA format.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(UINT32)).
		 */
		void setColors(UINT32* buffer, UINT32 size);

		/**
		 * Reads the first UV channel coordinates into the provided output buffer. Data will be copied and potentially 
		 * uncompressed to fit the output format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to output the coordinate data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices * 
		 *						sizeof(Vector2)).
		 */
		void getUV0(Vector2* buffer, UINT32 size);

		/**
		 * Writes the first UV channel coordinates from the provided output buffer. Data will be copied and potentially 
		 * compressed to fit the internal mesh data format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to read the coordinate data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector2)).
		 */
		void setUV0(Vector2* buffer, UINT32 size);

		/**
		 * Reads the second UV channel coordinates into the provided output buffer. Data will be copied and potentially 
		 * uncompressed to fit the output format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to output the coordinate data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices * 
		 *						sizeof(Vector2)).
		 */
		void getUV1(Vector2* buffer, UINT32 size);

		/**
		 * Writes the second UV channel coordinates from the provided output buffer. Data will be copied and potentially 
		 * compressed to fit the internal mesh data format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to read the coordinate data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector2)).
		 */
		void setUV1(Vector2* buffer, UINT32 size);

		/**
		 * Reads the bone weights and indices into the provided output buffer. Data will be copied and potentially 
		 * uncompressed to fit the output format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to output the bone weight data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices * 
		 *						sizeof(BoneWeight)).
		 */
		void getBoneWeights(BoneWeight* buffer, UINT32 size);

		/**
		 * Writes the bone weights and indices from the provided output buffer. Data will be copied and potentially 
		 * compressed to fit the internal mesh data format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to read the bone weight data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(BoneWeight)).
		 */
		void setBoneWeights(BoneWeight* buffer, UINT32 size);

		/**
		 * Reads the indices into the provided output buffer. Data will be copied and potentially uncompressed to fit the 
		 * output format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to output the index data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices * 
		 *						sizeof(INT32)).
		 */
		void getIndices(UINT32* buffer, UINT32 size);

		/**
		 * Writes the indices from the provided output buffer. Data will be copied and potentially compressed to fit the 
		 * internal mesh data format as needed.
		 *			
		 * @param[in]	buffer	Pre-allocated buffer to read the index data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(INT32)).
		 */
		void setIndices(UINT32* buffer, UINT32 size);

		/**	Returns the underlying MeshData structure. */
		SPtr<MeshData> getData() const { return mMeshData; }

		/**	Creates a new empty mesh data structure. */
		static SPtr<RendererMeshData> create(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType = IT_32BIT);

		/**	Creates a new mesh data structure using an existing mesh data buffer. */
		static SPtr<RendererMeshData> create(const SPtr<MeshData>& meshData);

		/**	Creates a vertex descriptor from a vertex layout enum. */
		static SPtr<VertexDataDesc> vertexLayoutVertexDesc(VertexLayout type);

	private:
		friend class CoreRenderer;

		RendererMeshData(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType = IT_32BIT);
		RendererMeshData(const SPtr<MeshData>& meshData);

		SPtr<MeshData> mMeshData;
	};

	/** @} */
	/** @endcond */
}