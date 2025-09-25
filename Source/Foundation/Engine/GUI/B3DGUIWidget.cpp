//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIWidget.h"

#include "B3DGUIManager.h"
#include "B3DGUINavGroup.h"
#include "B3DGUIPanel.h"
#include "B3DGUIRenderable.h"
#include "B3DGUIUtility.h"
#include "RTTI/B3DGUIWidgetRTTI.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DCamera.h"
#include "RenderAPI/B3DRenderTarget.h"
#include "RenderAPI/B3DViewport.h"
#include "Resources/B3DBuiltinResources.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

GUIWidget::GUIWidget(const HSceneObject& parent, const HCamera& camera)
	: Component(parent), mCamera(camera), mBatches(this)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	mNotifyFlags = TCF_Transform;
}

GUIWidget::GUIWidget()
	:GUIWidget(nullptr, nullptr)
{ }

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

void GUIWidget::SetDepth(u8 depth)
{
	mDepth = depth;
	mWidgetIsDirty = true;

	UpdateRootPanel();
}

bool GUIWidget::InBounds(const GUIPhysicalPoint& position) const
{
	Viewport* target = GetTarget();
	if(target == nullptr)
		return false;

	// Technically GUI widget bounds can be larger than the viewport, so make sure we clip to viewport first
	if(!target->GetPixelArea().Contains(position.To<i32>()))
		return false;

	const Matrix4& transform = SceneObject()->GetTransform().GetMatrix();

	Vector3 vecPos((float)position.X, (float)position.Y, 0.0f);
	vecPos = transform.Inverse().MultiplyAffine(vecPos);

	GUIPhysicalPoint localPos(Math::RoundToI32(vecPos.X), Math::RoundToI32(vecPos.Y));
	return mBounds.Contains(localPos);
}

Viewport* GUIWidget::GetTarget() const
{
	if(mCamera != nullptr)
		return mCamera->GetViewport().get();

	return nullptr;
}

void GUIWidget::SetDPIScale(float dpiScale)
{
	if(mDPIScale == dpiScale)
		return;

	mDPIScale = dpiScale;
	mWidgetIsDirty = true;

	UpdateRootPanel();
}

