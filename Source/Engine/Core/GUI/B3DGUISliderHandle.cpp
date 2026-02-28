//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUISliderHandle.h"

#include "B3DGUIUtility.h"
#include "B3DGUIVectorPaths.h"
#include "Image/B3DSpriteTexture.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

const String& GUISliderHandle::GetGuiTypeName()
{
	static String name = "SliderHandle";
	return name;
}

GUISliderHandle::GUISliderHandle(PrivatelyConstruct, GUISliderHandleContent content, const String& styleName, const GUISizeConstraints& sizeConstraints)
	: GUIInteractable(styleName, sizeConstraints), mFlags(content.Flags)
{
	if(content.Flags.IsSet(GUISliderHandleFlag::Resizeable))
	{
		if(content.Flags.IsSet(GUISliderHandleFlag::Horizontal))
			mBackgroundSprite.SetBackgroundPathBuilder(GUIResizableHorizontalScrollHandleVectorPathBuilder::Get());
		else
			mBackgroundSprite.SetBackgroundPathBuilder(GUIResizableVerticalScrollHandleVectorPathBuilder::Get());
	}
}

void GUISliderHandle::SetHandlePositionInPercent(float percent)
{
	float maximumPercent = 1.0f;
	if(mMinimumStepIncrement > 0.0f && percent < maximumPercent)
	{
		percent = (percent + mMinimumStepIncrement * 0.5f) - fmod(percent + mMinimumStepIncrement * 0.5f, mMinimumStepIncrement);
		maximumPercent = Math::Floor(1.0f / mMinimumStepIncrement) * mMinimumStepIncrement;
	}

	mHandlePositionInPercent = Math::Clamp(percent, 0.0f, maximumPercent);
}

void GUISliderHandle::UpdateRenderElements()
{
	mRenderElements.Clear();

	Vector2I offset(kZeroTag);
	Size2UI size;

	GUIPhysicalUnit handleSize = GetHandleSizeInPixels();
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		size.Width = (u32)handleSize;
		size.Height = (u32)mAbsoluteSize.Height;
		offset.X += (i32)GetHandlePositionInPixels();
	}
	else
	{
		size.Width = (u32)mAbsoluteSize.Width;
		size.Height = (u32)handleSize;
		offset.Y += (i32)GetHandlePositionInPixels();
	}

	const u64 batchId = (u64)GetParentWidget();
	const Color& tint = GetTint();

	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		GUIBackgroundSpriteCreateInformation createInformation(size, styleSheetRules, tint, batchId);
		createInformation.Offset = offset;

		mBackgroundSprite.BuildRenderElements(createInformation, mRenderElements);
	}

	GUIInteractable::UpdateRenderElements();
}

GUILogicalSize GUISliderHandle::CalculateUnconstrainedOptimalSize() const
{
	return GUILogicalSize(kMinimumHandleSize, kMinimumHandleSize);
}

