//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/StyleSheet/B3DGUIStyleSheetParser.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include <charconv>

#include "Resources/B3DBuiltinResources.h"
#include "Resources/B3DResources.h"
#include "Text/B3DStockIcons.h"

using namespace b3d;

GUIStyleSheetParser::GUIStyleSheetParser()
{
	mPropertyKeywords["width"] = { GUIStyleSheetPropertyType::Width, ValueType::Pixel };
	mPropertyKeywords["height"] = { GUIStyleSheetPropertyType::Height, ValueType::Pixel };
	mPropertyKeywords["min-width"] = { GUIStyleSheetPropertyType::MinWidth, ValueType::Pixel };
	mPropertyKeywords["min-height"] = { GUIStyleSheetPropertyType::MinHeight, ValueType::Pixel };
	mPropertyKeywords["max-width"] = { GUIStyleSheetPropertyType::MaxWidth, ValueType::Pixel };
	mPropertyKeywords["max-height"] = { GUIStyleSheetPropertyType::MaxHeight, ValueType::Pixel };

	// Margin
	mPropertyKeywords["margin"] = { GUIStyleSheetPropertyType::Margin, ValueType::Multiple };
	mPropertyKeywords["margin-top"] = { GUIStyleSheetPropertyType::MarginTop, ValueType::Pixel };
	mPropertyKeywords["margin-bottom"] = { GUIStyleSheetPropertyType::MarginBottom, ValueType::Pixel };
	mPropertyKeywords["margin-left"] = { GUIStyleSheetPropertyType::MarginLeft, ValueType::Pixel };
	mPropertyKeywords["margin-right"] = { GUIStyleSheetPropertyType::MarginRight, ValueType::Pixel };

	// Padding
	mPropertyKeywords["padding"] = { GUIStyleSheetPropertyType::Padding, ValueType::Multiple };
	mPropertyKeywords["padding-top"] = { GUIStyleSheetPropertyType::PaddingTop, ValueType::Pixel };
	mPropertyKeywords["padding-bottom"] = { GUIStyleSheetPropertyType::PaddingBottom, ValueType::Pixel };
	mPropertyKeywords["padding-left"] = { GUIStyleSheetPropertyType::PaddingLeft, ValueType::Pixel };
	mPropertyKeywords["padding-right"] = { GUIStyleSheetPropertyType::PaddingRight, ValueType::Pixel };

	// Color properties
	mPropertyKeywords["color"] = { GUIStyleSheetPropertyType::Color, ValueType::Color };
	mPropertyKeywords["opacity"] = { GUIStyleSheetPropertyType::Opacity, ValueType::Decimal };
	mPropertyKeywords["background-color"] = { GUIStyleSheetPropertyType::BackgroundColor, ValueType::Color };

	// Image properties
	mPropertyKeywords["background-image"] = { GUIStyleSheetPropertyType::BackgroundImage, ValueType::Multiple };

	// Text properties
	mPropertyKeywords["text-align"] = { GUIStyleSheetPropertyType::TextAlign, ValueType::TextAlign };
	mPropertyKeywords["vertical-align"] = { GUIStyleSheetPropertyType::VerticalAlign, ValueType::VerticalAlign };
	mPropertyKeywords["font-family"] = { GUIStyleSheetPropertyType::FontFamily, ValueType::Multiple };
	mPropertyKeywords["font-size"] = { GUIStyleSheetPropertyType::FontSize, ValueType::Decimal };
	mPropertyKeywords["b3d-word-wrap"] = { GUIStyleSheetPropertyType::WordWrap, ValueType::WordWrap };

	// Other properties
	mPropertyKeywords["visibility"] = { GUIStyleSheetPropertyType::Visibility, ValueType::Visibility };

	// Border properties
	mPropertyKeywords["border"] = { GUIStyleSheetPropertyType::Border, ValueType::Multiple };
	mPropertyKeywords["border-style"] = { GUIStyleSheetPropertyType::BorderStyle, ValueType::Multiple };
	mPropertyKeywords["border-width"] = { GUIStyleSheetPropertyType::BorderWidth, ValueType::Multiple };
	mPropertyKeywords["border-color"] = { GUIStyleSheetPropertyType::BorderColor, ValueType::Multiple };
	mPropertyKeywords["border-top"] = { GUIStyleSheetPropertyType::BorderTop, ValueType::Multiple };
	mPropertyKeywords["border-top-style"] = { GUIStyleSheetPropertyType::BorderTopStyle, ValueType::BorderStyle };
	mPropertyKeywords["border-top-width"] = { GUIStyleSheetPropertyType::BorderTopWidth, ValueType::Pixel };
	mPropertyKeywords["border-top-color"] = { GUIStyleSheetPropertyType::BorderTopColor, ValueType::Color };
	mPropertyKeywords["border-bottom"] = { GUIStyleSheetPropertyType::BorderBottom, ValueType::Multiple };
	mPropertyKeywords["border-bottom-style"] = { GUIStyleSheetPropertyType::BorderBottomStyle, ValueType::BorderStyle };
	mPropertyKeywords["border-bottom-width"] = { GUIStyleSheetPropertyType::BorderBottomWidth, ValueType::Pixel };
	mPropertyKeywords["border-bottom-color"] = { GUIStyleSheetPropertyType::BorderBottomColor, ValueType::Color };
	mPropertyKeywords["border-left"] = { GUIStyleSheetPropertyType::BorderLeft, ValueType::Multiple };
	mPropertyKeywords["border-left-style"] = { GUIStyleSheetPropertyType::BorderLeftStyle, ValueType::BorderStyle };
	mPropertyKeywords["border-left-width"] = { GUIStyleSheetPropertyType::BorderLeftWidth, ValueType::Pixel };
	mPropertyKeywords["border-left-color"] = { GUIStyleSheetPropertyType::BorderLeftColor, ValueType::Color };
	mPropertyKeywords["border-right"] = { GUIStyleSheetPropertyType::BorderRight, ValueType::Multiple };
	mPropertyKeywords["border-right-style"] = { GUIStyleSheetPropertyType::BorderRightStyle, ValueType::BorderStyle };
	mPropertyKeywords["border-right-width"] = { GUIStyleSheetPropertyType::BorderRightWidth, ValueType::Pixel };
	mPropertyKeywords["border-right-color"] = { GUIStyleSheetPropertyType::BorderRightColor, ValueType::Color };
	mPropertyKeywords["border-radius"] = { GUIStyleSheetPropertyType::BorderRadius, ValueType::Multiple };
	mPropertyKeywords["border-top-left-radius"] = { GUIStyleSheetPropertyType::BorderTopLeftRadius, ValueType::Pixel };
	mPropertyKeywords["border-top-right-radius"] = { GUIStyleSheetPropertyType::BorderTopRightRadius, ValueType::Pixel };
	mPropertyKeywords["border-bottom-left-radius"] = { GUIStyleSheetPropertyType::BorderBottomLeftRadius, ValueType::Pixel };
	mPropertyKeywords["border-bottom-right-radius"] = { GUIStyleSheetPropertyType::BorderBottomRightRadius, ValueType::Pixel };
}

