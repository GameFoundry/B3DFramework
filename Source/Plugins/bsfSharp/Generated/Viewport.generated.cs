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
	/// Viewport determines to which RenderTarget should rendering be performed. It allows you to render to a sub-region of 
	/// the target by specifying the area rectangle, and allows you to set up color/depth/stencil clear values for that 
	/// specific region.
	/// </summary>
	[ShowInInspector]
	public partial class Viewport : ScriptObject
	{
		private Viewport(bool __dummy0) { }
		protected Viewport() { }

		/// <summary>Creates a new viewport.</summary>
		public Viewport(RenderTarget target, float x = 0f, float y = 0f, float width = 1f, float height = 1f)
		{
			Internal_create(this, target, x, y, width, height);
		}

		/// <summary>Determines the render target the viewport is associated with.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RenderTarget Target
		{
			get { return Internal_getTarget(mCachedPtr); }
			set { Internal_setTarget(mCachedPtr, value); }
		}

		/// <summary>Determines the area that the viewport covers. Coordinates are in normalized [0, 1] range.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Rect2 Area
		{
			get
			{
				Rect2 temp;
				Internal_getArea(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setArea(mCachedPtr, ref value); }
		}

		/// <summary>Returns the area of the render target covered by the viewport, in pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Rect2I PixelArea
		{
			get
			{
				Rect2I temp;
				Internal_getPixelArea(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>
		/// Determines which portions of the render target should be cleared before rendering to this viewport is performed.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ClearFlags ClearFlags
		{
			get { return Internal_getClearFlags(mCachedPtr); }
			set { Internal_setClearFlags(mCachedPtr, value); }
		}

		/// <summary>Determines the color to clear the viewport to before rendering, if color clear is enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Color ClearColor
		{
			get
			{
				Color temp;
				Internal_getClearColorValue(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setClearColorValue(mCachedPtr, ref value); }
		}

		/// <summary>Determines the value to clear the depth buffer to before rendering, if depth clear is enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float ClearDepth
		{
			get { return Internal_getClearDepthValue(mCachedPtr); }
			set { Internal_setClearDepthValue(mCachedPtr, value); }
		}

		/// <summary>Determines the value to clear the stencil buffer to before rendering, if stencil clear is enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ushort ClearStencil
		{
			get { return Internal_getClearStencilValue(mCachedPtr); }
			set { Internal_setClearStencilValue(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setTarget(IntPtr thisPtr, RenderTarget target);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RenderTarget Internal_getTarget(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setArea(IntPtr thisPtr, ref Rect2 area);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getArea(IntPtr thisPtr, out Rect2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getPixelArea(IntPtr thisPtr, out Rect2I __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setClearFlags(IntPtr thisPtr, ClearFlags flags);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ClearFlags Internal_getClearFlags(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setClearColorValue(IntPtr thisPtr, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getClearColorValue(IntPtr thisPtr, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setClearDepthValue(IntPtr thisPtr, float depth);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getClearDepthValue(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setClearStencilValue(IntPtr thisPtr, ushort value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ushort Internal_getClearStencilValue(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(Viewport managedInstance, RenderTarget target, float x, float y, float width, float height);
	}

	/** @} */
}
