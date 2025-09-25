//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIManager.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/B3DGUIInteractable.h"
#include "Image/B3DSpriteTexture.h"
#include "Utility/B3DTime.h"
#include "Scene/B3DSceneObject.h"
#include "Material/B3DMaterial.h"
#include "Mesh/B3DMeshData.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Mesh/B3DMesh.h"
#include "Managers/B3DRenderWindowManager.h"
#include "Platform/B3DPlatform.h"
#include "Math/B3DArea2.h"
#include "B3DCoreApplication.h"
#include "B3DGUIMeshBatches.h"
#include "Error/B3DException.h"
#include "Input/B3DInput.h"
#include "GUI/B3DGUIInputCaret.h"
#include "GUI/B3DGUIInputSelection.h"
#include "GUI/B3DGUIContextMenu.h"
#include "GUI/B3DDragAndDrop.h"
#include "GUI/B3DGUIDropDownBoxManager.h"
#include "GUI/B3DGUIPanel.h"
#include "GUI/B3DGUINavGroup.h"
#include "Profiling/B3DProfilerCPU.h"
#include "Input/B3DVirtualInput.h"
#include "Platform/B3DCursor.h"
#include "CoreObject/B3DRenderThread.h"
#include "Renderer/B3DRendererManager.h"
#include "Renderer/B3DRenderer.h"
#include "Components/B3DCamera.h"
#include "GUI/B3DGUITooltipManager.h"
#include "Renderer/B3DRendererUtility.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "RenderAPI/B3DSamplerState.h"
#include "Resources/B3DBuiltinResources.h"
#include "2D/B3DSpriteManager.h"
#include "VectorGraphics/B3DVectorSpriteAtlas.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"

using namespace std::placeholders;

using namespace b3d;

const u32 GUIManager::kDragDistance = 3;
const float GUIManager::kTooltipHoverTime = 1.0f;

GUIManager::GUIManager()
{
	// Note: Hidden dependency. GUI must receive input events before other systems, in order so it can mark them as used
	// if required. e.g. clicking on a context menu should mark the event as used so that other non-GUI systems know
	// that they probably should not process such event themselves.
	mOnPointerMovedConn = GetInput().OnPointerMoved.Connect(std::bind(&GUIManager::OnPointerMoved, this, _1));
	mOnPointerPressedConn = GetInput().OnPointerPressed.Connect(std::bind(&GUIManager::OnPointerPressed, this, _1));
	mOnPointerReleasedConn = GetInput().OnPointerReleased.Connect(std::bind(&GUIManager::OnPointerReleased, this, _1));
	mOnPointerDoubleClick = GetInput().OnPointerDoubleClick.Connect(std::bind(&GUIManager::OnPointerDoubleClick, this, _1));
	mOnTextInputConn = GetInput().OnCharInput.Connect(std::bind(&GUIManager::OnTextInput, this, _1));
	mOnInputCommandConn = GetInput().OnInputCommand.Connect(std::bind(&GUIManager::OnInputCommandEntered, this, _1));
	mOnVirtualButtonDown = VirtualInput::Instance().OnButtonDown.Connect(std::bind(&GUIManager::OnVirtualButtonDown, this, _1, _2));

	mWindowGainedFocusConn = RenderWindowManager::Instance().OnFocusGained.Connect(std::bind(&GUIManager::OnWindowFocusGained, this, _1));
	mWindowLostFocusConn = RenderWindowManager::Instance().OnFocusLost.Connect(std::bind(&GUIManager::OnWindowFocusLost, this, _1));
	mMouseLeftWindowConn = RenderWindowManager::Instance().OnMouseLeftWindow.Connect(std::bind(&GUIManager::OnMouseLeftWindow, this, _1));

	mInputCaret = B3DNew<GUIInputCaret>();
	mInputSelection = B3DNew<GUIInputSelection>();

	DragAndDrop::StartUp();
	mDragEndedConn = DragAndDrop::Instance().OnDragEnded.Connect(std::bind(&GUIManager::OnMouseDragEnded, this, _1, _2));

	GUIDropDownBoxManager::StartUp();
	GUITooltipManager::StartUp();

	// Need to defer this call because I want to make sure all managers are initialized first
	DeferredCall(std::bind(&GUIManager::UpdateCaretTexture, this));
	DeferredCall(std::bind(&GUIManager::UpdateTextSelectionTexture, this));

	mVectorSpriteAtlas = B3DMakeUnique<GUIVectorSpriteAtlas>(GUIVectorSpriteAtlasSettings());
	mRenderer = RendererExtension::Create<render::GUIRenderer>(nullptr);
}

GUIManager::~GUIManager()
{
	GUITooltipManager::ShutDown();
	GUIDropDownBoxManager::ShutDown();
	DragAndDrop::ShutDown();

	// Make a copy of widgets, since destroying them will remove them from mWidgets and
	// we can't iterate over an array thats getting modified
	Vector<WidgetInfo> widgetCopy = mWidgets;
	for(auto& widget : widgetCopy)
		widget.Widget->Destroy(true);

	// Ensure everything queued get destroyed
	ProcessDestroyQueue();

	mOnPointerPressedConn.Disconnect();
	mOnPointerReleasedConn.Disconnect();
	mOnPointerMovedConn.Disconnect();
	mOnPointerDoubleClick.Disconnect();
	mOnTextInputConn.Disconnect();
	mOnInputCommandConn.Disconnect();
	mOnVirtualButtonDown.Disconnect();

	mDragEndedConn.Disconnect();

	mWindowGainedFocusConn.Disconnect();
	mWindowLostFocusConn.Disconnect();

	mMouseLeftWindowConn.Disconnect();

	B3DDelete(mInputCaret);
	B3DDelete(mInputSelection);
}

void GUIManager::DestroyRenderer(render::GUIRenderer* renderer)
{
	B3DDelete(renderer);
}

void GUIManager::RegisterWidget(GUIWidget* widget)
{
	const Viewport* renderTarget = widget->GetTarget();
	if(renderTarget == nullptr)
		return;

	mWidgets.push_back(WidgetInfo(widget));
}

void GUIManager::UnregisterWidget(GUIWidget* widget)
{
	{
		auto findIter = std::find_if(begin(mWidgets), end(mWidgets), [=](const WidgetInfo& x)
									 { return x.Widget == widget; });

		if(findIter != mWidgets.end())
			mWidgets.erase(findIter);
	}

	for(auto& entry : mElementsInFocus)
	{
		if(entry.Widget == widget)
			entry.Widget = nullptr;
	}

	for(auto& elementsPerWindow : mSavedFocusElements)
	{
		for(auto& entry : elementsPerWindow.second)
		{
			if(entry.Widget == widget)
				entry.Widget = nullptr;
		}
	}

	for(auto& entry : mElementsUnderPointer)
	{
		if(entry.Widget == widget)
			entry.Widget = nullptr;
	}

	for(auto& entry : mNewElementsUnderPointer)
	{
		if(entry.Widget == widget)
			entry.Widget = nullptr;
	}

	for(auto& entry : mActiveElements)
	{
		if(entry.Widget == widget)
			entry.Widget = nullptr;
	}

	auto widgetId = (u64)widget;
	GetRenderThread().PostCommand([renderer = mRenderer.get(),
								widgetId]()
							   { renderer->ClearDrawGroups(widgetId); }, "GUIRenderer::ClearDrawGroups");
}

