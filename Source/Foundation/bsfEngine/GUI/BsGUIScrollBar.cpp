//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollBar.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIElementStyle.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUISliderHandle.h"
#include "GUI/BsGUISpace.h"

using namespace std::placeholders;

namespace bs
{
	const u32 GUIScrollBar::ButtonScrollAmount = 10;

	const String& GUIScrollBar::GetHScrollHandleType()
	{
		static String typeName = "UIScrollBarHHandle";
		return typeName;
	}

	const String& GUIScrollBar::GetVScrollHandleType()
	{
		static String typeName = "UIScrollBarVHandle";
		return typeName;
	}

	GUIScrollBar::GUIScrollBar(bool horizontal, bool resizable, const String& styleName, const GUIDimensions& dimensions)
		: GUIElement(styleName, dimensions), mHorizontal(horizontal)
	{
		mImageSprite = bs_new<ImageSprite>();

		GUISliderHandleFlags flags;
		if(resizable)
			flags |= GUISliderHandleFlag::Resizeable;

		if(mHorizontal)
		{
			mLayout = GUILayoutX::Create();
			RegisterChildElementInternal(mLayout);

			mUpBtn = GUIButton::Create(HString(""), "ScrollLeftBtn");
			mDownBtn = GUIButton::Create(HString(""), "ScrollRightBtn");

			mHandleBtn = GUISliderHandle::Create(flags | GUISliderHandleFlag::Horizontal, GetSubStyleName(GetHScrollHandleType()));
		}
		else
		{
			mLayout = GUILayoutY::Create();
			RegisterChildElementInternal(mLayout);

			mUpBtn = GUIButton::Create(HString(""), "ScrollUpBtn");
			mDownBtn = GUIButton::Create(HString(""), "ScrollDownBtn");

			mHandleBtn = GUISliderHandle::Create(flags | GUISliderHandleFlag::Vertical, GetSubStyleName(GetVScrollHandleType()));
		}

		GUIElementOptions scrollUpBtnOptions = mUpBtn->GetOptionFlags();
		scrollUpBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

		mUpBtn->SetOptionFlags(scrollUpBtnOptions);

		GUIElementOptions scrollDownBtnOptions = mDownBtn->GetOptionFlags();
		scrollDownBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

		mDownBtn->SetOptionFlags(scrollDownBtnOptions);

		mLayout->AddNewElement<GUIFixedSpace>(2);
		mLayout->AddElement(mUpBtn);
		mLayout->AddElement(mHandleBtn);
		mLayout->AddElement(mDownBtn);
		mLayout->AddNewElement<GUIFixedSpace>(2);

		mHandleBtn->OnHandleMovedOrResized.Connect(std::bind(&::bs::GUIScrollBar::HandleMoved, this, _1, _2));

		mUpBtn->OnClick.Connect(std::bind(&::bs::GUIScrollBar::UpButtonClicked, this));
		mDownBtn->OnClick.Connect(std::bind(&::bs::GUIScrollBar::DownButtonClicked, this));
	}

	GUIScrollBar::~GUIScrollBar()
	{
		bs_delete(mImageSprite);

		GUIElement::Destroy(mUpBtn);
		GUIElement::Destroy(mDownBtn);
		GUIElement::Destroy(mHandleBtn);
	}

	void GUIScrollBar::UpdateRenderElementsInternal()
	{
		IMAGE_SPRITE_DESC desc;

		if(GetStyleInternal()->Normal.Texture != nullptr && GetStyleInternal()->Normal.Texture.IsLoaded())
			desc.Texture = GetStyleInternal()->Normal.Texture;

		desc.Width = mLayoutData.Area.Width;
		desc.Height = mLayoutData.Area.Height;
		desc.Color = GetTint();

		mImageSprite->Update(desc, (u64)GetParentWidgetInternal());

		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::Populate({ T::SpriteInfo(mImageSprite, 2) }, mRenderElements); // +2 depth because child buttons use +1
		}

