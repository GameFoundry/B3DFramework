//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	/// Renderable represents any visible object in the scene. It has a mesh, bounds and a set of materials. Renderer will 
	/// render any Renderable objects visible by a camera.
	/// </summary>
	[ShowInInspector]
	public partial class Renderable : Component
	{
		private Renderable(bool __dummy0) { }
		protected Renderable() { }

		/// <summary>
		/// Determines the mesh to render. All sub-meshes of the mesh will be rendered, and you may set individual materials for 
		/// each sub-mesh.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Mesh> Mesh
		{
			get { return Internal_getMesh(mCachedPtr); }
			set { Internal_setMesh(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines all materials used for rendering this renderable. Each of the materials is used for rendering a single 
		/// sub-mesh. If number of materials is larger than number of sub-meshes, they will be ignored. If lower, the remaining 
		/// materials will be removed.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Material>[] Materials
		{
			get { return Internal_getMaterials(mCachedPtr); }
			set { Internal_setMaterials(mCachedPtr, value); }
		}

		/// <summary>Factor to be applied to the cull distance set in the camera&apos;s render settings.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float CullDistance
		{
			get { return Internal_getCullDistanceFactor(mCachedPtr); }
			set { Internal_setCullDistanceFactor(mCachedPtr, value); }
		}

		/// <summary>
		/// If enabled this renderable will write per-pixel velocity information when rendered. This is required for effects such 
		/// as temporal anti-aliasing and motion blur, but comes with a minor performance overhead. If you are not using those 
		/// effects you can disable this for a performance gain.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool WriteVelocity
		{
			get { return Internal_getWriteVelocity(mCachedPtr); }
			set { Internal_setWriteVelocity(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the layer bitfield that controls whether a renderable is considered visible in a specific camera. 
		/// Renderable layer must match camera layer in order for the camera to render the component.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ulong Layers
		{
			get { return Internal_getLayer(mCachedPtr); }
			set { Internal_setLayer(mCachedPtr, value); }
		}

		/// <summary>Gets world bounds of the mesh rendered by this object.</summary>
		[NativeWrapper]
		public Bounds Bounds
		{
			get
			{
				Bounds temp;
				Internal_getBounds(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>
		/// Sets a material that will be used for rendering a sub-mesh with the specified index. If a sub-mesh doesn&apos;t have 
		/// a specific material set then the primary material will be used.
		/// </summary>
		public void SetMaterial(int idx, RRef<Material> material)
		{
			Internal_setMaterial(mCachedPtr, idx, material);
		}

		/// <summary>
		/// Sets a material that will be used for rendering a sub-mesh with the specified index. If a sub-mesh doesn&apos;t have 
		/// a specific material set then the primary material will be used.
		/// </summary>
		public void SetMaterial(RRef<Material> material)
		{
			Internal_setMaterial0(mCachedPtr, material);
		}

		/// <summary>Returns the material used for rendering a sub-mesh with the specified index.</summary>
		public RRef<Material> GetMaterial(int idx)
		{
			return Internal_getMaterial(mCachedPtr, idx);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMesh(IntPtr thisPtr, RRef<Mesh> mesh);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Mesh> Internal_getMesh(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMaterial(IntPtr thisPtr, int idx, RRef<Material> material);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMaterial0(IntPtr thisPtr, RRef<Material> material);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_getMaterial(IntPtr thisPtr, int idx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMaterials(IntPtr thisPtr, RRef<Material>[] materials);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material>[] Internal_getMaterials(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setCullDistanceFactor(IntPtr thisPtr, float factor);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getCullDistanceFactor(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setWriteVelocity(IntPtr thisPtr, bool enable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getWriteVelocity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLayer(IntPtr thisPtr, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_getLayer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getBounds(IntPtr thisPtr, out Bounds __output);
	}

	/** @} */
}
