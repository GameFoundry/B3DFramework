//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILabel.h"
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

	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		mTextSpriteInformation.InitializeFromStyleSheetRules(styleSheetRules);
		mTextSpriteInformation.Color *= GetTint();
	}

	mTextSprite->Update(mTextSpriteInformation, (u64)GetParentWidget());

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
	if(mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return Vector2I::kZero;

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	const Size2UI contentSize = GUIHelper::CalculateOptimalContentSizeWithPaddingAndBorder(mContent, styleSheetRules, GetSizeConstraints().MaxWidth);

	return Vector2I((i32)contentSize.Width, (i32)contentSize.Height);
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
