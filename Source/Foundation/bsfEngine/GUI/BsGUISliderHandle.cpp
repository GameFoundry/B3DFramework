//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISliderHandle.h"

#include "BsGUIVectorPaths.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIMouseEvent.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

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

	Vector2I offset;
	Size2UI size;

	u32 handleSize = GetHandleSizeInPixels();
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		size.Width = handleSize;
		size.Height = mLayoutData.Area.Height;
		offset.X += GetHandlePositionInPixels();
	}
	else
	{
		size.Width = mLayoutData.Area.Width;
		size.Height = handleSize;
		offset.Y += GetHandlePositionInPixels();
	}

	const u64 batchId = (u64)GetParentWidget();
	const Color& tint = GetTint();

	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		GUIBackgroundSpriteCreateInformation createInformation(size, styleSheetRules, tint, batchId);
		createInformation.Offset = offset;

		mBackgroundSprite.BuildRenderElements(createInformation, mRenderElements);
	}
	else
	{
		mBackgroundSprite.BuildRenderElements(size, *GetStyle(), mState, tint, batchId, mRenderElements, offset);
	}

	GUIInteractable::UpdateRenderElements();
}

Vector2I GUISliderHandle::CalculateUnconstrainedOptimalSize() const
{
	return Vector2I(kMinimumHandleSize, kMinimumHandleSize);
}