void GUIManager::Update()
{
	DragAndDrop::Instance().Update();

	// Show tooltip if needed
	if(mShowTooltip)
	{
		float diff = GetTime().GetRealTimeInSeconds() - mTooltipElementHoverStart;
		if(diff >= kTooltipHoverTime || GetInput().IsButtonHeld(ButtonCode::LeftControl) || GetInput().IsButtonHeld(ButtonCode::RightControl))
		{
			for(auto& entry : mElementsUnderPointer)
			{
				const String& tooltipText = entry.Element->GetTooltip();
				GUIWidget* parentWidget = entry.Element->GetParentWidget();

				if(!tooltipText.empty() && parentWidget != nullptr)
				{
					const RenderWindow* window = GetWidgetWindow(*parentWidget);
					if(window != nullptr)
					{
						GUIPhysicalPoint windowPos = window->ScreenToWindowPosition(GetInput().GetPointerPosition()).To<GUIPhysicalUnit>();

						GUITooltipManager::Instance().Show(*parentWidget, windowPos, tooltipText);
						break;
					}
				}
			}

			mShowTooltip = false;
		}
	}

	// Update layouts
	GetProfilerCPU().BeginSample("UpdateLayout");
	for(auto& widgetInfo : mWidgets)
	{
		widgetInfo.Widget->UpdateLayout();
	}
	GetProfilerCPU().EndSample("UpdateLayout");

	// Destroy all queued elements (and loop in case any new ones get queued during destruction)
	do
	{
		mNewElementsUnderPointer.clear();
		for(auto& elementInfo : mElementsUnderPointer)
		{
			if(!elementInfo.Element->IsPendingDestroy())
				mNewElementsUnderPointer.push_back(elementInfo);
		}

		mElementsUnderPointer.swap(mNewElementsUnderPointer);

		mNewActiveElements.clear();
		for(auto& elementInfo : mActiveElements)
		{
			if(!elementInfo.Element->IsPendingDestroy())
				mNewActiveElements.push_back(elementInfo);
		}

		mActiveElements.swap(mNewActiveElements);

		mNewElementsInFocus.clear();

		for(auto& elementInfo : mElementsInFocus)
		{
			if(!elementInfo.Element->IsPendingDestroy())
				mNewElementsInFocus.push_back(elementInfo);
		}

		mElementsInFocus.swap(mNewElementsInFocus);

		for(auto& elementsPerWindow : mSavedFocusElements)
		{
			mNewElementsInFocus.clear();
			for(auto& entry : elementsPerWindow.second)
			{
				if(!entry.Element->IsPendingDestroy())
					mNewElementsInFocus.push_back(entry);
			}

			elementsPerWindow.second.swap(mNewElementsInFocus);
		}

		if(mForcedClearFocus)
		{
			// Clear focus on all elements that aren't part of the forced focus list (in case they are already in focus)
			mCommandEvent.SetType(GUICommandEventType::FocusLost);

			for(auto iter = mElementsInFocus.begin(); iter != mElementsInFocus.end();)
			{
				const ElementFocusInfo& elementInfo = *iter;

				const auto iterFind = std::find_if(begin(mForcedFocusElements), end(mForcedFocusElements), [&elementInfo](const ElementForcedFocusInfo& x)
												   { return x.Focus && x.Element == elementInfo.Element; });

				if(iterFind == mForcedFocusElements.end())
				{
					SendCommandEvent(elementInfo.Element, mCommandEvent);
					iter = mElementsInFocus.erase(iter);
				}
				else
					++iter;
			}

			mForcedClearFocus = false;
		}

		for(auto& focusElementInfo : mForcedFocusElements)
		{
			if(focusElementInfo.Element->IsPendingDestroy())
				continue;

			const auto iterFind = std::find_if(mElementsInFocus.begin(), mElementsInFocus.end(), [&](const ElementFocusInfo& x)
											   { return x.Element == focusElementInfo.Element; });

			if(focusElementInfo.Focus)
			{
				// Gain focus unless already in focus
				if(iterFind == mElementsInFocus.end())
				{
					mElementsInFocus.push_back(ElementFocusInfo(focusElementInfo.Element, focusElementInfo.Element->GetParentWidget(), false));

					mCommandEvent = GUICommandEvent();
					mCommandEvent.SetType(GUICommandEventType::FocusGained);

					SendCommandEvent(focusElementInfo.Element, mCommandEvent);
				}
			}
			else
			{
				// Force clear focus
				if(iterFind != mElementsInFocus.end())
				{
					mCommandEvent = GUICommandEvent();
					mCommandEvent.SetType(GUICommandEventType::FocusLost);

					SendCommandEvent(iterFind->Element, mCommandEvent);
					B3DSwapAndErase(mElementsInFocus, iterFind);
				}
			}
		}

		mForcedFocusElements.clear();
	}
	while(ProcessDestroyQueueIteration());

	// Blink caret
	float curTime = GetTime().GetRealTimeInSeconds();

	if((curTime - mCaretLastBlinkTime) >= mCaretBlinkInterval)
	{
		mCaretLastBlinkTime = curTime;
		mIsCaretOn = !mIsCaretOn;

		mCommandEvent = GUICommandEvent();
		mCommandEvent.SetType(GUICommandEventType::Redraw);

		for(auto& elementInfo : mElementsInFocus)
		{
			SendCommandEvent(elementInfo.Element, mCommandEvent);
		}
	}

	// Update dirty widget render data
	for(auto& entry : mWidgets)
	{
		GUIWidget* widget = entry.Widget;
		GUIDrawGroupRenderDataUpdate updateData = widget->RebuildDirtyRenderData();

		HCamera camera;
		camera = widget->GetCamera();
		if(camera == nullptr)
			continue;

		auto widgetId = (u64)widget;
		GetRenderThread().PostCommand([renderer = mRenderer.get(),
									updateData = std::move(updateData),
									camera = B3DGetRenderProxy(camera).get(),
									widgetId,
									widgetDepth = widget->GetDepth(),
									worldTransform = widget->SceneObject()->GetTransform().GetMatrix()]()
								   { renderer->UpdateDrawGroups(camera, widgetId, widgetDepth, worldTransform, updateData); }, "GUIRenderer::UpdateDrawGroups");
	}

	// Note: It's important to call this after GUI elements rebuild their render data, as this will request new vector paths
	mVectorSpriteAtlas->Update();

	GetRenderThread().PostCommand([renderer = mRenderer.get(), time = GetTime().GetRealTimeInSeconds()]()
							   { renderer->Update(time); }, "GUIRenderer::Update");
}

void GUIManager::ProcessDestroyQueue()
{
	// Loop until everything empties
	while(ProcessDestroyQueueIteration())
	{}
}

void GUIManager::UpdateCaretTexture()
{
	if(mCaretImage == nullptr)
	{
		TextureCreateInformation textureCreateInformation; // Default
		textureCreateInformation.Name = "Input Caret";

		HTexture newTex = Texture::Create(textureCreateInformation);
		mCaretImage = SpriteTexture::Create(newTex);
	}

	const HTexture& texture = mCaretImage->GetAtlasTexture();
	SPtr<PixelData> data = texture->GetProperties().AllocBuffer(0, 0);

	data->SetColorAt(mCaretColor, 0, 0);
	texture->WriteData(data);
}

void GUIManager::UpdateTextSelectionTexture()
{
	if(mTextSelectionImage == nullptr)
	{
		TextureCreateInformation textureCreateInformation; // Default
		textureCreateInformation.Name = "Input Caret";

		HTexture newTex = Texture::Create(textureCreateInformation);
		mTextSelectionImage = SpriteTexture::Create(newTex);
	}

	const HTexture& texture = mTextSelectionImage->GetAtlasTexture();
	SPtr<PixelData> data = texture->GetProperties().AllocBuffer(0, 0);

	data->SetColorAt(mTextSelectionColor, 0, 0);
	texture->WriteData(data);
}

void GUIManager::OnMouseDragEnded(const PointerEvent& event, DragCallbackInfo& dragInfo)
{
	GUIMouseButton guiButton = ButtonToGuiButton(event.Button);

	if(DragAndDrop::Instance().IsDragInProgress() && guiButton == GUIMouseButton::Left)
	{
		const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIPhysicalPoint localPos(BsZero);

			if(elementInfo.Widget != nullptr)
				localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);

			bool acceptDrop = true;
			if(DragAndDrop::Instance().NeedsValidDropTarget())
			{
				acceptDrop = elementInfo.Element->AcceptDragAndDrop(localPos, DragAndDrop::Instance().GetDragTypeId());
			}

			if(acceptDrop)
			{
				mMouseEvent.SetDragAndDropDroppedData(localPos, DragAndDrop::Instance().GetDragData());
				dragInfo.Processed = SendMouseEvent(elementInfo.Element, mMouseEvent);

				if(dragInfo.Processed)
					return;
			}
		}
	}

	dragInfo.Processed = false;
}