bool GUISliderHandle::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	GUIPhysicalUnit handleSize = GetHandleSizeInPixels();

	if(ev.GetType() == GUIMouseEventType::MouseMove)
	{
		if(!IsDisabled())
		{
			if(mMouseOverHandle)
			{
				if(!IsOnHandle(ev.GetPosition()))
				{
					mMouseOverHandle = false;

					mState = GUIElementState::Normal;
					RemoveStateFlags(GUIElementStateFlag::Hover);
					MarkLayoutAsDirty();

					return true;
				}
			}
			else
			{
				if(IsOnHandle(ev.GetPosition()))
				{
					mMouseOverHandle = true;

					mState = GUIElementState::Hover;
					AddStateFlags(GUIElementStateFlag::Hover);
					MarkLayoutAsDirty();

					return true;
				}
			}
		}
	}

	bool jumpOnClick = mFlags.IsSet(GUISliderHandleFlag::JumpOnClick);
	if(ev.GetType() == GUIMouseEventType::MouseDown && (mMouseOverHandle || jumpOnClick))
	{
		if(!IsDisabled())
		{
			mState = GUIElementState::Active;
			AddStateFlags(GUIElementStateFlag::Active);
			MarkLayoutAsDirty();

			if(jumpOnClick)
			{
				GUIPhysicalUnit handlePosPx = 0;

				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					handlePosPx = ev.GetPosition().X - mAbsolutePosition.X - GUIPhysicalUnit((i32)((float)handleSize * 0.5f));
				else
					handlePosPx = ev.GetPosition().Y - mAbsolutePosition.Y - GUIPhysicalUnit((i32)((float)handleSize * 0.5f));

				SetHandlePositionInPixels(handlePosPx);
				OnHandleMovedOrResized(mHandlePositionInPercent, GetHandleSizeInPercent());
			}

			bool isResizeable = mFlags.IsSet(GUISliderHandleFlag::Resizeable);
			if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
			{
				GUIPhysicalUnit left = mAbsolutePosition.X + GetHandlePositionInPixels();
				const GUIPhysicalUnit resizableHandleSize = (i32)GUIResizableHorizontalScrollHandleVectorPathBuilder::kResizableHandleSize;

				if(isResizeable)
				{
					GUIPhysicalUnit right = left + handleSize;

					GUIPhysicalUnit clickPos = ev.GetPosition().X;
					if(clickPos >= left && clickPos < (left + resizableHandleSize))
						mDragState = DragState::LeftResize;
					else if(clickPos >= (right - resizableHandleSize) && clickPos < right)
						mDragState = DragState::RightResize;
					else
						mDragState = DragState::Normal;
				}
				else
					mDragState = DragState::Normal;

				mDragStartPos = ev.GetPosition().X - left;
			}
			else
			{
				GUIPhysicalUnit top = mAbsolutePosition.Y + GetHandlePositionInPixels();

				if(isResizeable)
				{
					GUIPhysicalUnit bottom = top + handleSize;
					const GUIPhysicalUnit resizableHandleSize = (i32)GUIResizableVerticalScrollHandleVectorPathBuilder::kResizableHandleSize;

					GUIPhysicalUnit clickPos = ev.GetPosition().Y;
					if(clickPos >= top && clickPos < (top + resizableHandleSize))
						mDragState = DragState::LeftResize;
					else if(clickPos >= (bottom - resizableHandleSize) && clickPos < bottom)
						mDragState = DragState::RightResize;
					else
						mDragState = DragState::Normal;
				}
				else
					mDragState = DragState::Normal;

				mDragStartPos = ev.GetPosition().Y - top;
			}

			mHandleDragged = true;
		}

		return true;
	}

	if(ev.GetType() == GUIMouseEventType::MouseDrag && mHandleDragged)
	{
		if(!IsDisabled())
		{
			if(mDragState == DragState::Normal)
			{
				GUIPhysicalUnit handlePosPx;
				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					handlePosPx = ev.GetPosition().X - mDragStartPos - mAbsolutePosition.X;
				else
					handlePosPx = ev.GetPosition().Y - mDragStartPos - mAbsolutePosition.Y;

				SetHandlePositionInPixels(handlePosPx);
				OnHandleMovedOrResized(mHandlePositionInPercent, GetHandleSizeInPercent());
			}
			else // Resizing
			{
				const GUIPhysicalUnit physicalMinimumHandleSize = GUIUtility::LogicalToPhysical(kMinimumHandleSize, GetAbsoluteScale());

				GUIPhysicalUnit clickPosPx;
				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					clickPosPx = ev.GetPosition().X - mAbsolutePosition.X;
				else
					clickPosPx = ev.GetPosition().Y - mAbsolutePosition.Y;

				GUIPhysicalUnit left = GetHandlePositionInPixels();
				GUIPhysicalUnit maxSize = GetTotalLength();

				GUIPhysicalUnit newHandleSize;
				float newHandlePos;
				if(mDragState == DragState::LeftResize)
				{
					GUIPhysicalUnit newLeft = clickPosPx - mDragStartPos;
					GUIPhysicalUnit right = left + handleSize;
					newLeft = Math::Clamp(newLeft, GUIPhysicalUnit(0), right);

					newHandleSize = Math::Max(physicalMinimumHandleSize, right - newLeft);
					newLeft = right - newHandleSize;

					float scrollableSize = (float)(maxSize - newHandleSize);
					if(scrollableSize > 0.0f)
						newHandlePos = (float)newLeft / scrollableSize;
					else
						newHandlePos = 0.0f;
				}
				else // Right resize
				{
					GUIPhysicalUnit newRight = clickPosPx;
					newHandleSize = Math::Max(physicalMinimumHandleSize, Math::Min(newRight, maxSize) - left);

					float scrollableSize = (float)(maxSize - newHandleSize);
					if(scrollableSize > 0.0f)
						newHandlePos = (float)left / scrollableSize;
					else
						newHandlePos = 0.0f;
				}

				SetHandleSizeInPercent((float)newHandleSize / (float)maxSize);
				SetHandlePositionInPercent(newHandlePos);

				OnHandleMovedOrResized(mHandlePositionInPercent, GetHandleSizeInPercent());
			}

			MarkLayoutAsDirty();
		}

		return true;
	}

	if(ev.GetType() == GUIMouseEventType::MouseOut)
	{
		if(!IsDisabled())
		{
			mMouseOverHandle = false;
			RemoveStateFlags(GUIElementStateFlag::Hover);

			if(!mHandleDragged)
			{
				mState = GUIElementState::Normal;
				MarkLayoutAsDirty();
			}
		}

		return true;
	}

	if(ev.GetType() == GUIMouseEventType::MouseUp)
	{
		if(!IsDisabled())
		{
			if(mMouseOverHandle)
				mState = GUIElementState::Hover;
			else
				mState = GUIElementState::Normal;

			RemoveStateFlags(GUIElementStateFlag::Active);
			if(!mHandleDragged)
			{
				// If we clicked above or below the scroll handle, scroll by one page
				GUIPhysicalUnit handlePosPx = GetHandlePositionInPixels();
				if(!mFlags.IsSet(GUISliderHandleFlag::JumpOnClick))
				{
					if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					{
						GUIPhysicalUnit handleLeft = mAbsolutePosition.X + handlePosPx;
						GUIPhysicalUnit handleRight = handleLeft + handleSize;

						if(ev.GetPosition().X < handleLeft)
							MoveOneStep(false);
						else if(ev.GetPosition().X > handleRight)
							MoveOneStep(true);
					}
					else
					{
						GUIPhysicalUnit handleTop = mAbsolutePosition.Y + handlePosPx;
						GUIPhysicalUnit handleBottom = handleTop + handleSize;

						if(ev.GetPosition().Y < handleTop)
							MoveOneStep(false);
						else if(ev.GetPosition().Y > handleBottom)
							MoveOneStep(true);
					}
				}
			}

			mHandleDragged = false;
			MarkLayoutAsDirty();
		}

		return true;
	}

	if(ev.GetType() == GUIMouseEventType::MouseDragEnd)
	{
		if(!IsDisabled())
		{
			mHandleDragged = false;
			if(mMouseOverHandle)
				mState = GUIElementState::Hover;
			else
				mState = GUIElementState::Normal;

			RemoveStateFlags(GUIElementStateFlag::Active);
			MarkLayoutAsDirty();
		}

		return true;
	}

	return false;
}