SPtr<GUIStyleSheet> GUIStyleSheetParser::Parse(const SPtr<SourceCode>& sourceCode)
{
	mLexer.StartScanning(sourceCode);

	// Grabs the first token
	GetCurrentTokenAndAdvance();

	mGlobalVariableContext = VariableContext();
	mParsedRulesets.clear();

	while(!IsCurrentToken(GUIStyleSheetTokenTypes::EndOfStream))
	{
		if(!TryParseRuleset())
			return nullptr;
	}

	SPtr<GUIStyleSheet> stylesheet = GUIStyleSheet::CreateShared(std::move(mParsedRulesets));
	mParsedRulesets.Clear();

	return stylesheet;
}

bool GUIStyleSheetParser::TryParseRuleset()
{
	TOptional<GUIStyleSheetSelectorList> selectorList = TryParseSelectorList();

	if(!selectorList)
	{
		Error("No selector provided.");
		return false;
	}

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::LeftCurly).has_value())
		return false;

	const bool isInGlobalVariableContext = selectorList->Selectors.Size() == 1 && selectorList->Selectors.back().SelectorType == GUIStyleSheetSelectorType::PseudoClass && selectorList->Selectors.back().Name == "root";

	GUIStyleSheetRuleset ruleset;
	mLocalVariableContext = VariableContext();
	while(!IsCurrentToken(GUIStyleSheetTokenTypes::RightCurly))
	{
		if(IsCurrentToken(GUIStyleSheetTokenTypes::EndOfStream))
		{
			Error("Unexpected end of stream.");
			return false;
		}

		if(IsCurrentToken(GUIStyleSheetTokenTypes::VariableIdentifier))
		{
			if(!TryParseVariable(isInGlobalVariableContext ? mGlobalVariableContext : mLocalVariableContext))
				return false;
		}
		else if(IsCurrentToken(GUIStyleSheetTokenTypes::Property))
		{
			if(!TryParseProperty(ruleset.Rules))
				return false;
		}
	}

	if(!GetCurrentTokenAndAdvance(TokenType::RightCurly).has_value())
		return false;

	ruleset.SelectorList = selectorList.value();
	mParsedRulesets.Add(ruleset);

	return true;
}

TOptional<GUIStyleSheetSelectorList> GUIStyleSheetParser::TryParseSelectorList()
{
	TOptional<GUIStyleSheetSelectorList> optionalSelectorList;

	// Parse selector name, which can be element, #id, .class, :pseudo-class or ::pseudo-element
	bool foundWhitespaceAfterLastElement = false;
	while(IsCurrentToken(TokenType::ElementSelector) || IsCurrentToken(GUIStyleSheetTokenTypes::ClassSelector) || IsCurrentToken(GUIStyleSheetTokenTypes::IdSelector) || IsCurrentToken(GUIStyleSheetTokenTypes::Colon))
	{
		GUIStyleSheetSelector selector;

		Token token = *GetCurrentTokenAndAdvance(false);
		switch(token.GetType())
		{
		case TokenType::ElementSelector:
			selector.SelectorType = GUIStyleSheetSelectorType::Element;
			selector.Name = token.GetSpelling();
			break;
		case TokenType::ClassSelector:
			selector.SelectorType = GUIStyleSheetSelectorType::Class;
			selector.Name = token.GetSpelling();
			break;
		case TokenType::IdSelector:
			selector.SelectorType = GUIStyleSheetSelectorType::Id;
			selector.Name = token.GetSpelling();
			break;
		case GUIStyleSheetTokenTypes::Colon:
		{
			if(IsCurrentToken(GUIStyleSheetTokenTypes::Colon))
			{
				GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Colon);
				
				TOptional<Token> pseudoElementToken = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::ElementSelector);
				if(!pseudoElementToken)
					return {};

				selector.SelectorType = GUIStyleSheetSelectorType::PseudoElement;
				selector.Name = pseudoElementToken->GetSpelling();
			}
			else
			{
				TOptional<Token> pseudoClassToken = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::PseudoClassSelector);
				if(!pseudoClassToken)
					return {};

				selector.SelectorType = GUIStyleSheetSelectorType::PseudoClass;
				selector.Name = pseudoClassToken->GetSpelling();
			}

			break;
		}
		default:
			B3D_ASSERT(false);
			break;
		}

		if(foundWhitespaceAfterLastElement)
			optionalSelectorList->Selectors.back().CombinatorType = GUIStyleSheetCombinatorType::AncestorOf;

		if(!optionalSelectorList.has_value())
			optionalSelectorList = GUIStyleSheetSelectorList();

		optionalSelectorList->Selectors.Add(selector);

		foundWhitespaceAfterLastElement = IsCurrentToken(TokenType::Space) || IsCurrentToken(TokenType::Newline);
		if(foundWhitespaceAfterLastElement)
			GetCurrentTokenAndAdvance();
	}

	return optionalSelectorList;
}

