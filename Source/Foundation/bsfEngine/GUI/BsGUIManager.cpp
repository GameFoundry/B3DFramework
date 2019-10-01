//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIManager.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIElement.h"
#include "Image/BsSpriteTexture.h"
#include "Utility/BsTime.h"
#include "Scene/BsSceneObject.h"
#include "Material/BsMaterial.h"
#include "Mesh/BsMeshData.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Mesh/BsMesh.h"
#include "Managers/BsRenderWindowManager.h"
#include "Platform/BsPlatform.h"
#include "Math/BsRect2I.h"
#include "BsCoreApplication.h"
#include "Error/BsException.h"
#include "Input/BsInput.h"
#include "GUI/BsGUIInputCaret.h"
#include "GUI/BsGUIInputSelection.h"
#include "GUI/BsGUIContextMenu.h"
#include "GUI/BsDragAndDropManager.h"
#include "GUI/BsGUIDropDownBoxManager.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUINavGroup.h"
#include "Profiling/BsProfilerCPU.h"
#include "Input/BsVirtualInput.h"
#include "Platform/BsCursor.h"
#include "CoreThread/BsCoreThread.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsCamera.h"
#include "GUI/BsGUITooltipManager.h"
#include "Renderer/BsRendererUtility.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsRenderTexture.h"
#include "RenderAPI/BsSamplerState.h"
#include "Managers/BsRenderStateManager.h"
#include "Resources/BsBuiltinResources.h"
#include "2D/BsSpriteManager.h"

using namespace std::placeholders;

namespace bs
{
	const UINT32 GUIManager::DRAG_DISTANCE = 3;
	const float GUIManager::TOOLTIP_HOVER_TIME = 1.0f;

	GUIManager::GUIManager()
	{
		// Note: Hidden dependency. GUI must receive input events before other systems, in order so it can mark them as used
		// if required. e.g. clicking on a context menu should mark the event as used so that other non-GUI systems know
		// that they probably should not process such event themselves.
		mOnPointerMovedConn = gInput().onPointerMoved.connect(std::bind(&GUIManager::onPointerMoved, this, _1));
		mOnPointerPressedConn = gInput().onPointerPressed.connect(std::bind(&GUIManager::onPointerPressed, this, _1));
		mOnPointerReleasedConn = gInput().onPointerReleased.connect(std::bind(&GUIManager::onPointerReleased, this, _1));
		mOnPointerDoubleClick = gInput().onPointerDoubleClick.connect(std::bind(&GUIManager::onPointerDoubleClick, this, _1));
		mOnTextInputConn = gInput().onCharInput.connect(std::bind(&GUIManager::onTextInput, this, _1));
		mOnInputCommandConn = gInput().onInputCommand.connect(std::bind(&GUIManager::onInputCommandEntered, this, _1));
		mOnVirtualButtonDown = VirtualInput::instance().onButtonDown.connect(std::bind(&GUIManager::onVirtualButtonDown, this, _1, _2));

		mWindowGainedFocusConn = RenderWindowManager::instance().onFocusGained.connect(std::bind(&GUIManager::onWindowFocusGained, this, _1));
		mWindowLostFocusConn = RenderWindowManager::instance().onFocusLost.connect(std::bind(&GUIManager::onWindowFocusLost, this, _1));
		mMouseLeftWindowConn = RenderWindowManager::instance().onMouseLeftWindow.connect(std::bind(&GUIManager::onMouseLeftWindow, this, _1));

		mInputCaret = bs_new<GUIInputCaret>();
		mInputSelection = bs_new<GUIInputSelection>();

		DragAndDropManager::startUp();
		mDragEndedConn = DragAndDropManager::instance().onDragEnded.connect(std::bind(&GUIManager::onMouseDragEnded, this, _1, _2));

		GUIDropDownBoxManager::startUp();
		GUITooltipManager::startUp();

		// Need to defer this call because I want to make sure all managers are initialized first
		deferredCall(std::bind(&GUIManager::updateCaretTexture, this));
		deferredCall(std::bind(&GUIManager::updateTextSelectionTexture, this));

		mRenderer = RendererExtension::create<ct::GUIRenderer>(nullptr);
	}

	GUIManager::~GUIManager()
	{
		GUITooltipManager::shutDown();
		GUIDropDownBoxManager::shutDown();
		DragAndDropManager::shutDown();

		// Make a copy of widgets, since destroying them will remove them from mWidgets and
		// we can't iterate over an array thats getting modified
		Vector<WidgetInfo> widgetCopy = mWidgets;
		for(auto& widget : widgetCopy)
			widget.widget->_destroy();

		// Ensure everything queued get destroyed
		processDestroyQueue();

		mOnPointerPressedConn.disconnect();
		mOnPointerReleasedConn.disconnect();
		mOnPointerMovedConn.disconnect();
		mOnPointerDoubleClick.disconnect();
		mOnTextInputConn.disconnect();
		mOnInputCommandConn.disconnect();
		mOnVirtualButtonDown.disconnect();

		mDragEndedConn.disconnect();

		mWindowGainedFocusConn.disconnect();
		mWindowLostFocusConn.disconnect();

		mMouseLeftWindowConn.disconnect();

		bs_delete(mInputCaret);
		bs_delete(mInputSelection);
	}

	void GUIManager::destroyCore(ct::GUIRenderer* core)
	{
		bs_delete(core);
	}

	void GUIManager::registerWidget(GUIWidget* widget)
	{
		const Viewport* renderTarget = widget->getTarget();
		if (renderTarget == nullptr)
			return;

		mWidgets.push_back(WidgetInfo(widget));
	}

	void GUIManager::unregisterWidget(GUIWidget* widget)
	{
		{
			auto findIter = std::find_if(begin(mWidgets), end(mWidgets), [=] (const WidgetInfo& x) { return x.widget == widget; } );

			if(findIter != mWidgets.end())
				mWidgets.erase(findIter);
		}

		for(auto& entry : mElementsInFocus)
		{
			if (entry.widget == widget)
				entry.widget = nullptr;
		}

		for(auto& elementsPerWindow : mSavedFocusElements)
		{
			for(auto& entry : elementsPerWindow.second)
			{
				if (entry.widget == widget)
					entry.widget = nullptr;
			}
		}

		for (auto& entry : mElementsUnderPointer)
		{
			if (entry.widget == widget)
				entry.widget = nullptr;
		}

		for (auto& entry : mActiveElements)
		{
			if (entry.widget == widget)
				entry.widget = nullptr;
		}

		SPtr<Camera> camera = widget->getCamera();
		if(camera != nullptr)
		{
			auto widgetId = (UINT64)widget;
			gCoreThread().queueCommand([
				renderer = mRenderer.get(),
				camera = camera->getCore(),
				widgetId]()
			{
				renderer->clearDrawGroups(camera, widgetId);
			});
		}
	}

