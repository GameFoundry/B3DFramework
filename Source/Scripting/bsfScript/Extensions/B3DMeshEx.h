//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Mesh/B3DMesh.h"
#include "Renderer/B3DRendererMeshData.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for Mesh, for adding additional functionality for the script version of the class. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(Mesh)) MeshEx
	{
	public:
		/**
		 * Creates a new mesh with enough space to hold the a number of primitives using the specified layout. All indices
		 * will be part of a single sub-mesh.
		 *
		 * @param[in]	numVertices		Number of vertices in the mesh.
		 * @param[in]	numIndices		Number of indices in the mesh. Must be a multiple of primitive size as specified
		 *								by provided topology.
		 * @param[in]	topology		Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is a triangle list, where three indices represent a single triangle.
		 * @param[in]	flags			Flags to control various mesh options.
		 * @param[in]	vertex			Controls how are vertices organized in the vertex buffer and what data they contain.
		 * @param[in]	index			Size of indices, use smaller size for better performance, however be careful not to
		 *								go over the number of vertices limited by the data type size.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Mesh))
		static HMesh Create(int numVertices, int numIndices, DrawOperationType topology = DOT_TRIANGLE_LIST, MeshFlags flags = MeshFlag::Static, VertexLayout vertex = VertexLayout::Position, IndexType index = IT_32BIT);

		/**
		 * Creates a new mesh with enough space to hold the a number of primitives using the specified layout. Indices can
		 * be referenced by multiple sub-meshes.
		 *
		 * @param[in]	numVertices		Number of vertices in the mesh.
		 * @param[in]	numIndices		Number of indices in the mesh. Must be a multiple of primitive size as specified
		 *								by provided topology.
		 * @param[in]	subMeshes		Defines how are indices separated into sub-meshes, and how are those sub-meshes
		 *								rendered. Sub-meshes may be rendered independently, each with a different material.
		 * @param[in]	flags			Flags to control various mesh options.
		 * @param[in]	vertex			Controls how are vertices organized in the vertex buffer and what data they contain.
		 * @param[in]	index			Size of indices, use smaller size for better performance, however be careful not to
		 *								go over the number of vertices limited by the data type size.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Mesh))
		static HMesh Create(int numVertices, int numIndices, const Vector<SubMesh>& subMeshes, MeshFlags flags = MeshFlag::Static, VertexLayout vertex = VertexLayout::Position, IndexType index = IT_32BIT);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 *
		 * @param[in]	data			Vertex and index data to initialize the mesh with.
		 * @param[in]	topology		Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is a triangle list, where three indices represent a single triangle.
		 * @param[in]	flags			Flags to control various mesh options.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Mesh))
		static HMesh Create(const SPtr<RendererMeshData>& data, DrawOperationType topology = DOT_TRIANGLE_LIST, MeshFlags flags = MeshFlag::Static);

		/**
		 * Creates a new mesh with enough space to hold the a number of primitives using the specified layout. Indices can
		 * be referenced by multiple sub-meshes.
		 *
		 * @param[in]	data			Vertex and index data to initialize the mesh with.
		 * @param[in]	subMeshes		Defines how are indices separated into sub-meshes, and how are those sub-meshes
		 *								rendered. Sub-meshes may be rendered independently, each with a different material.
		 * @param[in]	flags			Flags to control various mesh options.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Mesh))
		static HMesh Create(const SPtr<RendererMeshData>& data, const Vector<SubMesh>& subMeshes, MeshFlags flags = MeshFlag::Static);

		/** Returns all sub-meshes contained in the mesh. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Mesh), Property(Getter), ExportName(SubMeshes))
		static Vector<SubMesh> GetSubMeshes(const HMesh& thisPtr);

		/** Returns the number of sub-meshes contained in this mesh. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Mesh), Property(Getter), ExportName(SubMeshCount))
		static u32 GetSubMeshCount(const HMesh& thisPtr);

		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Mesh), InteropOnly(true))
		static void GetBounds(const HMesh& thisPtr, AABox* box, Sphere* sphere);

		/**
		 * Accesses the vertex and index data of the mesh. If reading, mesh must have been created with the
		 * MeshUsage::CPUCached flag. If writing the caller must ensure the data matches mesh's vertex/index counts, vertex
		 * layout and index format.
		 */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Mesh), Property(Getter), ExportName(MeshData))
		static SPtr<RendererMeshData> GetMeshData(const HMesh& thisPtr);
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Mesh), Property(Setter), ExportName(MeshData))
		static void SetMeshData(const HMesh& thisPtr, const SPtr<RendererMeshData>& value);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
