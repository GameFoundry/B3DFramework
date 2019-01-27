//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsRendererMeshData.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for Mesh, for adding additional functionality for the script version of the class. */
	class BS_SCRIPT_EXPORT(e:Mesh) MeshEx
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
		 * @param[in]	usage			Optimizes performance depending on planned usage of the mesh.
		 * @param[in]	vertex			Controls how are vertices organized in the vertex buffer and what data they contain.
		 * @param[in]	index			Size of indices, use smaller size for better performance, however be careful not to
		 *								go over the number of vertices limited by the data type size.
		 */
		BS_SCRIPT_EXPORT(ec:Mesh)
		static HMesh create(int numVertices, int numIndices, DrawOperationType topology = DOT_TRIANGLE_LIST,
			MeshUsage usage = MU_STATIC, VertexLayout vertex = VertexLayout::Position, IndexType index = IT_32BIT);

		/**
		 * Creates a new mesh with enough space to hold the a number of primitives using the specified layout. Indices can
		 * be referenced by multiple sub-meshes.
		 * 
		 * @param[in]	numVertices		Number of vertices in the mesh.
		 * @param[in]	numIndices		Number of indices in the mesh. Must be a multiple of primitive size as specified
		 *								by provided topology.
		 * @param[in]	subMeshes		Defines how are indices separated into sub-meshes, and how are those sub-meshes
		 *								rendered. Sub-meshes may be rendered independently, each with a different material.
		 * @param[in]	usage			Optimizes performance depending on planned usage of the mesh.
		 * @param[in]	vertex			Controls how are vertices organized in the vertex buffer and what data they contain.
		 * @param[in]	index			Size of indices, use smaller size for better performance, however be careful not to
		 *								go over the number of vertices limited by the data type size.
		 */
		BS_SCRIPT_EXPORT(ec:Mesh)
		static HMesh create(int numVertices, int numIndices, const Vector<SubMesh>& subMeshes, 
			MeshUsage usage = MU_STATIC, VertexLayout vertex = VertexLayout::Position, IndexType index = IT_32BIT);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 * 
		 * @param[in]	data			Vertex and index data to initialize the mesh with.
		 * @param[in]	topology		Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is a triangle list, where three indices represent a single triangle.
		 * @param[in]	usage			Optimizes performance depending on planned usage of the mesh.
		 */
		BS_SCRIPT_EXPORT(ec:Mesh)
		static HMesh create(const SPtr<RendererMeshData>& data, DrawOperationType topology = DOT_TRIANGLE_LIST, 
			MeshUsage usage = MU_STATIC);

		/**
		 * Creates a new mesh with enough space to hold the a number of primitives using the specified layout. Indices can
		 * be referenced by multiple sub-meshes.
		 * 
		 * @param[in]	data			Vertex and index data to initialize the mesh with.
		 * @param[in]	subMeshes		Defines how are indices separated into sub-meshes, and how are those sub-meshes
		 *								rendered. Sub-meshes may be rendered independently, each with a different material.
		 * @param[in]	usage			Optimizes performance depending on planned usage of the mesh.
		 */
		BS_SCRIPT_EXPORT(ec:Mesh)
		static HMesh create(const SPtr<RendererMeshData>& data, const Vector<SubMesh>& subMeshes, 
			MeshUsage usage = MU_STATIC);

		/** Returns all sub-meshes contained in the mesh. */
		BS_SCRIPT_EXPORT(e:Mesh,pr:getter,n:SubMeshes)
		static Vector<SubMesh> getSubMeshes(const HMesh& thisPtr);
		
		/** Returns the number of sub-meshes contained in this mesh. */
		BS_SCRIPT_EXPORT(e:Mesh,pr:getter,n:SubMeshCount)
		static UINT32 getSubMeshCount(const HMesh& thisPtr);
		
		BS_SCRIPT_EXPORT(e:Mesh,in:true)
		static void getBounds(const HMesh& thisPtr, AABox* box, Sphere* sphere);
		
		/**
		 * Accesses the vertex and index data of the mesh. If reading, mesh must have been created with the
		 * MeshUsage::CPUCached flag. If writing the caller must ensure the data matches mesh's vertex/index counts, vertex
		 * layout and index format.
		 */
		BS_SCRIPT_EXPORT(e:Mesh,pr:getter,n:MeshData)
		static SPtr<RendererMeshData> getMeshData(const HMesh& thisPtr);
		BS_SCRIPT_EXPORT(e:Mesh,pr:setter,n:MeshData)
		static void setMeshData(const HMesh& thisPtr, const SPtr<RendererMeshData>& value);
	};

	/** @endcond */
	/** @} */
}
