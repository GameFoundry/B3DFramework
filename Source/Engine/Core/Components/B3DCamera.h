//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "CoreObject/B3DCoreObject.h"
#include "Math/B3DConvexVolume.h"
#include "Math/B3DTransform.h"
#include "Math/B3DRay.h"

namespace b3d
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/** Flags for controlling Camera options. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) CameraFlag
	{
		/**
		 * If set the camera will only render when requested by the user through Camera::notifyNeedsRedraw().
		 * Otherwise the camera will render every frame (unless disabled).
		 */
		OnDemand = 1 << 0,
	};

	using CameraFlags = Flags<CameraFlag>;
	B3D_FLAGS_OPERATORS(CameraFlag)

	/** @} */

	/** @addtogroup Rendering-Internal
	 *  @{
	 */

	/**	Signals which portion of a Camera is dirty. */
	enum class CameraDirtyFlag
	{
		// First few bits reserved by ComponentDirtyFlag
		RenderSettings = 1 << 4,
		Redraw = 1 << 5,
		Viewport = 1 << 31
	};

	/** Templated common base class for both main and render thread implementations of Camera. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TCamera : public CoreVariantType<CoreObject, IsRenderProxy>
	{
		using ViewportType = CoreVariantType<Viewport, IsRenderProxy>;
		using RenderSettingsType = CoreVariantType<RenderSettings, IsRenderProxy>;
		using SceneInstanceType = CoreVariantType<SceneInstance, IsRenderProxy>;
		using Super = CoreVariantType<CoreObject, IsRenderProxy>;

	public:
		TCamera();
		virtual ~TCamera() = default;

		/** Determines flags used for controlling the camera behaviour. */
		B3D_SCRIPT_EXPORT(ExportName(Flags), Property(Setter))
		void SetFlags(CameraFlags flags);

		/** @copydoc SetFlags */
		B3D_SCRIPT_EXPORT(ExportName(Flags), Property(Getter))
		CameraFlags GetFlags() const { return mCameraFlags; }

		/**
		 * Determines the camera horizontal field of view. This determines how wide the camera viewing angle is along the
		 * horizontal axis. Vertical FOV is calculated from the horizontal FOV and the aspect ratio.
		 */
		B3D_SCRIPT_EXPORT(ExportName(FieldOfView), Property(Setter), UIValueRange([ 1, 360 ]), UI(AsSlider), UIOrder(-1))
		void SetHorizontalFOV(const Radian& fovy);

		/** @copydoc SetHorizontalFOV */
		B3D_SCRIPT_EXPORT(ExportName(FieldOfView), Property(Getter), UIValueRange([ 1, 360 ]), UI(AsSlider), UIOrder(-1))
		const Radian& GetHorizontalFOV() const { return mHorzFOV; }

		/**
		 * Determines the distance from the frustum to the near clipping plane. Anything closer than the near clipping plane will
		 * not be rendered. Decreasing this value decreases depth buffer precision.
		 */
		B3D_SCRIPT_EXPORT(ExportName(NearClipPlane), Property(Setter))
		void SetNearClipDistance(float nearDist);

		/** @copydoc SetNearClipDistance */
		B3D_SCRIPT_EXPORT(ExportName(NearClipPlane), Property(Getter))
		float GetNearClipDistance() const { return mNearDist; }

		/**
		 * Determines the distance from the frustum to the far clipping plane. Anything farther than the far clipping plane will
		 * not be rendered. Increasing this value decreases depth buffer precision.
		 */
		B3D_SCRIPT_EXPORT(ExportName(FarClipPlane), Property(Setter))
		void SetFarClipDistance(float farDist);

		/** @copydoc SetFarClipDistance */
		B3D_SCRIPT_EXPORT(ExportName(FarClipPlane), Property(Getter))
		float GetFarClipDistance() const { return mFarDist; }

		/**	Determines the current viewport aspect ratio (width / height). */
		B3D_SCRIPT_EXPORT(ExportName(AspectRatio), Property(Setter))
		void SetAspectRatio(float ratio);

		/** @copydoc SetAspectRatio */
		B3D_SCRIPT_EXPORT(ExportName(AspectRatio), Property(Getter))
		float GetAspectRatio() const;

		/**
		 * Manually set the extents of the frustum that will be used when calculating the projection matrix. This will
		 * prevents extents for being automatically calculated from aspect and near plane so it is up to the caller to keep
		 * these values accurate.
		 *
		 * @param	left	The position where the left clip plane intersect the near clip plane, in view space.
		 * @param	right	The position where the right clip plane intersect the near clip plane, in view space.
		 * @param	top	The position where the top clip plane intersect the near clip plane, in view space.
		 * @param	bottom	The position where the bottom clip plane intersect the near clip plane, in view space.
		 */
		void SetFrustumExtents(float left, float right, float top, float bottom);

		/**
		 * Resets frustum extents so they are automatically derived from other values. This is only relevant if you have
		 * previously set custom extents.
		 */
		void ResetFrustumExtents();

		/** Returns the extents of the frustum in view space at the near plane. */
		void GetFrustumExtents(float& outLeft, float& outRight, float& outTop, float& outBottom) const;

		/**
		 * Returns the standard projection matrix that determines how are 3D points projected to two dimensions. The layout
		 * of this matrix depends on currently used GPU backend.
		 */
		B3D_SCRIPT_EXPORT(ExportName(ProjMatrix), Property(Getter))
		const Matrix4& GetProjectionMatrix() const;

		/**
		 * Returns the standard projection matrix that determines how are 3D points projected to two dimensions. Returned
		 * matrix is standard following right-hand rules and depth range of [-1, 1]. Note that currently used GPU backend
		 * might expect different rules, in which case use GetProjectionMatrix() to retrieve an adjusted matrix.
		 */
		const Matrix4& GetUnadjustedProjectionMatrix() const;

		/** Gets the camera view matrix. Used for positioning/orienting the camera. */
		B3D_SCRIPT_EXPORT(ExportName(ViewMatrix), Property(Getter))
		const Matrix4& GetViewMatrix() const;

		/**
		 * Sets whether the camera should use the custom projection matrix. When this is enabled camera will no longer
		 * calculate its projection matrix based on field of view, aspect and other parameters and caller will be resonsible
		 * to keep the projection matrix up to date.
		 */
		void SetCustomProjectionMatrix(bool enable, const Matrix4& projectionMatrix = Matrix4::kIdentity);

		/** Returns the extents of the frustum in view space at the near plane. */
		bool IsCustomProjectionMatrixEnabled() const { return mCustomProjMatrix; }

		/**
		 * Sets whether the camera should use the custom view matrix. When this is enabled camera will no longer calculate
		 * its view matrix based on position/orientation and caller will be resonsible to keep the view matrix up to date.
		 */
		void SetCustomViewMatrix(bool enable, const Matrix4& viewMatrix = Matrix4::kIdentity);

		/** Returns true if a custom view matrix is used. */
		bool IsCustomViewMatrixEnabled() const { return mCustomViewMatrix; }

		/** Returns a convex volume representing the visible area of the camera, in local space. */
		const ConvexVolume& GetFrustum() const;

		/** Returns a convex volume representing the visible area of the camera, in world space. */
		ConvexVolume GetWorldFrustum() const;

		/**	Returns the bounding of the frustum. */
		const AABox& GetBoundingBox() const;

		/**
		 * Determines the type of projection used by the camera. Projection type controls how is 3D geometry projected onto a
		 * 2D plane.
		 */
		B3D_SCRIPT_EXPORT(ExportName(ProjectionType), Property(Setter), UIOrder(-2))
		void SetProjectionType(ProjectionType pt);

		/** @copydoc SetProjectionType */
		B3D_SCRIPT_EXPORT(ExportName(ProjectionType), Property(Getter), UIOrder(-2))
		ProjectionType GetProjectionType() const;

		/**
		 * Sets the orthographic window size, for use with orthographic rendering only.
		 *
		 * @param	width	Width of the window in world units.
		 * @param	height	Height of the window in world units.
		 *
		 * @note
		 * Calling this method will recalculate the aspect ratio, use SetOrthographicHeight() or SetOrthographicWidth() alone
		 * if you wish to preserve the aspect ratio but just fit one or other dimension to a particular size.
		 */
		void SetOrthographicSize(float width, float height);

		/**
		 * Determines the type of projection used by the camera. Projection type controls how is 3D geometry projected onto a
		 * 2D plane.
		 */
		B3D_SCRIPT_EXPORT(ExportName(OrthographicHeight), Property(Setter), UIOrder(-1))
		void SetOrthographicHeight(float height);

		/** @copydoc SetOrthographicHeight */
		B3D_SCRIPT_EXPORT(ExportName(OrthographicHeight), Property(Getter), UIOrder(-1))
		float GetOrthographicHeight() const;

		/**
		 * Determines the orthographic window width, for use with orthographic rendering only. The height of the window
		 * will be calculated from the aspect ratio. Value is specified in world units.
		 */
		B3D_SCRIPT_EXPORT(ExportName(OrthographicWidth), Property(Setter), UI(Hide))
		void SetOrthographicWidth(float width);

		/** @copydoc GetOrthographicWidth */
		B3D_SCRIPT_EXPORT(ExportName(OrthographicWidth), Property(Getter), UI(Hide))
		float GetOrthographicWidth() const;

		/**
		 * Determines a priority that determines in which orders the cameras are rendered. This only applies to cameras rendering
		 * to the same render target. Higher value means the camera will be rendered sooner.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Priority), Property(Setter))
		void SetPriority(i32 priority)
		{
			mPriority = priority;
			MarkRenderProxyDataDirty();
		}

		/** @copydoc SetPriority */
		B3D_SCRIPT_EXPORT(ExportName(Priority), Property(Getter))
		i32 GetPriority() const { return mPriority; }

		/**	Determines layer bitfield that is used when determining which object should the camera render. */
		B3D_SCRIPT_EXPORT(ExportName(Layers), Property(Setter), UI(AsLayerMask))
		void SetLayers(u64 layers)
		{
			mLayers = layers;
			MarkRenderProxyDataDirty();
		}

		/** @copydoc SetLayers */
		B3D_SCRIPT_EXPORT(ExportName(Layers), Property(Getter), UI(AsLayerMask))
		u64 GetLayers() const { return mLayers; }

		/**
		 * Determines number of samples to use when rendering to this camera. Values larger than 1 will enable MSAA
		 * rendering.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SampleCount), Property(Setter))
		void SetSampleCount(u32 count)
		{
			mMSAA = count;
			MarkRenderProxyDataDirty();
		}

		/** @copydoc SetSampleCount */
		B3D_SCRIPT_EXPORT(ExportName(SampleCount), Property(Getter))
		u32 GetSampleCount() const { return mMSAA; }

		/**	Returns the viewport used by the camera. */
		B3D_SCRIPT_EXPORT(ExportName(Viewport), Property(Getter))
		SPtr<ViewportType> GetViewport() const { return mViewport; }

		/**
		 * Settings that control rendering for this view. They determine how will the renderer process this view, which
		 * effects will be enabled, and what properties will those effects use.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RenderSettings), Property(Setter), ApplyOnDirty(true))
		void SetRenderSettings(const SPtr<RenderSettingsType>& settings)
		{
			mRenderSettings = settings;
			MarkRenderProxyDataDirty((ComponentDirtyFlag)CameraDirtyFlag::RenderSettings);
		}

		/** @copydoc SetRenderSettings() */
		B3D_SCRIPT_EXPORT(ExportName(RenderSettings), Property(Getter), ApplyOnDirty(true))
		const SPtr<RenderSettingsType>& GetRenderSettings() const { return mRenderSettings; }

		/**
		 * Notifies a on-demand camera that it should re-draw its contents on the next frame. Ignored for a camera
		 * that isn't on-demand.
		 */
		B3D_SCRIPT_EXPORT()
		void NotifyNeedsRedraw() { MarkRenderProxyDataDirty((ComponentDirtyFlag)CameraDirtyFlag::Redraw); }

		/**
		 * Converts a point in world space to screen coordinates.
		 *
		 * @param	worldPoint	3D point in world space.
		 * @return	2D point on the render target attached to the camera's viewport, in pixels.
		 */
		B3D_SCRIPT_EXPORT()
		Vector2I WorldToScreenPoint(const Vector3& worldPoint) const;

		/**
		 * Converts a point in world space to normalized device coordinates.
		 *
		 * @param	worldPoint	3D point in world space.
		 * @return	2D point in normalized device coordinates ([-1, 1] range), relative to the camera's viewport.
		 */
		B3D_SCRIPT_EXPORT()
		Vector2 WorldToNDCPoint(const Vector3& worldPoint) const;

		/**
		 * Converts a point in world space to view space coordinates.
		 *
		 * @param	worldPoint	3D point in world space.
		 * @return	3D point relative to the camera's coordinate system.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 WorldToViewPoint(const Vector3& worldPoint) const;

		/**
		 * Converts a point in screen space to a point in world space.
		 *
		 * @param	screenPoint	2D point on the render target attached to the camera's viewport, in pixels.
		 * @param	depth	Depth to place the world point at, in world coordinates. The depth is applied to the
		 *	vector going from camera origin to the point on the near plane.
		 * @return	3D point in world space.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 ScreenToWorldPoint(const Vector2I& screenPoint, float depth = 0.5f) const;

		/**
		 * Converts a point in screen space (pixels corresponding to render target attached to the camera) to a point in
		 * world space.
		 *
		 * @param	screenPoint	Point to transform.
		 * @param	deviceDepth	Depth to place the world point at, in normalized device coordinates.
		 * @return	3D point in world space.
		 */
		Vector3 ScreenToWorldPointDeviceDepth(const Vector2I& screenPoint, float deviceDepth = 0.5f) const;

		/**
		 * Converts a point in screen space to a point in view space.
		 *
		 * @param	screenPoint	2D point on the render target attached to the camera's viewport, in pixels.
		 * @param	depth	Depth to place the world point at, in device depth. The depth is applied to the
		 *	vector going from camera origin to the point on the near plane.
		 * @return	3D point relative to the camera's coordinate system.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 ScreenToViewPoint(const Vector2I& screenPoint, float depth = 0.5f) const;

		/**
		 * Converts a point in screen space to normalized device coordinates.
		 *
		 * @param	screenPoint	2D point on the render target attached to the camera's viewport, in pixels.
		 * @return	2D point in normalized device coordinates ([-1, 1] range), relative to
		 *	the camera's viewport.
		 */
		B3D_SCRIPT_EXPORT()
		Vector2 ScreenToNDCPoint(const Vector2I& screenPoint) const;

		/**
		 * Converts a point in view space to world space.
		 *
		 * @param	viewPoint	3D point relative to the camera's coordinate system.
		 * @return	3D point in world space.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 ViewToWorldPoint(const Vector3& viewPoint) const;

		/**
		 * Converts a point in view space to screen space.
		 *
		 * @param	viewPoint	3D point relative to the camera's coordinate system.
		 * @return	2D point on the render target attached to the camera's viewport, in pixels.
		 */
		B3D_SCRIPT_EXPORT()
		Vector2I ViewToScreenPoint(const Vector3& viewPoint) const;

		/**
		 * Converts a point in view space to normalized device coordinates.
		 *
		 * @param	viewPoint	3D point relative to the camera's coordinate system.
		 * @return	2D point in normalized device coordinates ([-1, 1] range), relative to
		 *	the camera's viewport.
		 */
		B3D_SCRIPT_EXPORT()
		Vector2 ViewToNDCPoint(const Vector3& viewPoint) const;

		/**
		 * Converts a point in normalized device coordinates to world space.
		 *
		 * @param	ndcPoint	2D point in normalized device coordinates ([-1, 1] range), relative to
		 *	the camera's viewport.
		 * @param	depth	Depth to place the world point at. The depth is applied to the
		 *	vector going from camera origin to the point on the near plane.
		 * @return	3D point in world space.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 NDCToWorldPoint(const Vector2& ndcPoint, float depth = 0.5f) const;

		/**
		 * Converts a point in normalized device coordinates to view space.
		 *
		 * @param	ndcPoint	2D point in normalized device coordinates ([-1, 1] range), relative to
		 *	the camera's viewport.
		 * @param	depth	Depth to place the world point at. The depth is applied to the
		 *	vector going from camera origin to the point on the near plane.
		 * @return	3D point relative to the camera's coordinate system.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 NDCToViewPoint(const Vector2& ndcPoint, float depth = 0.5f) const;

		/**
		 * Converts a point in normalized device coordinates to screen space.
		 *
		 * @param	ndcPoint	2D point in normalized device coordinates ([-1, 1] range), relative to
		 *	the camera's viewport.
		 * @return	2D point on the render target attached to the camera's viewport, in pixels.
		 */
		B3D_SCRIPT_EXPORT()
		Vector2I NDCToScreenPoint(const Vector2& ndcPoint) const;

		/**
		 * Converts a point in screen space to a ray in world space.
		 *
		 * @param	screenPoint	2D point on the render target attached to the camera's viewport, in pixels.
		 * @return	Ray in world space, originating at the selected point on the camera near plane.
		 */
		B3D_SCRIPT_EXPORT()
		Ray ScreenPointToRay(const Vector2I& screenPoint) const;

		/**
		 * Projects a point in view space to normalized device coordinates. Similar to ViewToNDCPoint() but preserves
		 * the depth component.
		 *
		 * @param	point	3D point relative to the camera's coordinate system.
		 * @return	3D point in normalized device coordinates ([-1, 1] range), relative to the
		 *	camera's viewport. Z value range depends on active render API.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 ProjectPoint(const Vector3& point) const;

		/**
		 * Un-projects a point in normalized device space to view space.
		 *
		 * @param	point	3D point in normalized device coordinates ([-1, 1] range), relative to the
		 *	camera's viewport. Z value range depends on active render API.
		 * @return	3D point relative to the camera's coordinate system.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 UnprojectPoint(const Vector3& point) const;

		static const float kInfiniteFarPlaneAdjust; /**< Small constant used to reduce far plane projection to avoid inaccuracies. */
	protected:
		/** @copydoc CoreObject::MarkRenderProxyDataDirty */
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		/** Returns the world space transform of the object. */
		const Transform& GetTransform() const;

		/**	Calculate projection parameters that are used when constructing the projection matrix. */
		void CalculateProjectionParameters(float& left, float& right, float& bottom, float& top) const;

		/**	Recalculate frustum if dirty. */
		void UpdateFrustum() const;

		/**	Recalculate frustum planes if dirty. */
		void UpdateFrustumPlanes() const;

		/**	Checks if the frustum requires updating. */
		bool IsFrustumOutOfDate() const;

		/**	Notify camera that the frustum requires to be updated. */
		void InvalidateFrustum() const;

		/**	Returns a rectangle that defines the viewport position and size, in pixels. */
		Area2I GetViewportArea() const;

		u64 mLayers = 0xFFFFFFFFFFFFFFFF; /**< Bitfield that can be used for filtering what objects the camera sees. */

		ProjectionType mProjType = PT_PERSPECTIVE; /**< Type of camera projection. */
		Radian mHorzFOV = Degree(90.0f); /**< Horizontal field of view represents how wide is the camera angle. */
		float mFarDist = 500.0f; /**< Clip any objects further than this. Larger value decreases depth precision at smaller depths. */
		float mNearDist = 0.05f; /**< Clip any objects close than this. Smaller value decreases depth precision at larger depths. */
		float mAspect = 1.33333333333333f; /**< Width/height viewport ratio. */
		float mOrthoHeight = 5; /**< Height in world units used for orthographic cameras. */
		i32 mPriority = 0; /**< Determines in what order will the camera be rendered. Higher priority means the camera will be rendered sooner. */
		bool mMain = false; /**< Determines does this camera render to the main render surface. */
		CameraFlags mCameraFlags; /**< Flags for controlling various behaviour. */

		bool mCustomProjMatrix = false; /**< Is custom projection matrix set. */
		bool mCustomViewMatrix = false; /**< Is custom view matrix set. */
		u8 mMSAA = 1; /**< Number of samples to render the scene with. */

		bool mFrustumExtentsManuallySet = false; /**< Are frustum extents manually set. */

		mutable Matrix4 mProjMatrixRS = kZeroTag; /**< Cached render-system specific projection matrix. */
		mutable Matrix4 mProjMatrix = kZeroTag; /**< Cached projection matrix that determines how are 3D points projected to a 2D viewport. */

		mutable Matrix4 mViewMatrix = kZeroTag; /**< Cached view matrix that determines camera position/orientation. */
		mutable bool mRecalcView : 1; /**< Should view matrix be recalculated. */

		mutable ConvexVolume mFrustum; /**< Main clipping planes describing cameras visible area. */
		mutable bool mRecalcFrustum : 1; /**< Should frustum be recalculated. */
		mutable bool mRecalcFrustumPlanes : 1; /**< Should frustum planes be recalculated. */
		mutable float mLeft, mRight, mTop, mBottom; /**< Frustum extents. */
		mutable AABox mBoundingBox; /**< Frustum bounding box. */

		SPtr<ViewportType> mViewport; /**< Viewport that describes a 2D rendering surface. */
		SPtr<RenderSettingsType> mRenderSettings; /**< Settings used to control rendering for this camera. */
	};

	/** @} */
	/** @addtogroup Rendering
	 *  @{
	 */

	/**
	 * Camera determines how is world geometry projected onto a 2D surface. You may position and orient it in space, set
	 * options like aspect ratio and field or view and it outputs view and projection matrices required for rendering.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Camera : public Component, public TCamera<false>
	{
	public:
		Camera(const HSceneObject& parent);
		virtual ~Camera() = default;

		/**
		 * Determines whether this is the main application camera. Main camera controls the final render surface that is
		 * displayed to the user.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Main), Property(Setter))
		void SetMain(bool main);

		/** @copydoc SetMain */
		B3D_SCRIPT_EXPORT(ExportName(Main), Property(Getter))
		bool IsMain() const { return mMain; }

		/**
		 * Requests an asynchronous capture of the next rendered frame from this camera.
		 *
		 * @return	Async operation with captured pixel data, or nullptr if camera is inactive.
		 */
		B3D_SCRIPT_EXPORT()
		TAsyncOp<SPtr<PixelData>> RequestCapture();

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void Initialize() override;
		void OnCreated() override;
		void OnBeginPlay() override;
		void OnEnabled() override;
		void OnDisabled() override;
		void OnDestroyed() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

	protected:
		struct FullSyncPacket;
		struct RedrawSyncPacket;
		struct TransformSyncPacket;

		friend class render::Camera;

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CameraRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		Camera(); // Serialization only
	};

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace render
	{
		/** @copydoc b3d::Camera */
		class B3D_EXPORT Camera : public TCamera<true>
		{
		public:
			~Camera();

			/** @copydoc b3d::Camera::SetMain() */
			bool IsMain() const { return mMain; }

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

			/** Returns the world space transform for the camera. */
			const Transform& GetWorldTransform() const { return mTransform; }

		protected:
			friend class b3d::Camera;

			Camera(const SPtr<SceneInstance>& scene, const SPtr<RenderTarget>& target = nullptr, float left = 0.0f, float top = 0.0f, float width = 1.0f, float height = 1.0f);
			Camera(const SPtr<SceneInstance>& scene, const SPtr<Viewport>& viewport);

			void Initialize() override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			u32 mRendererId;
			Transform mTransform;
			bool mActive = true;
			SPtr<SceneInstance> mSceneInstance;
		};
	} // namespace render

	/** @} */
} // namespace b3d
