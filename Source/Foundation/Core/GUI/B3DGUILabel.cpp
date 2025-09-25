//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUILabel.h"

#include "B3DGUIUtility.h"
#include "2D/B3DTextSprite.h"
#include "Image/B3DSpriteTexture.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

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

	mTextSpriteInformation.Size = mAbsoluteSize.To<i32>();
	mTextSpriteInformation.Text = (String)mContent.Text;

	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		mTextSpriteInformation.InitializeFromStyleSheetRules(styleSheetRules);
		mTextSpriteInformation.Color *= GetTint();
		mTextSpriteInformation.FontSize *= mAbsoluteScale;
	}

	mTextSprite->Update(mTextSpriteInformation, (u64)GetParentWidget());

	const GUIPhysicalPoint contentOffset = GetScaledContentOffset();
	const GUIPhysicalArea contentBounds = GetScaledContentBounds();

	const Area2 textBounds(
		(float)contentOffset.X, (float)contentOffset.Y,
		(float)contentBounds.Width, (float)contentBounds.Height);

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Append({ T::SpriteInfo(mTextSprite, 0, textBounds) }, mRenderElements);
	}

	GUIInteractable::UpdateRenderElements();
}

GUILogicalSize GUILabel::CalculateUnconstrainedOptimalSize() const
{
	if(mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return GUILogicalSize::kZero;

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	return GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(mContent, styleSheetRules, GetSizeConstraints().MaximumWidth);
}

void GUILabel::SetContent(const GUIContent& content)
{
	GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());
	mContent = content;
	GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());

	if(originalSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

const String& GUILabel::GetGuiTypeName()
{
	static String typeName = "Label";
	return typeName;
}
