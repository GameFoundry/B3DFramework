//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIManager.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUINavGroup.h"
#include "Mesh/BsMesh.h"
#include "Components/BsCCamera.h"
#include "RenderAPI/BsViewport.h"
#include "Scene/BsSceneObject.h"
#include "Resources/BsBuiltinResources.h"
#include "RenderAPI/BsRenderTexture.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

GUIWidget::GUIWidget(const SPtr<Camera>& camera)
	: mCamera(camera), mBatches(this)
{
	Construct(camera);
}

GUIWidget::GUIWidget(const HCamera& camera)
	: mCamera(camera->GetCameraInternal()), mBatches(this)
{
	Construct(mCamera);
}

void GUIWidget::Construct(const SPtr<Camera>& camera)
{
	mStyleSheetCascade = GetBuiltinResources().GetDefaultGUIStyleSheetCascade();

	if(mCamera != nullptr)
	{
		SPtr<RenderTarget> target = mCamera->GetViewport()->GetTarget();

		if(target != nullptr)
			mCachedRTId = target->GetInternalId();
	}

	mDefaultNavGroup = GUINavGroup::Create();

	GUIManager::Instance().RegisterWidget(this);

	mPanel = GUIPanel::Create();
	mPanel->ChangeParentWidget(this);
	UpdateRootPanel();
}

GUIWidget::~GUIWidget()
{
	DestroyInternal();
}

SPtr<GUIWidget> GUIWidget::Create(const SPtr<Camera>& camera)
{
	return B3DMakeSharedFromExisting(new(B3DAllocate<GUIWidget>()) GUIWidget(camera));
}

SPtr<GUIWidget> GUIWidget::Create(const HCamera& camera)
{
	return B3DMakeSharedFromExisting(new(B3DAllocate<GUIWidget>()) GUIWidget(camera));
}

void GUIWidget::DestroyInternal()
{
	if(mPanel != nullptr)
	{
		mPanel->Destroy();
		mPanel = nullptr;
	}

	if(mCamera != nullptr)
	{
		GUIManager::Instance().UnregisterWidget(this);
		mCamera = nullptr;
	}

	mElements.clear();
	mDirtyContents.clear();
	mDirtyLayoutOrAbsoluteCoordinates.clear();
}

void GUIWidget::SetDepth(u8 depth)
{
	mDepth = depth;
	mWidgetIsDirty = true;

	UpdateRootPanel();
}

Viewport* GUIWidget::GetTarget() const
{
	if(mCamera != nullptr)
		return mCamera->GetViewport().get();

	return nullptr;
}

void GUIWidget::UpdateTransformInternal(const HSceneObject& parent)
{
	// If the widgets parent scene object moved, we need to mark it as dirty
	// as the GUIManager batching relies on object positions, so it needs to be updated.
	const float diffEpsilon = 0.0001f;

	const Transform& tfrm = parent->GetTransform();
	Vector3 position = tfrm.GetPosition();
	Quaternion rotation = tfrm.GetRotation();
	Vector3 scale = tfrm.GetScale();

	if(!mWidgetIsDirty)
	{
		if(!Math::ApproxEquals(mPosition, position, diffEpsilon))
			mWidgetIsDirty = true;
		else
		{
			if(!Math::ApproxEquals(mRotation, rotation, diffEpsilon))
				mWidgetIsDirty = true;
			else
			{
				if(Math::ApproxEquals(mScale, scale))
					mWidgetIsDirty = true;
			}
		}
	}

	mPosition = position;
	mRotation = rotation;
	mScale = scale;
	mTransform = parent->GetWorldMatrix();
}

void GUIWidget::UpdateRTInternal()
{
	SPtr<RenderTarget> rt;
	u64 newRTId = 0;
	if(mCamera != nullptr)
	{
		rt = mCamera->GetViewport()->GetTarget();
		if(rt != nullptr)
			newRTId = rt->GetInternalId();
	}

	if(mCachedRTId != newRTId)
	{
		mCachedRTId = newRTId;
		UpdateRootPanel();
	}
}