void GUIWidget::SetCamera(const HCamera& camera)
{
	HCamera newCamera = camera;
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
	auto iterFind = std::find(mElements.begin(), mElements.end(), guiElement);

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
		{
			mDirtyContents.insert(renderable); // Ensures GUIRenderElements are created

			mBatches.Add(renderable);
			mBatches.MarkContentDirty(renderable);
		}
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

void GUIWidget::UpdateBounds() const
{
	if(!mElements.empty())
		mBounds = mElements[0]->GetAbsoluteClippedArea();

	for(auto& elem : mElements)
	{
		GUIPhysicalArea elemBounds = elem->GetAbsoluteClippedArea();
		mBounds.Encapsulate(elemBounds);
	}
}

void GUIWidget::UpdateRootPanel()
{
	Viewport* target = GetTarget();
	if(target == nullptr)
		return;

	Area2I area = target->GetPixelArea();

	const GUIPhysicalSize physicalSize((i32)area.Width, (i32)area.Height);
	const GUILogicalSize size = GUIUtility::PhysicalToLogical(physicalSize, mDPIScale);

	GUILayoutData layoutData;
	layoutData.RelativePosition = GUILogicalPoint::kZero;
	layoutData.Size = size;
	layoutData.SetWidgetDepth(mDepth);

	mPanel->SetWidth(size.Width);
	mPanel->SetHeight(size.Height);

	mPanel->SetLayoutData(layoutData);
	mPanel->UpdateAbsoluteCoordinates(GUIPhysicalPointF::kZero, mDPIScale, GUIPhysicalAreaF(0.0f, 0.0f, (float)area.Width, (float)area.Height));
	mPanel->MarkLayoutAsDirty();
}

void GUIWidget::UpdateRenderTarget()
{
	SPtr<RenderTarget> rt;
	u64 newRTId = 0;
	if(mCamera != nullptr)
	{
		rt = mCamera->GetViewport()->GetTarget();
		if(rt != nullptr)
		{
			newRTId = rt->GetInternalId();
			mDPIScale = rt->GetProperties().DPIScale;
		}
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
		Area2I area = target->GetPixelArea();

		const GUIPhysicalSize physicalSize((i32)area.Width, (i32)area.Height);
		const GUILogicalSize widgetSize = GUIUtility::PhysicalToLogical(physicalSize, mDPIScale);

		const GUILogicalSize& panelSize = mPanel->GetLayoutData().Size;

		if(panelSize.Width != widgetSize.Width || panelSize.Height != widgetSize.Height)
		{
			UpdateRootPanel();
			OnOwnerTargetResized();
		}
	}

	// Perform layout updates
	for(auto it = mDirtyLayoutOrAbsoluteCoordinates.begin(); it != mDirtyLayoutOrAbsoluteCoordinates.end(); )
	{
		GUIElement* const element = *it;

		if(element != nullptr && !element->IsLayoutDirty()) // If not dirty, we likely already updated layout for one of its parents, or we just need to update absolute coordinates
		{
			++it;
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
	const bool isPanelOptimized = parent != nullptr && parent->Is<GUIPanel>();

	GUIElement* const updateParent = isPanelOptimized ? parent : element;

	// For GUIPanel we can do an optimization and update only the element in question instead
	// of all the children
	if(isPanelOptimized)
	{
		GUIPanel* panel = static_cast<GUIPanel*>(updateParent);

		GUIElement* dirtyElement = element;
		dirtyElement->UpdateOptimalLayoutSizes();

		GUIConstrainedSizeRange elementSizeRange = panel->GetChildConstrainedSizeRange(dirtyElement);
		const GUILogicalArea relativeElementArea = panel->CalculateRelativeElementArea(panel->GetLayoutData().Size.To<GUILogicalUnit>(), dirtyElement, elementSizeRange);

		const GUILayoutData parentPanelLayoutData = panel->GetLayoutData();
		GUILayoutData childLayoutData = parentPanelLayoutData;
		panel->UpdateDepthRangeInternal(childLayoutData);

		childLayoutData.RelativePosition = GUILogicalPoint(relativeElementArea.X, relativeElementArea.Y);
		childLayoutData.Size = GUILogicalSize(relativeElementArea.Width, relativeElementArea.Height);

		dirtyElement->SetLayoutData(childLayoutData);
		dirtyElement->UpdateLayoutForChildren();
		dirtyElement->UpdateAbsoluteCoordinates(panel->GetIntermediateAbsolutePosition(), mDPIScale, panel->GetIntermediateAbsoluteClippedArea());
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

GUIDrawGroupRenderDataUpdate GUIWidget::RebuildDirtyRenderData()
{
	if(!GetEnabled())
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

void GUIWidget::OnCreated()
{
	mStyleSheetCascade = GetBuiltinResources().GetDefaultGUIStyleSheetCascade();

	if(mCamera != nullptr)
	{
		SPtr<RenderTarget> target = mCamera->GetViewport()->GetTarget();

		if(target != nullptr)
		{
			mCachedRTId = target->GetInternalId();
			mDPIScale = target->GetProperties().DPIScale;
		}
	}

	mDefaultNavGroup = GUINavGroup::Create();

	GUIManager::Instance().RegisterWidget(this);

	mPanel = GUIPanel::Create();
	mPanel->ChangeParentWidget(this);
	UpdateRootPanel();
}

void GUIWidget::OnDestroyed()
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

void GUIWidget::Update()
{
	UpdateRenderTarget();
}

void GUIWidget::OnTransformChanged(TransformChangedFlags flags)
{
	mWidgetIsDirty = true;
}

RTTIType* GUIWidget::GetRttiStatic()
{
	return GUIWidgetRTTI::Instance();
}

RTTIType* GUIWidget::GetRtti() const
{
	return GUIWidget::GetRttiStatic();
}
