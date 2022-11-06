//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
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

		public MeshData(int numVertices, int numIndices, VertexLayout layout, IndexType indexType = IndexType.Index32)
		{
			Internal_Create(this, numVertices, numIndices, layout, indexType);
		}

		/// <summary>An array of all vertex positions. Only valid if the vertex layout contains vertex positions.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3[] Positions
		{
			get { return Internal_GetPositions(mCachedPtr); }
			set { Internal_SetPositions(mCachedPtr, value); }
		}

		/// <summary>An array of all vertex normals. Only valid if the vertex layout contains vertex normals.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3[] Normals
		{
			get { return Internal_GetNormals(mCachedPtr); }
			set { Internal_SetNormals(mCachedPtr, value); }
		}

		/// <summary>An array of all vertex tangents. Only valid if the vertex layout contains vertex tangents.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector4[] Tangents
		{
			get { return Internal_GetTangents(mCachedPtr); }
			set { Internal_SetTangents(mCachedPtr, value); }
		}

		/// <summary>An array of all vertex colors. Only valid if the vertex layout contains vertex colors.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Color[] Colors
		{
			get { return Internal_GetColors(mCachedPtr); }
			set { Internal_SetColors(mCachedPtr, value); }
		}

		/// <summary>
		/// An array of all vertex texture coordinates in the UV0 channel. Only valid if the vertex layout contains UV0 
		/// coordinates.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector2[] UV0
		{
			get { return Internal_GetUV0(mCachedPtr); }
			set { Internal_SetUV0(mCachedPtr, value); }
		}

		/// <summary>
		/// An array of all vertex texture coordinates in the UV1 channel. Only valid if the vertex layout contains UV1 
		/// coordinates.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector2[] UV1
		{
			get { return Internal_GetUV1(mCachedPtr); }
			set { Internal_SetUV1(mCachedPtr, value); }
		}

		/// <summary>An array of all vertex bone weights. Only valid if the vertex layout contains bone weights.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public BoneWeight[] BoneWeights
		{
			get { return Internal_GetBoneWeights(mCachedPtr); }
			set { Internal_SetBoneWeights(mCachedPtr, value); }
		}

		/// <summary>An array of all indices.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int[] Indices
		{
			get { return Internal_GetIndices(mCachedPtr); }
			set { Internal_SetIndices(mCachedPtr, value); }
		}

		/// <summary>Returns the number of vertices contained in the mesh.</summary>
		[NativeWrapper]
		public int VertexCount
		{
			get { return Internal_GetVertexCount(mCachedPtr); }
		}

		/// <summary>Returns the number of indices contained in the mesh.</summary>
		[NativeWrapper]
		public int IndexCount
		{
			get { return Internal_GetIndexCount(mCachedPtr); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(MeshData managedInstance, int numVertices, int numIndices, VertexLayout layout, IndexType indexType);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3[] Internal_GetPositions(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPositions(IntPtr thisPtr, Vector3[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3[] Internal_GetNormals(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNormals(IntPtr thisPtr, Vector3[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector4[] Internal_GetTangents(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTangents(IntPtr thisPtr, Vector4[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Color[] Internal_GetColors(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetColors(IntPtr thisPtr, Color[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2[] Internal_GetUV0(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUV0(IntPtr thisPtr, Vector2[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2[] Internal_GetUV1(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUV1(IntPtr thisPtr, Vector2[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern BoneWeight[] Internal_GetBoneWeights(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBoneWeights(IntPtr thisPtr, BoneWeight[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int[] Internal_GetIndices(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIndices(IntPtr thisPtr, int[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetVertexCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetIndexCount(IntPtr thisPtr);
	}

	/** @} */
}