bool GUIStyleSheetParser::TryParseProperty(GUIStyleSheetRules& inOutValue)
{
	TOptional<Token> propertyToken = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Property);
	if(!propertyToken)
		return false;

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Colon))
		return false;

	if(auto foundProperty = mPropertyKeywords.find(propertyToken->GetSpelling()); foundProperty != mPropertyKeywords.end())
	{
#define CASE_PARSE(PropertyName, FieldName)                                                      \
	case GUIStyleSheetPropertyType::PropertyName:                                                \
		{                                                                                        \
			if(!TryParsePropertyValue(foundProperty->second.ValueType, inOutValue.FieldName))    \
				return false;                                                                    \
                                                                                                 \
			inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::PropertyName] = true; \
			break;                                                                               \
		}

		switch(foundProperty->second.PropertyType)
		{
			// Size
			CASE_PARSE(Width, Size.Width)
			CASE_PARSE(Height, Size.Height)
			CASE_PARSE(MinWidth, MinimumSize.Width)
			CASE_PARSE(MinHeight, MinimumSize.Height)
			CASE_PARSE(MaxWidth, MaximumSize.Width)
			CASE_PARSE(MaxHeight, MaximumSize.Height)

			// Margin
			CASE_PARSE(MarginLeft, Margins.Left)
			CASE_PARSE(MarginRight, Margins.Right)
			CASE_PARSE(MarginTop, Margins.Top)
			CASE_PARSE(MarginBottom, Margins.Bottom)

			// Padding
			CASE_PARSE(PaddingLeft, Padding.Left)
			CASE_PARSE(PaddingRight, Padding.Right)
			CASE_PARSE(PaddingTop, Padding.Top)
			CASE_PARSE(PaddingBottom, Padding.Bottom)

			// Color
			CASE_PARSE(Color, Color)
			CASE_PARSE(Opacity, Opacity)
			CASE_PARSE(BackgroundColor, BackgroundColor)

			// Image
			CASE_PARSE(BackgroundImage, BackgroundImage)

			// Other
			CASE_PARSE(Visibility, Visibility)

			// Text
			CASE_PARSE(FontFamily, Font)
			CASE_PARSE(FontSize, FontSize)
			CASE_PARSE(TextAlign, HorizontalTextAlignment)
			CASE_PARSE(VerticalAlign, VerticalTextAlignment)
			CASE_PARSE(WordWrap, WordWrap)

			// Border
			CASE_PARSE(BorderLeftWidth, BorderLeft.Width)
			CASE_PARSE(BorderLeftColor, BorderLeft.Color)
			CASE_PARSE(BorderLeftStyle, BorderLeft.Style)

			CASE_PARSE(BorderRightWidth, BorderRight.Width)
			CASE_PARSE(BorderRightColor, BorderRight.Color)
			CASE_PARSE(BorderRightStyle, BorderRight.Style)

			CASE_PARSE(BorderTopWidth, BorderTop.Width)
			CASE_PARSE(BorderTopColor, BorderTop.Color)
			CASE_PARSE(BorderTopStyle, BorderTop.Style)

			CASE_PARSE(BorderBottomWidth, BorderBottom.Width)
			CASE_PARSE(BorderBottomColor, BorderBottom.Color)
			CASE_PARSE(BorderBottomStyle, BorderBottom.Style)

			CASE_PARSE(BorderTopLeftRadius, BorderTopLeftRadius)
			CASE_PARSE(BorderTopRightRadius, BorderTopRightRadius)
			CASE_PARSE(BorderBottomLeftRadius, BorderBottomLeftRadius)
			CASE_PARSE(BorderBottomRightRadius, BorderBottomRightRadius)

			// Shorthands
		case GUIStyleSheetPropertyType::Margin:
			{
				if(!TryParseRectOffset(inOutValue.Margins))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::MarginLeft] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::MarginRight] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::MarginTop] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::MarginBottom] = true;
				break;
			}

		case GUIStyleSheetPropertyType::Padding:
			{
				if(!TryParseRectOffset(inOutValue.Padding))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::PaddingLeft] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::PaddingRight] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::PaddingTop] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::PaddingBottom] = true;
				break;
			}

		case GUIStyleSheetPropertyType::Border:
			{
				GUIStyleSheetBorderElement borderElement;
				if(!TryParseBorderElement(borderElement))
					return false;

				inOutValue.BorderLeft = borderElement;
				inOutValue.BorderRight = borderElement;
				inOutValue.BorderTop = borderElement;
				inOutValue.BorderBottom = borderElement;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderLeftWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderLeftColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderLeftStyle] = true;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightStyle] = true;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopStyle] = true;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightStyle] = true;
				
				break;
			}

		case GUIStyleSheetPropertyType::BorderLeft:
			{
				if(!TryParseBorderElement(inOutValue.BorderLeft))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderLeftWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderLeftColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderLeftStyle] = true;
				break;
			}

		case GUIStyleSheetPropertyType::BorderRight:
			{
				if(!TryParseBorderElement(inOutValue.BorderRight))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightStyle] = true;
				break;
			}

		case GUIStyleSheetPropertyType::BorderTop:
			{
				if(!TryParseBorderElement(inOutValue.BorderTop))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopStyle] = true;
				break;
			}

		case GUIStyleSheetPropertyType::BorderBottom:
			{
				if(!TryParseBorderElement(inOutValue.BorderBottom))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderBottomWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderBottomColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderBottomStyle] = true;
				break;
			}

		case GUIStyleSheetPropertyType::BorderRadius:
			{
				if(!TryParseBorderRadius(inOutValue.BorderTopLeftRadius, inOutValue.BorderTopRightRadius, inOutValue.BorderBottomLeftRadius, inOutValue.BorderBottomRightRadius))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopLeftRadius] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopRightRadius] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderBottomLeftRadius] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderBottomRightRadius] = true;
				break;
			}

		case GUIStyleSheetPropertyType::BorderWidth:
			{
				if(!TryParseBorderWidth(inOutValue.BorderTop.Width, inOutValue.BorderRight.Width, inOutValue.BorderBottom.Width, inOutValue.BorderLeft.Width))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderBottomWidth] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderLeftWidth] = true;
				break;
			}

		case GUIStyleSheetPropertyType::BorderColor:
			{
				if(!TryParseBorderColor(inOutValue.BorderTop.Color, inOutValue.BorderRight.Color, inOutValue.BorderBottom.Color, inOutValue.BorderLeft.Color))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderBottomColor] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderLeftColor] = true;
				break;
			}

		case GUIStyleSheetPropertyType::BorderStyle:
			{
				if(!TryParseBorderStyle(inOutValue.BorderTop.Style, inOutValue.BorderRight.Style, inOutValue.BorderBottom.Style, inOutValue.BorderLeft.Style))
					return false;

				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderTopStyle] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderRightStyle] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderBottomStyle] = true;
				inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::BorderLeftStyle] = true;
				break;
			}

		default:
			Error(StringFormat::Format("Unrecognized property '{0}'.", propertyToken->GetSpelling()));
			return false;
		}
	}
	else
	{
		Error(StringFormat::Format("Unrecognized property '{0}'.", propertyToken->GetSpelling()));
		return false;
	}

#undef CASE_PARSE 

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Semicolon))
		return false;

	return true;
}