	void GUIManager::update()
	{
		DragAndDropManager::instance()._update();

		// Show tooltip if needed
		if (mShowTooltip)
		{
			float diff = gTime().getTime() - mTooltipElementHoverStart;
			if (diff >= TOOLTIP_HOVER_TIME || gInput().isButtonHeld(BC_LCONTROL) || gInput().isButtonHeld(BC_RCONTROL))
			{
				for(auto& entry : mElementsUnderPointer)
				{
					const String& tooltipText = entry.element->_getTooltip();
					GUIWidget* parentWidget = entry.element->_getParentWidget();

					if (!tooltipText.empty() && parentWidget != nullptr)
					{
						const RenderWindow* window = getWidgetWindow(*parentWidget);
						if (window != nullptr)
						{
							Vector2I windowPos = window->screenToWindowPos(gInput().getPointerPosition());

							GUITooltipManager::instance().show(*parentWidget, windowPos, tooltipText);
							break;
						}
					}
				}

				mShowTooltip = false;
			}
		}

		// Update layouts
		gProfilerCPU().beginSample("UpdateLayout");
		for(auto& widgetInfo : mWidgets)
		{
			widgetInfo.widget->_updateLayout();
		}
		gProfilerCPU().endSample("UpdateLayout");

		// Destroy all queued elements (and loop in case any new ones get queued during destruction)
		do
		{
			mNewElementsUnderPointer.clear();
			for (auto& elementInfo : mElementsUnderPointer)
			{
				if (!elementInfo.element->_isDestroyed())
					mNewElementsUnderPointer.push_back(elementInfo);
			}

			mElementsUnderPointer.swap(mNewElementsUnderPointer);

			mNewActiveElements.clear();
			for (auto& elementInfo : mActiveElements)
			{
				if (!elementInfo.element->_isDestroyed())
					mNewActiveElements.push_back(elementInfo);
			}

			mActiveElements.swap(mNewActiveElements);

			mNewElementsInFocus.clear();

			for (auto& elementInfo : mElementsInFocus)
			{
				if (!elementInfo.element->_isDestroyed())
					mNewElementsInFocus.push_back(elementInfo);
			}

			mElementsInFocus.swap(mNewElementsInFocus);

			for (auto& elementsPerWindow : mSavedFocusElements)
			{
				mNewElementsInFocus.clear();
				for (auto& entry : elementsPerWindow.second)
				{
					if (!entry.element->_isDestroyed())
						mNewElementsInFocus.push_back(entry);
				}

				elementsPerWindow.second.swap(mNewElementsInFocus);
			}

			if(mForcedClearFocus)
			{
				// Clear focus on all elements that aren't part of the forced focus list (in case they are already in focus)
				mCommandEvent.setType(GUICommandEventType::FocusLost);

				for (auto iter = mElementsInFocus.begin(); iter != mElementsInFocus.end();)
				{
					const ElementFocusInfo& elementInfo = *iter;

					const auto iterFind = std::find_if(begin(mForcedFocusElements), end(mForcedFocusElements),
						[&elementInfo](const ElementForcedFocusInfo& x)
					{
						return x.focus && x.element == elementInfo.element;
					});

					if (iterFind == mForcedFocusElements.end())
					{
						sendCommandEvent(elementInfo.element, mCommandEvent);
						iter = mElementsInFocus.erase(iter);
					}
					else
						++iter;
				}

				mForcedClearFocus = false;
			}

			for (auto& focusElementInfo : mForcedFocusElements)
			{
				if (focusElementInfo.element->_isDestroyed())
					continue;

				const auto iterFind = std::find_if(mElementsInFocus.begin(), mElementsInFocus.end(),
					[&](const ElementFocusInfo& x) { return x.element == focusElementInfo.element; });

				if (focusElementInfo.focus)
				{
					// Gain focus unless already in focus
					if (iterFind == mElementsInFocus.end())
					{
						mElementsInFocus.push_back(ElementFocusInfo(focusElementInfo.element,
							focusElementInfo.element->_getParentWidget(), false));

						mCommandEvent = GUICommandEvent();
						mCommandEvent.setType(GUICommandEventType::FocusGained);

						sendCommandEvent(focusElementInfo.element, mCommandEvent);
					}
				}
				else
				{
					// Force clear focus
					if(iterFind != mElementsInFocus.end())
					{
						mCommandEvent = GUICommandEvent();
						mCommandEvent.setType(GUICommandEventType::FocusLost);

						sendCommandEvent(iterFind->element, mCommandEvent);
						bs_swap_and_erase(mElementsInFocus, iterFind);
					}
				}
			}

			mForcedFocusElements.clear();

		} while (processDestroyQueueIteration());

		// Blink caret
		float curTime = gTime().getTime();

		if ((curTime - mCaretLastBlinkTime) >= mCaretBlinkInterval)
		{
			mCaretLastBlinkTime = curTime;
			mIsCaretOn = !mIsCaretOn;

			mCommandEvent = GUICommandEvent();
			mCommandEvent.setType(GUICommandEventType::Redraw);

			for (auto& elementInfo : mElementsInFocus)
			{
				sendCommandEvent(elementInfo.element, mCommandEvent);
			}
		}

		// Update dirty widget render data
		for(auto& entry : mWidgets)
		{
			GUIWidget* widget = entry.widget;
			GUIDrawGroupRenderDataUpdate updateData = widget->rebuildDirtyRenderData();

			SPtr<Camera> camera;
			camera = widget->getCamera();
			if (camera == nullptr)
				continue;
			
			auto widgetId = (UINT64)widget;
			gCoreThread().queueCommand([
				renderer = mRenderer.get(),
				updateData = std::move(updateData),
				camera = camera->getCore(),
				widgetId,
				widgetDepth = widget->getDepth(),
				worldTransform = widget->getWorldTfrm()]()
			{
				renderer->updateDrawGroups(camera, widgetId, widgetDepth, worldTransform, updateData);
			});
		}

		gCoreThread().queueCommand([renderer = mRenderer.get(), time = gTime().getTime()](){ renderer->update(time); });
	}

	void GUIManager::processDestroyQueue()
	{
		// Loop until everything empties
		while (processDestroyQueueIteration())
		{ }
	}

	void GUIManager::updateCaretTexture()
	{
		if(mCaretTexture == nullptr)
		{
			TEXTURE_DESC texDesc; // Default

			HTexture newTex = Texture::create(texDesc);
			mCaretTexture = SpriteTexture::create(newTex);
		}

		const HTexture& tex = mCaretTexture->getTexture();
		SPtr<PixelData> data = tex->getProperties().allocBuffer(0, 0);

		data->setColorAt(mCaretColor, 0, 0);
		tex->writeData(data);
	}

	void GUIManager::updateTextSelectionTexture()
	{
		if(mTextSelectionTexture == nullptr)
		{
			TEXTURE_DESC texDesc; // Default

			HTexture newTex = Texture::create(texDesc);
			mTextSelectionTexture = SpriteTexture::create(newTex);
		}

		const HTexture& tex = mTextSelectionTexture->getTexture();
		SPtr<PixelData> data = tex->getProperties().allocBuffer(0, 0);

		data->setColorAt(mTextSelectionColor, 0, 0);
		tex->writeData(data);
	}

