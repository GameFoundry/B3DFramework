//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Mesh/B3DMeshData.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** cond INTERNAL */
	/** @addtogroup Renderer
	 *  @{
	 */

	/**
	 * Available vertex layouts that specify what data is provided per-vertex in a mesh. Combinations other than those
	 * provided are allowed.
	 */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) VertexLayout
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

	/** Contains mesh vertex and index data used for initializing, updating and reading mesh data from Mesh. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) RendererMeshData : public IScriptExportable
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
		void GetPositions(Vector3* buffer, u32 size);

		/**
		 * Writes the vertex positions from the provided output buffer. Data will be copied and potentially compressed to
		 * fit the internal mesh data format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to read the position data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector3)).
		 */
		void SetPositions(Vector3* buffer, u32 size);

		/**
		 * Reads the vertex normals into the provided output buffer. Data will be copied and potentially uncompressed to
		 * fit the output format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to output the normal data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices *
		 *						sizeof(Vector3)).
		 */
		void GetNormals(Vector3* buffer, u32 size);

		/**
		 * Writes the vertex normals from the provided output buffer. Data will be copied and potentially compressed to
		 * fit the internal mesh data format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to read the normal data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector3)).
		 */
		void SetNormals(Vector3* buffer, u32 size);

		/**
		 * Reads the vertex tangents into the provided output buffer. Data will be copied and potentially uncompressed to
		 * fit the output format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to output the tangent data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices *
		 *						sizeof(Vector4)).
		 */
		void GetTangents(Vector4* buffer, u32 size);

		/**
		 * Writes the vertex tangents from the provided output buffer. Data will be copied and potentially compressed to
		 * fit the internal mesh data format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to read the tangent data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector4)).
		 */
		void SetTangents(Vector4* buffer, u32 size);

		/**
		 * Reads the vertex colors into the provided output buffer. Data will be copied and potentially uncompressed to
		 * fit the output format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to output the color data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices *
		 *						sizeof(Color)).
		 */
		void GetColors(Color* buffer, u32 size);

		/**
		 * Writes the vertex colors from the provided output buffer. Data will be copied and potentially compressed to
		 * fit the internal mesh data format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to read the color data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Color)).
		 */
		void SetColors(Color* buffer, u32 size);

		/**
		 * Writes the vertex colors from the provided output buffer. Data will be copied and potentially compressed to
		 * fit the internal mesh data format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to read the color data from. Colors should be in RGBA format.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(u32)).
		 */
		void SetColors(u32* buffer, u32 size);

		/**
		 * Reads the first UV channel coordinates into the provided output buffer. Data will be copied and potentially
		 * uncompressed to fit the output format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to output the coordinate data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices *
		 *						sizeof(Vector2)).
		 */
		void GetUV0(Vector2* buffer, u32 size);

		/**
		 * Writes the first UV channel coordinates from the provided output buffer. Data will be copied and potentially
		 * compressed to fit the internal mesh data format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to read the coordinate data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector2)).
		 */
		void SetUV0(Vector2* buffer, u32 size);

		/**
		 * Reads the second UV channel coordinates into the provided output buffer. Data will be copied and potentially
		 * uncompressed to fit the output format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to output the coordinate data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices *
		 *						sizeof(Vector2)).
		 */
		void GetUV1(Vector2* buffer, u32 size);

		/**
		 * Writes the second UV channel coordinates from the provided output buffer. Data will be copied and potentially
		 * compressed to fit the internal mesh data format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to read the coordinate data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(Vector2)).
		 */
		void SetUV1(Vector2* buffer, u32 size);

		/**
		 * Reads the bone weights and indices into the provided output buffer. Data will be copied and potentially
		 * uncompressed to fit the output format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to output the bone weight data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices *
		 *						sizeof(BoneWeight)).
		 */
		void GetBoneWeights(BoneWeight* buffer, u32 size);

		/**
		 * Writes the bone weights and indices from the provided output buffer. Data will be copied and potentially
		 * compressed to fit the internal mesh data format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to read the bone weight data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(BoneWeight)).
		 */
		void SetBoneWeights(BoneWeight* buffer, u32 size);

		/**
		 * Reads the indices into the provided output buffer. Data will be copied and potentially uncompressed to fit the
		 * output format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to output the index data to.
		 * @param[in]	size	Size of the pre-allocated buffer. Must be big enough to fit all contents (numVertices *
		 *						sizeof(i32)).
		 */
		void GetIndices(u32* buffer, u32 size);

		/**
		 * Writes the indices from the provided output buffer. Data will be copied and potentially compressed to fit the
		 * internal mesh data format as needed.
		 *
		 * @param[in]	buffer	Pre-allocated buffer to read the index data from.
		 * @param[in]	size	Size of the input buffer. Must be (numVertices * sizeof(i32)).
		 */
		void SetIndices(u32* buffer, u32 size);

		/**	Returns the underlying MeshData structure. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Data))
		SPtr<MeshData> GetData() const { return mMeshData; }

		/**	Creates a new empty mesh data structure. */
		static SPtr<RendererMeshData> Create(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType = IT_32BIT);

		/**	Creates a new mesh data structure using an existing mesh data buffer. */
		static SPtr<RendererMeshData> Create(const SPtr<MeshData>& meshData);

		/**	Creates a vertex descriptor from a vertex layout enum. */
		static SPtr<VertexDescription> VertexLayoutVertexDesc(VertexLayout type);

		/** Converts a generic mesh data into mesh data format expected by the renderer. */
		static SPtr<MeshData> Convert(const SPtr<MeshData>& meshData);

	private:
		friend class render::Renderer;

		RendererMeshData(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType = IT_32BIT);
		RendererMeshData(const SPtr<MeshData>& meshData);

		SPtr<MeshData> mMeshData;
	};

	/** @} */
	/** @endcond */
} // namespace b3d