bool GUIStyleSheetParser::TryParseVariable(VariableContext& inOutVariableContext)
{
	TOptional<Token> variableIdentifierToken = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::VariableIdentifier);
	if(!variableIdentifierToken)
		return false;

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Colon))
		return false;

	const String& identifier = variableIdentifierToken->GetSpelling();

	VariableValue value;
	switch(GetCurrentTokenType())
	{
	case GUIStyleSheetTokenTypes::IdSelector:
	case GUIStyleSheetTokenTypes::ColorRGB:
	case GUIStyleSheetTokenTypes::ColorHSL:
	case GUIStyleSheetTokenTypes::ColorRGBA:
	case GUIStyleSheetTokenTypes::ColorHSLA:
	case GUIStyleSheetTokenTypes::ColorHex:
	{
		value.Color = Color();
		if(!TryParseColor(value.Color))
			return false;

		value.Type = ValueType::Color;
		break;
	}
	case GUIStyleSheetTokenTypes::URL:
	{
		String parsedValue;
		if(!TryParseURL(parsedValue))
			return false;

		value.UnsignedInteger = (u32)mStringLiterals.size();
		value.Type = ValueType::URL;

		mStringLiterals.push_back(parsedValue);
		break;
	}
	case GUIStyleSheetTokenTypes::Icon:
	{
		String parsedIconName;
		float parsedIconSize;
		if(!TryParseIcon(parsedIconName, parsedIconSize))
			return false;

		value.Icon.NameStringId = (u32)mStringLiterals.size();
		value.Icon.IconSize = parsedIconSize;
		value.Type = ValueType::Icon;

		mStringLiterals.push_back(parsedIconName);
		break;
	}
	case GUIStyleSheetTokenTypes::StringLiteral:
	{
		String parsedValue;
		if(!TryParseStringLiteral(parsedValue))
			return false;

		value.UnsignedInteger = (u32)mStringLiterals.size();
		value.Type = ValueType::String;

		mStringLiterals.push_back(parsedValue);
		break;
	}
	case GUIStyleSheetTokenTypes::DecimalLiteral:
	{
		if(!TryParseDecimalLiteral(value.Float))
			return false;

		value.Type = ValueType::Decimal;
		break;
	}
	case GUIStyleSheetTokenTypes::IntegerLiteral:
	{
		if(!TryParseIntegerLiteral(value.SignedInteger))
			return false;

		value.Type = ValueType::Integer;
		break;
	}
	case GUIStyleSheetTokenTypes::PixelsLiteral:
	{
		if(!TryParsePixelLiteral(value.SignedInteger))
			return false;

		value.Type = ValueType::Pixel;
		break;
	}
	case GUIStyleSheetTokenTypes::PercentLiteral:
	{
		if(!TryParsePercentLiteral(value.Float))
			return false;

		value.Type = ValueType::Percent;
		break;
	}
	case GUIStyleSheetTokenTypes::BorderStyle:
	{
		GUIBorderElementStyle parsedValue;
		if(!TryParseBorderStyle(parsedValue))
			return false;

		value.UnsignedInteger = (u32)parsedValue;
		value.Type = ValueType::BorderStyle;
		break;
	}
	case GUIStyleSheetTokenTypes::None:
	{
		if(!TryParseNone())
			return false;

		value.UnsignedInteger = 0;
		value.Type = ValueType::None;
		break;
	}
	case GUIStyleSheetTokenTypes::TextAlign:
	{
		GUIHorizontalTextAlignment parsedValue;
		if(!TryParseTextAlign(parsedValue))
			return false;

		value.UnsignedInteger = (u32)parsedValue;
		value.Type = ValueType::TextAlign;
		break;
	}
	case GUIStyleSheetTokenTypes::VerticalAlign:
	{
		GUIVerticalTextAlignment parsedValue;
		if(!TryParseVerticalAlign(parsedValue))
			return false;

		value.UnsignedInteger = (u32)parsedValue;
		value.Type = ValueType::VerticalAlign;
		break;
	}
	case GUIStyleSheetTokenTypes::Visibility:
	{
		GUIElementVisibility parsedValue;
		if(!TryParseVisibility(parsedValue))
			return false;

		value.UnsignedInteger = (u32)parsedValue;
		value.Type = ValueType::Visibility;
		break;
	}
	case GUIStyleSheetTokenTypes::WordWrap:
	{
		GUIWordWrapMode parsedValue;
		if(!TryParseWordWrapMode(parsedValue))
			return false;

		value.UnsignedInteger = (u32)parsedValue;
		value.Type = ValueType::WordWrap;
		break;
	}
	default:
		ErrorUnexpected();
		return false;
	}

	if(auto found = inOutVariableContext.Variables.find(identifier); found != inOutVariableContext.Variables.end())
	{
		if(found->second.Type != value.Type)
		{
			Error(StringUtil::Format("Variable '{0}' previously defined as '{1}', but now defined as '{2}'.", identifier, ValueTypeToString(found->second.Type), ValueTypeToString(value.Type)));
			return false;
		}
	}

	inOutVariableContext.Variables[identifier] = value;

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Semicolon))
		return false;

	return true;
}

bool GUIStyleSheetParser::TryParseAndLookupVariableValue(ValueType expectedType, VariableValue& outValue)
{
	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Variable))
		return false;

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::LeftParenthesis))
		return false;

	TOptional<Token> variableIdentifierToken = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::VariableIdentifier);
	if(!variableIdentifierToken)
		return false;

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::RightParenthesis))
		return false;

	const String& variableIdentifier = variableIdentifierToken->GetSpelling();
	auto fnEnsureType = [this, expectedType, &variableIdentifier](const VariableValue& value) -> bool
	{
		// For properties accepting multiple different variable types, we let the higher level code do the type checking
		if(expectedType != ValueType::Multiple && !CanCastValue(expectedType, value.Type))
		{
			Error(StringFormat::Format("Provided variable '{0}' is of incorrect type. Expected '{1}' but variable is '{2}'.", variableIdentifier, ValueTypeToString(expectedType), ValueTypeToString(value.Type)));
			return false;
		}

		return true;
	};

	if(auto found = mLocalVariableContext.Variables.find(variableIdentifier); found != mLocalVariableContext.Variables.end())
	{
		if(!fnEnsureType(found->second))
			return false;

		outValue = found->second;
		return true;
	}

	if(auto found = mGlobalVariableContext.Variables.find(variableIdentifier); found != mGlobalVariableContext.Variables.end())
	{
		if(!fnEnsureType(found->second))
			return false;

		outValue = found->second;
		return true;
	}

	Error(StringFormat::Format("Variable '{0}' not previously defined.", variableIdentifier));
	return false;
}

bool GUIStyleSheetParser::TryParseIntegerLiteral(i32& outValue)
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::IntegerLiteral);
	if(!token)
		return false;

	return TryParseInteger(token->GetSpelling(), outValue);
}

bool GUIStyleSheetParser::TryParseIntegerLiteral(u32& outValue)
{
	i32 signedValue;
	if(!TryParseIntegerLiteral(signedValue))
		return false;

	outValue = (u32)signedValue;
	return true;
}

