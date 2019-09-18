//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "Math/BsMatrix4.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"
#include "Math/BsVector2I.h"
#include "Math/BsAABox.h"
#include "Math/BsQuaternion.h"
#include "Math/BsRay.h"
#include "CoreThread/BsCoreObject.h"
#include "Math/BsConvexVolume.h"
#include "Renderer/BsRenderSettings.h"
#include "Scene/BsSceneActor.h"

namespace bs
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/** Flags for controlling Camera options. */
	enum class BS_SCRIPT_EXPORT(m:Rendering) CameraFlag
	{
		/**
		 * If set the camera will only render when requested by the user through Camera::notifyNeedsRedraw().
		 * Otherwise the camera will render every frame (unless disabled).
		 */
		OnDemand = 1 << 0,
	};

	using CameraFlags = Flags<CameraFlag>;
	BS_FLAGS_OPERATORS(CameraFlag)

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/**	Signals which portion of a Camera is dirty. */
	enum class CameraDirtyFlag
	{
		// First few bits reserved by ActorDirtyFlag
		RenderSettings = 1 << 4,
		Redraw = 1 << 5,
		Viewport = 1 << 31
	};

	/** @} */
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Common base class for both sim and core thread implementations of Camera. */
	class BS_CORE_EXPORT CameraBase : public SceneActor
	{
	public:
		virtual ~CameraBase() = default;

		/** @copydoc SceneActor::setTransform */
		void setTransform(const Transform& transform) override;

		/** Determines flags used for controlling the camera behaviour. */
		void setFlags(CameraFlags flag);

		/** @copydoc setFlags() */
		CameraFlags getFlags() const { return mCameraFlags; }

		/**
		 * Determines the camera horizontal field of view. This determines how wide the camera viewing angle is along the
		 * horizontal axis. Vertical FOV is calculated from the horizontal FOV and the aspect ratio.
		 */
		virtual void setHorzFOV(const Radian& fovy);

		/** @copydoc setHorzFOV() */
		virtual const Radian& getHorzFOV() const { return mHorzFOV; }

		/**
		 * Determines the distance from the frustum to the near clipping plane. Anything closer than the near clipping plane will
		 * not be rendered. Decreasing this value decreases depth buffer precision.
		 */
		virtual void setNearClipDistance(float nearDist);

		/** @copydoc setNearClipDistance() */
		virtual float getNearClipDistance() const { return mNearDist; }

		/**
		 * Determines the distance from the frustum to the far clipping plane. Anything farther than the far clipping plane will
		 * not be rendered. Increasing this value decreases depth buffer precision.
		 */
		virtual void setFarClipDistance(float farDist);

		/** @copydoc setFarClipDistance() */
		virtual float getFarClipDistance() const { return mFarDist; }

		/**	Determines the current viewport aspect ratio (width / height). */
		virtual void setAspectRatio(float ratio);

		/** @copydoc setAspectRatio() */
		virtual float getAspectRatio() const;

		/** Manually set the extents of the frustum that will be used when calculating the projection matrix. This will
		 * prevents extents for being automatically calculated from aspect and near plane so it is up to the caller to keep
		 * these values accurate.
		 *
		 * @param[in] left		The position where the left clip plane intersect the near clip plane, in view space.
		 * @param[in] right		The position where the right clip plane intersect the near clip plane, in view space.
		 * @param[in] top		The position where the top clip plane intersect the near clip plane, in view space.
		 * @param[in] bottom	The position where the bottom clip plane intersect the near clip plane, in view space.
		*/
		virtual void setFrustumExtents(float left, float right, float top, float bottom);

		/**
		 * Resets frustum extents so they are automatically derived from other values. This is only relevant if you have
		 * previously set custom extents.
		 */
		virtual void resetFrustumExtents();

		/** Returns the extents of the frustum in view space at the near plane. */
		virtual void getFrustumExtents(float& outleft, float& outright, float& outtop, float& outbottom) const;

		/**
		 * Returns the standard projection matrix that determines how are 3D points projected to two dimensions. The layout
		 * of this matrix depends on currently used render system.
		 *
		 * @note	
		 * You should use this matrix when sending the matrix to the render system to make sure everything works
		 * consistently when other render systems are used.
		 */
		virtual const Matrix4& getProjectionMatrixRS() const;

		/** Returns the inverse of the render-system specific projection matrix. See getProjectionMatrixRS(). */
		virtual const Matrix4& getProjectionMatrixRSInv() const;

		/**
		 * Returns the standard projection matrix that determines how are 3D points projected to two dimensions. Returned
		 * matrix is standard following right-hand rules and depth range of [-1, 1]. In case you need a render-system specific
		 * projection matrix call getProjectionMatrixRS().
		 */
		virtual const Matrix4& getProjectionMatrix() const;

		/** Returns the inverse of the projection matrix. See getProjectionMatrix(). */
		virtual const Matrix4& getProjectionMatrixInv() const;

		/** Gets the camera view matrix. Used for positioning/orienting the camera. */
		virtual const Matrix4& getViewMatrix() const;

		/** Returns the inverse of the view matrix. See getViewMatrix(). */
		virtual const Matrix4& getViewMatrixInv() const;

		/**
		 * Sets whether the camera should use the custom view matrix. When this is enabled camera will no longer calculate
		 * its view matrix based on position/orientation and caller will be resonsible to keep the view matrix up to date.
		 */
		virtual void setCustomViewMatrix(bool enable, const Matrix4& viewMatrix = Matrix4::IDENTITY);

		/** Returns true if a custom view matrix is used. */
		virtual bool isCustomViewMatrixEnabled() const { return mCustomViewMatrix; }
		
		/**
		 * Sets whether the camera should use the custom projection matrix. When this is enabled camera will no longer
		 * calculate its projection matrix based on field of view, aspect and other parameters and caller will be resonsible
		 * to keep the projection matrix up to date.
		 */
		virtual void setCustomProjectionMatrix(bool enable, const Matrix4& projectionMatrix = Matrix4::IDENTITY);

		/** Returns true if a custom projection matrix is used. */
		virtual bool isCustomProjectionMatrixEnabled() const { return mCustomProjMatrix; }

		/** Returns a convex volume representing the visible area of the camera, in local space. */
		virtual const ConvexVolume& getFrustum() const;

		/** Returns a convex volume representing the visible area of the camera, in world space. */
		virtual ConvexVolume getWorldFrustum() const;

		/**	Returns the bounding of the frustum. */
		const AABox& getBoundingBox() const;

		/**
		 * Determines the type of projection used by the camera. Projection type controls how is 3D geometry projected onto a
		 * 2D plane.
		 */
		virtual void setProjectionType(ProjectionType pt);

		/** @copydoc setProjectionType() */
		virtual ProjectionType getProjectionType() const;

		/**
		 * Sets the orthographic window height, for use with orthographic rendering only.
		 *
		 * @param[in]	w	Width of the window in world units.
		 * @param[in]	h	Height of the window in world units.
		 *
		 * @note	
		 * Calling this method will recalculate the aspect ratio, use setOrthoWindowHeight() or setOrthoWindowWidth() alone
		 * if you wish to preserve the aspect ratio but just fit one or other dimension to a particular size.
		 */
		virtual void setOrthoWindow(float w, float h);

		/**
		 * Determines the orthographic window height, for use with orthographic rendering only. The width of the window
		 * will be calculated from the aspect ratio. Value is specified in world units.
		 */
		virtual void setOrthoWindowHeight(float h);

		/** @copydoc setOrthoWindowHeight() */
		virtual float getOrthoWindowHeight() const;

		/**
		 * Determines the orthographic window width, for use with orthographic rendering only. The height of the window
		 * will be calculated from the aspect ratio. Value is specified in world units.
		 */
		virtual void setOrthoWindowWidth(float w);

		/** @copydoc setOrthoWindowWidth() */
		virtual float getOrthoWindowWidth() const;

		/**
		 * Determines a priority that determines in which orders the cameras are rendered. This only applies to cameras rendering
		 * to the same render target. Higher value means the camera will be rendered sooner.
		 */
		void setPriority(INT32 priority) { mPriority = priority; _markCoreDirty(); }

		/** @copydoc setPriority() */
		INT32 getPriority() const { return mPriority; }

		/**	Determines layer bitfield that is used when determining which object should the camera render. */
		void setLayers(UINT64 layers) { mLayers = layers; _markCoreDirty(); }

		/** @copydoc setLayers() */
		UINT64 getLayers() const { return mLayers; }

		/**
		 * Determines number of samples to use when rendering to this camera. Values larger than 1 will enable MSAA
		 * rendering.
		 */
		void setMSAACount(UINT32 count) { mMSAA = count; _markCoreDirty(); }

		/** @copydoc setMSAACount() */
		UINT32 getMSAACount() const { return mMSAA; }

		/**
		 * Notifies a on-demand camera that it should re-draw its contents on the next frame. Ignored for a camera
		 * that isn't on-demand.
		 */
		void notifyNeedsRedraw() { _markCoreDirty((ActorDirtyFlag)CameraDirtyFlag::Redraw); }
		
		/**
		 * Converts a point in world space to screen coordinates.
		 *
		 * @param[in]	worldPoint		3D point in world space.
		 * @return						2D point on the render target attached to the camera's viewport, in pixels.
		 */
		Vector2I worldToScreenPoint(const Vector3& worldPoint) const;

		/**	
		 * Converts a point in world space to normalized device coordinates.
		 *
		 * @param[in]	worldPoint		3D point in world space.
		 * @return						2D point in normalized device coordinates ([-1, 1] range), relative to the camera's viewport.
		 */
		Vector2 worldToNdcPoint(const Vector3& worldPoint) const;

		/**
		 * Converts a point in world space to view space coordinates.
		 *
		 * @param[in]	worldPoint		3D point in world space.
		 * @return						3D point relative to the camera's coordinate system.
		 */
		Vector3 worldToViewPoint(const Vector3& worldPoint) const;

		/**
		 * Converts a point in screen space to a point in world space.
		 *
		 * @param[in]	screenPoint	2D point on the render target attached to the camera's viewport, in pixels.
		 * @param[in]	depth		Depth to place the world point at, in world coordinates. The depth is applied to the
		 *							vector going from camera origin to the point on the near plane.
		 * @return					3D point in world space.
		 */
		Vector3 screenToWorldPoint(const Vector2I& screenPoint, float depth = 0.5f) const;

		/**
		* Converts a point in screen space (pixels corresponding to render target attached to the camera) to a point in
		* world space.
		*
		* @param[in]	screenPoint	Point to transform.
		* @param[in]	deviceDepth	Depth to place the world point at, in normalized device coordinates.
		* @return					3D point in world space.
		*/
		Vector3 screenToWorldPointDeviceDepth(const Vector2I& screenPoint, float deviceDepth = 0.5f) const;

		/**
		 * Converts a point in screen space to a point in view space.
		 *
		 * @param[in]	screenPoint	2D point on the render target attached to the camera's viewport, in pixels.
		 * @param[in]	depth		Depth to place the world point at, in device depth. The depth is applied to the
		 *							vector going from camera origin to the point on the near plane.
		 * @return					3D point relative to the camera's coordinate system.
		 */
		Vector3 screenToViewPoint(const Vector2I& screenPoint, float depth = 0.5f) const;

		/**
		 * Converts a point in screen space to normalized device coordinates.
		 *
		 * @param[in]	screenPoint		2D point on the render target attached to the camera's viewport, in pixels.
		 * @return						2D point in normalized device coordinates ([-1, 1] range), relative to
		 *								the camera's viewport.
		 */
		Vector2 screenToNdcPoint(const Vector2I& screenPoint) const;

		/**
		 * Converts a point in view space to world space.
		 *
		 * @param[in]	viewPoint		3D point relative to the camera's coordinate system.
		 * @return						3D point in world space.
		 */
		Vector3 viewToWorldPoint(const Vector3& viewPoint) const;

		/**
		 * Converts a point in view space to screen space.
		 *
		 * @param[in]	viewPoint		3D point relative to the camera's coordinate system.
		 * @return						2D point on the render target attached to the camera's viewport, in pixels.
		 */
		Vector2I viewToScreenPoint(const Vector3& viewPoint) const;

		/**
		 * Converts a point in view space to normalized device coordinates.
		 *
		 * @param[in]	viewPoint		3D point relative to the camera's coordinate system.
		 * @return						2D point in normalized device coordinates ([-1, 1] range), relative to
		 *								the camera's viewport.
		 */
		Vector2 viewToNdcPoint(const Vector3& viewPoint) const;

		/**
		 * Converts a point in normalized device coordinates to world space.
		 *
		 * @param[in]	ndcPoint	2D point in normalized device coordinates ([-1, 1] range), relative to
		 *							the camera's viewport.
		 * @param[in]	depth		Depth to place the world point at. The depth is applied to the
		 *							vector going from camera origin to the point on the near plane.
		 * @return					3D point in world space.
		 */
		Vector3 ndcToWorldPoint(const Vector2& ndcPoint, float depth = 0.5f) const;

		/**
		 * Converts a point in normalized device coordinates to view space.
		 *
		 * @param[in]	ndcPoint	2D point in normalized device coordinates ([-1, 1] range), relative to
		 *							the camera's viewport.
		 * @param[in]	depth		Depth to place the world point at. The depth is applied to the
		 *							vector going from camera origin to the point on the near plane.
		 * @return					3D point relative to the camera's coordinate system.
		 */
		Vector3 ndcToViewPoint(const Vector2& ndcPoint, float depth = 0.5f) const;

		/**
		 * Converts a point in normalized device coordinates to screen space.
		 *
		 * @param[in]	ndcPoint	2D point in normalized device coordinates ([-1, 1] range), relative to
		 *							the camera's viewport.
		 * @return					2D point on the render target attached to the camera's viewport, in pixels.
		 */
		Vector2I ndcToScreenPoint(const Vector2& ndcPoint) const;

		/**
		 * Converts a point in screen space to a ray in world space.
		 *
		 * @param[in]	screenPoint		2D point on the render target attached to the camera's viewport, in pixels.
		 * @return						Ray in world space, originating at the selected point on the camera near plane.
		 */
		Ray screenPointToRay(const Vector2I& screenPoint) const;

		/**	
		 * Projects a point in view space to normalized device coordinates. Similar to viewToNdcPoint() but preserves
		 * the depth component.
		 *
		 * @param[in]	point			3D point relative to the camera's coordinate system.
		 * @return						3D point in normalized device coordinates ([-1, 1] range), relative to the
		 *								camera's viewport. Z value range depends on active render API.
		 */
		Vector3 projectPoint(const Vector3& point) const;

		/**	Un-projects a point in normalized device space to view space.
		 *
		 * @param[in]	point			3D point in normalized device coordinates ([-1, 1] range), relative to the
		 *								camera's viewport. Z value range depends on active render API.
		 * @return						3D point relative to the camera's coordinate system.
		 */
		Vector3 unprojectPoint(const Vector3& point) const;

		static const float INFINITE_FAR_PLANE_ADJUST; /**< Small constant used to reduce far plane projection to avoid inaccuracies. */

	protected:
		CameraBase();

		/**	Calculate projection parameters that are used when constructing the projection matrix. */
		virtual void calcProjectionParameters(float& left, float& right, float& bottom, float& top) const;

		/**	Recalculate frustum if dirty. */
		virtual void updateFrustum() const;

		/**	Recalculate frustum planes if dirty. */
		virtual void updateFrustumPlanes() const;

		/**
		 * Update view matrix from parent position/orientation.
		 *
		 * @note	Does nothing when custom view matrix is set.
		 */
		virtual void updateView() const;

		/**	Checks if the frustum requires updating. */
		virtual bool isFrustumOutOfDate() const;

		/**	Notify camera that the frustum requires to be updated. */
		virtual void invalidateFrustum() const;

		/**	Returns a rectangle that defines the viewport position and size, in pixels. */
		virtual Rect2I getViewportRect() const = 0;

	protected:
		UINT64 mLayers = 0xFFFFFFFFFFFFFFFF; /**< Bitfield that can be used for filtering what objects the camera sees. */

		ProjectionType mProjType = PT_PERSPECTIVE; /**< Type of camera projection. */
		Radian mHorzFOV = Degree(90.0f); /**< Horizontal field of view represents how wide is the camera angle. */
		float mFarDist = 500.0f; /**< Clip any objects further than this. Larger value decreases depth precision at smaller depths. */
		float mNearDist = 0.05f; /**< Clip any objects close than this. Smaller value decreases depth precision at larger depths. */
		float mAspect = 1.33333333333333f; /**< Width/height viewport ratio. */
		float mOrthoHeight = 5; /**< Height in world units used for orthographic cameras. */
		INT32 mPriority = 0; /**< Determines in what order will the camera be rendered. Higher priority means the camera will be rendered sooner. */
		bool mMain = false; /**< Determines does this camera render to the main render surface. */
		CameraFlags mCameraFlags; /**< Flags for controlling various behaviour. */

		bool mCustomViewMatrix = false; /**< Is custom view matrix set. */
		bool mCustomProjMatrix = false; /**< Is custom projection matrix set. */
		UINT8 mMSAA = 1; /**< Number of samples to render the scene with. */

		bool mFrustumExtentsManuallySet = false; /**< Are frustum extents manually set. */

		mutable Matrix4 mProjMatrixRS = BsZero; /**< Cached render-system specific projection matrix. */
		mutable Matrix4 mProjMatrix = BsZero; /**< Cached projection matrix that determines how are 3D points projected to a 2D viewport. */
		mutable Matrix4 mViewMatrix = BsZero; /**< Cached view matrix that determines camera position/orientation. */
		mutable Matrix4 mProjMatrixRSInv = BsZero;
		mutable Matrix4 mProjMatrixInv = BsZero;
		mutable Matrix4 mViewMatrixInv = BsZero;

		mutable ConvexVolume mFrustum; /**< Main clipping planes describing cameras visible area. */
		mutable bool mRecalcFrustum : 1; /**< Should frustum be recalculated. */
		mutable bool mRecalcFrustumPlanes : 1; /**< Should frustum planes be recalculated. */
		mutable bool mRecalcView : 1; /**< Should view matrix be recalculated. */
		mutable float mLeft, mRight, mTop, mBottom; /**< Frustum extents. */
		mutable AABox mBoundingBox; /**< Frustum bounding box. */
	};

	/** Templated common base class for both sim and core thread implementations of Camera. */
	template<bool Core>
	class BS_CORE_EXPORT TCamera : public CameraBase
	{
		using ViewportType = CoreVariantType<Viewport, Core>;
		using RenderSettingsType = CoreVariantType<RenderSettings, Core>;

	public:
		TCamera();
		virtual ~TCamera() = default;

		/**	Returns the viewport used by the camera. */	
		SPtr<ViewportType> getViewport() const { return mViewport; }

		/**
		 * Settings that control rendering for this view. They determine how will the renderer process this view, which
		 * effects will be enabled, and what properties will those effects use.
		 */
		void setRenderSettings(const SPtr<RenderSettingsType>& settings)
			{ mRenderSettings = settings; _markCoreDirty((ActorDirtyFlag)CameraDirtyFlag::RenderSettings); }

		/** @copydoc setRenderSettings() */
		const SPtr<RenderSettingsType>& getRenderSettings() const { return mRenderSettings; }

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P p);

	protected:
		/** Viewport that describes a 2D rendering surface. */
		SPtr<ViewportType> mViewport;

		/** Settings used to control rendering for this camera. */
		SPtr<RenderSettingsType> mRenderSettings;
	};

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/**
	 * Camera determines how is world geometry projected onto a 2D surface. You may position and orient it in space, set
	 * options like aspect ratio and field or view and it outputs view and projection matrices required for rendering.
	 */
	class BS_CORE_EXPORT Camera : public IReflectable, public CoreObject, public TCamera<false>
	{
	public:
		/**
		 * Determines whether this is the main application camera. Main camera controls the final render surface that is
		 * displayed to the user.
		 */	
		void setMain(bool main);

		/** @copydoc setMain() */
		bool isMain() const { return mMain; }

		/** Retrieves an implementation of a camera handler usable only from the core thread. */
		SPtr<ct::Camera> getCore() const;

		/**	Creates a new camera that renders to the specified portion of the provided render target. */
		static SPtr<Camera> create();

		/**
		 * @name Internal
		 * @{
		 */

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc CoreObject::destroy */
		void destroy() override;

		/** @} */
	protected:
		/** @copydoc CameraBase */
		Rect2I getViewportRect() const override;

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc CameraBase::_markCoreDirty */
		void _markCoreDirty(ActorDirtyFlag flag = ActorDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/**	Creates a new camera without initializing it. */
		static SPtr<Camera> createEmpty();

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CameraRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	namespace ct
	{
	/** @copydoc bs::Camera */
	class BS_CORE_EXPORT Camera : public CoreObject, public TCamera<true>
	{
	public:
		~Camera();

		/** @copydoc bs::Camera::setMain() */
		bool isMain() const { return mMain; }

		/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
		void setRendererId(UINT32 id) { mRendererId = id; }

		/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
		UINT32 getRendererId() const { return mRendererId; }
		
	protected:
		friend class bs::Camera;

		Camera(SPtr<RenderTarget> target = nullptr,
			float left = 0.0f, float top = 0.0f, float width = 1.0f, float height = 1.0f);

		Camera(const SPtr<Viewport>& viewport);

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc CameraBase */
		Rect2I getViewportRect() const override;

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;

		UINT32 mRendererId;
	};
	}

	/** @} */
}