		GUIElement::UpdateRenderElementsInternal();
	}

	void GUIScrollBar::UpdateClippedBounds()
	{
		mClippedBounds = Rect2I(0, 0, 0, 0); // We don't want any mouse input for this element. This is just a container.
	}

	Vector2I GUIScrollBar::GetOptimalSizeInternal() const
	{
		return mLayout->GetOptimalSizeInternal();
	}

	u32 GUIScrollBar::GetRenderElementDepthRangeInternal() const
	{
		return 3;
	}

	void GUIScrollBar::FillBuffer(
		u8* vertices,
		u32* indices,
		u32 vertexOffset,
		u32 indexOffset,
		const Vector2I& offset,
		u32 maxNumVerts,
		u32 maxNumIndices,
		u32 renderElementIdx) const
	{
		u8* uvs = vertices + sizeof(Vector2);
		u32 vertexStride = sizeof(Vector2) * 2;
		u32 indexStride = sizeof(u32);

		Vector2I layoutOffset = Vector2I(mLayoutData.Area.X, mLayoutData.Area.Y) + offset;
		mImageSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, renderElementIdx, layoutOffset, mLayoutData.GetLocalClipRect());
	}

	void GUIScrollBar::StyleUpdated()
	{
		if(mHorizontal)
			mHandleBtn->SetStyle(GetSubStyleName(GetHScrollHandleType()));
		else
			mHandleBtn->SetStyle(GetSubStyleName(GetVScrollHandleType()));
	}

	void GUIScrollBar::HandleMoved(float handlePct, float sizePct)
	{
		if(!OnScrollOrResize.Empty())
			OnScrollOrResize(handlePct, sizePct);
	}

	void GUIScrollBar::UpButtonClicked()
	{
		float handleOffset = 0.0f;
		float scrollableSize = (float)mHandleBtn->GetScrollableSize();

		if(scrollableSize > 0.0f)
			handleOffset = ButtonScrollAmount / scrollableSize;

		Scroll(handleOffset);
	}

	void GUIScrollBar::DownButtonClicked()
	{
		float handleOffset = 0.0f;
		float scrollableSize = (float)mHandleBtn->GetScrollableSize();

		if(scrollableSize > 0.0f)
			handleOffset = ButtonScrollAmount / scrollableSize;

		Scroll(-handleOffset);
	}

	void GUIScrollBar::Scroll(float amount)
	{
		float newHandlePos = Math::Clamp01(mHandleBtn->GetHandlePos() - amount);

		float oldHandlePos = mHandleBtn->GetHandlePos();
		mHandleBtn->SetHandlePosInternal(newHandlePos);

		if(oldHandlePos != mHandleBtn->GetHandlePos())
		{
			mHandleBtn->MarkLayoutAsDirtyInternal();

			if(!OnScrollOrResize.Empty())
				OnScrollOrResize(newHandlePos, mHandleBtn->GetHandleSizePctInternal());
		}
	}

	void GUIScrollBar::SetHandleSizeInternal(float pct)
	{
		mHandleBtn->SetHandleSizeInternal(pct);
	}

	void GUIScrollBar::SetScrollPosInternal(float pct)
	{
		mHandleBtn->SetHandlePosInternal(pct);
	}

	float GUIScrollBar::GetScrollPos() const
	{
		return mHandleBtn->GetHandlePos();
	}

	void GUIScrollBar::SetScrollPos(float pct)
	{
		float oldHandlePos = mHandleBtn->GetHandlePos();
		mHandleBtn->SetHandlePosInternal(pct);

		if(oldHandlePos != mHandleBtn->GetHandlePos())
			mHandleBtn->MarkLayoutAsDirtyInternal();
	}

	float GUIScrollBar::GetHandleSize() const
	{
		return mHandleBtn->GetHandleSizePctInternal();
	}

	void GUIScrollBar::SetHandleSize(float pct)
	{
		mHandleBtn->SetHandleSizeInternal(pct);
		mHandleBtn->MarkLayoutAsDirtyInternal();
	}

	u32 GUIScrollBar::GetScrollableSize() const
	{
		return mHandleBtn->GetScrollableSize();
	}

	void GUIScrollBar::SetTint(const Color& color)
	{
		mUpBtn->SetTint(color);
		mDownBtn->SetTint(color);
		mHandleBtn->SetTint(color);

		GUIElement::SetTint(color);
	}
} // namespace bs