bool GUIStyleSheetParser::TryParsePixelLiteral(i32& outValue)
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::PixelsLiteral);
	if(!token)
		return false;

	return TryParseInteger(token->GetSpelling(), outValue);
}

bool GUIStyleSheetParser::TryParsePixelLiteral(u32& outValue)
{
	i32 signedValue;
	if(!TryParsePixelLiteral(signedValue))
		return false;

	outValue = (u32)Math::Max(0, signedValue);
	return true;
}

bool GUIStyleSheetParser::TryParseDecimalLiteral(float& outValue)
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::DecimalLiteral);
	if(!token)
		return false;

	return TryParseFloat(token->GetSpelling(), outValue);
}

bool GUIStyleSheetParser::TryParsePercentLiteral(float& outValue)
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::PercentLiteral);
	if(!token)
		return false;

	return TryParseFloat(token->GetSpelling(), outValue);
}

bool GUIStyleSheetParser::TryParseStringLiteral(String& outValue)
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::StringLiteral);
	if(!token)
		return false;

	outValue = token->GetSpelling();
	return true;
}

bool GUIStyleSheetParser::TryParseElementSelector(String& outValue)
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::ElementSelector);
	if(!token)
		return false;

	outValue = token->GetSpelling();
	return true;
}

bool GUIStyleSheetParser::TryParseNone()
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::None);
	return token.has_value();
}

bool GUIStyleSheetParser::TryParseColor(Color& outValue)
{
	TokenType currentTokenType = GetCurrentTokenType();

	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance();
	if(!token)
		return false;
			
	switch(currentTokenType)
	{
		// Hex literals are recognized as selectors due to # prefix
	case TokenType::IdSelector:
	case TokenType::ColorHex:
	{
		if(!TryParseHexColor(token->GetSpelling(), outValue))
			return false;

		return true;
	}
	case TokenType::ColorRGB:
	case TokenType::ColorRGBA:
	{
		if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::LeftParenthesis))
			return false;

		i32 red;
		if(!TryParseIntegerLiteral(red))
			return false;

		outValue.R = Math::Clamp01((float)red / 255.0f);
		SkipToken(GUIStyleSheetTokenTypes::Comma);

		i32 green;
		if(!TryParseIntegerLiteral(green))
			return false;

		outValue.G = Math::Clamp01((float)green / 255.0f);
		SkipToken(GUIStyleSheetTokenTypes::Comma);

		i32 blue;
		if(!TryParseIntegerLiteral(blue))
			return false;

		outValue.B = Math::Clamp01((float)blue / 255.0f);
		if(IsCurrentToken(GUIStyleSheetTokenTypes::Slash))
		{
			GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Slash);

			float alpha;
			if(!TryParseDecimalLiteral(alpha))
				return false;

			outValue.A = Math::Clamp01(alpha);
		}

		if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::RightParenthesis))
			return false;

		return true;
	}
	case TokenType::ColorHSL:
	case TokenType::ColorHSLA:
	{
		if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::LeftParenthesis))
			return false;

		float hue;
		if(!TryParseDecimalLiteral(hue))
			return false;

		hue = Math::Clamp01(hue / 360.0f);
		SkipToken(GUIStyleSheetTokenTypes::Comma);

		float saturation;
		if(!TryParsePercentLiteral(saturation))
			return false;

		saturation = Math::Clamp01(saturation / 100.0f);
		SkipToken(GUIStyleSheetTokenTypes::Comma);

		float lightness;
		if(!TryParsePercentLiteral(lightness))
			return false;

		lightness = Math::Clamp01(lightness / 100.0f);

		float alpha = 1.0f;
		if(IsCurrentToken(GUIStyleSheetTokenTypes::Slash))
		{
			GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Slash);

			if(!TryParseDecimalLiteral(alpha))
				return false;

			alpha = Math::Clamp01(alpha);
		}

		if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::RightParenthesis))
			return false;

		outValue = Color::FromHSL(hue, saturation, lightness, alpha);
		return true;
	}
	default:
		Error(StringUtil::Format("Unexpected token '{0}', expected a color.", Token::TypeToString(currentTokenType)));
		return false;
	}
}

bool GUIStyleSheetParser::TryParseURL(String& outValue)
{
	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::URL))
		return false;

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::LeftParenthesis))
		return false;

	// TODO: CSS also supports URLs without the quotes (""), but we don't support that yet
	if(!TryParseStringLiteral(outValue))
		return false;

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::RightParenthesis))
		return false;

	return true;
}

bool GUIStyleSheetParser::TryParseIcon(String& outIconName, float& outIconSize)
{
	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Icon))
		return false;

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::LeftParenthesis))
		return false;

	if(!TryParseElementSelector(outIconName))
		return false;

	outIconSize = 12.0f;
	if(IsCurrentToken(GUIStyleSheetTokenTypes::Comma))
	{
		B3D_ENSURE(GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Comma));

		if(!TryParseDecimalLiteral(outIconSize))
			return false;
	}

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::RightParenthesis))
		return false;

	return true;
}

bool GUIStyleSheetParser::TryParseImage(HSpriteImage& outValue)
{
	const bool isURL = IsCurrentToken(GUIStyleSheetTokenTypes::URL);
	const bool isIcon = IsCurrentToken(GUIStyleSheetTokenTypes::Icon);

	if(!isURL && !isIcon)
		return false;

	String urlOrIconName;
	float iconSize = 12.0f;
	if(isURL)
	{
		if(!TryParseURL(urlOrIconName))
			return false;
	}
	else
	{
		B3D_ENSURE(isIcon);

		if(!TryParseIcon(urlOrIconName, iconSize))
			return false;
	}

	if(isURL)
	{
		outValue = GetResources().Load<SpriteImage>(urlOrIconName, ResourceLoadOptions(false));

		if(!outValue.IsLoaded(false))
			Warning(StringUtil::Format("Unable to load image at path \"{0}\".", urlOrIconName));
	}
	else
	{
		B3D_ENSURE(isIcon);

		const StockIcon icon = StockIcons::Instance().ParseIconName(urlOrIconName);
		outValue = StockIcons::Instance().GetIcon(icon, iconSize);

		if(!outValue.IsLoaded(false))
			Warning(StringUtil::Format("Unable to find icon \"{0}\".", urlOrIconName));
	}

	return true;
}