void GUIManager::OnPointerMoved(const PointerEvent& event)
{
	if(event.IsUsed)
		return;

	bool buttonStates[(int)GUIMouseButton::Count];
	buttonStates[0] = event.ButtonStates[0];
	buttonStates[1] = event.ButtonStates[1];
	buttonStates[2] = event.ButtonStates[2];

	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	if(FindElementUnderPointer(screenPosition, buttonStates, event.Shift, event.Control, event.Alt))
		event.IsUsed = true;

	if(mDragState == DragState::HeldWithoutDrag)
	{
		u32 dist = (u32)mLastPointerClickPos.CalculateManhattanDistance(screenPosition);

		if(dist > kDragDistance)
		{
			for(auto& activeElement : mActiveElements)
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(activeElement.Widget, screenPosition);
				const GUIPhysicalPoint localDragStartPos = GetWidgetRelativePos(activeElement.Widget, mLastPointerClickPos);

				mMouseEvent.SetMouseDragStartData(localPos, localDragStartPos);
				if(SendMouseEvent(activeElement.Element, mMouseEvent))
					event.IsUsed = true;
			}

			mDragState = DragState::Dragging;
			mDragStartPos = screenPosition;
		}
	}

	// If mouse is being held down send MouseDrag events
	if(mDragState == DragState::Dragging)
	{
		for(auto& activeElement : mActiveElements)
		{
			if(mLastPointerScreenPos != event.ScreenPos)
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(activeElement.Widget, screenPosition);

				mMouseEvent.SetMouseDragData(localPos, screenPosition - mDragStartPos);
				if(SendMouseEvent(activeElement.Element, mMouseEvent))
					event.IsUsed = true;
			}
		}

		mLastPointerScreenPos = event.ScreenPos;

		// Also if drag is in progress send DragAndDrop events
		if(DragAndDrop::Instance().IsDragInProgress())
		{
			bool acceptDrop = true;
			for(auto& elementInfo : mElementsUnderPointer)
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);

				acceptDrop = true;
				if(DragAndDrop::Instance().NeedsValidDropTarget())
				{
					acceptDrop = elementInfo.Element->AcceptDragAndDrop(localPos, DragAndDrop::Instance().GetDragTypeId());
				}

				if(acceptDrop)
				{
					mMouseEvent.SetDragAndDropDraggedData(localPos, DragAndDrop::Instance().GetDragData());
					if(SendMouseEvent(elementInfo.Element, mMouseEvent))
					{
						event.IsUsed = true;
						break;
					}
				}
			}

			if(acceptDrop)
			{
				if(mActiveCursor != CursorType::ArrowDrag)
				{
					Cursor::Instance().SetCursor(CursorType::ArrowDrag);
					mActiveCursor = CursorType::ArrowDrag;
				}
			}
			else
			{
				if(mActiveCursor != CursorType::Deny)
				{
					Cursor::Instance().SetCursor(CursorType::Deny);
					mActiveCursor = CursorType::Deny;
				}
			}
		}
	}
	else // Otherwise, send MouseMove events if we are hovering over any element
	{
		if(mLastPointerScreenPos != event.ScreenPos)
		{
			bool moveProcessed = false;
			bool hasCustomCursor = false;
			for(auto& elementInfo : mElementsUnderPointer)
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);

				if(!moveProcessed)
				{
					// Send MouseMove event
					mMouseEvent.SetMouseMoveData(localPos);
					moveProcessed = SendMouseEvent(elementInfo.Element, mMouseEvent);

					if(moveProcessed)
						event.IsUsed = true;
				}

				if(mDragState == DragState::NoDrag)
				{
					CursorType newCursor = CursorType::Arrow;
					if(elementInfo.Element->HasCustomCursor(localPos, newCursor))
					{
						if(newCursor != mActiveCursor)
						{
							Cursor::Instance().SetCursor(newCursor);
							mActiveCursor = newCursor;
						}

						hasCustomCursor = true;
					}
				}

				if(moveProcessed)
					break;
			}

			// While dragging we don't want to modify the cursor
			if(mDragState == DragState::NoDrag)
			{
				if(!hasCustomCursor)
				{
					if(mActiveCursor != CursorType::Arrow)
					{
						Cursor::Instance().SetCursor(CursorType::Arrow);
						mActiveCursor = CursorType::Arrow;
					}
				}
			}
		}

		mLastPointerScreenPos = event.ScreenPos;

		if(Math::Abs(event.MouseWheelScrollAmount) > 0.00001f)
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				mMouseEvent.SetMouseWheelScrollData(event.MouseWheelScrollAmount);
				if(SendMouseEvent(elementInfo.Element, mMouseEvent))
				{
					event.IsUsed = true;
					break;
				}
			}
		}
	}
}

void GUIManager::OnPointerReleased(const PointerEvent& event)
{
	if(event.IsUsed)
		return;

	bool buttonStates[(int)GUIMouseButton::Count];
	buttonStates[0] = event.ButtonStates[0];
	buttonStates[1] = event.ButtonStates[1];
	buttonStates[2] = event.ButtonStates[2];

	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	if(FindElementUnderPointer(screenPosition, buttonStates, event.Shift, event.Control, event.Alt))
		event.IsUsed = true;

	mMouseEvent = GUIMouseEvent(buttonStates, event.Shift, event.Control, event.Alt);

	GUIMouseButton guiButton = ButtonToGuiButton(event.Button);

	// Send MouseUp event only if we are over the active element (we don't want to accidentally trigger other elements).
	// And only activate when a button that originally caused the active state is released, otherwise ignore it.
	if(mActiveMouseButton == guiButton)
	{
		for(auto& elementInfo : mElementsUnderPointer)
		{
			auto iterFind2 = std::find_if(mActiveElements.begin(), mActiveElements.end(), [&](const ElementInfo& x)
										  { return x.Element == elementInfo.Element; });

			if(iterFind2 != mActiveElements.end())
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);
				mMouseEvent.SetMouseUpData(localPos, guiButton);

				if(SendMouseEvent(elementInfo.Element, mMouseEvent))
				{
					event.IsUsed = true;
					break;
				}
			}
		}
	}

	// Send DragEnd event to whichever element is active
	bool acceptEndDrag = (mDragState == DragState::Dragging || mDragState == DragState::HeldWithoutDrag) && mActiveMouseButton == guiButton &&
		(guiButton == GUIMouseButton::Left);

	if(acceptEndDrag)
	{
		if(mDragState == DragState::Dragging)
		{
			for(auto& activeElement : mActiveElements)
			{
				GUIPhysicalPoint localPos = GetWidgetRelativePos(activeElement.Widget, screenPosition);

				mMouseEvent.SetMouseDragEndData(localPos);
				if(SendMouseEvent(activeElement.Element, mMouseEvent))
					event.IsUsed = true;
			}
		}

		mDragState = DragState::NoDrag;
	}

	if(mActiveMouseButton == guiButton)
	{
		mActiveElements.clear();
		mActiveMouseButton = GUIMouseButton::Left;
	}

	if(mActiveCursor != CursorType::Arrow)
	{
		Cursor::Instance().SetCursor(CursorType::Arrow);
		mActiveCursor = CursorType::Arrow;
	}
}

void GUIManager::OnPointerPressed(const PointerEvent& event)
{
	if(event.IsUsed)
		return;

	bool buttonStates[(int)GUIMouseButton::Count];
	buttonStates[0] = event.ButtonStates[0];
	buttonStates[1] = event.ButtonStates[1];
	buttonStates[2] = event.ButtonStates[2];

	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	if(FindElementUnderPointer(screenPosition, buttonStates, event.Shift, event.Control, event.Alt))
		event.IsUsed = true;

	// Determine elements that gained focus
	mNewElementsInFocus.clear();

	mCommandEvent = GUICommandEvent();

	// Determine elements that gained focus
	for(auto& elementInfo : mElementsUnderPointer)
	{
		auto iterFind = std::find_if(begin(mElementsInFocus), end(mElementsInFocus), [=](const ElementFocusInfo& x)
									 { return x.Element == elementInfo.Element; });

		if(iterFind == mElementsInFocus.end())
		{
			bool processed = !elementInfo.Element->GetOptionFlags().IsSet(GUIElementOption::ClickThrough);
			mNewElementsInFocus.push_back(ElementFocusInfo(elementInfo.Element, elementInfo.Widget, processed));

			if(processed)
				break;
		}
		else
		{
			mNewElementsInFocus.push_back(*iterFind);

			if(iterFind->UsesFocus)
				break;
		}
	}

	// Send focus loss events
	// Note: Focus loss must trigger before mouse press because things like input boxes often only confirm changes
	// made to them when focus is lost. So if the user is confirming some input via a press of the button focus loss
	// must trigger on the input box first to make sure its contents get saved.
	mCommandEvent.SetType(GUICommandEventType::FocusLost);

	for(auto& elementInfo : mElementsInFocus)
	{
		auto iterFind = std::find_if(begin(mNewElementsInFocus), end(mNewElementsInFocus), [=](const ElementFocusInfo& x)
									 { return x.Element == elementInfo.Element; });

		if(iterFind == mNewElementsInFocus.end())
			SendCommandEvent(elementInfo.Element, mCommandEvent);
	}

	// Send focus gain events
	mCommandEvent.SetType(GUICommandEventType::FocusGained);

	for(auto& elementInfo : mNewElementsInFocus)
	{
		auto iterFind = std::find_if(begin(mElementsInFocus), end(mElementsInFocus), [=](const ElementFocusInfo& x)
									 { return x.Element == elementInfo.Element; });

		if(iterFind == mElementsInFocus.end())
			SendCommandEvent(elementInfo.Element, mCommandEvent);
	}

	mElementsInFocus.swap(mNewElementsInFocus);

	// Send mouse press event
	mMouseEvent = GUIMouseEvent(buttonStates, event.Shift, event.Control, event.Alt);
	GUIMouseButton guiButton = ButtonToGuiButton(event.Button);

	// We only check for mouse down if mouse isn't already being held down, and we are hovering over an element
	if(mActiveElements.size() == 0)
	{
		mNewActiveElements.clear();
		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);
			mMouseEvent.SetMouseDownData(localPos, guiButton);

			bool processed = SendMouseEvent(elementInfo.Element, mMouseEvent);

			if(guiButton == GUIMouseButton::Left)
			{
				mDragState = DragState::HeldWithoutDrag;
				mLastPointerClickPos = event.ScreenPos.To<GUIPhysicalUnit>();
			}

			mNewActiveElements.push_back(ElementInfo(elementInfo.Element, elementInfo.Widget));
			mActiveMouseButton = guiButton;

			if(processed)
			{
				event.IsUsed = true;
				break;
			}
		}

		mActiveElements.swap(mNewActiveElements);
	}

	// If right click try to open context menu
	if(buttonStates[2])
	{
		for(auto& elementInfo : mElementsUnderPointer)
		{
			SPtr<GUIContextMenu> menu = elementInfo.Element->GetContextMenu();

			if(menu != nullptr && elementInfo.Widget != nullptr)
			{
				const RenderWindow* window = GetWidgetWindow(*elementInfo.Widget);
				if(window != nullptr)
				{
					const GUIPhysicalPoint windowPos = window->ScreenToWindowPosition(event.ScreenPos).To<GUIPhysicalUnit>();

					menu->Open(windowPos, *elementInfo.Widget);
					event.IsUsed = true;
					break;
				}
			}
		}
	}
}

