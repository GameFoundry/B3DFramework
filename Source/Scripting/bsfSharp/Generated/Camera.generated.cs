//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
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

		/// <summary>
		/// Determines whether this is the main application camera. Main camera controls the final render surface that is 
		/// displayed to the user.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Main
		{
			get { return Internal_IsMain(mCachedPtr); }
			set { Internal_SetMain(mCachedPtr, value); }
		}

		/// <summary>Determines flags used for controlling the camera behaviour.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CameraFlag Flags
		{
			get { return Internal_GetFlags(mCachedPtr); }
			set { Internal_SetFlags(mCachedPtr, value); }
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
				Internal_GetHorizontalFOV(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetHorizontalFOV(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the distance from the frustum to the near clipping plane. Anything closer than the near clipping plane 
		/// will not be rendered. Decreasing this value decreases depth buffer precision.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float NearClipPlane
		{
			get { return Internal_GetNearClipDistance(mCachedPtr); }
			set { Internal_SetNearClipDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the distance from the frustum to the far clipping plane. Anything farther than the far clipping plane will 
		/// not be rendered. Increasing this value decreases depth buffer precision.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FarClipPlane
		{
			get { return Internal_GetFarClipDistance(mCachedPtr); }
			set { Internal_SetFarClipDistance(mCachedPtr, value); }
		}

		/// <summary>Determines the current viewport aspect ratio (width / height).</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float AspectRatio
		{
			get { return Internal_GetAspectRatio(mCachedPtr); }
			set { Internal_SetAspectRatio(mCachedPtr, value); }
		}

		/// <summary>
		/// Returns the standard projection matrix that determines how are 3D points projected to two dimensions. The layout of 
		/// this matrix depends on currently used GPU backend.
		/// </summary>
		[NativeWrapper]
		public Matrix4 ProjMatrix
		{
			get
			{
				Matrix4 temp;
				Internal_GetProjectionMatrix(mCachedPtr, out temp);
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
				Internal_GetViewMatrix(mCachedPtr, out temp);
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
			get { return Internal_GetProjectionType(mCachedPtr); }
			set { Internal_SetProjectionType(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the type of projection used by the camera. Projection type controls how is 3D geometry projected onto a 2D 
		/// plane.
		/// </summary>
		[ShowInInspector]
		[Order(-1)]
		[NativeWrapper]
		public float OrthographicHeight
		{
			get { return Internal_GetOrthographicHeight(mCachedPtr); }
			set { Internal_SetOrthographicHeight(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the orthographic window width, for use with orthographic rendering only. The height of the window will be 
		/// calculated from the aspect ratio. Value is specified in world units.
		/// </summary>
		[NativeWrapper]
		public float OrthographicWidth
		{
			get { return Internal_GetOrthographicWidth(mCachedPtr); }
			set { Internal_SetOrthographicWidth(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a priority that determines in which orders the cameras are rendered. This only applies to cameras 
		/// rendering to the same render target. Higher value means the camera will be rendered sooner.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Priority
		{
			get { return Internal_GetPriority(mCachedPtr); }
			set { Internal_SetPriority(mCachedPtr, value); }
		}

		/// <summary>Determines layer bitfield that is used when determining which object should the camera render.</summary>
		[ShowInInspector]
		[LayerMask]
		[NativeWrapper]
		public ulong Layers
		{
			get { return Internal_GetLayers(mCachedPtr); }
			set { Internal_SetLayers(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines number of samples to use when rendering to this camera. Values larger than 1 will enable MSAA rendering.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int SampleCount
		{
			get { return Internal_GetSampleCount(mCachedPtr); }
			set { Internal_SetSampleCount(mCachedPtr, value); }
		}

		/// <summary>Returns the viewport used by the camera.</summary>
		[NativeWrapper]
		public Viewport Viewport
		{
			get { return Internal_GetViewport(mCachedPtr); }
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
			get { return Internal_GetRenderSettings(mCachedPtr); }
			set { Internal_SetRenderSettings(mCachedPtr, value); }
		}

		/// <summary>
		/// Notifies a on-demand camera that it should re-draw its contents on the next frame. Ignored for a camera that 
		/// isn&apos;t on-demand.
		/// </summary>
		public void NotifyNeedsRedraw()
		{
			Internal_NotifyNeedsRedraw(mCachedPtr);
		}

		/// <summary>Converts a point in world space to screen coordinates.</summary>
		/// <param name="worldPoint">3D point in world space.</param>
		/// <returns>2D point on the render target attached to the camera&apos;s viewport, in pixels.</returns>
		public TVector2<int> WorldToScreenPoint(Vector3 worldPoint)
		{
			TVector2<int> temp;
			Internal_WorldToScreenPoint(mCachedPtr, ref worldPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in world space to normalized device coordinates.</summary>
		/// <param name="worldPoint">3D point in world space.</param>
		/// <returns>2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.</returns>
		public TVector2<float> WorldToNDCPoint(Vector3 worldPoint)
		{
			TVector2<float> temp;
			Internal_WorldToNDCPoint(mCachedPtr, ref worldPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in world space to view space coordinates.</summary>
		/// <param name="worldPoint">3D point in world space.</param>
		/// <returns>3D point relative to the camera&apos;s coordinate system.</returns>
		public Vector3 WorldToViewPoint(Vector3 worldPoint)
		{
			Vector3 temp;
			Internal_WorldToViewPoint(mCachedPtr, ref worldPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in screen space to a point in world space.</summary>
		/// <param name="screenPoint">2D point on the render target attached to the camera&apos;s viewport, in pixels.</param>
		/// <param name="depth">
		/// Depth to place the world point at, in world coordinates. The depth is applied to the vector going from camera origin 
		/// to the point on the near plane.
		/// </param>
		/// <returns>3D point in world space.</returns>
		public Vector3 ScreenToWorldPoint(TVector2<int> screenPoint, float depth = 0.5f)
		{
			Vector3 temp;
			Internal_ScreenToWorldPoint(mCachedPtr, ref screenPoint, depth, out temp);
			return temp;
		}

		/// <summary>Converts a point in screen space to a point in view space.</summary>
		/// <param name="screenPoint">2D point on the render target attached to the camera&apos;s viewport, in pixels.</param>
		/// <param name="depth">
		/// Depth to place the world point at, in device depth. The depth is applied to the vector going from camera origin to 
		/// the point on the near plane.
		/// </param>
		/// <returns>3D point relative to the camera&apos;s coordinate system.</returns>
		public Vector3 ScreenToViewPoint(TVector2<int> screenPoint, float depth = 0.5f)
		{
			Vector3 temp;
			Internal_ScreenToViewPoint(mCachedPtr, ref screenPoint, depth, out temp);
			return temp;
		}

		/// <summary>Converts a point in screen space to normalized device coordinates.</summary>
		/// <param name="screenPoint">2D point on the render target attached to the camera&apos;s viewport, in pixels.</param>
		/// <returns>2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.</returns>
		public TVector2<float> ScreenToNDCPoint(TVector2<int> screenPoint)
		{
			TVector2<float> temp;
			Internal_ScreenToNDCPoint(mCachedPtr, ref screenPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in view space to world space.</summary>
		/// <param name="viewPoint">3D point relative to the camera&apos;s coordinate system.</param>
		/// <returns>3D point in world space.</returns>
		public Vector3 ViewToWorldPoint(Vector3 viewPoint)
		{
			Vector3 temp;
			Internal_ViewToWorldPoint(mCachedPtr, ref viewPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in view space to screen space.</summary>
		/// <param name="viewPoint">3D point relative to the camera&apos;s coordinate system.</param>
		/// <returns>2D point on the render target attached to the camera&apos;s viewport, in pixels.</returns>
		public TVector2<int> ViewToScreenPoint(Vector3 viewPoint)
		{
			TVector2<int> temp;
			Internal_ViewToScreenPoint(mCachedPtr, ref viewPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in view space to normalized device coordinates.</summary>
		/// <param name="viewPoint">3D point relative to the camera&apos;s coordinate system.</param>
		/// <returns>2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.</returns>
		public TVector2<float> ViewToNDCPoint(Vector3 viewPoint)
		{
			TVector2<float> temp;
			Internal_ViewToNDCPoint(mCachedPtr, ref viewPoint, out temp);
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
		public Vector3 NDCToWorldPoint(TVector2<float> ndcPoint, float depth = 0.5f)
		{
			Vector3 temp;
			Internal_NDCToWorldPoint(mCachedPtr, ref ndcPoint, depth, out temp);
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
		public Vector3 NDCToViewPoint(TVector2<float> ndcPoint, float depth = 0.5f)
		{
			Vector3 temp;
			Internal_NDCToViewPoint(mCachedPtr, ref ndcPoint, depth, out temp);
			return temp;
		}

		/// <summary>Converts a point in normalized device coordinates to screen space.</summary>
		/// <param name="ndcPoint">
		/// 2D point in normalized device coordinates ([-1, 1] range), relative to the camera&apos;s viewport.
		/// </param>
		/// <returns>2D point on the render target attached to the camera&apos;s viewport, in pixels.</returns>
		public TVector2<int> NDCToScreenPoint(TVector2<float> ndcPoint)
		{
			TVector2<int> temp;
			Internal_NDCToScreenPoint(mCachedPtr, ref ndcPoint, out temp);
			return temp;
		}

		/// <summary>Converts a point in screen space to a ray in world space.</summary>
		/// <param name="screenPoint">2D point on the render target attached to the camera&apos;s viewport, in pixels.</param>
		/// <returns>Ray in world space, originating at the selected point on the camera near plane.</returns>
		public Ray ScreenPointToRay(TVector2<int> screenPoint)
		{
			Ray temp;
			Internal_ScreenPointToRay(mCachedPtr, ref screenPoint, out temp);
			return temp;
		}

		/// <summary>
		/// Projects a point in view space to normalized device coordinates. Similar to ViewToNDCPoint() but preserves the depth 
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
			Internal_ProjectPoint(mCachedPtr, ref point, out temp);
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
			Internal_UnprojectPoint(mCachedPtr, ref point, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMain(IntPtr thisPtr, bool main);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsMain(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlags(IntPtr thisPtr, CameraFlag flags);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CameraFlag Internal_GetFlags(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHorizontalFOV(IntPtr thisPtr, ref Radian fovy);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetHorizontalFOV(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNearClipDistance(IntPtr thisPtr, float nearDist);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetNearClipDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFarClipDistance(IntPtr thisPtr, float farDist);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFarClipDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAspectRatio(IntPtr thisPtr, float ratio);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetAspectRatio(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetProjectionMatrix(IntPtr thisPtr, out Matrix4 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetViewMatrix(IntPtr thisPtr, out Matrix4 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetProjectionType(IntPtr thisPtr, ProjectionType pt);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ProjectionType Internal_GetProjectionType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOrthographicHeight(IntPtr thisPtr, float height);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetOrthographicHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOrthographicWidth(IntPtr thisPtr, float width);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetOrthographicWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPriority(IntPtr thisPtr, int priority);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetPriority(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLayers(IntPtr thisPtr, ulong layers);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetLayers(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSampleCount(IntPtr thisPtr, int count);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetSampleCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Viewport Internal_GetViewport(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRenderSettings(IntPtr thisPtr, RenderSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RenderSettings Internal_GetRenderSettings(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_NotifyNeedsRedraw(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_WorldToScreenPoint(IntPtr thisPtr, ref Vector3 worldPoint, out TVector2<int> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_WorldToNDCPoint(IntPtr thisPtr, ref Vector3 worldPoint, out TVector2<float> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_WorldToViewPoint(IntPtr thisPtr, ref Vector3 worldPoint, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScreenToWorldPoint(IntPtr thisPtr, ref TVector2<int> screenPoint, float depth, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScreenToViewPoint(IntPtr thisPtr, ref TVector2<int> screenPoint, float depth, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScreenToNDCPoint(IntPtr thisPtr, ref TVector2<int> screenPoint, out TVector2<float> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ViewToWorldPoint(IntPtr thisPtr, ref Vector3 viewPoint, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ViewToScreenPoint(IntPtr thisPtr, ref Vector3 viewPoint, out TVector2<int> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ViewToNDCPoint(IntPtr thisPtr, ref Vector3 viewPoint, out TVector2<float> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_NDCToWorldPoint(IntPtr thisPtr, ref TVector2<float> ndcPoint, float depth, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_NDCToViewPoint(IntPtr thisPtr, ref TVector2<float> ndcPoint, float depth, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_NDCToScreenPoint(IntPtr thisPtr, ref TVector2<float> ndcPoint, out TVector2<int> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScreenPointToRay(IntPtr thisPtr, ref TVector2<int> screenPoint, out Ray __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ProjectPoint(IntPtr thisPtr, ref Vector3 point, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_UnprojectPoint(IntPtr thisPtr, ref Vector3 point, out Vector3 __output);
	}

	/** @} */
}