void GUIWidget::UpdateLayout()
{
	// Check if render target size changed and update if needed
	// Note: Purposely not relying to the RenderTarget::onResized callback, as it will trigger /before/ Input events.
	// These events might trigger a resize, meaning the size would be delayed one frame, resulting in a visual artifact
	// where the GUI doesn't match the target size.
	Viewport* target = GetTarget();
	if(target != nullptr)
	{
		Rect2I area = target->GetPixelArea();
		u32 width = area.Width;
		u32 height = area.Height;

		const Size2UI& panelSize = mPanel->GetLayoutData().Size;
		if(panelSize.Width != width || panelSize.Height != height)
		{
			UpdateRootPanel();
			OnOwnerTargetResized();
		}
	}

	// Perform layout updates
	for(auto it = mDirtyLayoutOrAbsoluteCoordinates.begin(); it != mDirtyLayoutOrAbsoluteCoordinates.end(); )
	{
		GUIElement* const element = *it;

		if(element != nullptr && !element->IsLayoutDirty()) // If not dirty, we likely already updated layout for one of its parents
		{
			it = mDirtyLayoutOrAbsoluteCoordinates.erase(it);
			continue;
		}

		// Check if there are any parents that are also dirty, in which case update those
		GUIElement* rootDirtyElement = element;
		if(element != nullptr)
		{
			GUIElement* currentElement = element;
			while(true)
			{
				GUIElement* const parent = currentElement->GetParent();
				if(parent == nullptr)
					break;

				currentElement = parent;

				if(currentElement->IsLayoutDirty())
					rootDirtyElement = currentElement;
			}
		}
		else
			rootDirtyElement = mPanel;

		B3D_ASSERT(rootDirtyElement != nullptr || rootDirtyElement == mPanel);

		if(rootDirtyElement != nullptr)
			UpdateLayout(rootDirtyElement);
		else // Must be root panel
			UpdateLayout(mPanel);

		it = mDirtyLayoutOrAbsoluteCoordinates.erase(it);
		
	}

	// Do another pass or remaining elements, updating absolute coordinates
	for(auto it = mDirtyLayoutOrAbsoluteCoordinates.begin(); it != mDirtyLayoutOrAbsoluteCoordinates.end(); )
	{
		GUIElement* const element = *it;

		if(!element->AreAbsoluteCoordinatesDirty()) // If not dirty, we likely already updated absolute coordinates for one of its parents
		{
			it = mDirtyLayoutOrAbsoluteCoordinates.erase(it);
			continue;
		}

		// Check if there are any parents that are also dirty, in which case update those
		GUIElement* rootDirtyElement = element;
		if(element != nullptr)
		{
			GUIElement* currentElement = element;
			while(true)
			{
				GUIElement* const parent = currentElement->GetParent();
				if(parent == nullptr)
					break;

				currentElement = parent;

				if(currentElement->AreAbsoluteCoordinatesDirty())
					rootDirtyElement = currentElement;
			}
		}
		else
			rootDirtyElement = mPanel;

		B3D_ASSERT(rootDirtyElement != nullptr || rootDirtyElement == mPanel);
		rootDirtyElement->UpdateAbsoluteCoordinatesForChildren();

		// Mark meshes as dirty (clip bounds could have changed) & clear dirty flags
		auto fnMarkMeshAsDirtyAndClearDirtyFlags = [this](GUIElement* element, auto&& fnMarkMeshAsDirtyAndClearDirtyFlags) -> void
		{
			MarkMeshDirty(element);
			element->MarkAsClean();

			const TInlineArray<GUIElement*, 4>& visibleChildren = element->GetVisibleChildren();
			for(const auto child : visibleChildren)
				fnMarkMeshAsDirtyAndClearDirtyFlags(child, fnMarkMeshAsDirtyAndClearDirtyFlags);
		};

		fnMarkMeshAsDirtyAndClearDirtyFlags(rootDirtyElement, fnMarkMeshAsDirtyAndClearDirtyFlags);
		it = mDirtyLayoutOrAbsoluteCoordinates.erase(it);
	}
}

