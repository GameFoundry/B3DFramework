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

	[ShowInInspector]
	public partial class Renderable : Component
	{
		private Renderable(bool __dummy0) { }
		protected Renderable() { }

		[ShowInInspector]
		[NativeWrapper]
		public RRef<Mesh> Mesh
		{
			get { return Internal_GetMesh(mCachedPtr); }
			set { Internal_SetMesh(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public RRef<Material>[] Materials
		{
			get { return Internal_GetMaterials(mCachedPtr); }
			set { Internal_SetMaterials(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float CullDistance
		{
			get { return Internal_GetCullDistanceFactor(mCachedPtr); }
			set { Internal_SetCullDistanceFactor(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public bool WriteVelocity
		{
			get { return Internal_GetWriteVelocity(mCachedPtr); }
			set { Internal_SetWriteVelocity(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public ulong Layers
		{
			get { return Internal_GetLayer(mCachedPtr); }
			set { Internal_SetLayer(mCachedPtr, value); }
		}

		/// <summary>Gets world bounds of the mesh rendered by this object.</summary>
		[NativeWrapper]
		public Bounds Bounds
		{
			get
			{
				Bounds temp;
				Internal_GetBounds(mCachedPtr, out temp);
				return temp;
			}
		}

		public void SetMaterial(int idx, RRef<Material> material)
		{
			Internal_SetMaterial(mCachedPtr, idx, material);
		}

		public void SetMaterial(RRef<Material> material)
		{
			Internal_SetMaterial0(mCachedPtr, material);
		}

		public RRef<Material> GetMaterial(int idx)
		{
			return Internal_GetMaterial(mCachedPtr, idx);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMesh(IntPtr thisPtr, RRef<Mesh> mesh);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Mesh> Internal_GetMesh(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaterial(IntPtr thisPtr, int idx, RRef<Material> material);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaterial0(IntPtr thisPtr, RRef<Material> material);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_GetMaterial(IntPtr thisPtr, int idx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaterials(IntPtr thisPtr, RRef<Material>[] materials);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material>[] Internal_GetMaterials(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCullDistanceFactor(IntPtr thisPtr, float factor);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetCullDistanceFactor(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetWriteVelocity(IntPtr thisPtr, bool enable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetWriteVelocity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLayer(IntPtr thisPtr, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetLayer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetBounds(IntPtr thisPtr, out Bounds __output);
	}

	/** @} */
}
