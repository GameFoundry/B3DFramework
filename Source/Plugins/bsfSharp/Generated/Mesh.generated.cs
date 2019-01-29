//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Primary class for holding geometry. Stores data in the form of vertex buffers and optionally an index buffer, which  
	/// may be bound to the pipeline for drawing. May contain multiple sub-meshes.
	/// </summary>
	[ShowInInspector]
	public partial class Mesh : Resource
	{
		private Mesh(bool __dummy0) { }
		protected Mesh() { }

		/// <summary>
		/// Creates a new mesh with enough space to hold the a number of primitives using the specified layout. All indices will 
		/// be part of a single sub-mesh.
		/// </summary>
		/// <param name="numVertices">Number of vertices in the mesh.</param>
		/// <param name="numIndices">
		/// Number of indices in the mesh. Must be a multiple of primitive size as specified by provided topology.
		/// </param>
		/// <param name="topology">
		/// Determines how should the provided indices be interpreted by the pipeline. Default option is a triangle list, where 
		/// three indices represent a single triangle.
		/// </param>
		/// <param name="usage">Optimizes performance depending on planned usage of the mesh.</param>
		/// <param name="vertex">Controls how are vertices organized in the vertex buffer and what data they contain.</param>
		/// <param name="index">
		/// Size of indices, use smaller size for better performance, however be careful not to go over the number of vertices 
		/// limited by the data type size.
		/// </param>
		public Mesh(int numVertices, int numIndices, MeshTopology topology = MeshTopology.TriangleList, MeshUsage usage = MeshUsage.Static, VertexLayout vertex = VertexLayout.Position, IndexType index = IndexType.Index32)
		{
			Internal_create(this, numVertices, numIndices, topology, usage, vertex, index);
		}

		/// <summary>
		/// Creates a new mesh with enough space to hold the a number of primitives using the specified layout. Indices can be 
		/// referenced by multiple sub-meshes.
		/// </summary>
		/// <param name="numVertices">Number of vertices in the mesh.</param>
		/// <param name="numIndices">
		/// Number of indices in the mesh. Must be a multiple of primitive size as specified by provided topology.
		/// </param>
		/// <param name="subMeshes">
		/// Defines how are indices separated into sub-meshes, and how are those sub-meshes rendered. Sub-meshes may be rendered 
		/// independently, each with a different material.
		/// </param>
		/// <param name="usage">Optimizes performance depending on planned usage of the mesh.</param>
		/// <param name="vertex">Controls how are vertices organized in the vertex buffer and what data they contain.</param>
		/// <param name="index">
		/// Size of indices, use smaller size for better performance, however be careful not to go over the number of vertices 
		/// limited by the data type size.
		/// </param>
		public Mesh(int numVertices, int numIndices, SubMesh[] subMeshes, MeshUsage usage = MeshUsage.Static, VertexLayout vertex = VertexLayout.Position, IndexType index = IndexType.Index32)
		{
			Internal_create0(this, numVertices, numIndices, subMeshes, usage, vertex, index);
		}

		/// <summary>
		/// Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described by the 
		/// mesh data exactly. Mesh will have no sub-meshes.
		/// </summary>
		/// <param name="data">Vertex and index data to initialize the mesh with.</param>
		/// <param name="topology">
		/// Determines how should the provided indices be interpreted by the pipeline. Default option is a triangle list, where 
		/// three indices represent a single triangle.
		/// </param>
		/// <param name="usage">Optimizes performance depending on planned usage of the mesh.</param>
		public Mesh(MeshData data, MeshTopology topology = MeshTopology.TriangleList, MeshUsage usage = MeshUsage.Static)
		{
			Internal_create1(this, data, topology, usage);
		}

		/// <summary>
		/// Creates a new mesh with enough space to hold the a number of primitives using the specified layout. Indices can be 
		/// referenced by multiple sub-meshes.
		/// </summary>
		/// <param name="data">Vertex and index data to initialize the mesh with.</param>
		/// <param name="subMeshes">
		/// Defines how are indices separated into sub-meshes, and how are those sub-meshes rendered. Sub-meshes may be rendered 
		/// independently, each with a different material.
		/// </param>
		/// <param name="usage">Optimizes performance depending on planned usage of the mesh.</param>
		public Mesh(MeshData data, SubMesh[] subMeshes, MeshUsage usage = MeshUsage.Static)
		{
			Internal_create2(this, data, subMeshes, usage);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<Mesh> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Gets the skeleton required for animation of this mesh, if any is available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Skeleton Skeleton
		{
			get { return Internal_getSkeleton(mCachedPtr); }
		}

		/// <summary>Returns an object containing all shapes used for morph animation, if any are available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public MorphShapes MorphShapes
		{
			get { return Internal_getMorphShapes(mCachedPtr); }
		}

		/// <summary>Returns all sub-meshes contained in the mesh.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public SubMesh[] SubMeshes
		{
			get { return Internal_getSubMeshes(mCachedPtr); }
		}

		/// <summary>Returns the number of sub-meshes contained in this mesh.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint SubMeshCount
		{
			get { return Internal_getSubMeshCount(mCachedPtr); }
		}

		/// <summary>
		/// Accesses the vertex and index data of the mesh. If reading, mesh must have been created with the MeshUsage::CPUCached 
		/// flag. If writing the caller must ensure the data matches mesh's vertex/index counts, vertex layout and index format.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public MeshData MeshData
		{
			get { return Internal_getMeshData(mCachedPtr); }
			set { Internal_setMeshData(mCachedPtr, value); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<Mesh>(Mesh x)
		{ return Internal_GetRef(x.mCachedPtr); }

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Mesh> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Skeleton Internal_getSkeleton(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MorphShapes Internal_getMorphShapes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(Mesh managedInstance, int numVertices, int numIndices, MeshTopology topology, MeshUsage usage, VertexLayout vertex, IndexType index);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(Mesh managedInstance, int numVertices, int numIndices, SubMesh[] subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create1(Mesh managedInstance, MeshData data, MeshTopology topology, MeshUsage usage);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create2(Mesh managedInstance, MeshData data, SubMesh[] subMeshes, MeshUsage usage);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SubMesh[] Internal_getSubMeshes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getSubMeshCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getBounds(IntPtr thisPtr, out AABox box, out Sphere sphere);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MeshData Internal_getMeshData(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMeshData(IntPtr thisPtr, MeshData value);
	}

	/** @} */
}
