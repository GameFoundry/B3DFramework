//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISliderHandle.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIMouseEvent.h"

using namespace bs;

const u32 GUISliderHandle::kResizeHandleSize = 7;

const String& GUISliderHandle::GetGuiTypeName()
{
	static String name = "SliderHandle";
	return name;
}

GUISliderHandle::GUISliderHandle(GUISliderHandleFlags flags, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIElement(styleName, dimensions), mFlags(flags)
{
	mImageSprite = B3DNew<ImageSprite>();
}

GUISliderHandle::~GUISliderHandle()
{
	B3DDelete(mImageSprite);
}

GUISliderHandle* GUISliderHandle::Create(GUISliderHandleFlags flags, const String& styleName)
{
	return new(B3DAllocate<GUISliderHandle>()) GUISliderHandle(flags, GetStyleName<GUISliderHandle>(styleName), GUISizeConstraints::Create());
}

GUISliderHandle* GUISliderHandle::Create(GUISliderHandleFlags flags, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUISliderHandle>()) GUISliderHandle(flags, GetStyleName<GUISliderHandle>(styleName), GUISizeConstraints::Create(options));
}

void GUISliderHandle::SetHandleSizeInternal(float pct)
{
	mPctHandleSize = Math::Clamp01(pct);
}

void GUISliderHandle::SetHandlePosInternal(float pct)
{
	float maxPct = 1.0f;
	if(mStep > 0.0f && pct < maxPct)
	{
		pct = (pct + mStep * 0.5f) - fmod(pct + mStep * 0.5f, mStep);
		maxPct = Math::Floor(1.0f / mStep) * mStep;
	}

	mPctHandlePos = Math::Clamp(pct, 0.0f, maxPct);
}

float GUISliderHandle::GetHandlePos() const
{
	return mPctHandlePos;
}

float GUISliderHandle::GetStep() const
{
	return mStep;
}

void GUISliderHandle::SetStep(float step)
{
	mStep = Math::Clamp01(step);
}

u32 GUISliderHandle::GetScrollableSize() const
{
	return GetMaxSize() - GetHandleSize();
}

void GUISliderHandle::UpdateRenderElements()
{
	ImageSpriteInformation desc;

	HSpriteImage activeImage = GetStyle()->GetImageForState(mState);
	if(SpriteImage::CheckIsLoaded(activeImage))
		desc.Image = activeImage;

	u32 handleSize = GetHandleSize();
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		desc.Width = handleSize;
		desc.Height = mLayoutData.Area.Height;
	}
	else
	{
		desc.Width = mLayoutData.Area.Width;
		desc.Height = handleSize;
	}

	desc.BorderLeft = GetStyle()->Border.Left;
	desc.BorderRight = GetStyle()->Border.Right;
	desc.BorderTop = GetStyle()->Border.Top;
	desc.BorderBottom = GetStyle()->Border.Bottom;
	desc.Color = GetTint();
	mImageSprite->Update(desc, (u64)GetParentWidget());

	Rect2 area = Rect2::kEmpty;
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		area.X += GetHandlePosPx();
	}
	else
	{
		area.Y += GetHandlePosPx();
	}

	area.Width = (float)desc.Width;
	area.Height = (float)desc.Height;

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Populate({ T::SpriteInfo(mImageSprite, 0, area) }, mRenderElements);
	}

	GUIElement::UpdateRenderElements();
}

Vector2I GUISliderHandle::CalculateUnconstrainedOptimalSize() const
{
	return Vector2I(kMinimumHandleSize, kMinimumHandleSize);
}

