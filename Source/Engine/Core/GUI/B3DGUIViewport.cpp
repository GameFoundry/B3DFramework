//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIViewport.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "Components/B3DCamera.h"
#include "RenderAPI/B3DViewport.h"
#include "RenderAPI/B3DRenderTarget.h"

using namespace b3d;

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

GUILogicalSize GUIViewport::CalculateUnconstrainedOptimalSize() const
{
	return GUILogicalSize(0, 0);
}

void GUIViewport::FillBuffer(
	u8* vertices,
	u32* indices,
	u32 vertexOffset,
	u32 indexOffset,
	const Vector2I& offset,
	u32 maxVertexCount,
	u32 maxIndexCount,
	u32 renderElementIdx) const
{
}

void GUIViewport::UpdateRenderElements()
{
	// TODO - This doesn't get called if element mesh is dirty!!! and I need to update the viewport when offset changes (in which case mesh is marked as dirty)
	float currentAspect = (float)mAbsoluteSize.Width / (float)mAbsoluteSize.Height;
	Radian currentFOV = 2.0f * Math::Atan(Math::Tan(mVerticalFOV * 0.5f) * currentAspect);

	mCamera->SetHorizontalFOV(currentFOV);

	SPtr<Viewport> viewport = mCamera->GetViewport();
	SPtr<RenderTarget> renderTarget = viewport->GetTarget();
	const RenderTargetProperties& rtProps = renderTarget->GetProperties();

	float x = (float)mAbsolutePosition.X / (float)rtProps.Width;
	float y = (float)mAbsolutePosition.Y / (float)rtProps.Height;
	float width = (float)mAbsoluteSize.Width / (float)rtProps.Width;
	float height = (float)mAbsoluteSize.Height / (float)rtProps.Height;

	viewport->SetArea(Area2(x, y, width, height));
}

void GUIViewport::ChangeParentWidget(GUIWidget* widget)
{
	GUIInteractable::ChangeParentWidget(widget);

	if(widget != nullptr)
	{
		SPtr<RenderTarget> guiRenderTarget = widget->GetTarget()->GetTarget();
		SPtr<RenderTarget> cameraRenderTarget = mCamera->GetViewport()->GetTarget();

		B3D_ENSURE_LOG(guiRenderTarget == cameraRenderTarget, "Camera provided to GUIViewport must use the same render target as the GUIWidget this element is located on.");
	}
}
