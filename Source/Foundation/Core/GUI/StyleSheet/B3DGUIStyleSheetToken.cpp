//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/StyleSheet/B3DGUIStyleSheetToken.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;

GUIStyleSheetToken::GUIStyleSheetToken(const SourceCodePosition& position, GUIStyleSheetTokenTypes type)
	: mType(type), mPosition(position)
{ }

GUIStyleSheetToken::GUIStyleSheetToken(const SourceCodePosition& position, GUIStyleSheetTokenTypes type, const String& spelling)
	: mType(type), mPosition(position), mSpelling(spelling)
{ }

GUIStyleSheetToken::GUIStyleSheetToken(const SourceCodePosition& position, GUIStyleSheetTokenTypes type, String&& spelling)
	: mType(type), mPosition(position), mSpelling(std::move(spelling))
{ }

GUIStyleSheetToken::GUIStyleSheetToken(const GUIStyleSheetToken& other)
	: mType(other.mType), mPosition(other.mPosition), mSpelling(other.mSpelling)
{}

GUIStyleSheetToken::GUIStyleSheetToken(GUIStyleSheetToken&& other)
	: mType(other.mType), mPosition(std::move(other.mPosition)), mSpelling(std::move(other.mSpelling))
{ }

GUIStyleSheetToken& GUIStyleSheetToken::operator=(GUIStyleSheetToken&& other)
{
	mType = other.mType;
	mPosition = std::move(other.mPosition);
	mSpelling = std::move(other.mSpelling);

	return *this;
}

GUIStyleSheetToken& GUIStyleSheetToken::operator=(const GUIStyleSheetToken& other)
{
	mType = other.mType;
	mPosition = other.mPosition;
	mSpelling = other.mSpelling;

	return *this;
}

String GUIStyleSheetToken::TypeToString(const GUIStyleSheetTokenTypes type)
{
	switch(type)
	{
	case GUIStyleSheetTokenTypes::ElementSelector: return "Element selector";
	case GUIStyleSheetTokenTypes::PixelsLiteral: return "Pixel literal";
	case GUIStyleSheetTokenTypes::Undefined: return "Undefined";
	case GUIStyleSheetTokenTypes::IdSelector: return "ID selector";
	case GUIStyleSheetTokenTypes::ClassSelector: return "Class selector";
	case GUIStyleSheetTokenTypes::VariableIdentifier: return "Variable identifier";
	case GUIStyleSheetTokenTypes::StringLiteral: return "String literal";
	case GUIStyleSheetTokenTypes::DecimalLiteral: return "Decimal literal";
	case GUIStyleSheetTokenTypes::IntegerLiteral: return "Integer literal";
	case GUIStyleSheetTokenTypes::PercentLiteral: return "Percent literal";
	case GUIStyleSheetTokenTypes::Comma: return ",";
	case GUIStyleSheetTokenTypes::Colon: return ":";
	case GUIStyleSheetTokenTypes::Semicolon: return ";";
	case GUIStyleSheetTokenTypes::Slash: return "/";
	case GUIStyleSheetTokenTypes::LeftParenthesis: return "(";
	case GUIStyleSheetTokenTypes::RightParenthesis: return ")";
	case GUIStyleSheetTokenTypes::LeftCurly: return "{";
	case GUIStyleSheetTokenTypes::RightCurly: return "}";
	case GUIStyleSheetTokenTypes::Variable: return "var";
	case GUIStyleSheetTokenTypes::ColorRGB: return "rgb";
	case GUIStyleSheetTokenTypes::ColorHSL: return "hsl";
	case GUIStyleSheetTokenTypes::ColorRGBA: return "rgba";
	case GUIStyleSheetTokenTypes::ColorHSLA: return "hlsa";
	case GUIStyleSheetTokenTypes::URL: return "url";
	case GUIStyleSheetTokenTypes::Icon: return "icon";
	case GUIStyleSheetTokenTypes::ColorHex: return "Hex color";
	case GUIStyleSheetTokenTypes::Property: return "Property name";
	case GUIStyleSheetTokenTypes::BorderStyle: return "Border style";
	case GUIStyleSheetTokenTypes::TextAlign: return "Text align mode";
	case GUIStyleSheetTokenTypes::VerticalAlign: return "Vertical alignment mode";
	case GUIStyleSheetTokenTypes::WordWrap: return "Word wrap mode";
	case GUIStyleSheetTokenTypes::Visibility: return "Visibility";
	case GUIStyleSheetTokenTypes::PseudoClassSelector: return "Pseudo class selector";
	case GUIStyleSheetTokenTypes::None: return "None";
	case GUIStyleSheetTokenTypes::EndOfStream: return "End of stream";
	default: return "Unknown";
	}
}

String GUIStyleSheetToken::SpellContent() const
{
	if(GetType() == GUIStyleSheetTokenTypes::StringLiteral && GetSpelling().size() >= 2)
		return GetSpelling().substr(1, GetSpelling().size() - 2);

	return GetSpelling();
}