bool GUISliderHandle::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	u32 handleSize = GetHandleSize();

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

				SetHandlePosPx((i32)handlePosPx);
				OnHandleMovedOrResized(mPctHandlePos, GetHandleSizePctInternal());
			}

			bool isResizeable = mFlags.IsSet(GUISliderHandleFlag::Resizeable);
			if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
			{
				i32 left = (i32)mLayoutData.Area.X + GetHandlePosPx();

				if(isResizeable)
				{
					i32 right = left + handleSize;

					i32 clickPos = ev.GetPosition().X;
					if(clickPos >= left && clickPos < (left + (i32)kResizeHandleSize))
						mDragState = DragState::LeftResize;
					else if(clickPos >= (right - (i32)kResizeHandleSize) && clickPos < right)
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
				i32 top = (i32)mLayoutData.Area.Y + GetHandlePosPx();

				if(isResizeable)
				{
					i32 bottom = top + handleSize;

					i32 clickPos = ev.GetPosition().Y;
					if(clickPos >= top && clickPos < (top + (i32)kResizeHandleSize))
						mDragState = DragState::LeftResize;
					else if(clickPos >= (bottom - (i32)kResizeHandleSize) && clickPos < bottom)
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

				SetHandlePosPx(handlePosPx);
				OnHandleMovedOrResized(mPctHandlePos, GetHandleSizePctInternal());
			}
			else // Resizing
			{
				i32 clickPosPx;
				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					clickPosPx = ev.GetPosition().X - mLayoutData.Area.X;
				else
					clickPosPx = ev.GetPosition().Y - mLayoutData.Area.Y;

				i32 left = GetHandlePosPx();
				u32 maxSize = GetMaxSize();

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

				SetHandleSizeInternal(newHandleSize / (float)maxSize);
				SetHandlePosInternal(newHandlePos);

				OnHandleMovedOrResized(mPctHandlePos, GetHandleSizePctInternal());
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

			if(!mHandleDragged)
			{
				mState = GUIElementState::Normal;
				RemoveStateFlags(GUIElementStateFlag::Hover | GUIElementStateFlag::Active);
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
				i32 handlePosPx = GetHandlePosPx();
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
	const u32 handleSize = GetHandleSize();
	i32 handlePosPx = GetHandlePosPx();

	i32 stepSizePx;
	if(mStep > 0.0f)
		stepSizePx = (i32)(mStep * GetMaxSize());
	else
		stepSizePx = (i32)handleSize;

	handlePosPx += forward ? stepSizePx : -stepSizePx;

	SetHandlePosPx(handlePosPx);
	OnHandleMovedOrResized(mPctHandlePos, GetHandleSizePctInternal());

	MarkLayoutAsDirty();
}

bool GUISliderHandle::IsOnHandle(const Vector2I& pos) const
{
	u32 handleSize = GetHandleSize();
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		i32 left = (i32)mLayoutData.Area.X + GetHandlePosPx();
		i32 right = left + handleSize;

		if(pos.X >= left && pos.X < right)
			return true;
	}
	else
	{
		i32 top = (i32)mLayoutData.Area.Y + GetHandlePosPx();
		i32 bottom = top + handleSize;

		if(pos.Y >= top && pos.Y < bottom)
			return true;
	}

	return false;
}

i32 GUISliderHandle::GetHandlePosPx() const
{
	i32 maxScrollAmount = std::max(0, (i32)GetMaxSize() - (i32)GetHandleSize());
	return Math::FloorToInt(mPctHandlePos * maxScrollAmount);
}

u32 GUISliderHandle::GetHandleSize() const
{
	return std::max(kMinimumHandleSize, (u32)(GetMaxSize() * mPctHandleSize));
}

float GUISliderHandle::GetHandleSizePctInternal() const
{
	return mPctHandleSize;
}

void GUISliderHandle::SetHandlePosPx(i32 pos)
{
	float scrollableSize = (float)GetMaxSize() - GetHandleSize();

	if(scrollableSize > 0.0f)
		SetHandlePosInternal(pos / scrollableSize);
	else
		SetHandlePosInternal(0.0f);
}

u32 GUISliderHandle::GetMaxSize() const
{
	u32 maxSize = mLayoutData.Area.Height;
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
		maxSize = mLayoutData.Area.Width;

	return maxSize;
}