bool GUIStyleSheetParser::TryParseFont(HFont& outValue)
{
	const bool isURL = IsCurrentToken(GUIStyleSheetTokenTypes::URL);
	const bool isStringLiteral = IsCurrentToken(GUIStyleSheetTokenTypes::StringLiteral);

	if(!isURL && !isStringLiteral)
		return false;

	if(isStringLiteral)
	{
		String fontFamily;
		if(!TryParseStringLiteral(fontFamily))
			return false;

		outValue = GetBuiltinResources().GetFont(fontFamily);

		if(!outValue.IsLoaded(false))
			Warning(StringUtil::Format("Unable to load font family \"{0}\".", fontFamily));
	}
	else
	{
		B3D_ENSURE(isURL);

		String url;
		if(!TryParseURL(url))
			return false;

		outValue = GetResources().Load<Font>(url, ResourceLoadOptions(false));

		if(!outValue.IsLoaded(false))
			Warning(StringUtil::Format("Unable to load font at path \"{0}\".", url));
	}

	return true;
}

bool GUIStyleSheetParser::TryParseBorderStyle(GUIBorderElementStyle& outValue)
{
	if(IsCurrentToken(GUIStyleSheetTokenTypes::BorderStyle))
	{
		TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::BorderStyle);
		if(!token)
			return false;

		if(token->GetSpelling() == "solid")
		{
			outValue = GUIBorderElementStyle::Solid;
			return true;
		}
	}
	else if(IsCurrentToken(GUIStyleSheetTokenTypes::None))
	{
		TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::None);
		if(!token)
			return false;

		if(token->GetSpelling() == "none")
		{
			outValue = GUIBorderElementStyle::None;
			return true;
		}
	}

	return false;
}

bool GUIStyleSheetParser::TryParseTextAlign(GUIHorizontalTextAlignment& outValue)
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::TextAlign);
	if(!token)
		return false;

	if(token->GetSpelling() == "left")
	{
		outValue = GUIHorizontalTextAlignment::Left;
		return true;
	}
	else if(token->GetSpelling() == "center")
	{
		outValue = GUIHorizontalTextAlignment::Center;
		return true;
	}
	else if(token->GetSpelling() == "right")
	{
		outValue = GUIHorizontalTextAlignment::Right;
		return true;
	}

	return false;
}

bool GUIStyleSheetParser::TryParseVerticalAlign(GUIVerticalTextAlignment& outValue)
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::VerticalAlign);
	if(!token)
		return false;

	if(token->GetSpelling() == "top")
	{
		outValue = GUIVerticalTextAlignment::Top;
		return true;
	}
	else if(token->GetSpelling() == "middle")
	{
		outValue = GUIVerticalTextAlignment::Middle;
		return true;
	}
	else if(token->GetSpelling() == "bottom")
	{
		outValue = GUIVerticalTextAlignment::Bottom;
		return true;
	}

	return false;
}

bool GUIStyleSheetParser::TryParseWordWrapMode(GUIWordWrapMode& outValue)
{
	if(IsCurrentToken(GUIStyleSheetTokenTypes::WordWrap))
	{
		TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::WordWrap);
		if(!token)
			return false;

		if(token->GetSpelling() == "wrap-word")
		{
			outValue = GUIWordWrapMode::WrapWord;
			return true;
		}
	}
	else if(IsCurrentToken(GUIStyleSheetTokenTypes::None))
	{
		TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::None);
		if(!token)
			return false;

		if(token->GetSpelling() == "none")
		{
			outValue = GUIWordWrapMode::None;
			return true;
		}
	}

	return false;
}

bool GUIStyleSheetParser::TryParseVisibility(GUIElementVisibility& outValue)
{
	TOptional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Visibility);
	if(!token)
		return false;

	if(token->GetSpelling() == "hidden")
	{
		outValue = GUIElementVisibility::Hidden;
		return true;
	}
	if(token->GetSpelling() == "visible")
	{
		outValue = GUIElementVisibility::Visible;
		return true;
	}

	return false;
}

bool GUIStyleSheetParser::TryParseBorderElement(GUIStyleSheetBorderElement& outValue)
{
	bool hasWidth = false;
	bool hasStyle = false;
	bool hasColor = false;

	for(u32 propertyIndex = 0; propertyIndex < 3; ++propertyIndex)
	{
		if(IsCurrentToken(GUIStyleSheetTokenTypes::PixelsLiteral))
		{
			if(hasWidth)
			{
				ErrorUnexpected();
				return false;
			}

			if(!TryParsePropertyValue(ValueType::Pixel, outValue.Width))
				return false;

			hasWidth = true;
		}
		
		const bool isColor = IsCurrentToken(GUIStyleSheetTokenTypes::IdSelector) || IsCurrentToken(GUIStyleSheetTokenTypes::ColorHSL) || IsCurrentToken(GUIStyleSheetTokenTypes::ColorHSLA) || IsCurrentToken(GUIStyleSheetTokenTypes::ColorRGB) || IsCurrentToken(GUIStyleSheetTokenTypes::ColorRGBA) || IsCurrentToken(GUIStyleSheetTokenTypes::ColorHex);
		if(isColor)
		{
			if(hasColor)
			{
				ErrorUnexpected();
				return false;
			}

			if(!TryParsePropertyValue(ValueType::Color, outValue.Color))
				return false;

			hasColor = true;
		}

		if(IsCurrentToken(GUIStyleSheetTokenTypes::BorderStyle) || IsCurrentToken(GUIStyleSheetTokenTypes::None))
		{
			if(hasStyle)
			{
				ErrorUnexpected();
				return false;
			}

			if(!TryParsePropertyValue(ValueType::BorderStyle, outValue.Style))
				return false;

			hasStyle = true;
		}
	}

	return hasWidth || hasColor || hasStyle;
}

bool GUIStyleSheetParser::TryParseRectOffset(RectOffset& outValue)
{
	return TryParsePropertyValueSides(ValueType::Pixel, outValue.Top, outValue.Right, outValue.Bottom, outValue.Left);
}

bool GUIStyleSheetParser::TryParseBorderRadius(u32& outTopLeft, u32& outTopRight, u32& outBottomLeft, u32& outBottomRight)
{
	return TryParsePropertyValueSides(ValueType::Pixel, outTopLeft, outTopRight, outBottomRight, outBottomLeft);
}

bool GUIStyleSheetParser::TryParseBorderStyle(GUIBorderElementStyle& outTop, GUIBorderElementStyle& outRight, GUIBorderElementStyle& outBottom, GUIBorderElementStyle& outLeft)
{
	return TryParsePropertyValueSides(ValueType::BorderStyle, outTop, outRight, outBottom, outLeft);
}

bool GUIStyleSheetParser::TryParseBorderWidth(u32& outTop, u32& outRight, u32& outBottom, u32& outLeft)
{
	return TryParsePropertyValueSides(ValueType::Pixel, outTop, outRight, outBottom, outLeft);
}

