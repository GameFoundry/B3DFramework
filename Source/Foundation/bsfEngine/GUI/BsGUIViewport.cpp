//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIViewport.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUISizeConstraints.h"
#include "Components/BsCCamera.h"
#include "RenderAPI/BsViewport.h"
#include "RenderAPI/BsRenderTarget.h"
#include "Error/BsException.h"

using namespace bs;

const String& GUIViewport::GetGuiTypeName()
{
	static String name = "Viewport";
	return name;
}

GUIViewport::GUIViewport(const String& styleName, const HCamera& camera, float aspectRatio, Degree fieldOfView, const GUISizeConstraints& dimensions)
	: GUIInteractable(styleName, dimensions), mCamera(camera), mAspectRatio(aspectRatio), mFieldOfView(fieldOfView)
{
	mVerticalFOV = 2.0f * Math::Atan(Math::Tan(mFieldOfView.GetValueInRadians() * 0.5f) * (1.0f / mAspectRatio));
}

GUIViewport* GUIViewport::Create(const HCamera& camera, float aspectRatio, Degree fieldOfView, const String& styleName)
{
	return new(B3DAllocate<GUIViewport>()) GUIViewport(GetStyleClass<GUIViewport>(styleName), camera, aspectRatio, fieldOfView, GUISizeConstraints::Create());
}

GUIViewport* GUIViewport::Create(const GUIOptions& options, const HCamera& camera, float aspectRatio, Degree fieldOfView, const String& styleName)
{
	return new(B3DAllocate<GUIViewport>()) GUIViewport(GetStyleClass<GUIViewport>(styleName), camera, aspectRatio, fieldOfView, GUISizeConstraints::Create(options));
}

Vector2I GUIViewport::CalculateUnconstrainedOptimalSize() const
{
	return Vector2I(0, 0);
}

void GUIViewport::FillBuffer(
	u8* vertices,
	u32* indices,
	u32 vertexOffset,
	u32 indexOffset,
	const Vector2I& offset,
	u32 maxNumVerts,
	u32 maxNumIndices,
	u32 renderElementIdx) const
{
}

void GUIViewport::UpdateRenderElements()
{
	// TODO - This doesn't get called if element mesh is dirty!!! and I need to update the viewport when offset changes (in which case mesh is marked as dirty)
	float currentAspect = mLayoutData.Size.Width / (float)mLayoutData.Size.Height;
	Radian currentFOV = 2.0f * Math::Atan(Math::Tan(mVerticalFOV * 0.5f) * currentAspect);

	mCamera->SetHorzFov(currentFOV);

	SPtr<Viewport> viewport = mCamera->GetViewport();
	SPtr<RenderTarget> renderTarget = viewport->GetTarget();
	const RenderTargetProperties& rtProps = renderTarget->GetProperties();

	float x = mAbsolutePosition.X / (float)rtProps.Width;
	float y = mAbsolutePosition.Y / (float)rtProps.Height;
	float width = mLayoutData.Size.Width / (float)rtProps.Width;
	float height = mLayoutData.Size.Height / (float)rtProps.Height;

	viewport->SetArea(Rect2(x, y, width, height));
}

void GUIViewport::ChangeParentWidget(GUIWidget* widget)
{
	GUIInteractable::ChangeParentWidget(widget);

	if(widget != nullptr)
	{
		SPtr<RenderTarget> guiRenderTarget = widget->GetTarget()->GetTarget();
		SPtr<RenderTarget> cameraRenderTarget = mCamera->GetViewport()->GetTarget();

		if(guiRenderTarget != cameraRenderTarget)
			B3D_EXCEPT(InvalidParametersException, "Camera provided to GUIViewport must use the same render target as the GUIWidget this element is located on.")
	}
}