void GUIWidget::UpdateLayout(GUIElement* element)
{
	GUIElement* const parent = element->GetParent();
	const bool isPanelOptimized = parent != nullptr && parent->GetType() == GUIElement::Type::Panel;

	GUIElement* const updateParent = isPanelOptimized ? parent : element;

	// For GUIPanel we can do an optimization and update only the element in question instead
	// of all the children
	if(isPanelOptimized)
	{
		GUIPanel* panel = static_cast<GUIPanel*>(updateParent);

		GUIElement* dirtyElement = element;
		dirtyElement->UpdateOptimalLayoutSizes();

		GUIConstrainedSize elementSizeRange = panel->GetChildElementSizeRange(dirtyElement);
		const Rect2I relativeElementArea = panel->CalculateRelativeElementArea(panel->GetLayoutData().Size, dirtyElement, elementSizeRange);

		const GUILayoutData parentPanelLayoutData = panel->GetLayoutData();
		GUILayoutData childLayoutData = parentPanelLayoutData;
		panel->UpdateDepthRangeInternal(childLayoutData);

		childLayoutData.RelativePosition = GUILogicalPoint(relativeElementArea.X, relativeElementArea.Y);
		childLayoutData.Size = Size2UI(relativeElementArea.Width, relativeElementArea.Height);

		dirtyElement->SetLayoutData(childLayoutData);
		dirtyElement->UpdateLayoutForChildren();
		dirtyElement->UpdateAbsoluteCoordinates(panel->GetAbsolutePosition(), mDPIScale, panel->GetAbsoluteClippedArea());
	}
	else
	{
		updateParent->UpdateLayout();
	}

	// Mark dirty contents & clear dirty flags
	auto fnMarkContentsAsDirtyAndClearDirtyFlags = [this](GUIElement* element, auto&& fnMarkContentsAsDirtyAndClearDirtyFlags) -> void
	{
		MarkContentDirty(element);
		element->MarkAsClean();

		const TInlineArray<GUIElement*, 4>& visibleChildren = element->GetVisibleChildren();
		for(const auto child : visibleChildren)
			fnMarkContentsAsDirtyAndClearDirtyFlags(child, fnMarkContentsAsDirtyAndClearDirtyFlags);
	};

	fnMarkContentsAsDirtyAndClearDirtyFlags(element, fnMarkContentsAsDirtyAndClearDirtyFlags);
}

void GUIWidget::RegisterElement(GUIElement* guiElement)
{
	B3D_ASSERT(guiElement != nullptr && !guiElement->IsPendingDestroy());

	if(GUIRenderable* const renderable = B3DRTTICast<GUIRenderable>(guiElement))
	{
		mElements.push_back(renderable);
		mWidgetIsDirty = true;

		if(guiElement->IsLayoutDirty() || guiElement->AreAbsoluteCoordinatesDirty())
			mDirtyLayoutOrAbsoluteCoordinates.insert(guiElement);

		if(!guiElement->IsHiddenOrCulled())
		{
			mBatches.Add(renderable);
			mBatches.MarkContentDirty(renderable);
		}
	}
}

void GUIWidget::UnregisterElement(GUIElement* guiElement)
{
	B3D_ASSERT(guiElement != nullptr);

	// TODO - mElements should be a hash map otherwise this will be a bottleneck with a lot of elements
	auto iterFind = std::find(begin(mElements), end(mElements), guiElement);

	if(iterFind != mElements.end())
	{
		mElements.erase(iterFind);
		mWidgetIsDirty = true;
	}

	if(GUIRenderable* const renderable = B3DRTTICast<GUIRenderable>(guiElement))
	{
		mDirtyContents.erase(renderable);
		mDirtyLayoutOrAbsoluteCoordinates.erase(renderable);
		mBatches.Remove(renderable);
	}
}

void GUIWidget::NotifyElementVisibilityChanged(GUIElement* guiElement, bool isVisible)
{
	if(GUIRenderable* const renderable = B3DRTTICast<GUIRenderable>(guiElement))
	{
		if(isVisible)
			mBatches.Add(renderable);
		else
			mBatches.Remove(renderable);
	}
}

void GUIWidget::MarkMeshDirty(GUIElement* elem)
{
	mWidgetIsDirty = true;

	if(GUIRenderable *const renderable = B3DRTTICast<GUIRenderable>(elem))
		mBatches.MarkMeshDirty(renderable);
}