void GUIManager::OnPointerDoubleClick(const PointerEvent& event)
{
	if(event.IsUsed)
		return;

	bool buttonStates[(int)GUIMouseButton::Count];
	buttonStates[0] = event.ButtonStates[0];
	buttonStates[1] = event.ButtonStates[1];
	buttonStates[2] = event.ButtonStates[2];

	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	if(FindElementUnderPointer(screenPosition, buttonStates, event.Shift, event.Control, event.Alt))
		event.IsUsed = true;

	mMouseEvent = GUIMouseEvent(buttonStates, event.Shift, event.Control, event.Alt);

	GUIMouseButton guiButton = ButtonToGuiButton(event.Button);

	// We only check for mouse down if we are hovering over an element
	for(auto& elementInfo : mElementsUnderPointer)
	{
		GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);

		mMouseEvent.SetMouseDoubleClickData(localPos, guiButton);
		if(SendMouseEvent(elementInfo.Element, mMouseEvent))
		{
			event.IsUsed = true;
			break;
		}
	}
}

void GUIManager::OnInputCommandEntered(InputCommandType commandType)
{
	if(mElementsInFocus.empty())
		return;

	HideTooltip();

	// Tabs are handled by the GUI manager itself, while other events are passed to GUI elements
	if(commandType == InputCommandType::Tab)
	{
		TabFocusNext();
		return;
	}

	mCommandEvent = GUICommandEvent();

	switch(commandType)
	{
	case InputCommandType::Backspace:
		mCommandEvent.SetType(GUICommandEventType::Backspace);
		break;
	case InputCommandType::Delete:
		mCommandEvent.SetType(GUICommandEventType::Delete);
		break;
	case InputCommandType::Return:
		mCommandEvent.SetType(GUICommandEventType::Return);
		break;
	case InputCommandType::Confirm:
		mCommandEvent.SetType(GUICommandEventType::Confirm);
		break;
	case InputCommandType::Escape:
		mCommandEvent.SetType(GUICommandEventType::Escape);
		break;
	case InputCommandType::CursorMoveLeft:
		mCommandEvent.SetType(GUICommandEventType::MoveLeft);
		break;
	case InputCommandType::CursorMoveRight:
		mCommandEvent.SetType(GUICommandEventType::MoveRight);
		break;
	case InputCommandType::CursorMoveUp:
		mCommandEvent.SetType(GUICommandEventType::MoveUp);
		break;
	case InputCommandType::CursorMoveDown:
		mCommandEvent.SetType(GUICommandEventType::MoveDown);
		break;
	case InputCommandType::SelectLeft:
		mCommandEvent.SetType(GUICommandEventType::SelectLeft);
		break;
	case InputCommandType::SelectRight:
		mCommandEvent.SetType(GUICommandEventType::SelectRight);
		break;
	case InputCommandType::SelectUp:
		mCommandEvent.SetType(GUICommandEventType::SelectUp);
		break;
	case InputCommandType::SelectDown:
		mCommandEvent.SetType(GUICommandEventType::SelectDown);
		break;
	default:
		break;
	}

	for(auto& elementInfo : mElementsInFocus)
		SendCommandEvent(elementInfo.Element, mCommandEvent);
}

void GUIManager::OnVirtualButtonDown(const VirtualButton& button, u32 deviceIdx)
{
	HideTooltip();
	mVirtualButtonEvent.SetButton(button);

	for(auto& elementInFocus : mElementsInFocus)
	{
		bool processed = SendVirtualButtonEvent(elementInFocus.Element, mVirtualButtonEvent);

		if(processed)
			break;
	}
}

