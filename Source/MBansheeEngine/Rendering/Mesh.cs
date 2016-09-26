//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Rendering
     *  @{
     */

    /// <summary>
    /// Primary class for holding geometry. Stores data in the form of a vertex buffers and optionally index buffer, 
    /// which may be bound to the pipeline for drawing. May contain multiple sub-meshes.
    /// </summary>
    public class Mesh : Resource
    {
        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        private Mesh()
        { }

        /// <summary>
        /// Creates a new mesh with enough space to hold the a number of primitives using the specified layout. All indices
        /// will be part of a single sub-mesh.
        /// </summary>
        /// <param name="numVertices">Number of vertices in the mesh.</param>
        /// <param name="numIndices">Number of indices in the mesh. </param>
        /// <param name="topology">Determines how should the provided indices be interpreted by the pipeline. Default option
        ///                        is a triangle list, where three indices represent a single triangle.</param>
        /// <param name="usage">Optimizes performance depending on planned usage of the mesh.</param>
        /// <param name="vertex">Controls how are vertices organized in the vertex buffer and what data they contain.</param>
        /// <param name="index">Size of indices, use smaller size for better performance, however be careful not to go over
        ///                     the number of vertices limited by the size.</param>
        public Mesh(int numVertices, int numIndices, MeshTopology topology = MeshTopology.TriangleList,
            MeshUsage usage = MeshUsage.Default, VertexType vertex = VertexType.Position, 
            IndexType index = IndexType.Index32)
        {
            SubMesh[] subMeshes = {new SubMesh(0, numIndices, topology)};

            Internal_CreateInstance(this, numVertices, numIndices, subMeshes, usage, vertex, index);
        }

        /// <summary>
        /// Creates a new mesh with enough space to hold the a number of primitives using the specified layout. Indices can
        /// be references by multiple sub-meshes.
        /// </summary>
        /// <param name="numVertices">Number of vertices in the mesh.</param>
        /// <param name="numIndices">Number of indices in the mesh. </param>
        /// <param name="subMeshes">Defines how are indices separated into sub-meshes, and how are those sub-meshes rendered.
        ///                         Sub-meshes may be rendered independently.</param>
        /// <param name="usage">Optimizes performance depending on planned usage of the mesh.</param>
        /// <param name="vertex">Controls how are vertices organized in the vertex buffer and what data they contain.</param>
        /// <param name="index">Size of indices, use smaller size for better performance, however be careful not to go over
        ///                     the number of vertices limited by the size.</param>
        public Mesh(int numVertices, int numIndices, SubMesh[] subMeshes, MeshUsage usage = MeshUsage.Default,
            VertexType vertex = VertexType.Position, IndexType index = IndexType.Index32)
        {
            Internal_CreateInstance(this, numVertices, numIndices, subMeshes, usage, vertex, index);
        }

        /// <summary>
        /// Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
        /// by the mesh data exactly. Mesh will have no sub-meshes.
        /// </summary>
        /// <param name="data">Vertex and index data to initialize the mesh with.</param>
        /// <param name="topology">Determines how should the provided indices be interpreted by the pipeline. Default option
        ///                        is a triangle list, where three indices represent a single triangle.</param>
        /// <param name="usage">Optimizes performance depending on planned usage of the mesh.</param>
        public Mesh(MeshData data, MeshTopology topology = MeshTopology.TriangleList, MeshUsage usage = MeshUsage.Default)
        {
            int numIndices = 0;
            IntPtr dataPtr = IntPtr.Zero;

            if (data != null)
            {
                numIndices = data.IndexCount;
                dataPtr = data.GetCachedPtr();
            }

            SubMesh[] subMeshes = { new SubMesh(0, numIndices, topology) };

            Internal_CreateInstanceMeshData(this, dataPtr, subMeshes, usage);
        }

        /// <summary>
        /// Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
        /// by the mesh data exactly. Mesh will have specified the sub-meshes.
        /// </summary>
        /// <param name="data">Vertex and index data to initialize the mesh with.</param>
        /// <param name="subMeshes">Defines how are indices separated into sub-meshes, and how are those sub-meshes rendered.
        ///                         Sub-meshes may be rendered independently.</param>
        /// <param name="usage">Optimizes performance depending on planned usage of the mesh.</param>
        public Mesh(MeshData data, SubMesh[] subMeshes, MeshUsage usage = MeshUsage.Default)
        {
            IntPtr dataPtr = IntPtr.Zero;
            if (data != null)
                dataPtr = data.GetCachedPtr();

            Internal_CreateInstanceMeshData(this, dataPtr, subMeshes, usage);
        }

        /// <summary>
        /// Returns the number of sub-meshes contained in the mesh.
        /// </summary>
        public int SubMeshCount
        {
            get { return Internal_GetSubMeshCount(mCachedPtr); }
        }

        /// <summary>
        /// Returns all sub-meshes contained in the mesh.
        /// </summary>
        public SubMesh[] SubMeshes
        {
            get { return Internal_GetSubMeshes(mCachedPtr); }
        }

        /// <summary>
        /// Returns local bounds of the geometry contained in the vertex buffers for all sub-meshes.
        /// </summary>
        public Bounds Bounds
        {
            get
            {
                AABox box;
                Sphere sphere;

                Internal_GetBounds(mCachedPtr, out box, out sphere);

                return new Bounds(box, sphere);
            }
        }

        /// <summary>
        /// Accesses the vertex and index data of the mesh. If reading, mesh must have been created with the 
        /// <see cref="MeshUsage.CPUCached"/> flag. If writing the caller must ensure the data matches mesh's vertex/index
        /// counts, vertex layout and index format.
        /// </summary>
        public MeshData MeshData
        {
            get { return Internal_GetMeshData(mCachedPtr); }
            set
            {
                IntPtr meshDataPtr = IntPtr.Zero;
                if (value != null)
                    meshDataPtr = value.GetCachedPtr();

                Internal_SetMeshData(mCachedPtr, meshDataPtr);
            }
        }

        /// <summary>
        /// Gets the skeleton required for animation of this mesh, if any is available.
        /// </summary>
        public Skeleton Skeleton
        {
            get { return Internal_GetSkeleton(mCachedPtr); }
        }

        /// <summary>
        /// Returns an object containing all shapes used for morph animation, if any are available.
        /// </summary>
        public MorphShapes MorphShapes
        {
            get { return Internal_GetMorphShapes(mCachedPtr); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(Mesh instance, int numVertices,
            int numIndices, SubMesh[] subMeshes, MeshUsage usage, VertexType vertex, IndexType index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstanceMeshData(Mesh instance, IntPtr data, SubMesh[] subMeshes, 
            MeshUsage usage);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SubMesh[] Internal_GetSubMeshes(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetSubMeshCount(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Skeleton Internal_GetSkeleton(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern MorphShapes Internal_GetMorphShapes(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetBounds(IntPtr thisPtr, out AABox box, out Sphere sphere);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern MeshData Internal_GetMeshData(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMeshData(IntPtr thisPtr, IntPtr value);
    }

    /// <summary>
    /// Data about a sub-mesh range and the type of primitives contained in the range.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct SubMesh // Note: Must match C++ class SubMesh
    {
        public SubMesh(int indexOffset, int indexCount, MeshTopology topology = MeshTopology.TriangleList)
        {
            IndexOffset = indexOffset;
            IndexCount = indexCount;
            Topology = topology;
        }

        public int IndexOffset;
        public int IndexCount;
        public MeshTopology Topology;
    }

    /// <summary>
    /// Determines how are mesh indices interpreted by the renderer.
    /// </summary>
    public enum MeshTopology // Note: Must match C++ class MeshTopology
    {
        PointList = 1,
        LineList = 2,
        LineStrip = 3,
        TriangleList = 4,
        TriangleStrip = 5,
        TriangleFan = 6
    }

    /// <summary>
    /// Planned usage for the mesh that allow various optimizations.
    /// </summary>
    public enum MeshUsage // Note: Must match C++ enum MeshUsage
    {
        /// <summary>
        /// Specify for a mesh that is not often updated from the CPU.
        /// </summary>
        Default = 0x1,

        /// <summary>
        /// Specify for a mesh that is often updated from the CPU.
        /// </summary>
        Dynamic = 0x2,

        /// <summary>
        /// All mesh data will also be cached in CPU memory allowing the mesh data to be read.
        /// </summary>
        CPUCached = 0x1000
    }

    /** @} */
}
