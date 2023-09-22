//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/StyleSheet/BsGUIStyleSheetParser.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"
#include <charconv>

using namespace bs;

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

	// Text properties
	mPropertyKeywords["text-align"] = { GUIStyleSheetPropertyType::TextAlign, ValueType::TextAlign };
	mPropertyKeywords["vertical-align"] = { GUIStyleSheetPropertyType::VerticalAlign, ValueType::VerticalAlign };
	mPropertyKeywords["font-family"] = { GUIStyleSheetPropertyType::FontFamily, ValueType::String };
	mPropertyKeywords["font-size"] = { GUIStyleSheetPropertyType::FontSize, ValueType::Integer };

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

bool GUIStyleSheetParser::Parse(const SPtr<SourceCode>& sourceCode)
{
	mLexer.StartScanning(sourceCode);

	// Grabs the first token
	GetCurrentTokenAndAdvance();

	mGlobalVariableContext = VariableContext();
	while(!IsCurrentToken(GUIStyleSheetTokenTypes::EndOfStream))
	{
		if(!TryParseSelector())
			return false;
	}

	return true;
}

bool GUIStyleSheetParser::TryParseSelector()
{
	GUIStyleSheetStateStyle stateStyle; // TODO - Should probably do a lookup if an element with the same name is already specified, and just overwrite/append to it

	// Parse selector name, which can be element or #id, or empty (if empty, pseudo-class must be specified)
	bool foundSelectorName = false;
	if(IsCurrentToken(TokenType::ElementSelector) || IsCurrentToken(GUIStyleSheetTokenTypes::IdSelector))
	{
		Token selectorNameToken = *GetCurrentTokenAndAdvance();
		switch(selectorNameToken.GetType())
		{
		case TokenType::ElementSelector:
			stateStyle.SelectorType = GUIStyleSheetSelectorType::Element;
			break;
		case TokenType::IdSelector:
			stateStyle.SelectorType = GUIStyleSheetSelectorType::Id;
			break;
		default:
			B3D_ASSERT(false);
			break;
		}

		stateStyle.Selector = selectorNameToken.GetSpelling();
		foundSelectorName = true;
	}

	bool foundPseudoClass = false;
	if(IsCurrentToken(GUIStyleSheetTokenTypes::Colon))
	{
		GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Colon);

		Optional<Token> pseudoClassToken = *GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::PseudoClassSelector);
		if(!pseudoClassToken)
			return {};

		stateStyle.PseudoClass = pseudoClassToken->GetSpelling();
		foundPseudoClass = true;
	}

	if(!foundSelectorName && !foundPseudoClass)
	{
		Error("No selector name or pseudo-class provided.");
		return false;
	}

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::LeftCurly).has_value())
		return false;

	const bool isInGlobalVariableContext = foundSelectorName && stateStyle.Selector == "root";

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
			if(!TryParseProperty(stateStyle))
				return false;
		}
	}
		
	return true;
}

bool GUIStyleSheetParser::TryParseProperty(GUIStyleSheetStateStyle& inOutValue)
{
	Optional<Token> propertyToken = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Property);
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

			// Text
			CASE_PARSE(FontFamily, FontFamily)
			CASE_PARSE(FontSize, FontSize)
			CASE_PARSE(TextAlign, HorizontalTextAlignment)
			CASE_PARSE(VerticalAlign, VerticalTextAlignment)

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
				GUIBorderElement borderElement;
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
	Optional<Token> variableIdentifierToken = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::VariableIdentifier);
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
	{
		if(!TryParseColor(value.Color))
			return false;

		value.Type = ValueType::Color;
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

	Optional<Token> variableIdentifierToken = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::VariableIdentifier);
	if(!variableIdentifierToken)
		return false;

	if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::RightParenthesis))
		return false;

	const String& variableIdentifier = variableIdentifierToken->GetSpelling();
	auto fnEnsureType = [this, expectedType, &variableIdentifier](const VariableValue& value) -> bool
	{
		if(value.Type != expectedType)
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
	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::IntegerLiteral);
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
	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::PixelsLiteral);
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
	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::DecimalLiteral);
	if(!token)
		return false;

	return TryParseFloat(token->GetSpelling(), outValue);
}

bool GUIStyleSheetParser::TryParsePercentLiteral(float& outValue)
{
	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::PercentLiteral);
	if(!token)
		return false;

	return TryParseFloat(token->GetSpelling(), outValue);
}

