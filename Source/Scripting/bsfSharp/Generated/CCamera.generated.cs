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
	/// Camera determines how is world geometry projected onto a 2D surface. You may position and orient it in space, set 
	/// options like aspect ratio and field or view and it outputs view and projection matrices required for rendering.
	/// </summary>
	[ShowInInspector]
	public partial class Camera : Component
	{
		private Camera(bool __dummy0) { }
		protected Camera() { }

		/// <summary>Determines flags used for controlling the camera behaviour.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CameraFlag Flags
		{
			get { return Internal_getFlags(mCachedPtr); }
			set { Internal_setFlags(mCachedPtr, value); }
		}

		/// <summary>Returns the viewport used by the camera.</summary>
		[NativeWrapper]
		public Viewport Viewport
		{
			get { return Internal_getViewport(mCachedPtr); }
		}

		/// <summary>
		/// Determines the camera horizontal field of view. This determines how wide the camera viewing angle is along the 
		/// horizontal axis. Vertical FOV is calculated from the horizontal FOV and the aspect ratio.
		/// </summary>
		[ShowInInspector]
		[Range(1f, 360f, true)]
		[Order(-1)]
		[NativeWrapper]
		public Radian FieldOfView
		{
			get
			{
				Radian temp;
				Internal_getHorzFOV(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setHorzFOV(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the distance from the frustum to the near clipping plane. Anything closer than the near clipping plane 
		/// will not be rendered. Decreasing this value decreases depth buffer precision.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float NearClipPlane
		{
			get { return Internal_getNearClipDistance(mCachedPtr); }
			set { Internal_setNearClipDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the distance from the frustum to the far clipping plane. Anything farther than the far clipping plane will 
		/// not be rendered. Increasing this value decreases depth buffer precision.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FarClipPlane
		{
			get { return Internal_getFarClipDistance(mCachedPtr); }
			set { Internal_setFarClipDistance(mCachedPtr, value); }
		}

		/// <summary>Determines the current viewport aspect ratio (width / height).</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float AspectRatio
		{
			get { return Internal_getAspectRatio(mCachedPtr); }
			set { Internal_setAspectRatio(mCachedPtr, value); }
		}

		/// <summary>
		/// Returns the standard projection matrix that determines how are 3D points projected to two dimensions. The layout of 
		/// this matrix depends on currently used render system.
		/// </summary>
		[NativeWrapper]
		public Matrix4 ProjMatrix
		{
			get
			{
				Matrix4 temp;
				Internal_getProjectionMatrixRS(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Gets the camera view matrix. Used for positioning/orienting the camera.</summary>
		[NativeWrapper]
		public Matrix4 ViewMatrix
		{
			get
			{
				Matrix4 temp;
				Internal_getViewMatrix(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>
		/// Determines the type of projection used by the camera. Projection type controls how is 3D geometry projected onto a 2D 
		/// plane.
		/// </summary>
		[ShowInInspector]
		[Order(-2)]
		[NativeWrapper]
		public ProjectionType ProjectionType
		{
			get { return Internal_getProjectionType(mCachedPtr); }
			set { Internal_setProjectionType(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the orthographic window height, for use with orthographic rendering only. The width of the window will be 
		/// calculated from the aspect ratio. Value is specified in world units.
		/// </summary>
		[ShowInInspector]
		[Order(-1)]
		[NativeWrapper]
		public float OrthoHeight
		{
			get { return Internal_getOrthoWindowHeight(mCachedPtr); }
			set { Internal_setOrthoWindowHeight(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the orthographic window width, for use with orthographic rendering only. The height of the window will be 
		/// calculated from the aspect ratio. Value is specified in world units.
		/// </summary>
		[NativeWrapper]
		public float OrthoWidth
		{
			get { return Internal_getOrthoWindowWidth(mCachedPtr); }
			set { Internal_setOrthoWindowWidth(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a priority that determines in which orders the cameras are rendered. This only applies to cameras 
		/// rendering to the same render target. Higher value means the camera will be rendered sooner.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Priority
		{
			get { return Internal_getPriority(mCachedPtr); }
			set { Internal_setPriority(mCachedPtr, value); }
		}

		/// <summary>Determines layer bitfield that is used when determining which object should the camera render.</summary>
		[ShowInInspector]
		[LayerMask]
		[NativeWrapper]
		public ulong Layers
		{
			get { return Internal_getLayers(mCachedPtr); }
			set { Internal_setLayers(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines number of samples to use when rendering to this camera. Values larger than 1 will enable MSAA rendering.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int SampleCount
		{
			get { return Internal_getMSAACount(mCachedPtr); }
			set { Internal_setMSAACount(mCachedPtr, value); }
		}

		/// <summary>
		/// Settings that control rendering for this view. They determine how will the renderer process this view, which effects 
		/// will be enabled, and what properties will those effects use.
		/// </summary>
		[ShowInInspector]
		[ApplyOnDirty]
		[NativeWrapper]
		public RenderSettings RenderSettings
		{
			get { return Internal_getRenderSettings(mCachedPtr); }
			set { Internal_setRenderSettings(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines whether this is the main application camera. Main camera controls the final render surface that is 
		/// displayed to the user.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Main
		{
			get { return Internal_isMain(mCachedPtr); }
			set { Internal_setMain(mCachedPtr, value); }
		}

		/// <summary>
		/// Notifies a on-demand camera that it should re-draw its contents on the next frame. Ignored for a camera that 
		/// isn&apos;t on-demand.
		/// </summary>
		public void NotifyNeedsRedraw()
		{
			Internal_notifyNeedsRedraw(mCachedPtr);
		}

		/// <summary>Converts a point in world space to screen coordinates.</summary>
		/// <param name="worldPoint">3D point in world space.</param>
		/// <returns>2D point on the render target attached to the camera&apos;s viewport, in pixels.</returns>
		public Vector2I WorldToScreenPoint(Vector3 worldPoint)
		{
			Vector2I temp;
			Internal_worldToScreenPoint(mCachedPtr, ref worldPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in world space to normalized device coordinates.</summary>
		/// <param name="worldPoint">3D point in world space.</param>
		/// <returns>2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.</returns>
		public Vector2 WorldToNdcPoint(Vector3 worldPoint)
		{
			Vector2 temp;
			Internal_worldToNdcPoint(mCachedPtr, ref worldPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in world space to view space coordinates.</summary>
		/// <param name="worldPoint">3D point in world space.</param>
		/// <returns>3D point relative to the camera&apos;s coordinate system.</returns>
		public Vector3 WorldToViewPoint(Vector3 worldPoint)
		{
			Vector3 temp;
			Internal_worldToViewPoint(mCachedPtr, ref worldPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in screen space to a point in world space.</summary>
		/// <param name="screenPoint">2D point on the render target attached to the camera&apos;s viewport, in pixels.</param>
		/// <param name="depth">
		/// Depth to place the world point at, in world coordinates. The depth is applied to the vector going from camera origin 
		/// to the point on the near plane.
		/// </param>
		/// <returns>3D point in world space.</returns>
		public Vector3 ScreenToWorldPoint(Vector2I screenPoint, float depth = 0.5f)
		{
			Vector3 temp;
			Internal_screenToWorldPoint(mCachedPtr, ref screenPoint, depth, out temp);
			return temp;
		}

		/// <summary>Converts a point in screen space to a point in view space.</summary>
		/// <param name="screenPoint">2D point on the render target attached to the camera&apos;s viewport, in pixels.</param>
		/// <param name="depth">
		/// Depth to place the world point at, in device depth. The depth is applied to the vector going from camera origin to 
		/// the point on the near plane.
		/// </param>
		/// <returns>3D point relative to the camera&apos;s coordinate system.</returns>
		public Vector3 ScreenToViewPoint(Vector2I screenPoint, float depth = 0.5f)
		{
			Vector3 temp;
			Internal_screenToViewPoint(mCachedPtr, ref screenPoint, depth, out temp);
			return temp;
		}

		/// <summary>Converts a point in screen space to normalized device coordinates.</summary>
		/// <param name="screenPoint">2D point on the render target attached to the camera&apos;s viewport, in pixels.</param>
		/// <returns>2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.</returns>
		public Vector2 ScreenToNdcPoint(Vector2I screenPoint)
		{
			Vector2 temp;
			Internal_screenToNdcPoint(mCachedPtr, ref screenPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in view space to world space.</summary>
		/// <param name="viewPoint">3D point relative to the camera&apos;s coordinate system.</param>
		/// <returns>3D point in world space.</returns>
		public Vector3 ViewToWorldPoint(Vector3 viewPoint)
		{
			Vector3 temp;
			Internal_viewToWorldPoint(mCachedPtr, ref viewPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in view space to screen space.</summary>
		/// <param name="viewPoint">3D point relative to the camera&apos;s coordinate system.</param>
		/// <returns>2D point on the render target attached to the camera&apos;s viewport, in pixels.</returns>
		public Vector2I ViewToScreenPoint(Vector3 viewPoint)
		{
			Vector2I temp;
			Internal_viewToScreenPoint(mCachedPtr, ref viewPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in view space to normalized device coordinates.</summary>
		/// <param name="viewPoint">3D point relative to the camera&apos;s coordinate system.</param>
		/// <returns>2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.</returns>
		public Vector2 ViewToNdcPoint(Vector3 viewPoint)
		{
			Vector2 temp;
			Internal_viewToNdcPoint(mCachedPtr, ref viewPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in normalized device coordinates to world space.</summary>
		/// <param name="ndcPoint">
		/// 2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.
		/// </param>
		/// <param name="depth">
		/// Depth to place the world point at. The depth is applied to the vector going from camera origin to the point on the 
		/// near plane.
		/// </param>
		/// <returns>3D point in world space.</returns>
		public Vector3 NdcToWorldPoint(Vector2 ndcPoint, float depth = 0.5f)
		{
			Vector3 temp;
			Internal_ndcToWorldPoint(mCachedPtr, ref ndcPoint, depth, out temp);
			return temp;
		}

		/// <summary>Converts a point in normalized device coordinates to view space.</summary>
		/// <param name="ndcPoint">
		/// 2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.
		/// </param>
		/// <param name="depth">
		/// Depth to place the world point at. The depth is applied to the vector going from camera origin to the point on the 
		/// near plane.
		/// </param>
		/// <returns>3D point relative to the camera&apos;s coordinate system.</returns>
		public Vector3 NdcToViewPoint(Vector2 ndcPoint, float depth = 0.5f)
		{
			Vector3 temp;
			Internal_ndcToViewPoint(mCachedPtr, ref ndcPoint, depth, out temp);
			return temp;
		}

		/// <summary>Converts a point in normalized device coordinates to screen space.</summary>
		/// <param name="ndcPoint">
		/// 2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.
		/// </param>
		/// <returns>2D point on the render target attached to the camera&apos;s viewport, in pixels.</returns>
		public Vector2I NdcToScreenPoint(Vector2 ndcPoint)
		{
			Vector2I temp;
			Internal_ndcToScreenPoint(mCachedPtr, ref ndcPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in screen space to a ray in world space.</summary>
		/// <param name="screenPoint">2D point on the render target attached to the camera&apos;s viewport, in pixels.</param>
		/// <returns>Ray in world space, originating at the selected point on the camera near plane.</returns>
		public Ray ScreenPointToRay(Vector2I screenPoint)
		{
			Ray temp;
			Internal_screenPointToRay(mCachedPtr, ref screenPoint, out temp);
			return temp;
		}

		/// <summary>
		/// Projects a point in view space to normalized device coordinates. Similar to viewToNdcPoint() but preserves the depth 
		/// component.
		/// </summary>
		/// <param name="point">3D point relative to the camera&apos;s coordinate system.</param>
		/// <returns>
		/// 3D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport. Z value range 
		/// depends on active render API.
		/// </returns>
		public Vector3 ProjectPoint(Vector3 point)
		{
			Vector3 temp;
			Internal_projectPoint(mCachedPtr, ref point, out temp);
			return temp;
		}

		/// <summary>Un-projects a point in normalized device space to view space.</summary>
		/// <param name="point">
		/// 3D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport. Z value range 
		/// depends on active render API.
		/// </param>
		/// <returns>3D point relative to the camera&apos;s coordinate system.</returns>
		public Vector3 UnprojectPoint(Vector3 point)
		{
			Vector3 temp;
			Internal_unprojectPoint(mCachedPtr, ref point, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFlags(IntPtr thisPtr, CameraFlag flags);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CameraFlag Internal_getFlags(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Viewport Internal_getViewport(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setHorzFOV(IntPtr thisPtr, ref Radian fovy);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getHorzFOV(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setNearClipDistance(IntPtr thisPtr, float nearDist);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getNearClipDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFarClipDistance(IntPtr thisPtr, float farDist);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getFarClipDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setAspectRatio(IntPtr thisPtr, float ratio);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getAspectRatio(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getProjectionMatrixRS(IntPtr thisPtr, out Matrix4 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getViewMatrix(IntPtr thisPtr, out Matrix4 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setProjectionType(IntPtr thisPtr, ProjectionType pt);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ProjectionType Internal_getProjectionType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOrthoWindowHeight(IntPtr thisPtr, float h);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getOrthoWindowHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOrthoWindowWidth(IntPtr thisPtr, float w);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getOrthoWindowWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPriority(IntPtr thisPtr, int priority);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getPriority(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLayers(IntPtr thisPtr, ulong layers);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_getLayers(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMSAACount(IntPtr thisPtr, int count);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getMSAACount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setRenderSettings(IntPtr thisPtr, RenderSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RenderSettings Internal_getRenderSettings(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_notifyNeedsRedraw(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_worldToScreenPoint(IntPtr thisPtr, ref Vector3 worldPoint, out Vector2I __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_worldToNdcPoint(IntPtr thisPtr, ref Vector3 worldPoint, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_worldToViewPoint(IntPtr thisPtr, ref Vector3 worldPoint, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_screenToWorldPoint(IntPtr thisPtr, ref Vector2I screenPoint, float depth, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_screenToViewPoint(IntPtr thisPtr, ref Vector2I screenPoint, float depth, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_screenToNdcPoint(IntPtr thisPtr, ref Vector2I screenPoint, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_viewToWorldPoint(IntPtr thisPtr, ref Vector3 viewPoint, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_viewToScreenPoint(IntPtr thisPtr, ref Vector3 viewPoint, out Vector2I __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_viewToNdcPoint(IntPtr thisPtr, ref Vector3 viewPoint, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ndcToWorldPoint(IntPtr thisPtr, ref Vector2 ndcPoint, float depth, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ndcToViewPoint(IntPtr thisPtr, ref Vector2 ndcPoint, float depth, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ndcToScreenPoint(IntPtr thisPtr, ref Vector2 ndcPoint, out Vector2I __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_screenPointToRay(IntPtr thisPtr, ref Vector2I screenPoint, out Ray __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_projectPoint(IntPtr thisPtr, ref Vector3 point, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_unprojectPoint(IntPtr thisPtr, ref Vector3 point, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMain(IntPtr thisPtr, bool main);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_isMain(IntPtr thisPtr);
	}

	/** @} */
}