void GUIWidget::MarkContentDirty(GUIElement* elem)
{
	if(GUIRenderable *const renderable = B3DRTTICast<GUIRenderable>(elem))
	{
		if(renderable->IsHiddenOrCulled())
			return;

		mDirtyContents.insert(renderable);
		mBatches.MarkContentDirty(renderable);
	}
}

void GUIWidget::SetStyleSheetCascade(const SPtr<const GUIStyleSheetCascade>& styleSheetCascade)
{
	if(!B3D_ENSURE(styleSheetCascade != nullptr))
		return;

	mStyleSheetCascade = styleSheetCascade;

	for(auto& element : mElements)
		element->RefreshStyle();
}

const GUIStyleSheetCascade& GUIWidget::GetStyleSheetCascade() const
{
	if(mStyleSheetCascade != nullptr)
		return *mStyleSheetCascade;

	return GUIStyleSheetCascade::kEmpty;
}

void GUIWidget::SetCamera(const SPtr<Camera>& camera)
{
	SPtr<Camera> newCamera = camera;
	if(newCamera != nullptr)
	{
		if(newCamera->GetViewport()->GetTarget() == nullptr)
			newCamera = nullptr;
	}

	if(mCamera == newCamera)
		return;

	GUIManager::Instance().UnregisterWidget(this);
	mCamera = newCamera;
	GUIManager::Instance().RegisterWidget(this);

	UpdateRootPanel();
}

void GUIWidget::SetIsActive(bool active)
{
	mIsActive = active;
}

GUIDrawGroupRenderDataUpdate GUIWidget::RebuildDirtyRenderData()
{
	if(!mIsActive)
		return GUIDrawGroupRenderDataUpdate();

	const bool dirty = mWidgetIsDirty || !mDirtyContents.empty();

	if(dirty)
	{
		// Update render contents recursively because updates can cause child GUI elements to become dirty
		while(!mDirtyContents.empty())
		{
			mDirtyContentsTemp.swap(mDirtyContents);

			for(auto& dirtyElement : mDirtyContentsTemp)
				dirtyElement->UpdateRenderElements();

			mDirtyContentsTemp.clear();
		}

		UpdateBounds();
	}

	mWidgetIsDirty = false;
	return mBatches.RebuildDirty(dirty);
}

bool GUIWidget::InBounds(const Vector2I& position) const
{
	Viewport* target = GetTarget();
	if(target == nullptr)
		return false;

	// Technically GUI widget bounds can be larger than the viewport, so make sure we clip to viewport first
	if(!target->GetPixelArea().Contains(position))
		return false;

	Vector3 vecPos((float)position.X, (float)position.Y, 0.0f);
	vecPos = mTransform.Inverse().MultiplyAffine(vecPos);

	Vector2I localPos(Math::RoundToI32(vecPos.X), Math::RoundToI32(vecPos.Y));
	return mBounds.Contains(localPos);
}

void GUIWidget::UpdateBounds() const
{
	if(!mElements.empty())
		mBounds = mElements[0]->GetAbsoluteClippedArea();

	for(auto& elem : mElements)
	{
		Rect2I elemBounds = elem->GetAbsoluteClippedArea();
		mBounds.Encapsulate(elemBounds);
	}
}

void GUIWidget::OwnerWindowFocusChanged()
{
	OnOwnerWindowFocusChanged();
}

void GUIWidget::UpdateRootPanel()
{
	Viewport* target = GetTarget();
	if(target == nullptr)
		return;

	Rect2I area = target->GetPixelArea();
	u32 width = area.Width;
	u32 height = area.Height;

	GUILayoutData layoutData;
	layoutData.RelativePosition = GUILogicalPoint::kZero;
	layoutData.Size = Size2UI(width, height);
	layoutData.SetWidgetDepth(mDepth);

	mPanel->SetWidth(width);
	mPanel->SetHeight(height);

	mPanel->SetLayoutData(layoutData);
	mPanel->ResetAbsoluteBounds(mDPIScale);
	mPanel->MarkLayoutAsDirty();
}
