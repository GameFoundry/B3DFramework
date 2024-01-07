//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIElementStyle.h"
#include "2D/BsTextSprite.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIHelper.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

GUILabel::GUILabel(PrivatelyConstruct, const GUIContent& content, const String& styleClass, const GUISizeConstraints& dimensions)
	: GUIInteractable(styleClass, dimensions), mContent(content)
{
	mTextSprite = B3DNew<TextSprite>();
}

GUILabel::~GUILabel()
{
	B3DDelete(mTextSprite);
}

u32 GUILabel::GetRenderElementDepthRange() const
{
	return 2;
}

void GUILabel::UpdateRenderElements()
{
	mRenderElements.clear();

	GUISpriteHelper::BuildSpriteRenderElements(*this, GUIElementState::Normal, mBackgroundSprite);

	mTextSpriteInformation.Width = mLayoutData.Area.Width;
	mTextSpriteInformation.Height = mLayoutData.Area.Height;
	mTextSpriteInformation.Text = (String)mContent.Text;

	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		mTextSpriteInformation.InitializeFromStyleSheetRules(styleSheetRules);
		mTextSpriteInformation.Color *= GetTint();
	}
	else
	{
		mTextSpriteInformation.Font = GetStyle()->Font;
		mTextSpriteInformation.FontSize = GetStyle()->FontSize;
		mTextSpriteInformation.WordWrap = GetStyle()->WordWrap;
		mTextSpriteInformation.HorzAlign = GetStyle()->TextHorzAlign;
		mTextSpriteInformation.VertAlign = GetStyle()->TextVertAlign;
		mTextSpriteInformation.Color = GetTint() * GetStyle()->Normal.TextColor;
	}

	mTextSprite->Update(mTextSpriteInformation, (u64)GetParentWidget());

	const Rect2 backgroundBounds(
		0.0f, 0.0f,
		(float)mLayoutData.Area.Width, (float)mLayoutData.Area.Height);

	const Vector2I contentOffset = GetContentOffsetInElementSpace();
	Rect2I contentBounds = GetCachedContentBounds();

	const Rect2 textBounds(
		(float)contentOffset.X, (float)contentOffset.Y,
		(float)contentBounds.Width, (float)contentBounds.Height);

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Append({ T::SpriteInfo(mTextSprite, 0, textBounds) }, mRenderElements);
	}

	GUIInteractable::UpdateRenderElements();
}

Vector2I GUILabel::CalculateUnconstrainedOptimalSize() const
{
	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		const Size2UI contentSize = GUIHelper::CalculateOptimalContentSizeWithPaddingAndBorder(mContent, styleSheetRules, GetSizeConstraints().MaxWidth);

		return Vector2I(contentSize.Width, contentSize.Height);
	}
	else
		return GUIHelper::CalculateOptimalContentSize(mContent, *GetStyle(), GetSizeConstraints());
}

void GUILabel::SetContent(const GUIContent& content)
{
	Vector2I origSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
	mContent = content;
	Vector2I newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

const String& GUILabel::GetGuiTypeName()
{
	static String typeName = "Label";
	return typeName;
}