bool GUISliderHandle::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	u32 handleSize = GetHandleSizeInPixels();

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
				float handlePosPx = 0.0f;

				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					handlePosPx = (float)(ev.GetPosition().X - (i32)mLayoutData.Area.X - handleSize * 0.5f);
				else
					handlePosPx = (float)(ev.GetPosition().Y - (i32)mLayoutData.Area.Y - handleSize * 0.5f);

				SetHandlePositionInPixels((i32)handlePosPx);
				OnHandleMovedOrResized(mHandlePositionInPercent, GetHandleSizeInPercent());
			}

			bool isResizeable = mFlags.IsSet(GUISliderHandleFlag::Resizeable);
			if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
			{
				i32 left = (i32)mLayoutData.Area.X + GetHandlePositionInPixels();
				const i32 resizableHandleSize = (i32)GUIResizableHorizontalScrollHandleVectorPathBuilder::kResizableHandleSize;

				if(isResizeable)
				{
					i32 right = left + handleSize;

					i32 clickPos = ev.GetPosition().X;
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
				i32 top = (i32)mLayoutData.Area.Y + GetHandlePositionInPixels();

				if(isResizeable)
				{
					i32 bottom = top + handleSize;
					const i32 resizableHandleSize = (i32)GUIResizableVerticalScrollHandleVectorPathBuilder::kResizableHandleSize;

					i32 clickPos = ev.GetPosition().Y;
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
				i32 handlePosPx;
				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					handlePosPx = ev.GetPosition().X - mDragStartPos - mLayoutData.Area.X;
				else
					handlePosPx = ev.GetPosition().Y - mDragStartPos - mLayoutData.Area.Y;

				SetHandlePositionInPixels(handlePosPx);
				OnHandleMovedOrResized(mHandlePositionInPercent, GetHandleSizeInPercent());
			}
			else // Resizing
			{
				i32 clickPosPx;
				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					clickPosPx = ev.GetPosition().X - mLayoutData.Area.X;
				else
					clickPosPx = ev.GetPosition().Y - mLayoutData.Area.Y;

				i32 left = GetHandlePositionInPixels();
				u32 maxSize = GetTotalLength();

				i32 newHandleSize;
				float newHandlePos;
				if(mDragState == DragState::LeftResize)
				{
					i32 newLeft = clickPosPx - mDragStartPos;
					i32 right = left + handleSize;
					newLeft = Math::Clamp(newLeft, 0, right);

					newHandleSize = std::max((i32)kMinimumHandleSize, right - newLeft);
					newLeft = right - newHandleSize;

					float scrollableSize = (float)(maxSize - newHandleSize);
					if(scrollableSize > 0.0f)
						newHandlePos = newLeft / scrollableSize;
					else
						newHandlePos = 0.0f;
				}
				else // Right resize
				{
					i32 newRight = clickPosPx;
					newHandleSize = std::max((i32)kMinimumHandleSize, std::min(newRight, (i32)maxSize) - left);

					float scrollableSize = (float)(maxSize - newHandleSize);
					if(scrollableSize > 0.0f)
						newHandlePos = left / scrollableSize;
					else
						newHandlePos = 0.0f;
				}

				SetHandleSizeInPercent(newHandleSize / (float)maxSize);
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
				i32 handlePosPx = GetHandlePositionInPixels();
				if(!mFlags.IsSet(GUISliderHandleFlag::JumpOnClick))
				{
					if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					{
						i32 handleLeft = (i32)mLayoutData.Area.X + handlePosPx;
						i32 handleRight = handleLeft + handleSize;

						if(ev.GetPosition().X < handleLeft)
							MoveOneStep(false);
						else if(ev.GetPosition().X > handleRight)
							MoveOneStep(true);
					}
					else
					{
						i32 handleTop = (i32)mLayoutData.Area.Y + handlePosPx;
						i32 handleBottom = handleTop + handleSize;

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
	const u32 handleSize = GetHandleSizeInPixels();
	i32 handlePosPx = GetHandlePositionInPixels();

	i32 stepSizePx;
	if(mMinimumStepIncrement > 0.0f)
		stepSizePx = (i32)(mMinimumStepIncrement * GetTotalLength());
	else
		stepSizePx = (i32)handleSize;

	handlePosPx += forward ? stepSizePx : -stepSizePx;

	SetHandlePositionInPixels(handlePosPx);
	OnHandleMovedOrResized(mHandlePositionInPercent, GetHandleSizeInPercent());

	MarkLayoutAsDirty();
}

bool GUISliderHandle::IsOnHandle(const Vector2I& position) const
{
	u32 handleSize = GetHandleSizeInPixels();
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		i32 left = (i32)mLayoutData.Area.X + GetHandlePositionInPixels();
		i32 right = left + handleSize;

		if(position.X >= left && position.X < right)
			return true;
	}
	else
	{
		i32 top = (i32)mLayoutData.Area.Y + GetHandlePositionInPixels();
		i32 bottom = top + handleSize;

		if(position.Y >= top && position.Y < bottom)
			return true;
	}

	return false;
}

i32 GUISliderHandle::GetHandlePositionInPixels() const
{
	i32 maxScrollAmount = std::max(0, (i32)GetTotalLength() - (i32)GetHandleSizeInPixels());
	return Math::FloorToInt(mHandlePositionInPercent * maxScrollAmount);
}

u32 GUISliderHandle::GetHandleSizeInPixels() const
{
	if(mFlags.IsSet(GUISliderHandleFlag::Resizeable))
		return Math::Max(kMinimumHandleSize, (u32)(GetTotalLength() * mHandleSizeInPercent));

	const Vector2I optimalSize = CalculateConstrainedSize().Optimal;

	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
		return Math::Max(kMinimumHandleSize, (u32)optimalSize.X);

	return Math::Max(kMinimumHandleSize, (u32)optimalSize.Y);
}

void GUISliderHandle::SetHandlePositionInPixels(i32 position)
{
	float scrollableSize = (float)GetTotalLength() - GetHandleSizeInPixels();

	if(scrollableSize > 0.0f)
		SetHandlePositionInPercent(position / scrollableSize);
	else
		SetHandlePositionInPercent(0.0f);
}

u32 GUISliderHandle::GetTotalLength() const
{
	u32 maxSize = mLayoutData.Area.Height;
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
		maxSize = mLayoutData.Area.Width;

	return maxSize;
}
