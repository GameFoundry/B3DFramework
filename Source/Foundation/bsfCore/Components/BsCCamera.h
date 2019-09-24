//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsCamera.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Camera
	 * 			
	 * Wraps a Camera as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering,n:Camera) CCamera : public Component
	{
	public:
		CCamera(const HSceneObject& parent);
		virtual ~CCamera() = default;

		/** @copydoc Camera::setFlags */
		BS_SCRIPT_EXPORT(n:Flags,pr:setter)
		void setFlags(CameraFlags flags) { mInternal->setFlags(flags); }

		/** @copydoc Camera::getFlags */
		BS_SCRIPT_EXPORT(n:Flags,pr:getter)
		CameraFlags getFlags() const { return mInternal->getFlags(); }
		
		/** @copydoc Camera::getViewport */
		BS_SCRIPT_EXPORT(n:Viewport,pr:getter)
		SPtr<Viewport> getViewport() const { return mInternal->getViewport(); }

		/** @copydoc Camera::setHorzFOV */
		BS_SCRIPT_EXPORT(n:FieldOfView,pr:setter,range:[1,360],slider,order:-1)
		virtual void setHorzFOV(const Radian& fovy) { mInternal->setHorzFOV(fovy); }

		/** @copydoc Camera::getHorzFOV */
		BS_SCRIPT_EXPORT(n:FieldOfView,pr:getter,range:[1,360],slider,order:-1)
		virtual const Radian& getHorzFOV() const { return mInternal->getHorzFOV(); }

		/** @copydoc Camera::setNearClipDistance */
		BS_SCRIPT_EXPORT(n:NearClipPlane,pr:setter)
		virtual void setNearClipDistance(float nearDist) { mInternal->setNearClipDistance(nearDist); }

		/** @copydoc Camera::getNearClipDistance */
		BS_SCRIPT_EXPORT(n:NearClipPlane,pr:getter)
		virtual float getNearClipDistance() const { return mInternal->getNearClipDistance(); }

		/** @copydoc Camera::setFarClipDistance */
		BS_SCRIPT_EXPORT(n:FarClipPlane,pr:setter)
		virtual void setFarClipDistance(float farDist) { mInternal->setFarClipDistance(farDist); }

		/** @copydoc Camera::getFarClipDistance */
		BS_SCRIPT_EXPORT(n:FarClipPlane,pr:getter)
		virtual float getFarClipDistance() const { return mInternal->getFarClipDistance(); }

		/** @copydoc Camera::setAspectRatio */
		BS_SCRIPT_EXPORT(n:AspectRatio,pr:setter)
		virtual void setAspectRatio(float ratio) { mInternal->setAspectRatio(ratio); }

		/** @copydoc Camera::getAspectRatio */
		BS_SCRIPT_EXPORT(n:AspectRatio,pr:getter)
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
		BS_SCRIPT_EXPORT(n:ProjMatrix,pr:getter)
		virtual const Matrix4& getProjectionMatrixRS() const { return mInternal->getProjectionMatrixRS(); }

		/** @copydoc Camera::getProjectionMatrix */
		virtual const Matrix4& getProjectionMatrix() const { return mInternal->getProjectionMatrix(); }

		/** @copydoc Camera::getViewMatrix */
		BS_SCRIPT_EXPORT(n:ViewMatrix,pr:getter)
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
		BS_SCRIPT_EXPORT(n:ProjectionType,pr:setter,order:-2)
		virtual void setProjectionType(ProjectionType pt) { mInternal->setProjectionType(pt); }

		/** @copydoc Camera::getProjectionType */
		BS_SCRIPT_EXPORT(n:ProjectionType,pr:getter,order:-2)
		virtual ProjectionType getProjectionType() const { return mInternal->getProjectionType(); }

		/** @copydoc Camera::setOrthoWindow */
		virtual void setOrthoWindow(float w, float h) { mInternal->setOrthoWindow(w, h); }

		/** @copydoc Camera::setOrthoWindowHeight */
		BS_SCRIPT_EXPORT(n:OrthoHeight,pr:setter,order:-1)
		virtual void setOrthoWindowHeight(float h) { mInternal->setOrthoWindowHeight(h); }

		/** @copydoc Camera::getOrthoWindowHeight */
		BS_SCRIPT_EXPORT(n:OrthoHeight,pr:getter,order:-1)
		virtual float getOrthoWindowHeight() const { return mInternal->getOrthoWindowHeight(); }

		/** @copydoc Camera::setOrthoWindowWidth */
		BS_SCRIPT_EXPORT(n:OrthoWidth,pr:setter,hide)
		virtual void setOrthoWindowWidth(float w) { mInternal->setOrthoWindowWidth(w); }

		/** @copydoc Camera::getOrthoWindowWidth */
		BS_SCRIPT_EXPORT(n:OrthoWidth,pr:getter,hide)
		virtual float getOrthoWindowWidth() const { return mInternal->getOrthoWindowWidth(); }

		/** @copydoc Camera::setPriority */
		BS_SCRIPT_EXPORT(n:Priority,pr:setter)
		void setPriority(INT32 priority) { mInternal->setPriority(priority); }

		/** @copydoc Camera::getPriority */
		BS_SCRIPT_EXPORT(n:Priority,pr:getter)
		INT32 getPriority() const { return mInternal->getPriority(); }

		/** @copydoc Camera::setLayers */
		BS_SCRIPT_EXPORT(n:Layers,pr:setter,layerMask)
		void setLayers(UINT64 layers) { mInternal->setLayers(layers); }

		/** @copydoc Camera::getLayers */
		BS_SCRIPT_EXPORT(n:Layers,pr:getter,layerMask)
		UINT64 getLayers() const { return mInternal->getLayers(); }

		/** @copydoc Camera::setMSAACount */
		BS_SCRIPT_EXPORT(n:SampleCount,pr:setter)
		void setMSAACount(UINT32 count) { mInternal->setMSAACount(count); }

		/** @copydoc Camera::getMSAACount */
		BS_SCRIPT_EXPORT(n:SampleCount,pr:getter)
		UINT32 getMSAACount() const { return mInternal->getMSAACount(); }

		/** @copydoc Camera::setRenderSettings() */
		BS_SCRIPT_EXPORT(n:RenderSettings,pr:setter,applyOnDirty)
		void setRenderSettings(const SPtr<RenderSettings>& settings) { mInternal->setRenderSettings(settings); }

		/** @copydoc Camera::getRenderSettings() */
		BS_SCRIPT_EXPORT(n:RenderSettings,pr:getter,applyOnDirty)
		const SPtr<RenderSettings>& getRenderSettings() const { return mInternal->getRenderSettings(); }

		/** @copydoc Camera::notifyNeedsRedraw() */
		BS_SCRIPT_EXPORT()
		void notifyNeedsRedraw() { mInternal->notifyNeedsRedraw(); }
		
		/** @copydoc Camera::worldToScreenPoint */
		BS_SCRIPT_EXPORT()
		Vector2I worldToScreenPoint(const Vector3& worldPoint) const { updateView(); return mInternal->worldToScreenPoint(worldPoint); }

		/** @copydoc Camera::worldToNdcPoint */
		BS_SCRIPT_EXPORT()
		Vector2 worldToNdcPoint(const Vector3& worldPoint) const { updateView(); return mInternal->worldToNdcPoint(worldPoint); }

		/** @copydoc Camera::worldToViewPoint */
		BS_SCRIPT_EXPORT()
		Vector3 worldToViewPoint(const Vector3& worldPoint) const { updateView(); return mInternal->worldToViewPoint(worldPoint); }

		/** @copydoc Camera::screenToWorldPoint */
		BS_SCRIPT_EXPORT()
		Vector3 screenToWorldPoint(const Vector2I& screenPoint, float depth = 0.5f) const { updateView(); return mInternal->screenToWorldPoint(screenPoint, depth); }

		/** @copydoc Camera::screenToViewPoint */
		BS_SCRIPT_EXPORT()
		Vector3 screenToViewPoint(const Vector2I& screenPoint, float depth = 0.5f) const { return mInternal->screenToViewPoint(screenPoint, depth); }

		/** @copydoc Camera::screenToNdcPoint */
		BS_SCRIPT_EXPORT()
		Vector2 screenToNdcPoint(const Vector2I& screenPoint) const { return mInternal->screenToNdcPoint(screenPoint); }

		/** @copydoc Camera::viewToWorldPoint */
		BS_SCRIPT_EXPORT()
		Vector3 viewToWorldPoint(const Vector3& viewPoint) const { updateView(); return mInternal->viewToWorldPoint(viewPoint); }

		/** @copydoc Camera::viewToScreenPoint */
		BS_SCRIPT_EXPORT()
		Vector2I viewToScreenPoint(const Vector3& viewPoint) const { return mInternal->viewToScreenPoint(viewPoint); }

		/** @copydoc Camera::viewToNdcPoint */
		BS_SCRIPT_EXPORT()
		Vector2 viewToNdcPoint(const Vector3& viewPoint) const { return mInternal->viewToNdcPoint(viewPoint); }

		/** @copydoc Camera::ndcToWorldPoint */
		BS_SCRIPT_EXPORT()
		Vector3 ndcToWorldPoint(const Vector2& ndcPoint, float depth = 0.5f) const { updateView(); return mInternal->ndcToWorldPoint(ndcPoint, depth); }

		/** @copydoc Camera::ndcToViewPoint */
		BS_SCRIPT_EXPORT()
		Vector3 ndcToViewPoint(const Vector2& ndcPoint, float depth = 0.5f) const { return mInternal->ndcToViewPoint(ndcPoint, depth); }

		/** @copydoc Camera::ndcToScreenPoint */
		BS_SCRIPT_EXPORT()
		Vector2I ndcToScreenPoint(const Vector2& ndcPoint) const { return mInternal->ndcToScreenPoint(ndcPoint); }

		/** @copydoc Camera::screenPointToRay */
		BS_SCRIPT_EXPORT()
		Ray screenPointToRay(const Vector2I& screenPoint) const { updateView(); return mInternal->screenPointToRay(screenPoint); }

		/** @copydoc Camera::projectPoint */
		BS_SCRIPT_EXPORT()
		Vector3 projectPoint(const Vector3& point) const { return mInternal->projectPoint(point); }

		/** @copydoc Camera::unprojectPoint */
		BS_SCRIPT_EXPORT()
		Vector3 unprojectPoint(const Vector3& point) const { return mInternal->unprojectPoint(point); }

		/** @copydoc Camera::setMain */	
		BS_SCRIPT_EXPORT(n:Main,pr:setter)
		void setMain(bool main);

		/** @copydoc Camera::isMain */
		BS_SCRIPT_EXPORT(n:Main,pr:getter)
		bool isMain() const { return mInternal->isMain(); }

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

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::_instantiate */
		void _instantiate() override;

		/** @copydoc Component::onInitialized */
		void onInitialized() override;

		/** @copydoc Component::onDestroyed */
		void onDestroyed() override;

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