bool GUIStyleSheetParser::TryParseStringLiteral(String& outValue)
{
	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::PercentLiteral);
	if(!token)
		return false;

	outValue = token->GetSpelling();
	return true;
}

bool GUIStyleSheetParser::TryParseColor(Color& outValue)
{
	TokenType currentTokenType = GetCurrentTokenType();

	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance();
	if(!token)
		return false;
			
	switch(currentTokenType)
	{
		// Hex literals are recognized as selectors due to # prefix
	case TokenType::IdSelector:
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

bool GUIStyleSheetParser::TryParseBorderStyle(GUIBorderElementStyle& outValue)
{
	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::BorderStyle);
	if(!token)
		return false;

	if(token->GetSpelling() == "none")
	{
		outValue = GUIBorderElementStyle::None;
		return true;
	}
	else if(token->GetSpelling() == "solid")
	{
		outValue = GUIBorderElementStyle::Solid;
		return true;
	}

	return false;
}

bool GUIStyleSheetParser::TryParseTextAlign(GUIHorizontalTextAlignment& outValue)
{
	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::TextAlign);
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
	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::VerticalAlign);
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

bool GUIStyleSheetParser::TryParseBorderElement(GUIBorderElement& outValue)
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
		
		const bool isColor = IsCurrentToken(GUIStyleSheetTokenTypes::IdSelector) || IsCurrentToken(GUIStyleSheetTokenTypes::ColorHSL) || IsCurrentToken(GUIStyleSheetTokenTypes::ColorHSLA) || IsCurrentToken(GUIStyleSheetTokenTypes::ColorRGB) || IsCurrentToken(GUIStyleSheetTokenTypes::ColorRGBA);
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

		if(IsCurrentToken(GUIStyleSheetTokenTypes::BorderStyle))
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
	B3D_ASSERT(valueType != ValueType::Undefined && valueType != ValueType::Multiple);

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
	if(toParse.size() != 6 || toParse.size() != 8)
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

Optional<GUIStyleSheetParser::Token> GUIStyleSheetParser::GetCurrentTokenAndAdvance()
{
	if(mCurrentToken && mCurrentToken->GetType() == TokenType::EndOfStream)
		return Error("Unexpected end of stream.");

	Optional<Token> previousToken = mCurrentToken;
	mCurrentToken = mLexer.ScanNextToken();

	// TODO - Check for lexer error if it returned null

	return previousToken;
}

Optional<GUIStyleSheetParser::Token> GUIStyleSheetParser::GetCurrentTokenAndAdvance(TokenType expectedType)
{
	if(!mCurrentToken.has_value() || mCurrentToken->GetType() != expectedType)
		return ErrorUnexpected(expectedType);

	return GetCurrentTokenAndAdvance();
}

Optional<GUIStyleSheetParser::Token> GUIStyleSheetParser::GetCurrentTokenAndAdvance(TokenType expectedType, const String& spelling)
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

Optional<GUIStyleSheetParser::Token> GUIStyleSheetParser::Error(const String& message)
{
	mErrors = StringUtil::Format("Parser error ({0}): {1}", mCurrentToken->GetSourceCodePosition().ToString(), message);
	return {};
}

Optional<GUIStyleSheetParser::Token> GUIStyleSheetParser::ErrorUnexpected()
{
	mErrors = StringUtil::Format("Parser error ({0}): Unexpected token '{1}'", mCurrentToken->GetSourceCodePosition().ToString(), Token::TypeToString(mCurrentToken->GetType()));
	return {};
}

Optional<GUIStyleSheetParser::Token> GUIStyleSheetParser::ErrorUnexpected(TokenType expectedTokenType)
{
	mErrors = StringUtil::Format("Parser error ({0}): Unexpected token '{1}', expected '{2}'", mCurrentToken->GetSourceCodePosition().ToString(), Token::TypeToString(mCurrentToken->GetType()), Token::TypeToString(expectedTokenType));
	return {};
}

Optional<GUIStyleSheetParser::Token> GUIStyleSheetParser::ErrorUnexpected(const String& expectedTokenSpelling)
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
	case ValueType::BorderStyle: return "BorderStyle";
	case ValueType::TextAlign: return "TextAlign";
	case ValueType::VerticalAlign: return "VerticalAlign";
	}
}