void GUISliderHandle::MoveOneStep(bool forward)
{
	const GUIPhysicalUnit handleSize = GetHandleSizeInPixels();
	GUIPhysicalUnit handlePosPx = GetHandlePositionInPixels();

	GUIPhysicalUnit stepSizePx;
	if(mMinimumStepIncrement > 0.0f)
		stepSizePx = (i32)(mMinimumStepIncrement * (float)GetTotalLength());
	else
		stepSizePx = handleSize;

	handlePosPx += forward ? stepSizePx : -stepSizePx;

	SetHandlePositionInPixels(handlePosPx);
	OnHandleMovedOrResized(mHandlePositionInPercent, GetHandleSizeInPercent());

	MarkLayoutAsDirty();
}

bool GUISliderHandle::IsOnHandle(const GUIPhysicalPoint& position) const
{
	GUIPhysicalUnit handleSize = GetHandleSizeInPixels();
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		GUIPhysicalUnit left = mAbsolutePosition.X + GetHandlePositionInPixels();
		GUIPhysicalUnit right = left + handleSize;

		if(position.X >= left && position.X < right)
			return true;
	}
	else
	{
		GUIPhysicalUnit top = mAbsolutePosition.Y + GetHandlePositionInPixels();
		GUIPhysicalUnit bottom = top + handleSize;

		if(position.Y >= top && position.Y < bottom)
			return true;
	}

	return false;
}

GUIPhysicalUnit GUISliderHandle::GetHandlePositionInPixels() const
{
	GUIPhysicalUnit maxScrollAmount = Math::Max(GetTotalLength() - GetHandleSizeInPixels(), 0);
	return Math::FloorToInt(mHandlePositionInPercent * (float)maxScrollAmount);
}

GUIPhysicalUnit GUISliderHandle::GetHandleSizeInPixels() const
{
	const GUIPhysicalUnit physicalMinimumHandleSize = GUIUtility::LogicalToPhysical(kMinimumHandleSize, GetAbsoluteScale());

	return Math::Max(physicalMinimumHandleSize, GUIPhysicalUnit((i32)((float)GetTotalLength() * mHandleSizeInPercent)));
}

void GUISliderHandle::SetHandlePositionInPixels(GUIPhysicalUnit position)
{
	float scrollableSize = (float)GetTotalLength() - (float)GetHandleSizeInPixels();

	if(scrollableSize > 0.0f)
		SetHandlePositionInPercent((float)position / scrollableSize);
	else
		SetHandlePositionInPercent(0.0f);
}

GUIPhysicalUnit GUISliderHandle::GetTotalLength() const
{
	GUIPhysicalUnit maxSize = (i32)mAbsoluteSize.Height;
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
		maxSize = (i32)mAbsoluteSize.Width;

	return maxSize;
}