bool GUIManager::FindElementUnderPointer(const GUIPhysicalPoint& pointerScreenPos, bool buttonStates[3], bool shift, bool control, bool alt)
{
	Vector<const RenderWindow*> widgetWindows;
	for(auto& widgetInfo : mWidgets)
		widgetWindows.push_back(GetWidgetWindow(*widgetInfo.Widget));

#if B3D_DEBUG
	// Checks if all referenced windows actually exist
	Vector<RenderWindow*> activeWindows = RenderWindowManager::Instance().GetRenderWindows();
	for(auto& window : widgetWindows)
	{
		if(window == nullptr)
			continue;

		auto iterFind = std::find(begin(activeWindows), end(activeWindows), window);

		if(iterFind == activeWindows.end())
		{
			B3D_EXCEPT(InternalErrorException, "GUI manager has a reference to a window that doesn't exist. \
												  Please detach all GUIWidgets from windows before destroying a window.");
		}
	}
#endif

	mNewElementsUnderPointer.clear();

	const RenderWindow* windowUnderPointer = nullptr;
	UnorderedSet<const RenderWindow*> uniqueWindows;

	for(auto& window : widgetWindows)
	{
		if(window == nullptr)
			continue;

		uniqueWindows.insert(window);
	}

	RenderWindow* topMostModal = RenderWindowManager::Instance().GetTopMostModal();
	for(auto& window : uniqueWindows)
	{
		if(Platform::IsPointOverWindow(*window, pointerScreenPos.To<i32>()))
		{
			// If there's a top most modal window, it needs to be this one, otherwise we ignore input to that window
			if(topMostModal == nullptr || window == topMostModal)
				windowUnderPointer = window;

			break;
		}
	}

	if(windowUnderPointer != nullptr)
	{
		GUIPhysicalPoint windowPos = windowUnderPointer->ScreenToWindowPosition(pointerScreenPos.To<i32>()).To<GUIPhysicalUnit>();

		u32 widgetIdx = 0;
		for(auto& widgetInfo : mWidgets)
		{
			if(widgetWindows[widgetIdx] == nullptr)
			{
				widgetIdx++;
				continue;
			}

			GUIWidget* widget = widgetInfo.Widget;
			if(widgetWindows[widgetIdx] == windowUnderPointer && widget->InBounds(WindowToBridgedCoords(widget->GetTarget()->GetTarget(), windowPos)))
			{
				// Note: This should only be checking non-culled element (i.e. GUIElement::GetVisibleElements())
				const Vector<GUIRenderable*>& elements = widget->GetElements();
				GUIPhysicalPoint localPos = GetWidgetRelativePos(widget, pointerScreenPos);

				// Elements with lowest depth (most to the front) get handled first
				for(auto iter = elements.begin(); iter != elements.end(); ++iter)
				{
					GUIInteractable* const interactableElement = B3DRTTICast<GUIInteractable>(*iter);
					if(!interactableElement)
						continue;

					if(!interactableElement->IsHiddenOrCulled() && interactableElement->IsInInteractionBounds(localPos))
					{
						ElementInfoUnderPointer elementInfo(interactableElement, widget);

						auto iterFind = std::find_if(mElementsUnderPointer.begin(), mElementsUnderPointer.end(), [=](const ElementInfoUnderPointer& x)
													 { return x.Element == interactableElement; });

						if(iterFind != mElementsUnderPointer.end())
						{
							elementInfo.UsesMouseOver = iterFind->UsesMouseOver;
							elementInfo.ReceivedMouseOver = iterFind->ReceivedMouseOver;
						}

						mNewElementsUnderPointer.push_back(elementInfo);
					}
				}
			}

			widgetIdx++;
		}
	}

	std::sort(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(), [](const ElementInfoUnderPointer& a, const ElementInfoUnderPointer& b)
			  { return a.Element->GetDepth() < b.Element->GetDepth(); });

	// Send MouseOut and MouseOver events
	bool eventProcessed = false;

	for(auto& elementInfo : mNewElementsUnderPointer)
	{
		GUIInteractable* element = elementInfo.Element;
		GUIWidget* widget = elementInfo.Widget;

		if(elementInfo.ReceivedMouseOver)
		{
			elementInfo.IsHovering = true;
			if(elementInfo.UsesMouseOver)
				break;

			continue;
		}

		auto iterFind = std::find_if(mActiveElements.begin(), mActiveElements.end(), [&](const ElementInfo& x)
									 { return x.Element == element; });

		// Send MouseOver event
		if(mActiveElements.size() == 0 || iterFind != mActiveElements.end())
		{
			GUIPhysicalPoint localPos = GetWidgetRelativePos(widget, pointerScreenPos);

			mMouseEvent = GUIMouseEvent(buttonStates, shift, control, alt);

			mMouseEvent.SetMouseOverData(localPos);
			elementInfo.ReceivedMouseOver = true;
			elementInfo.IsHovering = true;
			if(SendMouseEvent(element, mMouseEvent))
			{
				eventProcessed = true;
				elementInfo.UsesMouseOver = true;
				break;
			}
		}
	}

	// Send DragAndDropLeft event - It is similar to MouseOut events but we send it to all
	// elements a user might hover over, while we send mouse over/out events only to active elements while dragging
	if(DragAndDrop::Instance().IsDragInProgress())
	{
		for(auto& elementInfo : mElementsUnderPointer)
		{
			auto iterFind = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(), [=](const ElementInfoUnderPointer& x)
										 { return x.Element == elementInfo.Element; });

			if(iterFind == mNewElementsUnderPointer.end())
			{
				GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, pointerScreenPos);

				mMouseEvent.SetDragAndDropLeftData(localPos, DragAndDrop::Instance().GetDragData());
				if(SendMouseEvent(elementInfo.Element, mMouseEvent))
				{
					eventProcessed = true;
					break;
				}
			}
		}
	}

	for(auto& elementInfo : mElementsUnderPointer)
	{
		GUIInteractable* element = elementInfo.Element;
		GUIWidget* widget = elementInfo.Widget;

		auto iterFind = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(), [=](const ElementInfoUnderPointer& x)
									 { return x.Element == element; });

		if(!elementInfo.ReceivedMouseOver)
			continue;

		if(iterFind == mNewElementsUnderPointer.end() || !iterFind->IsHovering)
		{
			auto iterFind2 = std::find_if(mActiveElements.begin(), mActiveElements.end(), [=](const ElementInfo& x)
										  { return x.Element == element; });

			// Send MouseOut event
			if(mActiveElements.size() == 0 || iterFind2 != mActiveElements.end())
			{
				GUIPhysicalPoint localPos = GetWidgetRelativePos(widget, pointerScreenPos);

				mMouseEvent.SetMouseOutData(localPos);
				if(SendMouseEvent(element, mMouseEvent))
				{
					eventProcessed = true;
					break;
				}
			}
		}
	}

	mElementsUnderPointer.swap(mNewElementsUnderPointer);

	// Tooltip
	HideTooltip();
	if(mElementsUnderPointer.size() > 0)
		mShowTooltip = true;

	mTooltipElementHoverStart = GetTime().GetRealTimeInSeconds();

	return eventProcessed;
}

void GUIManager::OnTextInput(const TextInputEvent& event)
{
	mTextInputEvent = GUITextInputEvent();
	mTextInputEvent.SetData(event.TextChar);

	for(auto& elementInFocus : mElementsInFocus)
	{
		if(SendTextInputEvent(elementInFocus.Element, mTextInputEvent))
			event.IsUsed = true;
	}
}

void GUIManager::OnWindowFocusGained(RenderWindow& win)
{
	for(auto& widgetInfo : mWidgets)
	{
		GUIWidget* widget = widgetInfo.Widget;
		if(GetWidgetWindow(*widget) == &win)
			widget->OwnerWindowFocusChanged();
	}

	auto iterFind = mSavedFocusElements.find(&win);
	if(iterFind != mSavedFocusElements.end())
	{
		Vector<ElementFocusInfo>& savedFocusedElements = iterFind->second;

		mNewElementsInFocus.clear();
		for(auto& focusedElement : mElementsInFocus)
		{
			if(focusedElement.Element->IsPendingDestroy())
				continue;

			auto iterFind2 = std::find_if(savedFocusedElements.begin(), savedFocusedElements.end(), [&focusedElement](const ElementFocusInfo& x)
										  { return x.Element == focusedElement.Element; });

			if(iterFind2 == savedFocusedElements.end())
			{
				mCommandEvent = GUICommandEvent();
				mCommandEvent.SetType(GUICommandEventType::FocusLost);

				SendCommandEvent(focusedElement.Element, mCommandEvent);
				savedFocusedElements.push_back(focusedElement);
			}
			else
				mNewElementsInFocus.push_back(focusedElement);
		}

		mElementsInFocus.swap(mNewElementsInFocus);

		for(auto& entry : savedFocusedElements)
		{
			if(entry.Element->IsPendingDestroy())
				continue;

			auto iterFind2 = std::find_if(mElementsInFocus.begin(), mElementsInFocus.end(), [&entry](const ElementFocusInfo& x)
										  { return x.Element == entry.Element; });

			if(iterFind2 == mElementsInFocus.end())
			{
				mCommandEvent = GUICommandEvent();
				mCommandEvent.SetType(GUICommandEventType::FocusGained);

				SendCommandEvent(entry.Element, mCommandEvent);
				mElementsInFocus.push_back(entry);
			}
		}

		mSavedFocusElements.erase(iterFind);
	}
}

void GUIManager::OnWindowFocusLost(RenderWindow& win)
{
	for(auto& widgetInfo : mWidgets)
	{
		GUIWidget* widget = widgetInfo.Widget;
		if(GetWidgetWindow(*widget) == &win)
			widget->OwnerWindowFocusChanged();
	}

	Vector<ElementFocusInfo>& savedFocusedElements = mSavedFocusElements[&win];
	savedFocusedElements.clear();

	mNewElementsInFocus.clear();
	for(auto& focusedElement : mElementsInFocus)
	{
		if(focusedElement.Element->IsPendingDestroy())
			continue;

		if(focusedElement.Widget != nullptr && GetWidgetWindow(*focusedElement.Widget) == &win)
		{
			mCommandEvent = GUICommandEvent();
			mCommandEvent.SetType(GUICommandEventType::FocusLost);

			SendCommandEvent(focusedElement.Element, mCommandEvent);
			savedFocusedElements.push_back(focusedElement);
		}
		else
			mNewElementsInFocus.push_back(focusedElement);
	}

	mElementsInFocus.swap(mNewElementsInFocus);
}

// We stop getting mouse move events once it leaves the window, so make sure
// nothing stays in hover state
void GUIManager::OnMouseLeftWindow(RenderWindow& win)
{
	mNewElementsUnderPointer.clear();

	for(auto& elementInfo : mElementsUnderPointer)
	{
		GUIInteractable* element = elementInfo.Element;
		GUIWidget* widget = elementInfo.Widget;

		if(widget != nullptr && widget->GetTarget()->GetTarget().get() != &win)
		{
			mNewElementsUnderPointer.push_back(elementInfo);
			continue;
		}

		auto iterFind = std::find_if(mActiveElements.begin(), mActiveElements.end(), [&](const ElementInfo& x)
									 { return x.Element == element; });

		// Send MouseOut event
		if(mActiveElements.size() == 0 || iterFind != mActiveElements.end())
		{
			GUIPhysicalPoint localPos = GetWidgetRelativePos(widget, GUIPhysicalPoint(BsZero));

			mMouseEvent.SetMouseOutData(localPos);
			SendMouseEvent(element, mMouseEvent);
		}
	}

	mElementsUnderPointer.swap(mNewElementsUnderPointer);

	HideTooltip();
	if(mDragState != DragState::Dragging)
	{
		if(mActiveCursor != CursorType::Arrow)
		{
			Cursor::Instance().SetCursor(CursorType::Arrow);
			mActiveCursor = CursorType::Arrow;
		}
	}
}

void GUIManager::HideTooltip()
{
	GUITooltipManager::Instance().Hide();
	mShowTooltip = false;
}

void GUIManager::QueueForDestroy(GUIElement* element)
{
	mScheduledForDestruction.push(element);
}