bool GUIStyleSheetParser::TryParseBorderColor(Color& outTop, Color& outRight, Color& outBottom, Color& outLeft)
{
	return TryParsePropertyValueSides(ValueType::Color, outTop, outRight, outBottom, outLeft);
}

template<class T>
bool GUIStyleSheetParser::TryParsePropertyValue(ValueType valueType, T& outValue)
{
	B3D_ASSERT(valueType != ValueType::Undefined);

	// If the property value is a variable name, look up the contents of that variable
	if(IsCurrentToken(GUIStyleSheetTokenTypes::Variable))
	{
		if(!TryParseAndLookupVariableValue(valueType, outValue))
			return false;

		return true;
	}

	if constexpr(std::is_same_v<T, u32> || std::is_same_v<T, i32>)
	{
		if(valueType == ValueType::Integer)
			return TryParseIntegerLiteral(outValue);
		if(valueType == ValueType::Pixel)
			return TryParsePixelLiteral(outValue);
	}
	else if constexpr(std::is_same_v<T, float>)
	{
		if(valueType == ValueType::Decimal)
			return TryParseDecimalLiteral(outValue);
		if(valueType == ValueType::Percent)
			return TryParsePercentLiteral(outValue);
	}
	else if constexpr(std::is_same_v<T, String>)
		return TryParseStringLiteral(outValue);
	else if constexpr(std::is_same_v<T, Color>)
		return TryParseColor(outValue);
	else if constexpr(std::is_same_v<T, GUIBorderElementStyle>)
		return TryParseBorderStyle(outValue);
	else if constexpr(std::is_same_v<T, GUIHorizontalTextAlignment>)
		return TryParseTextAlign(outValue);
	else if constexpr(std::is_same_v<T, GUIVerticalTextAlignment>)
		return TryParseVerticalAlign(outValue);
	else if constexpr(std::is_same_v<T, GUIWordWrapMode>)
		return TryParseWordWrapMode(outValue);
	else if constexpr(std::is_same_v<T, GUIElementVisibility>)
		return TryParseVisibility(outValue);
	else if constexpr(std::is_same_v<T, HSpriteImage>)
		return TryParseImage(outValue);
	else if constexpr(std::is_same_v<T, HFont>)
		return TryParseFont(outValue);

	Error("Internal error.");
	return false;
}

template<class T>
bool GUIStyleSheetParser::TryParsePropertyValueSides(ValueType valueType, T& outTop, T& outRight, T& outBottom, T& outLeft)
{
	u32 count = 0;
	do
	{
		T parsedValue;
		if(!TryParsePropertyValue(valueType, parsedValue))
			return false;

		if(count == 0)
		{
			outLeft = parsedValue;
			outRight = parsedValue;
			outTop = parsedValue;
			outBottom = parsedValue;
		}
		else if(count == 1)
		{
			outLeft = parsedValue;
			outRight = parsedValue;
		}
		else if(count == 2)
		{
			outBottom = parsedValue;
		}
		else if(count == 3)
		{
			outLeft = parsedValue;
		}

		if(IsCurrentToken(GUIStyleSheetTokenTypes::Semicolon))
			return true;

		count++;
	} while(count < 4); 

	return true;
}

template <class T>
bool GUIStyleSheetParser::TryParseAndLookupVariableValue(ValueType expectedType, T& outValue)
{
	VariableValue value;
	if(!TryParseAndLookupVariableValue(expectedType, value))
		return false;

	value.GetValue(outValue);
	return true;
}

bool GUIStyleSheetParser::TryParseAndLookupVariableValue(ValueType expectedType, String& outValue)
{
	VariableValue value;
	if(!TryParseAndLookupVariableValue(expectedType, value))
		return false;

	u32 stringLiteralIndex;
	value.GetValue(stringLiteralIndex);

	outValue = mStringLiterals[stringLiteralIndex];
	return true;
}

bool GUIStyleSheetParser::TryParseAndLookupVariableValue(ValueType expectedType, HSpriteImage& outValue)
{
	VariableValue value;
	if(!TryParseAndLookupVariableValue(expectedType, value))
		return false;

	if(value.Type == ValueType::URL)
	{
		u32 stringLiteralIndex;
		value.GetValue(stringLiteralIndex);

		const Path filePath = mStringLiterals[stringLiteralIndex];
		outValue = GetResources().Load<SpriteImage>(filePath, ResourceLoadOptions(false));

		if(!outValue.IsLoaded(false))
			Warning(StringUtil::Format("Unable to load image at path \"{0}\".", filePath));
	}
	else if(value.Type == ValueType::Icon)
	{
		IconValue iconValue;
		value.GetValue(iconValue);

		const String iconName = mStringLiterals[iconValue.NameStringId];
		const StockIcon icon = StockIcons::Instance().ParseIconName(iconName);
		outValue = StockIcons::Instance().GetIcon(icon, iconValue.IconSize);

		if(!outValue.IsLoaded(false))
			Warning(StringUtil::Format("Unable to find icon \"{0}\".", iconName));
	}
	else
	{
		Error(StringFormat::Format("Provided variable is of incorrect type. Expected 'url' or 'icon' but variable is '{0}'.", ValueTypeToString(value.Type)));
		return false;
	}

	return true;
}

bool GUIStyleSheetParser::TryParseAndLookupVariableValue(ValueType expectedType, HFont& outValue)
{
	VariableValue value;
	if(!TryParseAndLookupVariableValue(expectedType, value))
		return false;

	if(value.Type == ValueType::URL)
	{
		u32 stringLiteralIndex;
		value.GetValue(stringLiteralIndex);

		const Path filePath = mStringLiterals[stringLiteralIndex];
		outValue = GetResources().Load<Font>(filePath, ResourceLoadOptions(false));

		if(!outValue.IsLoaded(false))
			Warning(StringUtil::Format("Unable to load font at path \"{0}\".", filePath));
	}
	else if(value.Type == ValueType::String)
	{
		u32 stringLiteralIndex;
		value.GetValue(stringLiteralIndex);

		const String& fontFamily = mStringLiterals[stringLiteralIndex];
		outValue = GetBuiltinResources().GetFont(fontFamily); // TODO - Add improved lookup of fonts by name

		if(!outValue.IsLoaded(false))
			Warning(StringUtil::Format("Unable to load font family \"{0}\".", fontFamily));
	}
	else
	{
		Error(StringFormat::Format("Provided variable is of incorrect type. Expected 'url' or 'string literal' but variable is '{0}'.", ValueTypeToString(value.Type)));
		return false;
	}

	return true;
}

