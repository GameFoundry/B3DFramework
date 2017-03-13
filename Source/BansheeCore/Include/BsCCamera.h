//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCamera.h"
#include "BsComponent.h"

namespace bs 
{
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * @copydoc	Camera
	 * 			
	 * Wraps a Camera as a Component.
	 */
    class BS_CORE_EXPORT CCamera : public Component
    {
    public:
		CCamera(const HSceneObject& parent, SPtr<RenderTarget> target = nullptr,
			float left = 0.0f, float top = 0.0f, float width = 1.0f, float height = 1.0f);

        virtual ~CCamera();

		/** @copydoc Camera::getViewport */
		SPtr<Viewport> getViewport() const { return mInternal->getViewport(); }

		/** @copydoc Camera::setHorzFOV */
		virtual void setHorzFOV(const Radian& fovy) { mInternal->setHorzFOV(fovy); }

		/** @copydoc Camera::getHorzFOV */
		virtual const Radian& getHorzFOV() const { return mInternal->getHorzFOV(); }

		/** @copydoc Camera::setNearClipDistance */
		virtual void setNearClipDistance(float nearDist) { mInternal->setNearClipDistance(nearDist); }

		/** @copydoc Camera::getNearClipDistance */
		virtual float getNearClipDistance() const { return mInternal->getNearClipDistance(); }

		/** @copydoc Camera::setFarClipDistance */
		virtual void setFarClipDistance(float farDist) { mInternal->setFarClipDistance(farDist); }

		/** @copydoc Camera::getFarClipDistance */
		virtual float getFarClipDistance() const { return mInternal->getFarClipDistance(); }

		/** @copydoc Camera::setAspectRatio */
		virtual void setAspectRatio(float ratio) { mInternal->setAspectRatio(ratio); }

		/** @copydoc Camera::getAspectRatio */
		virtual float getAspectRatio() const { return mInternal->getAspectRatio(); }

		/** @copydoc Camera::setFrustumExtents */
		virtual void setFrustumExtents(float left, float right, float top, float bottom)
		{
			mInternal->setFrustumExtents(left, right, top, bottom);
		}

		/** @copydoc Camera::resetFrustumExtents */
		virtual void resetFrustumExtents() { mInternal->resetFrustumExtents(); }

		/** @copydoc Camera::getFrustumExtents */
		virtual void getFrustumExtents(float& outleft, float& outright, float& outtop, float& outbottom) const
		{
			mInternal->getFrustumExtents(outleft, outright, outtop, outbottom);
		}

		/** @copydoc Camera::getProjectionMatrixRS */
		virtual const Matrix4& getProjectionMatrixRS() const { return mInternal->getProjectionMatrixRS(); }

		/** @copydoc Camera::getProjectionMatrix */
		virtual const Matrix4& getProjectionMatrix() const { return mInternal->getProjectionMatrix(); }

		/** @copydoc Camera::getViewMatrix */
		virtual const Matrix4& getViewMatrix() const { updateView(); return mInternal->getViewMatrix(); }

		/** @copydoc Camera::setCustomViewMatrix */
		virtual void setCustomViewMatrix(bool enable, const Matrix4& viewMatrix = Matrix4::IDENTITY)
		{
			mInternal->setCustomViewMatrix(enable, viewMatrix);
		}

		/** @copydoc Camera::isCustomViewMatrixEnabled */
		virtual bool isCustomViewMatrixEnabled() const { return mInternal->isCustomViewMatrixEnabled(); }
		
		/** @copydoc Camera::setCustomProjectionMatrix */
		virtual void setCustomProjectionMatrix(bool enable, const Matrix4& projectionMatrix = Matrix4::IDENTITY)
		{
			mInternal->setCustomProjectionMatrix(enable, projectionMatrix);
		}

		/** @copydoc Camera::isCustomProjectionMatrixEnabled */
		virtual bool isCustomProjectionMatrixEnabled() const { return mInternal->isCustomProjectionMatrixEnabled(); }

		/** @copydoc Camera::getFrustum */
		virtual const ConvexVolume& getFrustum() const { return mInternal->getFrustum(); }

		/** @copydoc Camera::getWorldFrustum */
        virtual ConvexVolume getWorldFrustum() const;

		/** @copydoc Camera::getBoundingBox */
		const AABox& getBoundingBox() const { return mInternal->getBoundingBox(); }

		/** @copydoc Camera::setProjectionType */
		virtual void setProjectionType(ProjectionType pt) { mInternal->setProjectionType(pt); }

		/** @copydoc Camera::getProjectionType */
		virtual ProjectionType getProjectionType() const { return mInternal->getProjectionType(); }

		/** @copydoc Camera::setOrthoWindow */
		virtual void setOrthoWindow(float w, float h) { mInternal->setOrthoWindow(w, h); }

		/** @copydoc Camera::setOrthoWindowHeight */
		virtual void setOrthoWindowHeight(float h) { mInternal->setOrthoWindowHeight(h); }

		/** @copydoc Camera::setOrthoWindowWidth */
		virtual void setOrthoWindowWidth(float w) { mInternal->setOrthoWindowWidth(w); }

		/** @copydoc Camera::getOrthoWindowHeight */
		virtual float getOrthoWindowHeight() const { return mInternal->getOrthoWindowHeight(); }

		/** @copydoc Camera::getOrthoWindowWidth */
		virtual float getOrthoWindowWidth() const { return mInternal->getOrthoWindowWidth(); }