void GUIManager::SetFocus(GUIInteractable* element, bool focus, bool clear)
{
	ElementForcedFocusInfo efi;
	efi.Element = element;
	efi.Focus = focus;

	if(clear)
	{
		mForcedClearFocus = true;
		mForcedFocusElements.clear();
	}

	mForcedFocusElements.push_back(efi);
}

bool GUIManager::ProcessDestroyQueueIteration()
{
	Stack<GUIElement*> toDestroy = mScheduledForDestruction;
	mScheduledForDestruction = Stack<GUIElement*>();

	while(!toDestroy.empty())
	{
		B3DDelete(toDestroy.top());
		toDestroy.pop();
	}

	return !mScheduledForDestruction.empty();
}

void GUIManager::SetInputBridge(const SPtr<RenderTexture>& renderTex, const GUIInteractable* element)
{
	if(element == nullptr)
		mInputBridge.erase(renderTex);
	else
		mInputBridge[renderTex] = element;
}

GUIMouseButton GUIManager::ButtonToGuiButton(PointerEventButton pointerButton) const
{
	if(pointerButton == PointerEventButton::Left)
		return GUIMouseButton::Left;
	else if(pointerButton == PointerEventButton::Middle)
		return GUIMouseButton::Middle;
	else if(pointerButton == PointerEventButton::Right)
		return GUIMouseButton::Right;

	B3D_EXCEPT(InvalidParametersException, "Provided button is not a GUI supported mouse button.");
	return GUIMouseButton::Left;
}

GUIPhysicalPoint GUIManager::GetWidgetRelativePos(const GUIWidget* widget, const GUIPhysicalPoint& screenPos) const
{
	if(widget == nullptr)
		return screenPos;

	const RenderWindow* window = GetWidgetWindow(*widget);
	if(window == nullptr)
		return GUIPhysicalPoint(BsZero);

	GUIPhysicalPoint windowPos = window->ScreenToWindowPosition(screenPos.To<i32>()).To<GUIPhysicalUnit>();
	windowPos = WindowToBridgedCoords(widget->GetTarget()->GetTarget(), windowPos);

	const Matrix4& worldTfrm = widget->SceneObject()->GetTransform().GetMatrix();

	Vector4 vecLocalPos = worldTfrm.Inverse().MultiplyAffine(Vector4((float)windowPos.X, (float)windowPos.Y, 0.0f, 1.0f));
	GUIPhysicalPoint curLocalPos(Math::RoundToI32(vecLocalPos.X), Math::RoundToI32(vecLocalPos.Y));

	return curLocalPos;
}

GUIPhysicalPoint GUIManager::WindowToBridgedCoords(const SPtr<RenderTarget>& target, const GUIPhysicalPoint& windowPos) const
{
	// This cast might not be valid (the render target could be a window), but we only really need to cast
	// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
	// (in which case we know it is a RenderTexture)
	SPtr<const RenderTexture> renderTexture = std::static_pointer_cast<const RenderTexture>(target);
	const RenderTargetProperties& rtProps = renderTexture->GetProperties();

	auto iterFind = mInputBridge.find(renderTexture);
	if(iterFind != mInputBridge.end()) // Widget input is bridged, which means we need to transform the coordinates
	{
		const GUIInteractable* bridgeElement = iterFind->second;
		const GUIWidget* parentWidget = bridgeElement->GetParentWidget();
		if(parentWidget == nullptr)
			return windowPos;

		const Matrix4& worldTfrm = parentWidget->SceneObject()->GetTransform().GetMatrix();

		Vector4 vecLocalPos = worldTfrm.Inverse().MultiplyAffine(Vector4((float)windowPos.X, (float)windowPos.Y, 0.0f, 1.0f));
		GUIPhysicalArea bridgeBounds = bridgeElement->GetAbsoluteBounds();

		// Find coordinates relative to the bridge element
		float x = vecLocalPos.X - (float)bridgeBounds.X;
		float y = vecLocalPos.Y - (float)bridgeBounds.Y;

		float scaleX = bridgeBounds.Width > 0 ? rtProps.Width / (float)bridgeBounds.Width : 0.0f;
		float scaleY = bridgeBounds.Height > 0 ? rtProps.Height / (float)bridgeBounds.Height : 0.0f;

		return GUIPhysicalPoint(Math::RoundToI32(x * scaleX), Math::RoundToI32(y * scaleY));
	}

	return windowPos;
}

const RenderWindow* GUIManager::GetWidgetWindow(const GUIWidget& widget) const
{
	const Viewport* viewport = widget.GetTarget();
	if(viewport == nullptr)
		return nullptr;

	SPtr<RenderTarget> target = viewport->GetTarget();
	if(target == nullptr)
		return nullptr;

	// This cast might not be valid (the render target could be a window), but we only really need to cast
	// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
	// (in which case we know it is a RenderTexture)
	SPtr<const RenderTexture> renderTexture = std::static_pointer_cast<const RenderTexture>(target);

	auto iterFind = mInputBridge.find(renderTexture);
	if(iterFind != mInputBridge.end())
	{
		GUIWidget* parentWidget = iterFind->second->GetParentWidget();
		if(parentWidget == nullptr)
			return nullptr;

		if(parentWidget != &widget)
			return GetWidgetWindow(*parentWidget);
	}

	Vector<RenderWindow*> renderWindows = RenderWindowManager::Instance().GetRenderWindows();

	auto iterFindWin = std::find(renderWindows.begin(), renderWindows.end(), target.get());
	if(iterFindWin != renderWindows.end())
		return static_cast<RenderWindow*>(target.get());

	return nullptr;
}

SPtr<RenderWindow> GUIManager::GetBridgeWindow(const SPtr<RenderTexture>& target) const
{
	if(target == nullptr)
		return nullptr;

	while(true)
	{
		auto iterFind = mInputBridge.find(target);
		if(iterFind == mInputBridge.end())
			return nullptr;

		GUIWidget* parentWidget = iterFind->second->GetParentWidget();
		if(parentWidget == nullptr)
			return nullptr;

		SPtr<RenderTarget> curTarget = parentWidget->GetTarget()->GetTarget();
		if(curTarget == nullptr)
			return nullptr;

		if(curTarget == target)
			return nullptr;

		if(curTarget->GetProperties().IsWindow)
			return std::static_pointer_cast<RenderWindow>(curTarget);
	}

	return nullptr;
}

void GUIManager::GetBridgedElements(const GUIWidget* widget, TInlineArray<std::pair<const GUIInteractable*, SPtr<const RenderTarget>>, 4>& elements)
{
	if(widget == nullptr)
		return;

	for(auto& entry : mInputBridge)
	{
		const GUIInteractable* element = entry.second;
		GUIWidget* parentWidget = element->GetParentWidget();
		if(parentWidget == widget)
			elements.Add(std::make_pair(element, entry.first));
	}
}

void GUIManager::TabFocusFirst()
{
	u32 nearestDist = std::numeric_limits<u32>::max();
	GUIInteractable* closestElement = nullptr;

	// Find to top-left most element
	for(auto& widgetInfo : mWidgets)
	{
		const RenderWindow* window = GetWidgetWindow(*widgetInfo.Widget);

		const Vector<GUIRenderable*>& elements = widgetInfo.Widget->GetElements();
		for(auto& element : elements)
		{
			GUIInteractable* const interactableElement = B3DRTTICast<GUIInteractable>(element);
			if(!interactableElement)
				continue;

			const bool acceptsKeyFocus = interactableElement->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
			if(!acceptsKeyFocus || element->IsDisabled() || element->IsHidden())
				continue;

			const GUIPhysicalArea elemBounds = element->GetAbsoluteClippedArea();
			const bool isFullyClipped = elemBounds.Width == 0 || elemBounds.Height == 0;

			if(isFullyClipped)
				continue;

			GUIPhysicalPoint elementPos = elemBounds.GetPosition();
			Vector2I screenPos = window->WindowToScreenPosition(elementPos.To<i32>());

			const u32 dist = screenPos.SquaredLength();
			if(dist < nearestDist)
			{
				nearestDist = dist;
				closestElement = interactableElement;
			}
		}
	}

	if(closestElement == nullptr)
		return;

	// Don't use the element directly though, since its tab group could have explicit ordering
	const SPtr<GUINavGroup>& navGroup = closestElement->GetNavigationGroup();
	navGroup->FocusFirst();
}

void GUIManager::TabFocusNext()
{
	for(auto& entry : mElementsInFocus)
	{
		const SPtr<GUINavGroup>& navGroup = entry.Element->GetNavigationGroup();
		GUIElementOptions elementOptions = entry.Element->GetOptionFlags();
		if(elementOptions.IsSet(GUIElementOption::AcceptsKeyFocus) && navGroup != nullptr)
		{
			navGroup->FocusNext(entry.Element);
			return;
		}
	}

	// Nothing currently in focus
	TabFocusFirst();
}

