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
	/// Contains mesh vertex and index data used for initializing, updating and reading mesh data from Mesh.
	/// </summary>
	[ShowInInspector]
	public partial class MeshData : ScriptObject
	{
		private MeshData(bool __dummy0) { }
		protected MeshData() { }

		public MeshData(uint numVertices, uint numIndices, VertexLayout layout, IndexType indexType = IndexType.Index32)
		{
			Internal_create(this, numVertices, numIndices, layout, indexType);
		}

		/// <summary>An array of all vertex positions. Only valid if the vertex layout contains vertex positions.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3[] Positions
		{
			get { return Internal_getPositions(mCachedPtr); }
			set { Internal_setPositions(mCachedPtr, value); }
		}

		/// <summary>An array of all vertex normals. Only valid if the vertex layout contains vertex normals.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3[] Normals
		{
			get { return Internal_getNormals(mCachedPtr); }
			set { Internal_setNormals(mCachedPtr, value); }
		}

		/// <summary>An array of all vertex tangents. Only valid if the vertex layout contains vertex tangents.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector4[] Tangents
		{
			get { return Internal_getTangents(mCachedPtr); }
			set { Internal_setTangents(mCachedPtr, value); }
		}

		/// <summary>An array of all vertex colors. Only valid if the vertex layout contains vertex colors.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Color[] Colors
		{
			get { return Internal_getColors(mCachedPtr); }
			set { Internal_setColors(mCachedPtr, value); }
		}

		/// <summary>
		/// An array of all vertex texture coordinates in the UV0 channel. Only valid if the vertex layout contains UV0 
		/// coordinates.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector2[] UV0
		{
			get { return Internal_getUV0(mCachedPtr); }
			set { Internal_setUV0(mCachedPtr, value); }
		}

		/// <summary>
		/// An array of all vertex texture coordinates in the UV1 channel. Only valid if the vertex layout contains UV1 
		/// coordinates.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector2[] UV1
		{
			get { return Internal_getUV1(mCachedPtr); }
			set { Internal_setUV1(mCachedPtr, value); }
		}

		/// <summary>An array of all vertex bone weights. Only valid if the vertex layout contains bone weights.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public BoneWeight[] BoneWeights
		{
			get { return Internal_getBoneWeights(mCachedPtr); }
			set { Internal_setBoneWeights(mCachedPtr, value); }
		}

		/// <summary>An array of all indices.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint[] Indices
		{
			get { return Internal_getIndices(mCachedPtr); }
			set { Internal_setIndices(mCachedPtr, value); }
		}

		/// <summary>Returns the number of vertices contained in the mesh.</summary>
		[NativeWrapper]
		public int VertexCount
		{
			get { return Internal_getVertexCount(mCachedPtr); }
		}

		/// <summary>Returns the number of indices contained in the mesh.</summary>
		[NativeWrapper]
		public int IndexCount
		{
			get { return Internal_getIndexCount(mCachedPtr); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(MeshData managedInstance, uint numVertices, uint numIndices, VertexLayout layout, IndexType indexType);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3[] Internal_getPositions(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPositions(IntPtr thisPtr, Vector3[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3[] Internal_getNormals(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setNormals(IntPtr thisPtr, Vector3[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector4[] Internal_getTangents(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setTangents(IntPtr thisPtr, Vector4[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Color[] Internal_getColors(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setColors(IntPtr thisPtr, Color[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2[] Internal_getUV0(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setUV0(IntPtr thisPtr, Vector2[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2[] Internal_getUV1(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setUV1(IntPtr thisPtr, Vector2[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern BoneWeight[] Internal_getBoneWeights(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setBoneWeights(IntPtr thisPtr, BoneWeight[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint[] Internal_getIndices(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setIndices(IntPtr thisPtr, uint[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getVertexCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getIndexCount(IntPtr thisPtr);
	}

	/** @} */
}
