//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIElementStyle.h"
#include "2D/BsTextSprite.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIHelper.h"

using namespace bs;

GUILabel::GUILabel(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions)
	: GUIElement(styleName, dimensions), mContent(content), mImageSprite(nullptr)
{
	mTextSprite = B3DNew<TextSprite>();
}

GUILabel::~GUILabel()
{
	B3DDelete(mTextSprite);

	if(mImageSprite != nullptr)
		B3DDelete(mImageSprite);
}

u32 GUILabel::GetRenderElementDepthRange() const
{
	return 2;
}

void GUILabel::UpdateRenderElements()
{
	const HSpriteTexture& activeTex = GetStyle()->Normal.Texture;
	if(SpriteTexture::CheckIsLoaded(activeTex))
	{
		mImageDesc.Image = activeTex;

		if(mImageSprite == nullptr)
			mImageSprite = B3DNew<ImageSprite>();
	}
	else
	{
		mImageDesc.Image = nullptr;

		if(mImageSprite != nullptr)
		{
			B3DDelete(mImageSprite);
			mImageSprite = nullptr;
		}
	}

	if(mImageSprite != nullptr)
	{
		mImageDesc.Width = mLayoutData.Area.Width;
		mImageDesc.Height = mLayoutData.Area.Height;

		mImageDesc.BorderLeft = GetStyle()->Border.Left;
		mImageDesc.BorderRight = GetStyle()->Border.Right;
		mImageDesc.BorderTop = GetStyle()->Border.Top;
		mImageDesc.BorderBottom = GetStyle()->Border.Bottom;
		mImageDesc.Color = GetTint();

		mImageSprite->Update(mImageDesc, (u64)GetParentWidget());
	}

	mDesc.Font = GetStyle()->Font;
	mDesc.FontSize = GetStyle()->FontSize;
	mDesc.WordWrap = GetStyle()->WordWrap;
	mDesc.HorzAlign = GetStyle()->TextHorzAlign;
	mDesc.VertAlign = GetStyle()->TextVertAlign;
	mDesc.Width = mLayoutData.Area.Width;
	mDesc.Height = mLayoutData.Area.Height;
	mDesc.Text = mContent.Text;
	mDesc.Color = GetTint() * GetStyle()->Normal.TextColor;
	;

	mTextSprite->Update(mDesc, (u64)GetParentWidget());

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Populate({ T::SpriteInfo(mTextSprite), T::SpriteInfo(mImageSprite, 1) }, mRenderElements);
	}

	GUIElement::UpdateRenderElements();
}

Vector2I GUILabel::GetOptimalSize() const
{
	return GUIHelper::CalculateOptimalContentSize(mContent, *GetStyle(), GetDimensions());
}

void GUILabel::FillBuffer(
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

	u32 imageSpriteIdx = mTextSprite->GetRenderElementCount();

	if(renderElementIdx < imageSpriteIdx)
	{
		mTextSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, renderElementIdx, layoutOffset, mLayoutData.GetLocalClipRect());

		return;
	}

	mImageSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, imageSpriteIdx - renderElementIdx, layoutOffset, mLayoutData.GetLocalClipRect());
}

void GUILabel::SetContent(const GUIContent& content)
{
	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
	mContent = content;
	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

GUILabel* GUILabel::Create(const HString& text, const String& styleName)
{
	return Create(GUIContent(text), styleName);
}

GUILabel* GUILabel::Create(const HString& text, const GUIOptions& options, const String& styleName)
{
	return Create(GUIContent(text), options, styleName);
}

GUILabel* GUILabel::Create(const GUIContent& content, const String& styleName)
{
	return new(B3DAllocate<GUILabel>()) GUILabel(GetStyleName<GUILabel>(styleName), content, GUIDimensions::Create());
}

GUILabel* GUILabel::Create(const GUIContent& content, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUILabel>()) GUILabel(GetStyleName<GUILabel>(styleName), content, GUIDimensions::Create(options));
}

const String& GUILabel::GetGuiTypeName()
{
	static String typeName = "Label";
	return typeName;
}