bool GUIStyleSheetParser::TryParseInteger(const StringView& toParse, i32& outValue) const
{
	const char* const stringStart = toParse.data();
	const char* const stringEnd = stringStart + toParse.size();
	const std::from_chars_result parseResult = std::from_chars(stringStart, stringEnd, outValue);

	return parseResult.ptr == stringEnd && parseResult.ec == std::errc();
}

bool GUIStyleSheetParser::TryParseFloat(const StringView& toParse, float& outValue) const
{
	const char* const stringStart = toParse.data();
	const char* const stringEnd = stringStart + toParse.size();
	const std::from_chars_result parseResult = std::from_chars(stringStart, stringEnd, outValue, std::chars_format::fixed);

	return parseResult.ptr == stringEnd && parseResult.ec == std::errc();
}

bool GUIStyleSheetParser::TryParseHexColor(const StringView& toParse, Color& outValue) const
{
	if(toParse.size() != 6 && toParse.size() != 8)
		return false;

	const u32 channelCount = (u32)toParse.size() / 2;

	auto fnParseColorChannel = [&toParse](u32 channel, float& outValue) -> bool
	{
		const char* const data = toParse.data();
		const char* const stringStart = data + channel * 2;
		const char* const stringEnd = stringStart + 2;

		u32 outIntegerValue;
		const std::from_chars_result parseResult = std::from_chars(stringStart, stringEnd, outIntegerValue, 16);

		if(parseResult.ptr != stringEnd || parseResult.ec != std::errc())
			return false;

		outValue = (float)outIntegerValue / 255.0f;
		return true;
	};

	for(u32 channelIndex = 0; channelIndex < channelCount; ++channelIndex)
	{
		if(!fnParseColorChannel(channelIndex, outValue[channelIndex]))
		return false;
	}

	return true;
}

bool GUIStyleSheetParser::IsCurrentToken(TokenType type) const
{
	return mCurrentToken && mCurrentToken->GetType() == type;
}

bool GUIStyleSheetParser::IsCurrentToken(TokenType type, const String& spelling) const
{
	return mCurrentToken && mCurrentToken->GetType() == type && mCurrentToken->GetSpelling() == spelling;
}

TOptional<GUIStyleSheetParser::Token> GUIStyleSheetParser::GetCurrentTokenAndAdvance(bool skipWhitespace)
{
	if(mCurrentToken && mCurrentToken->GetType() == TokenType::EndOfStream)
		return Error("Unexpected end of stream.");

	TOptional<Token> previousToken = mCurrentToken;
	mCurrentToken = mLexer.ScanNextToken(skipWhitespace);

	if(!mCurrentToken)
	{
		Error(mLexer.GetErrors());
		return {};
	}

	return previousToken;
}

TOptional<GUIStyleSheetParser::Token> GUIStyleSheetParser::GetCurrentTokenAndAdvance(TokenType expectedType)
{
	if(!mCurrentToken.has_value() || mCurrentToken->GetType() != expectedType)
		return ErrorUnexpected(expectedType);

	return GetCurrentTokenAndAdvance();
}

TOptional<GUIStyleSheetParser::Token> GUIStyleSheetParser::GetCurrentTokenAndAdvance(TokenType expectedType, const String& spelling)
{
	if(!mCurrentToken.has_value() || mCurrentToken->GetType() != expectedType)
		return ErrorUnexpected(expectedType);

	if(mCurrentToken->GetSpelling() != spelling)
		return ErrorUnexpected(spelling);

	return GetCurrentTokenAndAdvance();
}

void GUIStyleSheetParser::SkipToken(TokenType type)
{
	if(!IsCurrentToken(type))
		return;

	GetCurrentTokenAndAdvance(type);
}

void GUIStyleSheetParser::Warning(const String& message)
{
	mWarnings << StringUtil::Format("Parser warning ({0}): {1}", mCurrentToken->GetSourceCodePosition().ToString(), message) << '\n';
}

TOptional<GUIStyleSheetParser::Token> GUIStyleSheetParser::Error(const String& message)
{
	mErrors = StringUtil::Format("Parser error ({0}): {1}", mCurrentToken->GetSourceCodePosition().ToString(), message);
	return {};
}

TOptional<GUIStyleSheetParser::Token> GUIStyleSheetParser::ErrorUnexpected()
{
	mErrors = StringUtil::Format("Parser error ({0}): Unexpected token '{1}'", mCurrentToken->GetSourceCodePosition().ToString(), Token::TypeToString(mCurrentToken->GetType()));
	return {};
}

TOptional<GUIStyleSheetParser::Token> GUIStyleSheetParser::ErrorUnexpected(TokenType expectedTokenType)
{
	mErrors = StringUtil::Format("Parser error ({0}): Unexpected token '{1}', expected '{2}'", mCurrentToken->GetSourceCodePosition().ToString(), Token::TypeToString(mCurrentToken->GetType()), Token::TypeToString(expectedTokenType));
	return {};
}

TOptional<GUIStyleSheetParser::Token> GUIStyleSheetParser::ErrorUnexpected(const String& expectedTokenSpelling)
{
	mErrors = StringUtil::Format("Parser error ({0}): Unexpected spelling for token '{1}' ({2}), expected '{3}'", mCurrentToken->GetSourceCodePosition().ToString(), Token::TypeToString(mCurrentToken->GetType()), mCurrentToken->GetSpelling(), expectedTokenSpelling);
	return {};
}

const char* GUIStyleSheetParser::ValueTypeToString(ValueType type)
{
	switch(type)
	{
	default:
	case ValueType::Undefined: return "Undefined";
	case ValueType::Integer: return "Integer";
	case ValueType::Pixel: return "Pixel";
	case ValueType::Decimal: return "Decimal";
	case ValueType::Percent: return "Percent";
	case ValueType::Color: return "Color";
	case ValueType::String: return "String";
	case ValueType::URL: return "URL";
	case ValueType::Icon: return "Icon";
	case ValueType::BorderStyle: return "BorderStyle";
	case ValueType::TextAlign: return "TextAlign";
	case ValueType::VerticalAlign: return "VerticalAlign";
	case ValueType::WordWrap: return "WordWrap";
	case ValueType::Visibility: return "Visibility";
	case ValueType::None: return "None";
	}
}

bool GUIStyleSheetParser::CanCastValue(ValueType expectedType, ValueType receivedType)
{
	if(expectedType == receivedType)
		return true;

	switch(expectedType)
	{
	case ValueType::BorderStyle:
		return receivedType == ValueType::None;
	case ValueType::WordWrap:
		return receivedType == ValueType::None;
	case ValueType::Decimal:
		return receivedType == ValueType::Integer;
	case ValueType::Integer:
		return receivedType == ValueType::Decimal;
	default:
		return false;
	}
}