	void GUIManager::onMouseDragEnded(const PointerEvent& event, DragCallbackInfo& dragInfo)
	{
		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		if(DragAndDropManager::instance().isDragInProgress() && guiButton == GUIMouseButton::Left)
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				Vector2I localPos;

				if(elementInfo.widget != nullptr)
					localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

				bool acceptDrop = true;
				if(DragAndDropManager::instance().needsValidDropTarget())
				{
					acceptDrop = elementInfo.element->_acceptDragAndDrop(localPos, DragAndDropManager::instance().getDragTypeId());
				}

				if(acceptDrop)
				{
					mMouseEvent.setDragAndDropDroppedData(localPos, DragAndDropManager::instance().getDragTypeId(), DragAndDropManager::instance().getDragData());
					dragInfo.processed = sendMouseEvent(elementInfo.element, mMouseEvent);

					if(dragInfo.processed)
						return;
				}
			}
		}

		dragInfo.processed = false;
	}

	void GUIManager::onPointerMoved(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		if(mDragState == DragState::HeldWithoutDrag)
		{
			UINT32 dist = mLastPointerClickPos.manhattanDist(event.screenPos);

			if(dist > DRAG_DISTANCE)
			{
				for(auto& activeElement : mActiveElements)
				{
					Vector2I localPos = getWidgetRelativePos(activeElement.widget, event.screenPos);
					Vector2I localDragStartPos = getWidgetRelativePos(activeElement.widget, mLastPointerClickPos);

					mMouseEvent.setMouseDragStartData(localPos, localDragStartPos);
					if(sendMouseEvent(activeElement.element, mMouseEvent))
						event.markAsUsed();
				}

				mDragState = DragState::Dragging;
				mDragStartPos = event.screenPos;
			}
		}

		// If mouse is being held down send MouseDrag events
		if(mDragState == DragState::Dragging)
		{
			for(auto& activeElement : mActiveElements)
			{
				if(mLastPointerScreenPos != event.screenPos)
				{
					Vector2I localPos = getWidgetRelativePos(activeElement.widget, event.screenPos);

					mMouseEvent.setMouseDragData(localPos, event.screenPos - mDragStartPos);
					if(sendMouseEvent(activeElement.element, mMouseEvent))
						event.markAsUsed();
				}
			}

			mLastPointerScreenPos = event.screenPos;

			// Also if drag is in progress send DragAndDrop events
			if(DragAndDropManager::instance().isDragInProgress())
			{
				bool acceptDrop = true;
				for(auto& elementInfo : mElementsUnderPointer)
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

					acceptDrop = true;
					if(DragAndDropManager::instance().needsValidDropTarget())
					{
						acceptDrop = elementInfo.element->_acceptDragAndDrop(localPos, DragAndDropManager::instance().getDragTypeId());
					}

					if(acceptDrop)
					{
						mMouseEvent.setDragAndDropDraggedData(localPos, DragAndDropManager::instance().getDragTypeId(), DragAndDropManager::instance().getDragData());
						if(sendMouseEvent(elementInfo.element, mMouseEvent))
						{
							event.markAsUsed();
							break;
						}
					}
				}

				if(acceptDrop)
				{
					if(mActiveCursor != CursorType::ArrowDrag)
					{
						Cursor::instance().setCursor(CursorType::ArrowDrag);
						mActiveCursor = CursorType::ArrowDrag;
					}
				}
				else
				{
					if(mActiveCursor != CursorType::Deny)
					{
						Cursor::instance().setCursor(CursorType::Deny);
						mActiveCursor = CursorType::Deny;
					}
				}				
			}
		}
		else // Otherwise, send MouseMove events if we are hovering over any element
		{
			if(mLastPointerScreenPos != event.screenPos)
			{
				bool moveProcessed = false;
				bool hasCustomCursor = false;
				for(auto& elementInfo : mElementsUnderPointer)
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

					if(!moveProcessed)
					{
						// Send MouseMove event
						mMouseEvent.setMouseMoveData(localPos);
						moveProcessed = sendMouseEvent(elementInfo.element, mMouseEvent);

						if(moveProcessed)
							event.markAsUsed();
					}

					if (mDragState == DragState::NoDrag)
					{
						CursorType newCursor = CursorType::Arrow;
						if(elementInfo.element->_hasCustomCursor(localPos, newCursor))
						{
							if(newCursor != mActiveCursor)
							{
								Cursor::instance().setCursor(newCursor);
								mActiveCursor = newCursor;
							}

							hasCustomCursor = true;
						}
					}

					if(moveProcessed)
						break;
				}

				// While dragging we don't want to modify the cursor
				if (mDragState == DragState::NoDrag)
				{
					if (!hasCustomCursor)
					{
						if (mActiveCursor != CursorType::Arrow)
						{
							Cursor::instance().setCursor(CursorType::Arrow);
							mActiveCursor = CursorType::Arrow;
						}
					}
				}
			}

			mLastPointerScreenPos = event.screenPos;

			if(Math::abs(event.mouseWheelScrollAmount) > 0.00001f)
			{
				for(auto& elementInfo : mElementsUnderPointer)
				{
					mMouseEvent.setMouseWheelScrollData(event.mouseWheelScrollAmount);
					if(sendMouseEvent(elementInfo.element, mMouseEvent))
					{
						event.markAsUsed();
						break;
					}
				}
			}
		}
	}

	void GUIManager::onPointerReleased(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		mMouseEvent = GUIMouseEvent(buttonStates, event.shift, event.control, event.alt);

		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		// Send MouseUp event only if we are over the active element (we don't want to accidentally trigger other elements).
		// And only activate when a button that originally caused the active state is released, otherwise ignore it.
		if(mActiveMouseButton == guiButton)
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				auto iterFind2 = std::find_if(mActiveElements.begin(), mActiveElements.end(),
					[&](const ElementInfo& x) { return x.element == elementInfo.element; });

				if(iterFind2 != mActiveElements.end())
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);
					mMouseEvent.setMouseUpData(localPos, guiButton);

					if(sendMouseEvent(elementInfo.element, mMouseEvent))
					{
						event.markAsUsed();
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
					Vector2I localPos = getWidgetRelativePos(activeElement.widget, event.screenPos);

					mMouseEvent.setMouseDragEndData(localPos);
					if(sendMouseEvent(activeElement.element, mMouseEvent))
						event.markAsUsed();
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
			Cursor::instance().setCursor(CursorType::Arrow);
			mActiveCursor = CursorType::Arrow;
		}
	}

	void GUIManager::onPointerPressed(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		// Determine elements that gained focus
		mNewElementsInFocus.clear();

		mCommandEvent = GUICommandEvent();

		// Determine elements that gained focus
		for (auto& elementInfo : mElementsUnderPointer)
		{
			auto iterFind = std::find_if(begin(mElementsInFocus), end(mElementsInFocus),
				[=](const ElementFocusInfo& x) { return x.element == elementInfo.element; });

			if (iterFind == mElementsInFocus.end())
			{
				bool processed = !elementInfo.element->getOptionFlags().isSet(GUIElementOption::ClickThrough);
				mNewElementsInFocus.push_back(ElementFocusInfo(elementInfo.element, elementInfo.widget, processed));

				if (processed)
					break;
			}
			else
			{
				mNewElementsInFocus.push_back(*iterFind);

				if (iterFind->usesFocus)
					break;
			}
		}

		// Send focus loss events
		// Note: Focus loss must trigger before mouse press because things like input boxes often only confirm changes
		// made to them when focus is lost. So if the user is confirming some input via a press of the button focus loss
		// must trigger on the input box first to make sure its contents get saved.
		mCommandEvent.setType(GUICommandEventType::FocusLost);

		for (auto& elementInfo : mElementsInFocus)
		{
			auto iterFind = std::find_if(begin(mNewElementsInFocus), end(mNewElementsInFocus),
				[=](const ElementFocusInfo& x) { return x.element == elementInfo.element; });

			if (iterFind == mNewElementsInFocus.end())
				sendCommandEvent(elementInfo.element, mCommandEvent);
		}

		// Send focus gain events
		mCommandEvent.setType(GUICommandEventType::FocusGained);

		for(auto& elementInfo : mNewElementsInFocus)
		{
			auto iterFind = std::find_if(begin(mElementsInFocus), end(mElementsInFocus),
				[=](const ElementFocusInfo& x) { return x.element == elementInfo.element; });

			if (iterFind == mElementsInFocus.end())
				sendCommandEvent(elementInfo.element, mCommandEvent);
		}

		mElementsInFocus.swap(mNewElementsInFocus);

		// Send mouse press event
		mMouseEvent = GUIMouseEvent(buttonStates, event.shift, event.control, event.alt);
		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		// We only check for mouse down if mouse isn't already being held down, and we are hovering over an element
		if(mActiveElements.size() == 0)
		{
			mNewActiveElements.clear();
			for(auto& elementInfo : mElementsUnderPointer)
			{
				Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);
				mMouseEvent.setMouseDownData(localPos, guiButton);

				bool processed = sendMouseEvent(elementInfo.element, mMouseEvent);

				if(guiButton == GUIMouseButton::Left)
				{
					mDragState = DragState::HeldWithoutDrag;
					mLastPointerClickPos = event.screenPos;
				}

				mNewActiveElements.push_back(ElementInfo(elementInfo.element, elementInfo.widget));
				mActiveMouseButton = guiButton;

				if(processed)
				{
					event.markAsUsed();
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
				SPtr<GUIContextMenu> menu = elementInfo.element->_getContextMenu();

				if(menu != nullptr && elementInfo.widget != nullptr)
				{
					const RenderWindow* window = getWidgetWindow(*elementInfo.widget);
					if (window != nullptr)
					{
						Vector2I windowPos = window->screenToWindowPos(event.screenPos);

						menu->open(windowPos, *elementInfo.widget);
						event.markAsUsed();
						break;
					}
				}
			}
		}
	}

	void GUIManager::onPointerDoubleClick(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		mMouseEvent = GUIMouseEvent(buttonStates, event.shift, event.control, event.alt);

		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		// We only check for mouse down if we are hovering over an element
		for(auto& elementInfo : mElementsUnderPointer)
		{
			Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

			mMouseEvent.setMouseDoubleClickData(localPos, guiButton);
			if(sendMouseEvent(elementInfo.element, mMouseEvent))
			{
				event.markAsUsed();
				break;
			}
		}
	}

	void GUIManager::onInputCommandEntered(InputCommandType commandType)
	{
		if(mElementsInFocus.empty())
			return;

		hideTooltip();

		// Tabs are handled by the GUI manager itself, while other events are passed to GUI elements
		if(commandType == InputCommandType::Tab)
		{
			tabFocusNext();
			return;
		}

		mCommandEvent = GUICommandEvent();

		switch(commandType)
		{
		case InputCommandType::Backspace:
			mCommandEvent.setType(GUICommandEventType::Backspace);
			break;
		case InputCommandType::Delete:
			mCommandEvent.setType(GUICommandEventType::Delete);
			break;
		case InputCommandType::Return:
			mCommandEvent.setType(GUICommandEventType::Return);
			break;
		case InputCommandType::Confirm:
			mCommandEvent.setType(GUICommandEventType::Confirm);
			break;
		case InputCommandType::Escape:
			mCommandEvent.setType(GUICommandEventType::Escape);
			break;
		case InputCommandType::CursorMoveLeft:
			mCommandEvent.setType(GUICommandEventType::MoveLeft);
			break;
		case InputCommandType::CursorMoveRight:
			mCommandEvent.setType(GUICommandEventType::MoveRight);
			break;
		case InputCommandType::CursorMoveUp:
			mCommandEvent.setType(GUICommandEventType::MoveUp);
			break;
		case InputCommandType::CursorMoveDown:
			mCommandEvent.setType(GUICommandEventType::MoveDown);
			break;
		case InputCommandType::SelectLeft:
			mCommandEvent.setType(GUICommandEventType::SelectLeft);
			break;
		case InputCommandType::SelectRight:
			mCommandEvent.setType(GUICommandEventType::SelectRight);
			break;
		case InputCommandType::SelectUp:
			mCommandEvent.setType(GUICommandEventType::SelectUp);
			break;
		case InputCommandType::SelectDown:
			mCommandEvent.setType(GUICommandEventType::SelectDown);
			break;
		default:
			break;
		}

		for(auto& elementInfo : mElementsInFocus)
			sendCommandEvent(elementInfo.element, mCommandEvent);
	}

	void GUIManager::onVirtualButtonDown(const VirtualButton& button, UINT32 deviceIdx)
	{
		hideTooltip();
		mVirtualButtonEvent.setButton(button);
		
		for(auto& elementInFocus : mElementsInFocus)
		{
			bool processed = sendVirtualButtonEvent(elementInFocus.element, mVirtualButtonEvent);

			if(processed)
				break;
		}
	}

	bool GUIManager::findElementUnderPointer(const Vector2I& pointerScreenPos, bool buttonStates[3], bool shift, bool control, bool alt)
	{
		Vector<const RenderWindow*> widgetWindows;
		for(auto& widgetInfo : mWidgets)
			widgetWindows.push_back(getWidgetWindow(*widgetInfo.widget));

#if BS_DEBUG_MODE
		// Checks if all referenced windows actually exist
		Vector<RenderWindow*> activeWindows = RenderWindowManager::instance().getRenderWindows();
		for(auto& window : widgetWindows)
		{
			if(window == nullptr)
				continue;

			auto iterFind = std::find(begin(activeWindows), end(activeWindows), window);

			if(iterFind == activeWindows.end())
			{
				BS_EXCEPT(InternalErrorException, "GUI manager has a reference to a window that doesn't exist. \
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

		RenderWindow* topMostModal = RenderWindowManager::instance().getTopMostModal();
		for(auto& window : uniqueWindows)
		{
			if(Platform::isPointOverWindow(*window, pointerScreenPos))
			{
				// If there's a top most modal window, it needs to be this one, otherwise we ignore input to that window
				if(topMostModal == nullptr || window == topMostModal)
					windowUnderPointer = window;

				break;
			}
		}

		if(windowUnderPointer != nullptr)
		{
			Vector2I windowPos = windowUnderPointer->screenToWindowPos(pointerScreenPos);

			UINT32 widgetIdx = 0;
			for(auto& widgetInfo : mWidgets)
			{
				if(widgetWindows[widgetIdx] == nullptr)
				{
					widgetIdx++;
					continue;
				}

				GUIWidget* widget = widgetInfo.widget;
				if(widgetWindows[widgetIdx] == windowUnderPointer
					&& widget->inBounds(windowToBridgedCoords(widget->getTarget()->getTarget(), windowPos)))
				{
					const Vector<GUIElement*>& elements = widget->getElements();
					Vector2I localPos = getWidgetRelativePos(widget, pointerScreenPos);

					// Elements with lowest depth (most to the front) get handled first
					for(auto iter = elements.begin(); iter != elements.end(); ++iter)
					{
						GUIElement* element = *iter;

						if(element->_isVisible() && element->_isInBounds(localPos))
						{
							ElementInfoUnderPointer elementInfo(element, widget);

							auto iterFind = std::find_if(mElementsUnderPointer.begin(), mElementsUnderPointer.end(),
								[=](const ElementInfoUnderPointer& x) { return x.element == element; });

							if (iterFind != mElementsUnderPointer.end())
							{
								elementInfo.usesMouseOver = iterFind->usesMouseOver;
								elementInfo.receivedMouseOver = iterFind->receivedMouseOver;
							}

							mNewElementsUnderPointer.push_back(elementInfo);
						}
					}
				}

				widgetIdx++;
			}
		}

		std::sort(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(),
			[](const ElementInfoUnderPointer& a, const ElementInfoUnderPointer& b)
		{
			return a.element->_getDepth() < b.element->_getDepth();
		});

		// Send MouseOut and MouseOver events
		bool eventProcessed = false;

		for (auto& elementInfo : mNewElementsUnderPointer)
		{
			GUIElement* element = elementInfo.element;
			GUIWidget* widget = elementInfo.widget;

			if (elementInfo.receivedMouseOver)
			{
				elementInfo.isHovering = true;
				if (elementInfo.usesMouseOver)
					break;

				continue;
			}

			auto iterFind = std::find_if(mActiveElements.begin(), mActiveElements.end(),
				[&](const ElementInfo& x) { return x.element == element; });

			// Send MouseOver event
			if (mActiveElements.size() == 0 || iterFind != mActiveElements.end())
			{
				Vector2I localPos = getWidgetRelativePos(widget, pointerScreenPos);

				mMouseEvent = GUIMouseEvent(buttonStates, shift, control, alt);

				mMouseEvent.setMouseOverData(localPos);
				elementInfo.receivedMouseOver = true;
				elementInfo.isHovering = true;
				if (sendMouseEvent(element, mMouseEvent))
				{
					eventProcessed = true;
					elementInfo.usesMouseOver = true;
					break;
				}
			}
		}

		// Send DragAndDropLeft event - It is similar to MouseOut events but we send it to all
		// elements a user might hover over, while we send mouse over/out events only to active elements while dragging
		if (DragAndDropManager::instance().isDragInProgress())
		{
			for (auto& elementInfo : mElementsUnderPointer)
			{
				auto iterFind = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(),
					[=](const ElementInfoUnderPointer& x) { return x.element == elementInfo.element; });

				if (iterFind == mNewElementsUnderPointer.end())
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, pointerScreenPos);

					mMouseEvent.setDragAndDropLeftData(localPos, DragAndDropManager::instance().getDragTypeId(), DragAndDropManager::instance().getDragData());
					if (sendMouseEvent(elementInfo.element, mMouseEvent))
					{
						eventProcessed = true;
						break;
					}
				}
			}
		}

		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIElement* element = elementInfo.element;
			GUIWidget* widget = elementInfo.widget;

			auto iterFind = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(),
				[=](const ElementInfoUnderPointer& x) { return x.element == element; });

			if (!elementInfo.receivedMouseOver)
				continue;

			if (iterFind == mNewElementsUnderPointer.end() || !iterFind->isHovering)
			{
				auto iterFind2 = std::find_if(mActiveElements.begin(), mActiveElements.end(),
					[=](const ElementInfo& x) { return x.element == element; });

				// Send MouseOut event
				if(mActiveElements.size() == 0 || iterFind2 != mActiveElements.end())
				{
					Vector2I localPos = getWidgetRelativePos(widget, pointerScreenPos);

					mMouseEvent.setMouseOutData(localPos);
					if (sendMouseEvent(element, mMouseEvent))
					{
						eventProcessed = true;
						break;
					}
				}
			}
		}

		mElementsUnderPointer.swap(mNewElementsUnderPointer);

		// Tooltip
		hideTooltip();
		if (mElementsUnderPointer.size() > 0)
			mShowTooltip = true;

		mTooltipElementHoverStart = gTime().getTime();

		return eventProcessed;
	}

	void GUIManager::onTextInput(const TextInputEvent& event)
	{
		mTextInputEvent = GUITextInputEvent();
		mTextInputEvent.setData(event.textChar);

		for(auto& elementInFocus : mElementsInFocus)
		{
			if(sendTextInputEvent(elementInFocus.element, mTextInputEvent))
				event.markAsUsed();
		}
	}

	void GUIManager::onWindowFocusGained(RenderWindow& win)
	{
		for(auto& widgetInfo : mWidgets)
		{
			GUIWidget* widget = widgetInfo.widget;
			if(getWidgetWindow(*widget) == &win)
				widget->ownerWindowFocusChanged();
		}

		auto iterFind = mSavedFocusElements.find(&win);
		if(iterFind != mSavedFocusElements.end())
		{
			Vector<ElementFocusInfo>& savedFocusedElements = iterFind->second;
			
			mNewElementsInFocus.clear();
			for(auto& focusedElement : mElementsInFocus)
			{
				if (focusedElement.element->_isDestroyed())
					continue;

				auto iterFind2 = std::find_if(savedFocusedElements.begin(), savedFocusedElements.end(),
					[&focusedElement](const ElementFocusInfo& x)
					{
						return x.element == focusedElement.element;
					});

				if(iterFind2 == savedFocusedElements.end())
				{
					mCommandEvent = GUICommandEvent();
					mCommandEvent.setType(GUICommandEventType::FocusLost);

					sendCommandEvent(focusedElement.element, mCommandEvent);
					savedFocusedElements.push_back(focusedElement);
				}
				else
					mNewElementsInFocus.push_back(focusedElement);
			}

			mElementsInFocus.swap(mNewElementsInFocus);

			for(auto& entry : savedFocusedElements)
			{
				if (entry.element->_isDestroyed())
					continue;

				auto iterFind2 = std::find_if(mElementsInFocus.begin(), mElementsInFocus.end(),
					[&entry](const ElementFocusInfo& x)
					{
						return x.element == entry.element;
					});

				if (iterFind2 == mElementsInFocus.end())
				{
					mCommandEvent = GUICommandEvent();
					mCommandEvent.setType(GUICommandEventType::FocusGained);

					sendCommandEvent(entry.element, mCommandEvent);
					mElementsInFocus.push_back(entry);
				}
			}

			mSavedFocusElements.erase(iterFind);
		}
	}

	void GUIManager::onWindowFocusLost(RenderWindow& win)
	{
		for(auto& widgetInfo : mWidgets)
		{
			GUIWidget* widget = widgetInfo.widget;
			if(getWidgetWindow(*widget) == &win)
				widget->ownerWindowFocusChanged();
		}

		Vector<ElementFocusInfo>& savedFocusedElements = mSavedFocusElements[&win];
		savedFocusedElements.clear();
		
		mNewElementsInFocus.clear();
		for(auto& focusedElement : mElementsInFocus)
		{
			if (focusedElement.element->_isDestroyed())
				continue;

			if (focusedElement.widget != nullptr && getWidgetWindow(*focusedElement.widget) == &win)
			{
				mCommandEvent = GUICommandEvent();
				mCommandEvent.setType(GUICommandEventType::FocusLost);

				sendCommandEvent(focusedElement.element, mCommandEvent);
				savedFocusedElements.push_back(focusedElement);
			}
			else
				mNewElementsInFocus.push_back(focusedElement);
		}

		mElementsInFocus.swap(mNewElementsInFocus);
	}

	// We stop getting mouse move events once it leaves the window, so make sure
	// nothing stays in hover state
	void GUIManager::onMouseLeftWindow(RenderWindow& win)
	{
		mNewElementsUnderPointer.clear();

		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIElement* element = elementInfo.element;
			GUIWidget* widget = elementInfo.widget;

			if (widget != nullptr && widget->getTarget()->getTarget().get() != &win)
			{
				mNewElementsUnderPointer.push_back(elementInfo);
				continue;
			}

			auto iterFind = std::find_if(mActiveElements.begin(), mActiveElements.end(),
				[&](const ElementInfo& x) { return x.element == element; });

			// Send MouseOut event
			if(mActiveElements.size() == 0 || iterFind != mActiveElements.end())
			{
				Vector2I localPos = getWidgetRelativePos(widget, Vector2I());

				mMouseEvent.setMouseOutData(localPos);
				sendMouseEvent(element, mMouseEvent);
			}
		}

		mElementsUnderPointer.swap(mNewElementsUnderPointer);

		hideTooltip();
		if(mDragState != DragState::Dragging)
		{
			if(mActiveCursor != CursorType::Arrow)
			{
				Cursor::instance().setCursor(CursorType::Arrow);
				mActiveCursor = CursorType::Arrow;
			}
		}
	}
	
	void GUIManager::hideTooltip()
	{
		GUITooltipManager::instance().hide();
		mShowTooltip = false;
	}

	void GUIManager::queueForDestroy(GUIElement* element)
	{
		mScheduledForDestruction.push(element);
	}

	void GUIManager::setFocus(GUIElement* element, bool focus, bool clear)
	{
		ElementForcedFocusInfo efi;
		efi.element = element;
		efi.focus = focus;

		if(clear)
		{
			mForcedClearFocus = true;
			mForcedFocusElements.clear();
		}

		mForcedFocusElements.push_back(efi);
	}

	bool GUIManager::processDestroyQueueIteration()
	{
		Stack<GUIElement*> toDestroy = mScheduledForDestruction;
		mScheduledForDestruction = Stack<GUIElement*>();

		while (!toDestroy.empty())
		{
			bs_delete(toDestroy.top());
			toDestroy.pop();
		}

		return !mScheduledForDestruction.empty();
	}

	void GUIManager::setInputBridge(const SPtr<RenderTexture>& renderTex, const GUIElement* element)
	{
		if (element == nullptr)
			mInputBridge.erase(renderTex);
		else
			mInputBridge[renderTex] = element;
	}

	GUIMouseButton GUIManager::buttonToGUIButton(PointerEventButton pointerButton) const
	{
		if(pointerButton == PointerEventButton::Left)
			return GUIMouseButton::Left;
		else if(pointerButton == PointerEventButton::Middle)
			return GUIMouseButton::Middle;
		else if(pointerButton == PointerEventButton::Right)
			return GUIMouseButton::Right;

		BS_EXCEPT(InvalidParametersException, "Provided button is not a GUI supported mouse button.");
		return GUIMouseButton::Left;
	}

	Vector2I GUIManager::getWidgetRelativePos(const GUIWidget* widget, const Vector2I& screenPos) const
	{
		if (widget == nullptr)
			return screenPos;

		const RenderWindow* window = getWidgetWindow(*widget);
		if(window == nullptr)
			return Vector2I();

		Vector2I windowPos = window->screenToWindowPos(screenPos);
		windowPos = windowToBridgedCoords(widget->getTarget()->getTarget(), windowPos);

		const Matrix4& worldTfrm = widget->getWorldTfrm();

		Vector4 vecLocalPos = worldTfrm.inverse().multiplyAffine(Vector4((float)windowPos.x, (float)windowPos.y, 0.0f, 1.0f));
		Vector2I curLocalPos(Math::roundToInt(vecLocalPos.x), Math::roundToInt(vecLocalPos.y));

		return curLocalPos;
	}

	Vector2I GUIManager::windowToBridgedCoords(const SPtr<RenderTarget>& target, const Vector2I& windowPos) const
	{
		// This cast might not be valid (the render target could be a window), but we only really need to cast
		// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
		// (in which case we know it is a RenderTexture)
		SPtr<const RenderTexture> renderTexture = std::static_pointer_cast<const RenderTexture>(target);
		const RenderTargetProperties& rtProps = renderTexture->getProperties();

		auto iterFind = mInputBridge.find(renderTexture);
		if(iterFind != mInputBridge.end()) // Widget input is bridged, which means we need to transform the coordinates
		{
			const GUIElement* bridgeElement = iterFind->second;
			const GUIWidget* parentWidget = bridgeElement->_getParentWidget();
			if (parentWidget == nullptr)
				return windowPos;

			const Matrix4& worldTfrm = parentWidget->getWorldTfrm();

			Vector4 vecLocalPos = worldTfrm.inverse().multiplyAffine(Vector4((float)windowPos.x, (float)windowPos.y, 0.0f, 1.0f));
			Rect2I bridgeBounds = bridgeElement->_getLayoutData().area;

			// Find coordinates relative to the bridge element
			float x = vecLocalPos.x - (float)bridgeBounds.x;
			float y = vecLocalPos.y - (float)bridgeBounds.y;

			float scaleX = bridgeBounds.width > 0 ? rtProps.width / (float)bridgeBounds.width : 0.0f;
			float scaleY = bridgeBounds.height > 0 ? rtProps.height / (float)bridgeBounds.height : 0.0f;

			return Vector2I(Math::roundToInt(x * scaleX), Math::roundToInt(y * scaleY));
		}

		return windowPos;
	}

	const RenderWindow* GUIManager::getWidgetWindow(const GUIWidget& widget) const
	{
		const Viewport* viewport = widget.getTarget();
		if (viewport == nullptr)
			return nullptr;

		SPtr<RenderTarget> target = viewport->getTarget();
		if (target == nullptr)
			return nullptr;

		// This cast might not be valid (the render target could be a window), but we only really need to cast
		// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
		// (in which case we know it is a RenderTexture)
		SPtr<const RenderTexture> renderTexture = std::static_pointer_cast<const RenderTexture>(target);

		auto iterFind = mInputBridge.find(renderTexture);
		if(iterFind != mInputBridge.end())
		{
			GUIWidget* parentWidget = iterFind->second->_getParentWidget();
			if (parentWidget == nullptr)
				return nullptr;

			if(parentWidget != &widget)
				return getWidgetWindow(*parentWidget);
		}

		Vector<RenderWindow*> renderWindows = RenderWindowManager::instance().getRenderWindows();

		auto iterFindWin = std::find(renderWindows.begin(), renderWindows.end(), target.get());
		if(iterFindWin != renderWindows.end())
			return static_cast<RenderWindow*>(target.get());

		return nullptr;
	}

	SPtr<RenderWindow> GUIManager::getBridgeWindow(const SPtr<RenderTexture>& target) const
	{
		if (target == nullptr)
			return nullptr;

		while (true)
		{
			auto iterFind = mInputBridge.find(target);
			if (iterFind == mInputBridge.end())
				return nullptr;

			GUIWidget* parentWidget = iterFind->second->_getParentWidget();
			if (parentWidget == nullptr)
				return nullptr;

			SPtr<RenderTarget> curTarget = parentWidget->getTarget()->getTarget();
			if (curTarget == nullptr)
				return nullptr;

			if (curTarget == target)
				return nullptr;

			if (curTarget->getProperties().isWindow)
				return std::static_pointer_cast<RenderWindow>(curTarget);
		}

		return nullptr;
	}

	void GUIManager::getBridgedElements(const GUIWidget* widget,
		SmallVector<std::pair<const GUIElement*, SPtr<const RenderTarget>>, 4>& elements)
	{
		if (widget == nullptr)
			return;

		for(auto& entry : mInputBridge)
		{
			const GUIElement* element = entry.second;
			GUIWidget* parentWidget = element->_getParentWidget();
			if (parentWidget == widget)
				elements.add(std::make_pair(element, entry.first));
		}
	}

	void GUIManager::tabFocusFirst()
	{
		UINT32 nearestDist = std::numeric_limits<UINT32>::max();
		GUIElement* closestElement = nullptr;

		// Find to top-left most element
		for (auto& widgetInfo : mWidgets)
		{
			const RenderWindow* window = getWidgetWindow(*widgetInfo.widget);

			const Vector<GUIElement*>& elements = widgetInfo.widget->getElements();
			for (auto& element : elements)
			{
				const bool acceptsKeyFocus = element->getOptionFlags().isSet(GUIElementOption::AcceptsKeyFocus);
				if (!acceptsKeyFocus || element->_isDisabled() || !element->_isVisible())
					continue;

				const Rect2I elemBounds = element->_getClippedBounds();
				const bool isFullyClipped = element->_getClippedBounds().width == 0 ||
					element->_getClippedBounds().height == 0;

				if (isFullyClipped)
					continue;

				Vector2I elementPos(elemBounds.x, elemBounds.y);
				Vector2I screenPos = window->windowToScreenPos(elementPos);

				const UINT32 dist = screenPos.squaredLength();
				if (dist < nearestDist)
				{
					nearestDist = dist;
					closestElement = element;
				}
			}
		}

		if (closestElement == nullptr)
			return;

		// Don't use the element directly though, since its tab group could have explicit ordering
		const SPtr<GUINavGroup>& navGroup = closestElement->_getNavGroup();
		navGroup->focusFirst();
	}

	void GUIManager::tabFocusNext()
	{
		for(auto& entry : mElementsInFocus)
		{
			const SPtr<GUINavGroup>& navGroup = entry.element->_getNavGroup();
			GUIElementOptions elementOptions = entry.element->getOptionFlags();
			if(elementOptions.isSet(GUIElementOption::AcceptsKeyFocus) && navGroup != nullptr)
			{
				navGroup->focusNext(entry.element);
				return;
			}
		}

		// Nothing currently in focus
		tabFocusFirst();
	}

	bool GUIManager::sendMouseEvent(GUIElement* element, const GUIMouseEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_mouseEvent(event);
	}

	bool GUIManager::sendTextInputEvent(GUIElement* element, const GUITextInputEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_textInputEvent(event);
	}

	bool GUIManager::sendCommandEvent(GUIElement* element, const GUICommandEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_commandEvent(event);
	}

	bool GUIManager::sendVirtualButtonEvent(GUIElement* element, const GUIVirtualButtonEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_virtualButtonEvent(event);
	}

	GUIManager& gGUIManager()
	{
		return GUIManager::instance();
	}

	namespace ct
	{
	GUISpriteParamBlockDef gGUISpriteParamBlockDef;

	GUIRenderer::GUIRenderer()
		:RendererExtension(RenderLocation::Overlay, 10)
	{ }

	void GUIRenderer::initialize(const Any& data)
	{
		SAMPLER_STATE_DESC ssDesc;
		ssDesc.magFilter = FO_POINT;
		ssDesc.minFilter = FO_POINT;
		ssDesc.mipFilter = FO_POINT;

		mSamplerState = RenderStateManager::instance().createSamplerState(ssDesc);
	}

	RendererExtensionRequest GUIRenderer::check(const Camera& camera)
	{
		auto iterFind = mPerCameraData.find(&camera);
		if (iterFind == mPerCameraData.end())
			return RendererExtensionRequest::DontRender;

		Vector<GUIWidgetRenderData>& widgetRenderData = iterFind->second;
		bool needsRedraw = false;
		for (auto& widget : widgetRenderData)
		{
			for (auto& drawGroup : widget.drawGroups)
			{
				if (!drawGroup.nonCachedElements.empty() || drawGroup.requiresRedraw)
				{
					needsRedraw = true;
					break;
				}

				for(auto& renderTargetElem : drawGroup.renderTargetElements)
				{
					if(renderTargetElem.lastUpdateCount != renderTargetElem.target->getUpdateCount())
					{
						needsRedraw = true;
						break;
					}
				}
			}
		}

		return needsRedraw ? RendererExtensionRequest::ForceRender : RendererExtensionRequest::RenderIfTargetDirty;
	}

	void GUIRenderer::render(const Camera& camera, const RendererViewContext& viewContext)
	{
		Vector<GUIWidgetRenderData>& widgetRenderData = mPerCameraData[&camera];

		float invViewportWidth = 1.0f / (camera.getViewport()->getPixelArea().width * 0.5f);
		float invViewportHeight = 1.0f / (camera.getViewport()->getPixelArea().height * 0.5f);
		bool viewflipYFlip = gCaps().conventions.ndcYAxis == Conventions::Axis::Down;

		RenderAPI& rapi = RenderAPI::instance();
		for (auto& widget : widgetRenderData)
		{
			for(auto& drawGroup : widget.drawGroups)
			{
				for (auto& entry : drawGroup.nonCachedElements)
				{
					const SPtr<GpuParamBlockBuffer>& buffer = widget.paramBlocks[entry.bufferIdx];
					updateParamBlockBuffer(buffer, invViewportWidth, invViewportHeight,
						viewflipYFlip, widget.worldTransform, entry);
				}

				for(auto& renderTargetElem : drawGroup.renderTargetElements)
				{
					if(renderTargetElem.lastUpdateCount != renderTargetElem.target->getUpdateCount())
					{
						renderTargetElem.lastUpdateCount = renderTargetElem.target->getUpdateCount();
						drawGroup.requiresRedraw = true;
					}
				}
				
				if (!drawGroup.requiresRedraw)
					continue;

				float invDrawGroupWidth = 1.0f / (drawGroup.bounds.width * 0.5f);
				float invDrawGroupHeight = 1.0f / (drawGroup.bounds.height * 0.5f);
				
				for(auto& entry : drawGroup.cachedElements)
				{
					const SPtr<GpuParamBlockBuffer>& buffer = widget.paramBlocks[entry.bufferIdx];
					updateParamBlockBuffer(buffer, invDrawGroupWidth, invDrawGroupHeight,
						viewflipYFlip, Matrix4::IDENTITY, entry);
				}

				// Update draw group param buffer
				{
					const SPtr<GpuParamBlockBuffer>& buffer = widget.paramBlocks[drawGroup.bufferIdx];

					gGUISpriteParamBlockDef.gTint.set(buffer, Color::White);
					gGUISpriteParamBlockDef.gWorldTransform.set(buffer, widget.worldTransform);
					gGUISpriteParamBlockDef.gInvViewportWidth.set(buffer, invViewportWidth);
					gGUISpriteParamBlockDef.gInvViewportHeight.set(buffer, invViewportHeight);
					gGUISpriteParamBlockDef.gViewportYFlip.set(buffer, viewflipYFlip ? -1.0f : 1.0f);
					gGUISpriteParamBlockDef.gUVSizeOffset.set(buffer, Vector4(1.0f, 1.0f, 0.0f, 0.0f));

					buffer->flushToGPU();
				}

				// We need to write the alpha of the first (deepest) element for each pixel

				// Note: We should cache/pool the stencil buffer texture (ideally just re-use a single one)
				//  - If pooling we probably need to round draw group sizes to certain pow-2 sizes and then
				//    use viewport to render to relevant bits
				SPtr<Texture> colorTex = drawGroup.destination->getColorTexture(0);
				const TextureProperties& colorProps = colorTex->getProperties();
				
				TEXTURE_DESC texDesc;
				texDesc.width = colorProps.getWidth();
				texDesc.height = colorProps.getHeight();
				texDesc.format = PF_D24S8; // TODO: Can we create a stencil only texture here?
				texDesc.usage = TU_DEPTHSTENCIL;

				SPtr<Texture> stencilTexture = Texture::create(texDesc);

				RENDER_TEXTURE_DESC rtDesc;
				rtDesc.colorSurfaces[0].texture = colorTex;
				rtDesc.depthStencilSurface.texture = stencilTexture;

				// Draw the alpha only first
				// Note: Can we avoid drawing each element twice?
				SPtr<RenderTexture> alphaRenderTexture = RenderTexture::create(rtDesc);
				rapi.setRenderTarget(alphaRenderTexture);
				rapi.clearRenderTarget(FBT_COLOR | FBT_STENCIL, Color::ZERO, 1.0f, 0);

				for (auto& entry : drawGroup.cachedElements)
				{
					// TODO - I shouldn't be re-applying the entire material for each entry, instead just check which programs
					// changed, and apply only those + the modified constant buffers and/or texture.

					const SPtr<GpuParamBlockBuffer>& buffer = widget.paramBlocks[entry.bufferIdx];
					entry.material->render(entry.isLine ? widget.lineMesh : widget.triangleMesh, entry.subMesh, entry.texture,
						mSamplerState, buffer, entry.additionalData, true);
				}

				// Draw the color values
				rapi.setRenderTarget(drawGroup.destination);

				for (auto& entry : drawGroup.cachedElements)
				{
					// TODO - I shouldn't be re-applying the entire material for each entry, instead just check which programs
					// changed, and apply only those + the modified constant buffers and/or texture.

					const SPtr<GpuParamBlockBuffer>& buffer = widget.paramBlocks[entry.bufferIdx];
					entry.material->render(entry.isLine ? widget.lineMesh : widget.triangleMesh, entry.subMesh, entry.texture,
						mSamplerState, buffer, entry.additionalData, false);
				}

				drawGroup.requiresRedraw = false;
			}
		}

		// Restore original render target
		rapi.setRenderTarget(viewContext.currentTarget);

		for (auto& widget : widgetRenderData)
		{
			for (auto& drawGroup : widget.drawGroups)
			{
				// Draw non-cached elements
				for (auto& entry : drawGroup.nonCachedElements)
				{
					// TODO - I shouldn't be re-applying the entire material for each entry, instead just check which programs
					// changed, and apply only those + the modified constant buffers and/or texture.

					const SPtr<GpuParamBlockBuffer>& buffer = widget.paramBlocks[entry.bufferIdx];
					entry.material->render(entry.isLine ? widget.lineMesh : widget.triangleMesh, entry.subMesh, entry.texture,
						mSamplerState, buffer, entry.additionalData, false);
				}

				// Draw the group itself
				const SPtr<GpuParamBlockBuffer>& buffer = widget.paramBlocks[drawGroup.bufferIdx];

				SpriteMaterial* batchedMat = SpriteManager::instance().getImageMaterial(SpriteMaterialTransparency::Premultiplied, false);
				batchedMat->render(widget.drawGroupMesh, drawGroup.subMesh, drawGroup.destination->getColorTexture(0),
					mSamplerState, buffer, nullptr, false);
			}
		}
	}

	void GUIRenderer::update(float time)
	{
		mTime = time;
	}

	void GUIRenderer::updateDrawGroups(const SPtr<Camera>& camera, UINT64 widgetId, UINT32 widgetDepth, const Matrix4& worldTransform,
		 const GUIDrawGroupRenderDataUpdate& data)
	{
		auto iterFind = mPerCameraData.find(camera.get());
		if (iterFind == mPerCameraData.end())
			mReferencedCameras.insert(camera);

		Vector<GUIWidgetRenderData>& widgets = mPerCameraData[camera.get()];
		GUIWidgetRenderData* widget;

		auto iterFind2 = std::find_if(widgets.begin(), widgets.end(), [widgetId](auto& x) { return x.widgetId == widgetId; });
		if (iterFind2 == widgets.end())
		{
			widgets.push_back(GUIWidgetRenderData());
			widget = &widgets.back();

			widget->widgetId = widgetId;
		}
		else
			widget = &(*iterFind2);

		if(!data.newDrawGroups.empty())
		{
			widget->drawGroups = data.newDrawGroups;

			// Allocate GPU buffers containing the material parameters
			UINT32 numBuffers = (UINT32)widget->drawGroups.size();
			for (auto& drawGroup : widget->drawGroups)
				numBuffers += (UINT32)drawGroup.nonCachedElements.size() + (UINT32)drawGroup.cachedElements.size();

			auto numAllocatedBuffers = (UINT32)widget->paramBlocks.size();
			if (numBuffers > numAllocatedBuffers)
			{
				widget->paramBlocks.resize(numBuffers);

				for (UINT32 i = numAllocatedBuffers; i < numBuffers; i++)
					widget->paramBlocks[i] = gGUISpriteParamBlockDef.createBuffer();
			}

			UINT32 curBufferIdx = 0;
			for (auto& drawGroup : widget->drawGroups)
			{
				drawGroup.bufferIdx = curBufferIdx++;
				
				for (auto& entry : drawGroup.nonCachedElements)
					entry.bufferIdx = curBufferIdx++;

				for (auto& entry : drawGroup.cachedElements)
					entry.bufferIdx = curBufferIdx++;
			}

			// Rebuild draw group mesh
			auto numQuads = (UINT32)widget->drawGroups.size();
			if (numQuads > 0)
			{
				bool flipUVY = gCaps().conventions.uvYAxis == Conventions::Axis::Up;
				float uvTop = flipUVY ? 1.0f : 0.0f;
				float uvBottom = flipUVY ? 0.0f : 1.0f;
			
				UINT32 numVertices = numQuads * 4;
				UINT32 numIndices = numQuads * 6;

				SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
				vertexDesc->addVertElem(VET_FLOAT2, VES_POSITION);
				vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

				SPtr<MeshData> meshData = MeshData::create(numVertices, numIndices, vertexDesc);

				auto vertexData = (Vector2*)meshData->getElementData(VES_POSITION);
				UINT32* indices = meshData->getIndices32();

				UINT32 quadIdx = 0;
				for (auto& drawGroup : widget->drawGroups)
				{
					float left = (float)drawGroup.bounds.x;
					float top = (float)drawGroup.bounds.y;
					float right = left + drawGroup.bounds.width;
					float bottom = top + drawGroup.bounds.height;
					
					*vertexData = Vector2(left, top);
					vertexData++;
					
					*vertexData = Vector2(0, uvTop);
					vertexData++;
					
					*vertexData = Vector2(right, top);
					vertexData++;

					*vertexData = Vector2(1.0f, uvTop);
					vertexData++;

					*vertexData = Vector2(left, bottom);
					vertexData++;

					*vertexData = Vector2(0.0f, uvBottom);
					vertexData++;

					*vertexData = Vector2(right, bottom);
					vertexData++;

					*vertexData = Vector2(1.0f, uvBottom);
					vertexData++;

					indices[quadIdx * 6 + 0] = quadIdx * 4 + 0;
					indices[quadIdx * 6 + 1] = quadIdx * 4 + 1;
					indices[quadIdx * 6 + 2] = quadIdx * 4 + 2;
					indices[quadIdx * 6 + 3] = quadIdx * 4 + 1;
					indices[quadIdx * 6 + 4] = quadIdx * 4 + 3;
					indices[quadIdx * 6 + 5] = quadIdx * 4 + 2;
					
					drawGroup.subMesh = SubMesh(quadIdx * 6, 6, DOT_TRIANGLE_LIST);
					quadIdx++;
				}
				
				widget->drawGroupMesh = Mesh::create(meshData, MU_STATIC, DOT_TRIANGLE_LIST);
			}
			else
				widget->drawGroupMesh = nullptr;
		}

		assert(data.groupDirtyState.size() == widget->drawGroups.size());

		for(UINT32 i = 0; i < (UINT32)data.groupDirtyState.size(); i++)
		{
			if (data.groupDirtyState[i])
				widget->drawGroups[i].requiresRedraw = true;
		}

		widget->triangleMesh = data.triangleMesh;
		widget->lineMesh = data.lineMesh;
		widget->worldTransform = worldTransform;

		if(widget->widgetDepth != widgetDepth)
		{
			widget->widgetDepth = widgetDepth;

			std::sort(widgets.begin(), widgets.end(), [](auto& x, auto& y)
			{
					return x.widgetDepth > y.widgetDepth;
			});
		}
	}

	void GUIRenderer::clearDrawGroups(const SPtr<Camera>& camera, UINT64 widgetId)
	{
		auto iterFind = mPerCameraData.find(camera.get());
		if (iterFind == mPerCameraData.end())
			return;

		Vector<GUIWidgetRenderData>& widgetData = iterFind->second;
		auto iterFind2 = std::find_if(widgetData.begin(), widgetData.end(), [widgetId](auto& x) { return x.widgetId == widgetId; });
		if (iterFind2 == widgetData.end())
			return;

		widgetData.erase(iterFind2);

		if (widgetData.empty())
		{
			mPerCameraData.erase(iterFind);
			mReferencedCameras.erase(camera);
		}
	}

	void GUIRenderer::updateParamBlockBuffer(const SPtr<GpuParamBlockBuffer>& buffer, float invViewportWidth,
		float invViewportHeight, bool flipY, const Matrix4& tfrm, GUIMeshRenderData& renderData) const
	{
		gGUISpriteParamBlockDef.gTint.set(buffer, renderData.tint);
		gGUISpriteParamBlockDef.gWorldTransform.set(buffer, tfrm);
		gGUISpriteParamBlockDef.gInvViewportWidth.set(buffer, invViewportWidth);
		gGUISpriteParamBlockDef.gInvViewportHeight.set(buffer, invViewportHeight);
		gGUISpriteParamBlockDef.gViewportYFlip.set(buffer, flipY ? -1.0f : 1.0f);

		float t = std::max(0.0f, mTime - renderData.animationStartTime);
		if(renderData.spriteTexture)
		{
			UINT32 row;
			UINT32 column;
			renderData.spriteTexture->getAnimationFrame(t, row, column);

			float invWidth = 1.0f / renderData.spriteTexture->getAnimation().numColumns;
			float invHeight = 1.0f / renderData.spriteTexture->getAnimation().numRows;

			Vector4 sizeOffset(invWidth, invHeight, column * invWidth, row * invHeight);
			gGUISpriteParamBlockDef.gUVSizeOffset.set(buffer, sizeOffset);
		}
		else
			gGUISpriteParamBlockDef.gUVSizeOffset.set(buffer, Vector4(1.0f, 1.0f, 0.0f, 0.0f));

		buffer->flushToGPU();
	}
	}
}