bool GUIManager::SendMouseEvent(GUIInteractable* element, const GUIMouseEvent& event)
{
	if(element->IsPendingDestroy())
		return false;

	return element->DoOnMouseEvent(event);
}

bool GUIManager::SendTextInputEvent(GUIInteractable* element, const GUITextInputEvent& event)
{
	if(element->IsPendingDestroy())
		return false;

	return element->DoOnTextInputEvent(event);
}

bool GUIManager::SendCommandEvent(GUIInteractable* element, const GUICommandEvent& event)
{
	if(element->IsPendingDestroy())
		return false;

	return element->DoOnCommandEvent(event);
}

bool GUIManager::SendVirtualButtonEvent(GUIInteractable* element, const GUIVirtualButtonEvent& event)
{
	if(element->IsPendingDestroy())
		return false;

	return element->DoOnVirtualButtonEvent(event);
}

namespace b3d
{
	struct GpuBackendConventions;

	GUIManager& GetGUIManager()
{
	return GUIManager::Instance();
}
} // namespace b3d

namespace b3d { namespace render
{
	/** If enabled, regions of the GUI that are being redrawn will be drawn in a special debug material so they are easily noticeable. */
	static constexpr bool kEnableGUIRegionDebugDrawing = false;

	/** If true, all draw regions will be redrawn every frame, regardless if dirty or not. */
	static constexpr bool kRedrawAllRegions = false;

GUIRenderer::GUIRenderer()
	: RendererExtension(RenderLocation::Overlay, 10)
{}

void GUIRenderer::Initialize(const Any& data)
{
	const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

	SamplerStateInformation ssDesc;
	ssDesc.MagFilter = FO_POINT;
	ssDesc.MinFilter = FO_POINT;
	ssDesc.MipFilter = FO_POINT;

	mSamplerState = gpuDevice->FindOrCreateSamplerState(ssDesc);
}

RendererExtensionRequest GUIRenderer::Check(const Camera& camera)
{
	auto iterFind = mPerCameraData.find(&camera);
	if(iterFind == mPerCameraData.end())
		return RendererExtensionRequest::DontRender;

	GUICameraRenderData& cameraRenderData = iterFind->second;
	Vector<GUIWidgetRenderData>& widgetRenderData = cameraRenderData.WidgetRenderData;
	bool needsRedraw = !cameraRenderData.DirtyRegions.empty() || !cameraRenderData.LastFrameDirtyDebugDrawRegions.empty();
	if(!needsRedraw)
	{
		for(auto& widget : widgetRenderData)
		{
			for(auto& drawGroup : widget.DrawGroups)
			{
				for(auto& renderTargetElem : drawGroup.RenderTargetElements)
				{
					if(renderTargetElem.LastUpdateCount != renderTargetElem.Target->GetUpdateCount())
					{
						needsRedraw = true;
						break;
					}
				}
			}
		}
	}

	return needsRedraw ? RendererExtensionRequest::ForceRender : RendererExtensionRequest::RenderIfTargetDirty;
}

void GUIRenderer::Render(const Camera& camera, const RendererViewContext& viewContext)
{
	// TODO - Sprite animation might be broken. I need to continually mark the animated region as dirty.

	FrameScope frameScope;
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	GUICameraRenderData& cameraRenderData = mPerCameraData[&camera];
	Vector<GUIWidgetRenderData>& widgetRenderData = cameraRenderData.WidgetRenderData;

	const bool viewflipYFlip = gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down;

	GpuCommandBuffer& commandBuffer = *viewContext.CommandBuffer;
	commandBuffer.BeginLabel("GUIRenderer::Render");

	// Re-create cached render texture if needed
	const SPtr<RenderTarget> renderTarget = viewContext.CurrentTarget;
	const u32 renderTargetWidth = renderTarget->GetProperties().Width;
	const u32 renderTargetHeight = renderTarget->GetProperties().Height;

	const bool rebuildCachedRenderTexture = cameraRenderData.CachedRenderTexture == nullptr ||
		cameraRenderData.CachedRenderTexture->GetProperties().Width != renderTargetWidth ||
		cameraRenderData.CachedRenderTexture->GetProperties().Height != renderTargetHeight;

	if(rebuildCachedRenderTexture)
	{
		TextureCreateInformation cachedColorTextureCreateInformation;
		cachedColorTextureCreateInformation.Width = renderTargetWidth;
		cachedColorTextureCreateInformation.Height = renderTargetHeight;
		cachedColorTextureCreateInformation.Format = PF_RGBA8;
		cachedColorTextureCreateInformation.Usage = TU_RENDERTARGET;

		const SPtr<Texture> cachedColorTexture = gpuDevice->CreateTexture(cachedColorTextureCreateInformation);

		RenderTextureCreateInformation cachedRenderTextureCreateInformation;
		cachedRenderTextureCreateInformation.ColorSurfaces[0].Texture = cachedColorTexture;

		cameraRenderData.CachedRenderTexture = RenderTexture::Create(cachedRenderTextureCreateInformation);
	}

	commandBuffer.SetRenderTarget(cameraRenderData.CachedRenderTexture, 0, RT_ALL);

	if(rebuildCachedRenderTexture)
		commandBuffer.ClearRenderTarget(FBT_COLOR, Color::kZero);

	for(auto& widget : widgetRenderData)
	{
		for(auto& drawGroup : widget.DrawGroups)
		{
			for(auto& renderTargetElem : drawGroup.RenderTargetElements)
			{
				if(renderTargetElem.LastUpdateCount != renderTargetElem.Target->GetUpdateCount())
				{
					renderTargetElem.LastUpdateCount = renderTargetElem.Target->GetUpdateCount();
					Area2I::AddUnique(renderTargetElem.Area, cameraRenderData.DirtyRegions);
				}
			}
		}
	}

	auto fnCreateClipRegionBuffer = [this, &gpuDevice](GUIWidgetRenderData& widgetRenderData, u32 bufferIndex, const FrameVector<Area2I>& clipRegions) -> SPtr<GpuBuffer>
	{
		struct ClipRegionArea
		{
			Vector2I TopLeft;
			Vector2I BottomRight;
		};

		const u32 clipRegionCount = (u32)clipRegions.size();
		const u32 requiredBufferSize = sizeof(ClipRegionArea) * clipRegionCount;
		SPtr<GpuBuffer> dirtyRegionBuffer = widgetRenderData.DirtyRegionBuffers[bufferIndex];
		if (dirtyRegionBuffer == nullptr || dirtyRegionBuffer->GetTotalSize()< requiredBufferSize)
		{
			GpuBufferInformation gpuBufferInformation;
			gpuBufferInformation.Type = GpuBufferType::StructuredStorage;
			gpuBufferInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
			gpuBufferInformation.StructuredStorage.Count = clipRegionCount;
			gpuBufferInformation.StructuredStorage.ElementSize = sizeof(ClipRegionArea);

			dirtyRegionBuffer = gpuDevice->CreateGpuBuffer(gpuBufferInformation);
			widgetRenderData.DirtyRegionBuffers[bufferIndex] = dirtyRegionBuffer;
		}

		ClipRegionArea* destination = reinterpret_cast<ClipRegionArea*>(dirtyRegionBuffer->Lock(GBL_WRITE_ONLY_DISCARD));

		for (u32 dirtyRegionIndex = 0; dirtyRegionIndex < clipRegionCount; ++dirtyRegionIndex)
		{
			const Area2I& dirtyRegion = clipRegions[dirtyRegionIndex];

			destination[dirtyRegionIndex].TopLeft = dirtyRegion.GetPosition();
			destination[dirtyRegionIndex].BottomRight = dirtyRegion.GetPosition() + Vector2I(dirtyRegion.GetSize().Width, dirtyRegion.GetSize().Height);
		}

		dirtyRegionBuffer->Unlock();

		return dirtyRegionBuffer;
	};

	auto fnDrawRegions = [this, &fnCreateClipRegionBuffer, &widgetRenderData, &commandBuffer, renderTargetWidth, renderTargetHeight, viewflipYFlip, rebuildCachedRenderTexture](const Vector<Area2I>& regions, bool useDebugMaterial)
	{
		const Area2I clipRectangle(0, 0, renderTargetWidth, renderTargetHeight);

		FrameVector<Area2I> clippedRegions;
		clippedRegions.reserve(regions.size());

		// Clear all regions
		for(Area2I region : regions)
		{
			region.Clip(clipRectangle);
			clippedRegions.push_back(region);

			const Area2 normalizedRegionArea =
				Area2(
					region.X / (float)renderTargetWidth,
					region.Y / (float)renderTargetHeight,
					region.Width / (float)renderTargetWidth,
					region.Height / (float)renderTargetHeight);

			// TODO - This could probably be more efficient by drawing N regions in a single draw call
			commandBuffer.SetViewport(normalizedRegionArea);

			if(!rebuildCachedRenderTexture)
				commandBuffer.ClearViewport(FBT_COLOR, Color::kZero);
		}

		commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

		const Vector2I viewportOffset(0, 0);
		const float inverseRegionWidth = 1.0f / (renderTargetWidth* 0.5f);
		const float inverseRegionHeight = 1.0f / (renderTargetHeight * 0.5f);

		struct MeshToDraw
		{
			const GUIMeshRenderData* RenderData = nullptr;
			FrameVector<Area2I> OverlappingRegions;
		};

		FrameVector<MeshToDraw> meshesToRedraw;
		for(GUIWidgetRenderData& widget : widgetRenderData)
		{
			meshesToRedraw.clear();

			for(auto drawGroupIterator = widget.DrawGroups.rbegin(); drawGroupIterator != widget.DrawGroups.rend(); ++drawGroupIterator)
			{
				const GUIBatchRenderData& drawGroup = *drawGroupIterator;

				// Check if the draw group overlaps any dirty regions, if not we can skip it
				bool isDrawGroupOverlappingAny = false;
				for (const Area2I& region : regions)
				{
					if (drawGroup.Bounds.Overlaps(region))
					{
						isDrawGroupOverlappingAny = true;
						break;
					}
				}

				if (!isDrawGroupOverlappingAny)
					continue;

				// Find exact elements of the draw group which overlap dirty regions and record those elements and their overlapping regions
				for(const GUIMeshRenderData& meshRenderData : drawGroup.Elements)
				{
					FrameVector<Area2I> overlappingDirtyRegions;
					for (const Area2I& region : clippedRegions)
					{
						if (meshRenderData.Bounds.Overlaps(region))
							overlappingDirtyRegions.push_back(region);
					}

					if(overlappingDirtyRegions.empty())
						continue;

					// Note: We will unnecessarily do this update multiple times if the same mesh overlaps multiple dirty regions
					const SPtr<GpuBuffer>& buffer = widget.GUIMeshUniformBuffers[meshRenderData.UniformBufferIndex];
					SpriteMaterial::PopulateUniformBuffer(buffer, viewportOffset, inverseRegionWidth, inverseRegionHeight, viewflipYFlip, mTime, (u32)overlappingDirtyRegions.size(), widget.WorldTransform, meshRenderData.MaterialInformation);

					meshesToRedraw.push_back({ &meshRenderData, std::move(overlappingDirtyRegions) });
				}
			}

			for(const MeshToDraw& meshToDraw : meshesToRedraw)
			{
				const GUIMeshRenderData* const meshRenderData = meshToDraw.RenderData;
				SpriteMaterial* const material = meshRenderData->Material;

				// Note: Sprite material is being re-bound for each draw. We should ensure the material is bound only when it actually changes.
				const SPtr<GpuBuffer>& uniformBuffer = widget.GUIMeshUniformBuffers[meshRenderData->UniformBufferIndex];
				const SPtr<GpuBuffer>& clipRegionBuffer = fnCreateClipRegionBuffer(widget, meshRenderData->UniformBufferIndex, meshToDraw.OverlappingRegions);

				if(!kEnableGUIRegionDebugDrawing || !useDebugMaterial)
				{
					material->Render(commandBuffer, meshRenderData->Mesh, meshRenderData->SubMesh, meshRenderData->MaterialInformation.Texture, mSamplerState, uniformBuffer, clipRegionBuffer, (u32)meshToDraw.OverlappingRegions.size(), meshRenderData->MaterialInformation.AdditionalData);
				}
				else
				{
					material->Render(commandBuffer, meshRenderData->Mesh, meshRenderData->SubMesh, Texture::kPink, mSamplerState, uniformBuffer, clipRegionBuffer, (u32)meshToDraw.OverlappingRegions.size(), meshRenderData->MaterialInformation.AdditionalData);
				}
			}
		}
	};

	if(!rebuildCachedRenderTexture && !kRedrawAllRegions)
	{
		fnDrawRegions(cameraRenderData.LastFrameDirtyDebugDrawRegions, false);
		fnDrawRegions(cameraRenderData.DirtyRegions, true);
	}
	else
	{
		fnDrawRegions({ Area2I(0, 0, renderTargetWidth, renderTargetHeight) }, true);
	}

	cameraRenderData.LastFrameDirtyDebugDrawRegions.clear();
	if(kEnableGUIRegionDebugDrawing)
		std::swap(cameraRenderData.DirtyRegions, cameraRenderData.LastFrameDirtyDebugDrawRegions);

	cameraRenderData.DirtyRegions.clear();

	// Blit cached texture into main output
	// Note: This could be optimized by blitting only the modified regions
	commandBuffer.SetRenderTarget(renderTarget, 0, RT_ALL);
	commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

	GetRendererUtility().Blend(commandBuffer, cameraRenderData.CachedRenderTexture->GetColorTexture(0), Area2I::kEmpty, false, false, true);

	// Restore original viewport
	commandBuffer.SetViewport(camera.GetViewport()->GetArea());
	commandBuffer.EndLabel();
}

void GUIRenderer::Update(float time)
{
	mTime = time;
}

void GUIRenderer::UpdateDrawGroups(const Camera* camera, u64 widgetId, u32 widgetDepth, const Matrix4& worldTransform, const GUIDrawGroupRenderDataUpdate& data)
{
	mWidgetToCameraMap[widgetId] = camera;

	GUICameraRenderData& cameraRenderData = mPerCameraData[camera];
	Vector<GUIWidgetRenderData>& widgets = cameraRenderData.WidgetRenderData;
	GUIWidgetRenderData* widget;

	auto found = std::find_if(widgets.begin(), widgets.end(), [widgetId](auto& x) { return x.WidgetId == widgetId; });
	if(found == widgets.end())
	{
		widgets.push_back(GUIWidgetRenderData());
		widget = &widgets.back();

		widget->WidgetId = widgetId;
	}
	else
		widget = &(*found);

	if(!data.NewBatches.empty())
	{
		widget->DrawGroups = data.NewBatches;

		// Allocate GPU buffers containing the material parameters
		u32 bufferCount = 0;
		for(auto& drawGroup : widget->DrawGroups)
			bufferCount += (u32)drawGroup.Elements.size();

		const u32 allocatedBufferCount = (u32)widget->GUIMeshUniformBuffers.size();
		if(bufferCount > allocatedBufferCount)
		{
			widget->GUIMeshUniformBuffers.resize(bufferCount);
			widget->DirtyRegionBuffers.resize(bufferCount);

			for(u32 i = allocatedBufferCount; i < bufferCount; i++)
				widget->GUIMeshUniformBuffers[i] = gGUISpriteUniformBufferDefinition.CreateBuffer();
		}

		u32 currentBufferIndex = 0;
		for(auto& drawGroup : widget->DrawGroups)
		{
			for(auto& entry : drawGroup.Elements)
				entry.UniformBufferIndex = currentBufferIndex++;
		}
	}

	widget->WorldTransform = worldTransform;

	for(const Area2I& dirtyRegion : data.DirtyRegions)
		Area2I::AddUnique(dirtyRegion, cameraRenderData.DirtyRegions);

	if(widget->WidgetDepth != widgetDepth)
	{
		widget->WidgetDepth = widgetDepth;

		std::sort(widgets.begin(), widgets.end(), [](auto& x, auto& y)
				  { return x.WidgetDepth > y.WidgetDepth; });
	}
}

void GUIRenderer::ClearDrawGroups(u64 widgetId)
{
	const Camera* camera = nullptr;
	if(auto found = mWidgetToCameraMap.find(widgetId); found != mWidgetToCameraMap.end())
		camera = found->second;

	auto iterFind = mPerCameraData.find(camera);
	if(iterFind == mPerCameraData.end())
		return;

	Vector<GUIWidgetRenderData>& widgetData = iterFind->second.WidgetRenderData;
	auto iterFind2 = std::find_if(widgetData.begin(), widgetData.end(), [widgetId](auto& x)
								  { return x.WidgetId == widgetId; });
	if(iterFind2 == widgetData.end())
		return;

	for(const auto& drawGroup : iterFind2->DrawGroups)
		Area2I::AddUnique(drawGroup.Bounds, iterFind->second.DirtyRegions);

	widgetData.erase(iterFind2);

	if(widgetData.empty())
		mPerCameraData.erase(iterFind);

	mWidgetToCameraMap.erase(widgetId);
}

}}
