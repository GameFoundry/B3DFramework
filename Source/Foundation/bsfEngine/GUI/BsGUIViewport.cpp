//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIViewport.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUIDimensions.h"
#include "Components/BsCCamera.h"
#include "RenderAPI/BsViewport.h"
#include "RenderAPI/BsRenderTarget.h"
#include "Error/BsException.h"

namespace bs
{
	const String& GUIViewport::getGUITypeName()
	{
		static String name = "Viewport";
		return name;
	}

	GUIViewport::GUIViewport(const String& styleName, const HCamera& camera,
		float aspectRatio, Degree fieldOfView, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions), mCamera(camera), mAspectRatio(aspectRatio),
		mFieldOfView(fieldOfView)
	{
		mVerticalFOV = 2.0f * Math::atan(Math::tan(mFieldOfView.valueRadians() * 0.5f) * (1.0f / mAspectRatio));
	}

	GUIViewport* GUIViewport::create(const HCamera& camera, float aspectRatio, Degree fieldOfView, const String& styleName)
	{
		return new (bs_alloc<GUIViewport>()) GUIViewport(getStyleName<GUIViewport>(styleName), camera, aspectRatio, fieldOfView, GUIDimensions::create());
	}

	GUIViewport* GUIViewport::create(const GUIOptions& options, const HCamera& camera,
		float aspectRatio, Degree fieldOfView, const String& styleName)
	{
		return new (bs_alloc<GUIViewport>()) GUIViewport(getStyleName<GUIViewport>(styleName), camera, aspectRatio, fieldOfView, GUIDimensions::create(options));
	}

	void GUIViewport::updateClippedBounds()
	{
		mClippedBounds = mLayoutData.area;
		mClippedBounds.clip(mLayoutData.clipRect);
	}

	Vector2I GUIViewport::_getOptimalSize() const
	{
		return Vector2I(0, 0);
	}

	void GUIViewport::_fillBuffer(
		UINT8* vertices,
		UINT32* indices,
		UINT32 vertexOffset,
		UINT32 indexOffset,
		const Vector2I& offset,
		UINT32 maxNumVerts,
		UINT32 maxNumIndices,
		UINT32 renderElementIdx) const
	{

	}

	void GUIViewport::updateRenderElementsInternal()
	{
		// TODO - This doesn't get called if element mesh is dirty!!! and I need to update the viewport when offset changes (in which case mesh is marked as dirty)
		float currentAspect = mLayoutData.area.width / (float)mLayoutData.area.height;
		Radian currentFOV = 2.0f * Math::atan(Math::tan(mVerticalFOV * 0.5f) * currentAspect);

		mCamera->setHorzFOV(currentFOV);

		SPtr<Viewport> viewport = mCamera->getViewport();
		SPtr<RenderTarget> renderTarget = viewport->getTarget();
		const RenderTargetProperties& rtProps = renderTarget->getProperties();

		float x = mLayoutData.area.x / (float)rtProps.width;
		float y = mLayoutData.area.y / (float)rtProps.height;
		float width = mLayoutData.area.width / (float)rtProps.width;
		float height = mLayoutData.area.height / (float)rtProps.height;

		viewport->setArea(Rect2(x, y, width, height));
	}

	void GUIViewport::_changeParentWidget(GUIWidget* widget)
	{
		GUIElement::_changeParentWidget(widget);

		if(widget != nullptr)
		{
			SPtr<RenderTarget> guiRenderTarget = widget->getTarget()->getTarget();
			SPtr<RenderTarget> cameraRenderTarget = mCamera->getViewport()->getTarget();

			if(guiRenderTarget != cameraRenderTarget)
				BS_EXCEPT(InvalidParametersException, "Camera provided to GUIViewport must use the same render target as the GUIWidget this element is located on.")
		}
	}
}