		/** @copydoc Camera::getPriority */
		INT32 getPriority() const { return mInternal->getPriority(); }

		/** @copydoc Camera::setPriority */
		void setPriority(INT32 priority) { mInternal->setPriority(priority); }

		/** @copydoc Camera::getLayers */
		UINT64 getLayers() const { return mInternal->getLayers(); }

		/** @copydoc Camera::setLayers */
		void setLayers(UINT64 layers) { mInternal->setLayers(layers); }

		/** @copydoc Camera::getMSAACount */
		UINT32 getMSAACount() const { return mInternal->getMSAACount(); }

		/** @copydoc Camera::setMSAACount */
		void setMSAACount(UINT32 count) { mInternal->setMSAACount(count); }

		/** Returns settings that are used for controling post-process operations like tonemapping. */
		const SPtr<PostProcessSettings>& getPostProcessSettings() const { return mInternal->getPostProcessSettings(); }

		/** Sets settings that are used for controling post-process operations like tonemapping. */
		void setPostProcessSettings(const SPtr<PostProcessSettings>& settings) { mInternal->setPostProcessSettings(settings); }

		/** @copydoc Camera::getFlags */
		CameraFlags getFlags() const { return mInternal->getFlags(); }

		/** @copydoc Camera::setFlag */
		void setFlag(const CameraFlag& flag, bool enable) { mInternal->setFlag(flag, enable); }

		/** @copydoc Camera::worldToScreenPoint */
		Vector2I worldToScreenPoint(const Vector3& worldPoint) const { updateView(); return mInternal->worldToScreenPoint(worldPoint); }

		/** @copydoc Camera::worldToNdcPoint */
		Vector2 worldToNdcPoint(const Vector3& worldPoint) const { updateView(); return mInternal->worldToNdcPoint(worldPoint); }

		/** @copydoc Camera::worldToViewPoint */
		Vector3 worldToViewPoint(const Vector3& worldPoint) const { updateView(); return mInternal->worldToViewPoint(worldPoint); }

		/** @copydoc Camera::screenToWorldPoint */
		Vector3 screenToWorldPoint(const Vector2I& screenPoint, float depth = 0.5f) const { updateView(); return mInternal->screenToWorldPoint(screenPoint, depth); }

		/** @copydoc Camera::screenToViewPoint */
		Vector3 screenToViewPoint(const Vector2I& screenPoint, float depth = 0.5f) const { return mInternal->screenToViewPoint(screenPoint, depth); }

		/** @copydoc Camera::screenToNdcPoint */
		Vector2 screenToNdcPoint(const Vector2I& screenPoint) const { return mInternal->screenToNdcPoint(screenPoint); }

		/** @copydoc Camera::viewToWorldPoint */
		Vector3 viewToWorldPoint(const Vector3& viewPoint) const { updateView(); return mInternal->viewToWorldPoint(viewPoint); }

		/** @copydoc Camera::viewToScreenPoint */
		Vector2I viewToScreenPoint(const Vector3& viewPoint) const { return mInternal->viewToScreenPoint(viewPoint); }

		/** @copydoc Camera::viewToNdcPoint */
		Vector2 viewToNdcPoint(const Vector3& viewPoint) const { return mInternal->viewToNdcPoint(viewPoint); }

		/** @copydoc Camera::ndcToWorldPoint */
		Vector3 ndcToWorldPoint(const Vector2& ndcPoint, float depth = 0.5f) const { updateView(); return mInternal->ndcToWorldPoint(ndcPoint, depth); }

		/** @copydoc Camera::ndcToViewPoint */
		Vector3 ndcToViewPoint(const Vector2& ndcPoint, float depth = 0.5f) const { return mInternal->ndcToViewPoint(ndcPoint, depth); }

		/** @copydoc Camera::ndcToScreenPoint */
		Vector2I ndcToScreenPoint(const Vector2& ndcPoint) const { return mInternal->ndcToScreenPoint(ndcPoint); }

		/** @copydoc Camera::screenPointToRay */
		Ray screenPointToRay(const Vector2I& screenPoint) const { updateView(); return mInternal->screenPointToRay(screenPoint); }

		/** @copydoc Camera::projectPoint */
		Vector3 projectPoint(const Vector3& point) const { return mInternal->projectPoint(point); }

		/** @copydoc Camera::unprojectPoint */
		Vector3 unprojectPoint(const Vector3& point) const { return mInternal->unprojectPoint(point); }

		/** @copydoc Camera::isMain */	
		bool isMain() const { return mInternal->isMain(); }

		/** @copydoc Camera::setMain */	
		void setMain(bool main);

		/** @name Internal 
		 *  @{
		 */

		/** Returns the internal camera that is used for majority of operations by this component. */
		SPtr<Camera> _getCamera() const { updateView(); return mInternal; }

		/** @} */

    protected:
		/** Checks if the world transform of the camera changed, and if needed updates the view matrix. */
		void updateView() const;

		mutable SPtr<Camera> mInternal;

		// Only valid during construction
		SPtr<RenderTarget> mTarget;
		float mLeft;
		float mTop;
		float mWidth;
		float mHeight;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onInitialized */
		void onInitialized() override;

		/** @copydoc Component::onDestroyed */
		void onDestroyed() override;

	public:
		/** @copydoc Component::update */
		void update() override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CCameraRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CCamera();// Serialization only
     };

	 /** @} */
}