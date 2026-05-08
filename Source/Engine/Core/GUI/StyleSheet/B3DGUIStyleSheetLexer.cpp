//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/StyleSheet/B3DGUIStyleSheetLexer.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;

GUIStyleSheetLexer::GUIStyleSheetLexer()
{
	// Size properties
	mPropertyKeywords["width"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["height"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["min-width"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["min-height"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["max-width"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["max-height"] = GUIStyleSheetTokenTypes::Property;

	// Margin
	mPropertyKeywords["margin"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["margin-top"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["margin-bottom"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["margin-left"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["margin-right"] = GUIStyleSheetTokenTypes::Property;

	// Padding
	mPropertyKeywords["padding"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["padding-top"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["padding-bottom"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["padding-left"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["padding-right"] = GUIStyleSheetTokenTypes::Property;

	// Color properties
	mPropertyKeywords["color"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["opacity"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["background-color"] = GUIStyleSheetTokenTypes::Property;

	// Image properties
	mPropertyKeywords["background-image"] = GUIStyleSheetTokenTypes::Property;

	// Text properties
	mPropertyKeywords["text-align"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["vertical-align"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["font-family"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["font-size"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["b3d-word-wrap"] = GUIStyleSheetTokenTypes::Property;

	// Border properties
	mPropertyKeywords["border"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-style"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-width"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-color"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-top"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-top-style"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-top-width"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-top-color"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-bottom"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-bottom-style"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-bottom-width"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-bottom-color"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-left"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-left-style"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-left-width"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-left-color"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-right"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-right-style"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-right-width"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-right-color"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-radius"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-top-left-radius"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-top-right-radius"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-bottom-left-radius"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["border-bottom-right-radius"] = GUIStyleSheetTokenTypes::Property;

	// Border styles
	mPropertyKeywords["solid"] = GUIStyleSheetTokenTypes::BorderStyle;

	// Text align
	mPropertyKeywords["left"] = GUIStyleSheetTokenTypes::TextAlign;
	mPropertyKeywords["center"] = GUIStyleSheetTokenTypes::TextAlign;
	mPropertyKeywords["right"] = GUIStyleSheetTokenTypes::TextAlign;

	// Vertical align
	mPropertyKeywords["top"] = GUIStyleSheetTokenTypes::VerticalAlign;
	mPropertyKeywords["middle"] = GUIStyleSheetTokenTypes::VerticalAlign;
	mPropertyKeywords["bottom"] = GUIStyleSheetTokenTypes::VerticalAlign;

	// Word wrap
	mPropertyKeywords["wrap-word"] = GUIStyleSheetTokenTypes::WordWrap;

	// Visibility
	mPropertyKeywords["visibility"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["hidden"] = GUIStyleSheetTokenTypes::Visibility;
	mPropertyKeywords["visible"] = GUIStyleSheetTokenTypes::Visibility;

	// None
	mPropertyKeywords["none"] = GUIStyleSheetTokenTypes::None;

	// Pseudo-class
	mPropertyKeywords["active"] = GUIStyleSheetTokenTypes::PseudoClassSelector;
	mPropertyKeywords["hover"] = GUIStyleSheetTokenTypes::PseudoClassSelector;
	mPropertyKeywords["focus"] = GUIStyleSheetTokenTypes::PseudoClassSelector;
	mPropertyKeywords["checked"] = GUIStyleSheetTokenTypes::PseudoClassSelector;
	mPropertyKeywords["disabled"] = GUIStyleSheetTokenTypes::PseudoClassSelector;
	mPropertyKeywords["root"] = GUIStyleSheetTokenTypes::PseudoClassSelector;

	// Keywords
	mPropertyKeywords["var"] = GUIStyleSheetTokenTypes::Variable;
	mPropertyKeywords["rgb"] = GUIStyleSheetTokenTypes::ColorRGB;
	mPropertyKeywords["hsl"] = GUIStyleSheetTokenTypes::ColorHSL;
	mPropertyKeywords["rgba"] = GUIStyleSheetTokenTypes::ColorRGBA;
	mPropertyKeywords["hsla"] = GUIStyleSheetTokenTypes::ColorHSLA;
	mPropertyKeywords["url"] = GUIStyleSheetTokenTypes::URL;
	mPropertyKeywords["icon"] = GUIStyleSheetTokenTypes::Icon;
}

bool GUIStyleSheetLexer::StartScanning(const TShared<SourceCode>& sourceCode)
{
	if(!sourceCode || !sourceCode->IsValid())
		return false;

	mSourceCode = sourceCode;
	GetCurrentCharacterAndAdvance();

	return true;
}

void GUIStyleSheetLexer::SaveCurrentSourcePosition()
{
	mCurrentPosition = mSourceCode->GetPosition();
}

bool GUIStyleSheetLexer::GetCurrentCharacterAndAdvance(char expected, char& outCharacter)
{
	if(mCurrentCharacter != expected)
	{
		ErrorUnexpected(expected);
		return false;
	}

	outCharacter = GetCurrentCharacterAndAdvance();
	return true;
}

char GUIStyleSheetLexer::GetCurrentCharacterAndAdvance()
{
	const char previousCharacter = mCurrentCharacter;
	mCurrentCharacter = mSourceCode->GetNextCharacter();

	return previousCharacter;
}

GUIStyleSheetToken GUIStyleSheetLexer::CreateToken(const TokenType& type, bool takeCharacter)
{
	if(takeCharacter)
	{
		String spelling;
		spelling += GetCurrentCharacterAndAdvance();

		return Token(mCurrentPosition, type, std::move(spelling));
	}

	return Token(mCurrentPosition, type);
}

GUIStyleSheetToken GUIStyleSheetLexer::CreateToken(const TokenType& type, String& spelling, bool takeCharacter)
{
	if(takeCharacter)
		spelling += GetCurrentCharacterAndAdvance();

	return Token(mCurrentPosition, type, std::move(spelling));
}

GUIStyleSheetToken GUIStyleSheetLexer::CreateToken(const TokenType& type, String& spelling, const SourceCodePosition& sourceCodePosition, bool takeCharacter)
{
	if(takeCharacter)
		spelling += GetCurrentCharacterAndAdvance();

	return Token(sourceCodePosition, type, std::move(spelling));
}

TOptional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanNextToken(bool skipWhitespace)
{
	if(skipWhitespace)
		SkipWhiteSpaces();

	// Check for end-of-file
	if(IsCurrentCharacter(0))
	{
		SaveCurrentSourcePosition();
		return CreateToken(TokenType::EndOfStream);
	}

	// Scan next token
	SaveCurrentSourcePosition();
	return ScanToken();
}

void GUIStyleSheetLexer::SkipMatching(const Function<bool(char)>& predicate)
{
	while(predicate(GetCurrentCharacter()))
		GetCurrentCharacterAndAdvance();
}

void GUIStyleSheetLexer::SkipWhiteSpaces(bool includeNewLines)
{
	while(std::isspace(GetCurrentCharacter()) && (includeNewLines || !IsCurrentCharacterNewLine()))
		GetCurrentCharacterAndAdvance();
}

TOptional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanToken()
{
	if(std::isspace(GetCurrentCharacter()))
		return CreateToken(TokenType::Space, true);

	if(IsCurrentCharacterNewLine())
		return CreateToken(TokenType::Newline, true);

	if(std::isalpha(GetCurrentCharacter()) || IsCurrentCharacter('_') || IsCurrentCharacter('-'))
		return ScanIdentifier(false);

	if(IsCurrentCharacter('#'))
		return ScanElementSelectorOrHexColor();

	if(IsCurrentCharacter('.'))
		return ScanNumberOrClassSelector();

	if(std::isdigit(GetCurrentCharacter()))
		return ScanNumber(false);

	if(IsCurrentCharacter('\"'))
		return ScanStringLiteral();

	switch(GetCurrentCharacter())
	{
		case '(': return CreateToken(TokenType::LeftParenthesis, true);
		case ')': return CreateToken(TokenType::RightParenthesis, true);
		case '{': return CreateToken(TokenType::LeftCurly, true);
		case '}': return CreateToken(TokenType::RightCurly, true);
		case ',': return CreateToken(TokenType::Comma, true);
		case ':': return CreateToken(TokenType::Colon, true);
		case ';': return CreateToken(TokenType::Semicolon, true);
		case '/': return CreateToken(TokenType::Slash, true);
	}

	return ErrorUnexpected();
}

TOptional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanIdentifier(bool isStartingWithDot)
{
	// Special handling if first characters are  "--"
	const bool isFirstCharacterHyphen = IsCurrentCharacter('-');

	char firstCharacter = GetCurrentCharacterAndAdvance();
	const bool isVariable = !isStartingWithDot && isFirstCharacterHyphen && IsCurrentCharacter('-'); // If starting with --, it's a variable definition

	String spelling;
	if(isVariable)
	{
		char unused;
		if(!GetCurrentCharacterAndAdvance('-', unused))
			return {};

		// First character of the name can be a letter, '_' or '-'.
		if(std::isalpha(GetCurrentCharacter()) || IsCurrentCharacter('_') || IsCurrentCharacter('-'))
			spelling += GetCurrentCharacterAndAdvance();
		else
			return {};
	}
	else
	{
		// First character of the name can be a letter, '_' or '-'. 
		if(std::isalpha(firstCharacter) || firstCharacter == '_' || firstCharacter == '-')
			spelling += firstCharacter;
		else
			return {};
	}

	while(std::isalnum(GetCurrentCharacter()) || IsCurrentCharacter('_') || IsCurrentCharacter('-'))
		spelling += GetCurrentCharacterAndAdvance();

	String lowerCaseSpelling = spelling;
	StringUtility::ToLowerCase(lowerCaseSpelling);

	if(!isStartingWithDot)
	{
		if(auto it = mPropertyKeywords.find(lowerCaseSpelling); it != mPropertyKeywords.end())
			return CreateToken(it->second, spelling);

		if(isVariable)
			return CreateToken(TokenType::VariableIdentifier, spelling);
	}
	else
		return CreateToken(TokenType::ClassSelector, spelling);

	return CreateToken(TokenType::ElementSelector, spelling);
}

TOptional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanElementSelectorOrHexColor()
{
	String spelling;

	char character;
	if(!GetCurrentCharacterAndAdvance('#', character))
		return {};

	// First character of the name can be a letter, '_' or '-' for ID selector case
	if(std::isalpha(GetCurrentCharacter()) || IsCurrentCharacter('_') || IsCurrentCharacter('-'))
	{
		spelling += GetCurrentCharacterAndAdvance();

		while(std::isalnum(GetCurrentCharacter()) || IsCurrentCharacter('_') || IsCurrentCharacter('-'))
			spelling += GetCurrentCharacterAndAdvance();

		return CreateToken(TokenType::IdSelector, spelling);
	}

	if(std::isdigit(GetCurrentCharacter()))
	{
		auto fnIsHexCharacter = [](char character)
		{
			character = (char)std::tolower(character);
			return std::isdigit(character) || character == 'a' || character == 'b' || character == 'c' || character == 'd' || character == 'e' || character == 'f';
		};

		spelling += GetCurrentCharacterAndAdvance();
		u32 count = 1;

		while(fnIsHexCharacter(GetCurrentCharacter()))
		{
			if(count >= 8)
				return ErrorUnexpected();

			spelling += GetCurrentCharacterAndAdvance();
			count++;
		}

		return CreateToken(TokenType::ColorHex, spelling);
	}

	return Error("# must be followed by selector name or hex color.");
}

TOptional<GUIStyleSheetToken> GUIStyleSheetLexer::ScanStringLiteral()
{
	String spelling;

	char character;
	if(!GetCurrentCharacterAndAdvance('\"', character))
		return {};

	while(!IsCurrentCharacter('\"'))
	{
		if(IsCurrentCharacter(0))
			return Error("Unexpected end of stream");

		spelling += GetCurrentCharacterAndAdvance();
	}

	if(!GetCurrentCharacterAndAdvance('\"', character))
		return {};

	return CreateToken(TokenType::StringLiteral, spelling);
}

TOptional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanNumberOrClassSelector()
{
	char character;
	if(!GetCurrentCharacterAndAdvance('.', character))
		return {};

	character = GetCurrentCharacter();
	if(isdigit(character))
		return ScanNumber(true);

	return ScanIdentifier(true);
}

TOptional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanNumber(bool isStartingWithDot)
{
	String spelling;

	auto fnScanDigitSequence = [this](String& spelling)
	{
		const bool result = (std::isdigit(GetCurrentCharacter()) != 0);

		while(std::isdigit(GetCurrentCharacter()))
			spelling += GetCurrentCharacterAndAdvance();

		return result;
	};

	const bool hasDigitsBeforeDot = !isStartingWithDot && fnScanDigitSequence(spelling);

	if(IsCurrentCharacter('.'))
	{
		GetCurrentCharacterAndAdvance();
		isStartingWithDot = true;
	}

	TokenType type = GUIStyleSheetTokenTypes::Undefined;
	if(isStartingWithDot)
	{
		spelling += '.';

		const bool hasDigitsAfterDot = fnScanDigitSequence(spelling);
		if(!hasDigitsBeforeDot && !hasDigitsAfterDot)
			return Error("Error missing decimal part after decimal '.'.");

		type = GUIStyleSheetTokenTypes::DecimalLiteral;
		if(IsCurrentCharacter('%'))
		{
			GetCurrentCharacterAndAdvance();
			type = GUIStyleSheetTokenTypes::PercentLiteral;
		}
	}
	else
	{
		type = GUIStyleSheetTokenTypes::IntegerLiteral;

		if(IsCurrentCharacter('p') || IsCurrentCharacter('P'))
		{
			GetCurrentCharacterAndAdvance();
			if(IsCurrentCharacter('x') || IsCurrentCharacter('X'))
			{
				GetCurrentCharacterAndAdvance();
				type = GUIStyleSheetTokenTypes::PixelsLiteral;
			}
			else
				return ErrorUnexpected();
		}
	}

	return CreateToken(type, spelling);
}

TOptional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::Error(const String& message)
{
	GetCurrentCharacterAndAdvance();
	mErrors = StringUtility::Format("Lexer error ({0}): {1}", mCurrentPosition.ToString(), message);
	return {};
}

TOptional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ErrorUnexpected()
{
	mErrors = StringUtility::Format("Lexer error ({0}): Unexpected character '{1}'", mCurrentPosition.ToString(), GetCurrentCharacterAndAdvance());
	return {};
}

TOptional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ErrorUnexpected(char expectedCharacter)
{
	mErrors = StringUtility::Format("Lexer error ({0}): Unexpected character '{1}', expected '{2}", mCurrentPosition.ToString(), GetCurrentCharacterAndAdvance(), expectedCharacter);
	return {};
}
