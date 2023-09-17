//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIStyleSheet.h"

#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"
#include <charconv>

using namespace bs;

class SourceCodePosition
{
public:
	SourceCodePosition() = default;
	SourceCodePosition(u32 row, u32 column, const String& filename);
	SourceCodePosition(const SourceCodePosition& other);
	SourceCodePosition(SourceCodePosition&& other);

	SourceCodePosition& operator=(const SourceCodePosition& other);

	/** Returns the source position as string in the format "Row:Column", e.g. "75:10". */
	String ToString(bool printFilename = true) const;

	u32 GetRow() const { return mRow; }
	u32 GetColumn() const { return mColumn; }

	/** Increases the current row count by 1 and sets the column to 0. */
	void MoveToNextRow();

	/** Increases the current column by 1. */
	void MoveToNextColumn();

	/** Returns true if this is a valid source position. False if row and column are 0. */
	bool IsValid() const;

	/** Resets the source position to (0:0). */
	void Reset();

	bool operator<(const SourceCodePosition& rhs) const;

	operator bool() const { return IsValid(); }

	static const SourceCodePosition kInvalid;
private:
	u32 mRow = ~0u;
	u32 mColumn = ~0u;
	String mFilename;
};

const SourceCodePosition SourceCodePosition::kInvalid{};

SourceCodePosition::SourceCodePosition(u32 row, u32 column, const String& filename)
	: mRow(row), mColumn(column), mFilename(filename)
{ }

SourceCodePosition::SourceCodePosition(const SourceCodePosition& other)
{
	mRow = other.mRow;
	mColumn = other.mColumn;
	mFilename = other.mFilename;
}

SourceCodePosition::SourceCodePosition(SourceCodePosition&& other)
{
	mRow = other.mRow;
	mColumn = other.mColumn;
	mFilename = std::move(other.mFilename);
}

SourceCodePosition& SourceCodePosition::operator=(const SourceCodePosition& other)
{
	mRow = other.mRow;
	mColumn = other.mColumn;
	mFilename = other.mFilename;

	return *this;
}

String SourceCodePosition::ToString(bool printFilename) const
{
	StringStream stringStream;

	if(printFilename && !mFilename.empty())
		stringStream << mFilename << ":";

	stringStream << mRow << ":" << mColumn;
	return stringStream.str();
}

void SourceCodePosition::MoveToNextRow()
{
	++mRow;
	mColumn = 0;
}

void SourceCodePosition::MoveToNextColumn()
{
	++mColumn;
}

bool SourceCodePosition::IsValid() const
{
	return mRow != ~0u && mColumn != ~0u;
}

void SourceCodePosition::Reset()
{
	mRow = ~0u;
	mColumn = ~0u;
}

bool SourceCodePosition::operator<(const SourceCodePosition& rhs) const
{
	if(mFilename.data() < rhs.mFilename.data())
		return true;
	else if(mFilename.data() > rhs.mFilename.data())
		return false;

	if(mRow < rhs.mRow)
		return true;

	if(mRow > rhs.mRow)
		return false;

	return mColumn < rhs.mColumn;
}


class SourceCode
{
public:
	SourceCode(const String& source);

	/** Returns true if this is a valid source code stream. */
	bool IsValid() const;

	/** Returns the next character from the source, and advances the cursor. */
	char GetNextCharacter();

	/** Ignores the next character and advances the cursor. */
	void SkipNextCharacter() { GetNextCharacter(); }

	/** Returns the current source position. */
	const SourceCodePosition& GetPosition() const { return mPosition; }

	/** Returns the current source line. */
	const String& GetLine() const { return mCurrentLine; }

protected:
	SourceCode() = default;

	/** Returns the line (if it has already been read) by the zero-based line index. */
	String GetPreviouslyReadLine(u32 lineIndex) const;

	StringStream mStream;
	String mCurrentLine;
	Vector<String> mReadLines;
	SourceCodePosition mPosition;
};

SourceCode::SourceCode(const String& source)
{
	mStream << source;
	mStream.seekg(0, std::ios::beg);
}

bool SourceCode::IsValid() const
{
	return mStream.good();
}

char SourceCode::GetNextCharacter()
{
	// Check if reader is at end-of-line
	while(mPosition.GetColumn() >= mCurrentLine.size())
	{
		// Check if end-of-file is reached.
		if(!IsValid() || mStream.eof())
			return 0;

		// Read new line in source file
		std::getline(mStream, mCurrentLine);
		mCurrentLine += '\n';
		mPosition.MoveToNextRow();

		// Store current line for later reports
		mReadLines.push_back(mCurrentLine);
	}

	// Increment column and return current character
	const char character = mCurrentLine[mPosition.GetColumn()];
	mPosition.MoveToNextColumn();

	return character;
}

String SourceCode::GetPreviouslyReadLine(u32 lineIndex) const
{
	return lineIndex < mReadLines.size() ? mReadLines[lineIndex] : "";
}

enum class GUIStyleSheetTokenTypes
{
	Undefined,
	ElementSelector, // -?[_a-zA-Z]+[_a-zA-Z0-9-]*
	IdSelector, // #-?[_a-zA-Z]+[_a-zA-Z0-9-]*
	VariableIdentifier, // ---?[_a-zA-Z]+[_a-zA-Z0-9-]*

	StringLiteral, // "anything"
	DecimalLiteral, // e.g. 0.5
	IntegerLiteral, // e.g. 100
	PixelsLiteral, // 200px
	PercentLiteral, // 50% or 25.5%

	Comma, // ,
	Colon, // :
	Semicolon, // ;
	Slash, // /

	LeftParenthesis, // (
	RightParenthesis, // )
	LeftCurly, // {
	RightCurly, // }

	Variable, // var

	ColorRGB, // rgb
	ColorHSL, // hsl
	ColorRGBA, // rgba
	ColorHSLA, // hsla

	Property, // width, height, color, border-radius, font-size, etc.
	BorderStyle, // none, solid
	TextAlign, // left, center, right
	VerticalAlign, // top, middle, bottom
	PseudoClassSelector, // active, hover, focus, checked, disabled

	EndOfStream
};

/** Token parsed by GUIStyleSheetLexer. */
class GUIStyleSheetToken
{
public:
	GUIStyleSheetToken() = default;
	GUIStyleSheetToken(const SourceCodePosition& sourceCodePosition, GUIStyleSheetTokenTypes type);
	GUIStyleSheetToken(const SourceCodePosition& sourceCodePosition, GUIStyleSheetTokenTypes type, const String& spelling);
	GUIStyleSheetToken(const SourceCodePosition& sourceCodePosition, GUIStyleSheetTokenTypes type, String&& spelling);

	GUIStyleSheetToken(const GUIStyleSheetToken& other);
	GUIStyleSheetToken(GUIStyleSheetToken&& other);

	GUIStyleSheetToken& operator=(GUIStyleSheetToken&& other);
	GUIStyleSheetToken& operator=(const GUIStyleSheetToken& other);

	/** Returns a descriptive string for the specified token type. */
	static String TypeToString(GUIStyleSheetTokenTypes type);

	/** Returns the token spelling of the content (e.g. only the content of a string literal within the quotes). */
	String SpellContent() const;

	/** Returns the token type. */
	GUIStyleSheetTokenTypes GetType() const { return mType; }

	/** Returns the token source position. */
	const SourceCodePosition& GetSourceCodePosition() const { return mPosition; }

	/** Returns the token spelling. */
	const String& GetSpelling() const { return mSpelling; }

private:
	GUIStyleSheetTokenTypes mType = GUIStyleSheetTokenTypes::Undefined;
	SourceCodePosition mPosition;
	String mSpelling; // TODO - Should be string view
};

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
	case GUIStyleSheetTokenTypes::ElementSelector: return "Identifier";
		// TODO
	default: return "";
	}
}

String GUIStyleSheetToken::SpellContent() const
{
	if(GetType() == GUIStyleSheetTokenTypes::StringLiteral && GetSpelling().size() >= 2)
		return GetSpelling().substr(1, GetSpelling().size() - 2);

	return GetSpelling();
}

class GUIStyleSheetLexer
{
	using Token = GUIStyleSheetToken;
	using TokenType = GUIStyleSheetTokenTypes;
public:
	GUIStyleSheetLexer();
	bool StartScanning(const SPtr<SourceCode>& sourceCode);
	Optional<GUIStyleSheetToken> ScanNextToken();

	const String& GetErrors() const { return mErrors; }
	
private:
	bool IsCurrentCharacterNewLine() const { return (mCurrentCharacter == '\n' || mCurrentCharacter == '\r'); }
	bool IsCurrentCharacter(char character) const { return (mCurrentCharacter == character); }
	char GetCurrentCharacter() const { return mCurrentCharacter; }
	char GetCurrentCharacterAndAdvance();
	bool GetCurrentCharacterAndAdvance(char expected, char& outCharacter);

	GUIStyleSheetToken CreateToken(const TokenType& type, bool takeCharacter = false);
	GUIStyleSheetToken CreateToken(const TokenType& type, String& spelling, bool takeCharacter = false);
	GUIStyleSheetToken CreateToken(const TokenType& type, String& spelling, const SourceCodePosition& sourceCodePosition, bool takeCharacter = false);

	void SkipMatching(const Function<bool(char)>& predicate);
	void SkipWhiteSpaces(bool includeNewLines = true);

	void SaveCurrentSourcePosition();

	Optional<Token> ScanToken();
	Optional<Token> ScanIdentifier();
	Optional<Token> ScanStringLiteral();
	Optional<Token> ScanNumber();

	Optional<Token> Error(const String& message);
	Optional<Token> ErrorUnexpected();
	Optional<Token> ErrorUnexpected(char expectedCharacter);

	SPtr<SourceCode> mSourceCode;
	char mCurrentCharacter = 0;
	SourceCodePosition mCurrentPosition;
	String mErrors;

	UnorderedMap<String, TokenType> mPropertyKeywords;
};

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

	// Text properties
	mPropertyKeywords["text-align"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["vertical-align"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["font-family"] = GUIStyleSheetTokenTypes::Property;
	mPropertyKeywords["font-size"] = GUIStyleSheetTokenTypes::Property;

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
	mPropertyKeywords["none"] = GUIStyleSheetTokenTypes::BorderStyle;
	mPropertyKeywords["solid"] = GUIStyleSheetTokenTypes::BorderStyle;

	// Text align
	mPropertyKeywords["left"] = GUIStyleSheetTokenTypes::TextAlign;
	mPropertyKeywords["center"] = GUIStyleSheetTokenTypes::TextAlign;
	mPropertyKeywords["right"] = GUIStyleSheetTokenTypes::TextAlign;

	// Vertical align
	mPropertyKeywords["top"] = GUIStyleSheetTokenTypes::VerticalAlign;
	mPropertyKeywords["middle"] = GUIStyleSheetTokenTypes::VerticalAlign;
	mPropertyKeywords["bottom"] = GUIStyleSheetTokenTypes::VerticalAlign;

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
}

bool GUIStyleSheetLexer::StartScanning(const SPtr<SourceCode>& sourceCode)
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

Optional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanNextToken()
{
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

Optional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanToken()
{
	if(std::isalpha(GetCurrentCharacter()) || IsCurrentCharacter('_') || IsCurrentCharacter('-') || IsCurrentCharacter('#'))
		return ScanIdentifier();

	if(IsCurrentCharacter('.') || std::isdigit(GetCurrentCharacter()))
		return ScanNumber();

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

Optional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanIdentifier()
{
	// Special handling if first characters are '#' or "--"
	const bool isNameIdentifier = IsCurrentCharacter('#');
	const bool isFirstCharacterHyphen = IsCurrentCharacter('-');

	const char firstCharacter = GetCurrentCharacterAndAdvance();
	const bool isVariable = isFirstCharacterHyphen && IsCurrentCharacter('-'); // If starting with --, it's a variable definition

	String spelling;
	if(isVariable)
	{
		char unused;
		if(!GetCurrentCharacterAndAdvance('-', unused))
			return {};
	}
	else if(!isNameIdentifier)
		spelling += firstCharacter;

	// First character of the name can be a letter, '_' or '-'. Special case for '-' as we already parsed it above in case this is not a variable or a name identifier.
	if(std::isalpha(GetCurrentCharacter()) || IsCurrentCharacter('_') || ((isVariable || isNameIdentifier) && IsCurrentCharacter('-')))
	{
		spelling += GetCurrentCharacterAndAdvance();

		while(std::isalnum(GetCurrentCharacter()) || IsCurrentCharacter('_') || IsCurrentCharacter('-'))
			spelling += GetCurrentCharacterAndAdvance();
	}

	String lowerCaseSpelling = spelling;
	StringUtil::ToLowerCase(lowerCaseSpelling);

	if(auto it = mPropertyKeywords.find(lowerCaseSpelling); it != mPropertyKeywords.end())
		return CreateToken(it->second, spelling);

	if(isNameIdentifier)
		return CreateToken(TokenType::IdSelector, spelling);

	if(isVariable)
		return CreateToken(TokenType::VariableIdentifier, spelling);

	return CreateToken(TokenType::ElementSelector, spelling);
}

Optional<GUIStyleSheetToken> GUIStyleSheetLexer::ScanStringLiteral()
{
	String spelling;

	char character;
	if(!GetCurrentCharacterAndAdvance('\"', character))
		return {};

	spelling += character;

	while(!IsCurrentCharacter('\"'))
	{
		if(IsCurrentCharacter(0))
			return Error("Unexpected end of stream");

		spelling += GetCurrentCharacterAndAdvance();
	}

	if(!GetCurrentCharacterAndAdvance('\"', character))
		return {};

	spelling += character;

	return CreateToken(TokenType::StringLiteral, spelling);
}

Optional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ScanNumber()
{
	String spelling;

	auto fnScanDigitSequence = [this](String& spelling)
	{
		const bool result = (std::isdigit(GetCurrentCharacter()) != 0);

		while(std::isdigit(GetCurrentCharacter()))
			spelling += GetCurrentCharacterAndAdvance();

		return result;
	};

	const bool hasDigitsBeforeDot = fnScanDigitSequence(spelling);

	TokenType type = GUIStyleSheetTokenTypes::Undefined;
	if(IsCurrentCharacter('.'))
	{
		spelling += GetCurrentCharacterAndAdvance();

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
				type = GUIStyleSheetTokenTypes::PercentLiteral;
			}
			else
				return ErrorUnexpected();
		}
	}

	return CreateToken(type, spelling);
}

Optional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::Error(const String& message)
{
	GetCurrentCharacterAndAdvance();
	mErrors = StringUtil::Format("Lexer error ({0}): {1}", mCurrentPosition.ToString(), message);
	return {};
}

Optional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ErrorUnexpected()
{
	mErrors = StringUtil::Format("Lexer error ({0}): Unexpected character '{1}'", mCurrentPosition.ToString(), GetCurrentCharacterAndAdvance());
	return {};
}

Optional<GUIStyleSheetLexer::Token> GUIStyleSheetLexer::ErrorUnexpected(char expectedCharacter)
{
	mErrors = StringUtil::Format("Lexer error ({0}): Unexpected character '{1}', expected '{2}", mCurrentPosition.ToString(), GetCurrentCharacterAndAdvance(), expectedCharacter);
	return {};
}

class GUIStyleSheetParser
{
	using Token = GUIStyleSheetToken;
	using TokenType = GUIStyleSheetTokenTypes;
	using Lexer = GUIStyleSheetLexer;
public:
	GUIStyleSheetParser();
	bool Parse(const SPtr<SourceCode>& sourceCode);
	const String& GetErrors() const { return mErrors; }
	
private:
	bool IsCurrentToken(TokenType type) const;
	bool IsCurrentToken(TokenType type, const String& spelling) const;

	bool TryParseIntegerLiteral(i32& outValue);
	bool TryParseIntegerLiteral(u32& outValue);
	bool TryParsePixelLiteral(i32& outValue);
	bool TryParsePixelLiteral(u32& outValue);
	bool TryParseDecimalLiteral(float& outValue);
	bool TryParsePercentLiteral(float& outValue);
	bool TryParseStringLiteral(String& outValue);
	bool TryParseColor(Color& outValue);
	bool TryParseRectOffset(RectOffset& outValue);
	bool TryParseBorderStyle(GUIBorderElementStyle& outValue);
	bool TryParseTextAlign(GUIHorizontalTextAlignment& outValue);
	bool TryParseVerticalAlign(GUIVerticalTextAlignment& outValue);
	bool TryParseProperty(GUIStyleSheetStateStyle& inOutValue);
	bool TryParseSelector();

	bool TryParseInteger(const StringView& toParse, i32& outValue) const;
	bool TryParseFloat(const StringView& toParse, float& outValue) const;
	bool TryParseHexColor(const StringView& toParse, Color& outValue) const;

	TokenType GetCurrentTokenType() const { return mCurrentToken.has_value() ? mCurrentToken->GetType() : GUIStyleSheetTokenTypes::Undefined; }
	Optional<Token> GetCurrentToken() const { return mCurrentToken; }
	Optional<Token> GetCurrentTokenAndAdvance();
	Optional<Token> GetCurrentTokenAndAdvance(TokenType expectedType);
	Optional<Token> GetCurrentTokenAndAdvance(TokenType expectedType, const String& spelling);
	void SkipToken(TokenType type);

	Optional<Token> Error(const String& message);
	Optional<Token> ErrorUnexpected();
	Optional<Token> ErrorUnexpected(TokenType expectedTokenType);
	Optional<Token> ErrorUnexpected(const String& expectedTokenSpelling);

	SPtr<SourceCode> mSourceCode;
	GUIStyleSheetLexer mLexer;
	Optional<Token> mCurrentToken;

	String mErrors;

	UnorderedMap<String, GUIStyleSheetPropertyType> mPropertyKeywords;
};

GUIStyleSheetParser::GUIStyleSheetParser()
{
	mPropertyKeywords["width"] = GUIStyleSheetPropertyType::Width;
	mPropertyKeywords["height"] = GUIStyleSheetPropertyType::Height;
	mPropertyKeywords["min-width"] = GUIStyleSheetPropertyType::MinWidth;
	mPropertyKeywords["min-height"] = GUIStyleSheetPropertyType::MinHeight;
	mPropertyKeywords["max-width"] = GUIStyleSheetPropertyType::MaxWidth;
	mPropertyKeywords["max-height"] = GUIStyleSheetPropertyType::MaxHeight;

	// Margin
	mPropertyKeywords["margin"] = GUIStyleSheetPropertyType::Margin;
	mPropertyKeywords["margin-top"] = GUIStyleSheetPropertyType::MarginTop;
	mPropertyKeywords["margin-bottom"] = GUIStyleSheetPropertyType::MarginBottom;
	mPropertyKeywords["margin-left"] = GUIStyleSheetPropertyType::MarginLeft;
	mPropertyKeywords["margin-right"] = GUIStyleSheetPropertyType::MarginRight;

	// Padding
	mPropertyKeywords["padding"] = GUIStyleSheetPropertyType::Padding;
	mPropertyKeywords["padding-top"] = GUIStyleSheetPropertyType::PaddingTop;
	mPropertyKeywords["padding-bottom"] = GUIStyleSheetPropertyType::PaddingBottom;
	mPropertyKeywords["padding-left"] = GUIStyleSheetPropertyType::PaddingLeft;
	mPropertyKeywords["padding-right"] = GUIStyleSheetPropertyType::PaddingRight;

	// Color properties
	mPropertyKeywords["color"] = GUIStyleSheetPropertyType::Color;
	mPropertyKeywords["opacity"] = GUIStyleSheetPropertyType::Opacity;
	mPropertyKeywords["background-color"] = GUIStyleSheetPropertyType::BackgroundColor;

	// Text properties
	mPropertyKeywords["text-align"] = GUIStyleSheetPropertyType::TextAlign;
	mPropertyKeywords["vertical-align"] = GUIStyleSheetPropertyType::VerticalAlign;
	mPropertyKeywords["font-family"] = GUIStyleSheetPropertyType::FontFamily;
	mPropertyKeywords["font-size"] = GUIStyleSheetPropertyType::FontSize;

	// Border properties
	mPropertyKeywords["border"] = GUIStyleSheetPropertyType::Border;
	mPropertyKeywords["border-style"] = GUIStyleSheetPropertyType::BorderStyle;
	mPropertyKeywords["border-width"] = GUIStyleSheetPropertyType::BorderWidth;
	mPropertyKeywords["border-color"] = GUIStyleSheetPropertyType::BorderColor;
	mPropertyKeywords["border-top"] = GUIStyleSheetPropertyType::BorderTop;
	mPropertyKeywords["border-top-style"] = GUIStyleSheetPropertyType::BorderTopStyle;
	mPropertyKeywords["border-top-width"] = GUIStyleSheetPropertyType::BorderTopWidth;
	mPropertyKeywords["border-top-color"] = GUIStyleSheetPropertyType::BorderTopColor;
	mPropertyKeywords["border-bottom"] = GUIStyleSheetPropertyType::BorderBottom;
	mPropertyKeywords["border-bottom-style"] = GUIStyleSheetPropertyType::BorderBottomStyle;
	mPropertyKeywords["border-bottom-width"] = GUIStyleSheetPropertyType::BorderBottomWidth;
	mPropertyKeywords["border-bottom-color"] = GUIStyleSheetPropertyType::BorderBottomColor;
	mPropertyKeywords["border-left"] = GUIStyleSheetPropertyType::BorderLeft;
	mPropertyKeywords["border-left-style"] = GUIStyleSheetPropertyType::BorderLeftStyle;
	mPropertyKeywords["border-left-width"] = GUIStyleSheetPropertyType::BorderLeftWidth;
	mPropertyKeywords["border-left-color"] = GUIStyleSheetPropertyType::BorderLeftColor;
	mPropertyKeywords["border-right"] = GUIStyleSheetPropertyType::BorderRight;
	mPropertyKeywords["border-right-style"] = GUIStyleSheetPropertyType::BorderRightStyle;
	mPropertyKeywords["border-right-width"] = GUIStyleSheetPropertyType::BorderRightWidth;
	mPropertyKeywords["border-right-color"] = GUIStyleSheetPropertyType::BorderRightColor;
	mPropertyKeywords["border-radius"] = GUIStyleSheetPropertyType::BorderRadius;
	mPropertyKeywords["border-top-left-radius"] = GUIStyleSheetPropertyType::BorderTopLeftRadius;
	mPropertyKeywords["border-top-right-radius"] = GUIStyleSheetPropertyType::BorderTopRightRadius;
	mPropertyKeywords["border-bottom-left-radius"] = GUIStyleSheetPropertyType::BorderBottomLeftRadius;
	mPropertyKeywords["border-bottom-right-radius"] = GUIStyleSheetPropertyType::BorderBottomRightRadius;
}

bool GUIStyleSheetParser::Parse(const SPtr<SourceCode>& sourceCode)
{
	mLexer.StartScanning(sourceCode);

	// Grabs the first token
	GetCurrentTokenAndAdvance();

	// TODO well defined:
	// - Add parse for 9 border shorthands
	// - Add parse for margin & padding shorthands
	// - Add parse for text properties
	// - Add token to string mapping
	// - Implement style lookup
	// - Implement variable parse

	// TODO: Proof of concept for reading variable values

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

	while(!IsCurrentToken(GUIStyleSheetTokenTypes::RightCurly))
	{
		if(IsCurrentToken(GUIStyleSheetTokenTypes::EndOfStream))
		{
			Error("Unexpected end of stream.");
			return false;
		}

		if(IsCurrentToken(GUIStyleSheetTokenTypes::VariableIdentifier))
		{
			// TODO - Parse variables
			// - If this is a global selector, register all variables in a global lookup table in some context struct we pass around
			// - Otherwise, register variables in local context

			if(!GetCurrentTokenAndAdvance(GUIStyleSheetTokenTypes::Semicolon))
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
#define CASE_PARSE(Type, PropertyName, FieldName)                                                \
	case GUIStyleSheetPropertyType::PropertyName:                                                \
		{                                                                                        \
			if(!TryParse##Type(inOutValue.FieldName))                                            \
				return false;                                                                    \
                                                                                                 \
			inOutValue.OverridenProperties[(u32)GUIStyleSheetPropertyType::PropertyName] = true; \
			break;                                                                               \
		}

		switch(foundProperty->second)
		{
			// Size
			CASE_PARSE(PixelLiteral, Width, Size.Width)
			CASE_PARSE(PixelLiteral, Height, Size.Height)
			CASE_PARSE(PixelLiteral, MinWidth, MinimumSize.Width)
			CASE_PARSE(PixelLiteral, MinHeight, MinimumSize.Height)
			CASE_PARSE(PixelLiteral, MaxWidth, MaximumSize.Width)
			CASE_PARSE(PixelLiteral, MaxHeight, MaximumSize.Height)

			// Margin
			CASE_PARSE(PixelLiteral, MarginLeft, Margins.Left)
			CASE_PARSE(PixelLiteral, MarginRight, Margins.Right)
			CASE_PARSE(PixelLiteral, MarginTop, Margins.Top)
			CASE_PARSE(PixelLiteral, MarginBottom, Margins.Bottom)

			// Padding
			CASE_PARSE(PixelLiteral, PaddingLeft, Padding.Left)
			CASE_PARSE(PixelLiteral, PaddingRight, Padding.Right)
			CASE_PARSE(PixelLiteral, PaddingTop, Padding.Top)
			CASE_PARSE(PixelLiteral, PaddingBottom, Padding.Bottom)

			// Color
			CASE_PARSE(Color, Color, Color)
			CASE_PARSE(DecimalLiteral, Opacity, Opacity)
			CASE_PARSE(Color, BackgroundColor, BackgroundColor)

			// Text
			CASE_PARSE(StringLiteral, FontFamily, FontFamily)
			CASE_PARSE(IntegerLiteral, FontSize, FontSize)
			CASE_PARSE(TextAlign, TextAlign, HorizontalTextAlignment)
			CASE_PARSE(VerticalAlign, VerticalAlign, VerticalTextAlignment)

			// Border
			CASE_PARSE(PixelLiteral, BorderLeftWidth, BorderLeft.Width)
			CASE_PARSE(Color, BorderLeftColor, BorderLeft.Color)
			CASE_PARSE(BorderStyle, BorderLeftStyle, BorderLeft.Style)

			CASE_PARSE(PixelLiteral, BorderRightWidth, BorderRight.Width)
			CASE_PARSE(Color, BorderRightColor, BorderRight.Color)
			CASE_PARSE(BorderStyle, BorderRightStyle, BorderRight.Style)

			CASE_PARSE(PixelLiteral, BorderTopWidth, BorderTop.Width)
			CASE_PARSE(Color, BorderTopColor, BorderTop.Color)
			CASE_PARSE(BorderStyle, BorderTopStyle, BorderTop.Style)

			CASE_PARSE(PixelLiteral, BorderBottomWidth, BorderBottom.Width)
			CASE_PARSE(Color, BorderBottomColor, BorderBottom.Color)
			CASE_PARSE(BorderStyle, BorderBottomStyle, BorderBottom.Style)

			CASE_PARSE(PixelLiteral, BorderTopLeftRadius, BorderTopLeftRadius)
			CASE_PARSE(PixelLiteral, BorderTopRightRadius, BorderTopRightRadius)
			CASE_PARSE(PixelLiteral, BorderBottomLeftRadius, BorderBottomLeftRadius)
			CASE_PARSE(PixelLiteral, BorderBottomRightRadius, BorderBottomRightRadius)

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

			// TODO - Other shorthands
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

bool GUIStyleSheetParser::IsCurrentToken(TokenType type) const
{
	return mCurrentToken && mCurrentToken->GetType() == type;
}

bool GUIStyleSheetParser::IsCurrentToken(TokenType type, const String& spelling) const
{
	return mCurrentToken && mCurrentToken->GetType() == type && mCurrentToken->GetSpelling() == spelling;
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


//bool GUIStyleSheetParser::TryParsePercentOrDecimalLiteral(float& outValue)
//{
//	if(!IsCurrentToken(GUIStyleSheetTokenTypes::DecimalLiteral) && !IsCurrentToken(GUIStyleSheetTokenTypes::PercentLiteral))
//	{
//		Error(StringUtil::Format("Unexpected token '{0}', expected '{1}' or '{2}'", Token::TypeToString(mCurrentToken->GetType()), Token::TypeToString(GUIStyleSheetTokenTypes::DecimalLiteral), Token::TypeToString(GUIStyleSheetTokenTypes::PercentLiteral)));
//		return false;
//	}
//
//	Optional<GUIStyleSheetToken> token = GetCurrentTokenAndAdvance();
//	if(!token)
//		return false;
//
//	return TryParseFloat(token->GetSpelling(), outValue);
//}
//
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

bool GUIStyleSheetParser::TryParseRectOffset(RectOffset& outValue)
{
	i32 parsedValue;
	if(!TryParsePixelLiteral(parsedValue))
		return false;

	outValue.Left = parsedValue;
	outValue.Right = parsedValue;
	outValue.Top = parsedValue;
	outValue.Bottom = parsedValue;

	if(IsCurrentToken(GUIStyleSheetTokenTypes::Semicolon))
		return true;

	if(!TryParsePixelLiteral(parsedValue))
		return false;

	outValue.Left = parsedValue;
	outValue.Right = parsedValue;

	if(IsCurrentToken(GUIStyleSheetTokenTypes::Semicolon))
		return true;

	if(!TryParsePixelLiteral(parsedValue))
		return false;

	outValue.Bottom = parsedValue;

	if(IsCurrentToken(GUIStyleSheetTokenTypes::Semicolon))
		return true;

	if(!TryParsePixelLiteral(parsedValue))
		return false;

	outValue.Left = parsedValue;
	return true;
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

Optional<GUIStyleSheetParser::Token> GUIStyleSheetParser::GetCurrentTokenAndAdvance()
{
	if(mCurrentToken && mCurrentToken->GetType() == TokenType::EndOfStream)
		return Error("Unexpected end of stream.");

	Optional<Token> previousToken = mCurrentToken;
	mCurrentToken = mLexer.ScanNextToken();

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

void GUIStyleSheetStateStyle::Override(const GUIStyleSheetStateStyle& other)
{
#define OVERRIDE_PROPERTY(PropertyName, FieldName)                              \
	if(other.OverridenProperties[(u32)GUIStyleSheetPropertyType::PropertyName]) \
		(FieldName) = other.FieldName;

	OVERRIDE_PROPERTY(Width, Size.Width)
	OVERRIDE_PROPERTY(Height, Size.Height)
	OVERRIDE_PROPERTY(MinWidth, MinimumSize.Width)
	OVERRIDE_PROPERTY(MinHeight, MinimumSize.Height)
	OVERRIDE_PROPERTY(MaxWidth, MaximumSize.Width)
	OVERRIDE_PROPERTY(MaxHeight, MaximumSize.Height)

	OVERRIDE_PROPERTY(MarginTop, Margins.Top)
	OVERRIDE_PROPERTY(MarginBottom, Margins.Bottom)
	OVERRIDE_PROPERTY(MarginLeft, Margins.Left)
	OVERRIDE_PROPERTY(MarginRight, Margins.Right)

	OVERRIDE_PROPERTY(PaddingTop, Padding.Top)
	OVERRIDE_PROPERTY(PaddingBottom, Padding.Bottom)
	OVERRIDE_PROPERTY(PaddingLeft, Padding.Left)
	OVERRIDE_PROPERTY(PaddingRight, Padding.Right)

	OVERRIDE_PROPERTY(Color, Color)
	OVERRIDE_PROPERTY(Opacity, Opacity)
	OVERRIDE_PROPERTY(BackgroundColor, BackgroundColor)

	OVERRIDE_PROPERTY(TextAlign, HorizontalTextAlignment)
	OVERRIDE_PROPERTY(VerticalAlign, VerticalTextAlignment)
	OVERRIDE_PROPERTY(FontFamily, FontFamily)
	OVERRIDE_PROPERTY(FontSize, FontSize)

	OVERRIDE_PROPERTY(BorderTopStyle, BorderTop.Style)
	OVERRIDE_PROPERTY(BorderTopWidth, BorderTop.Width)
	OVERRIDE_PROPERTY(BorderTopColor, BorderTop.Color)

	OVERRIDE_PROPERTY(BorderBottomStyle, BorderBottom.Style)
	OVERRIDE_PROPERTY(BorderBottomWidth, BorderBottom.Width)
	OVERRIDE_PROPERTY(BorderBottomColor, BorderBottom.Color)

	OVERRIDE_PROPERTY(BorderLeftStyle, BorderLeft.Style)
	OVERRIDE_PROPERTY(BorderLeftWidth, BorderLeft.Width)
	OVERRIDE_PROPERTY(BorderLeftColor, BorderLeft.Color)

	OVERRIDE_PROPERTY(BorderRightStyle, BorderRight.Style)
	OVERRIDE_PROPERTY(BorderRightWidth, BorderRight.Width)
	OVERRIDE_PROPERTY(BorderRightColor, BorderRight.Color)

	OVERRIDE_PROPERTY(BorderTopLeftRadius, BorderTopLeftRadius);
	OVERRIDE_PROPERTY(BorderTopRightRadius, BorderTopRightRadius);
	OVERRIDE_PROPERTY(BorderBottomLeftRadius, BorderBottomLeftRadius);
	OVERRIDE_PROPERTY(BorderBottomRightRadius, BorderBottomRightRadius);

#undef OVERRIDE_PROPERTY
	
}

Optional<GUIStyleSheet> GUIStyleSheet::Parse(const Path& file)
{
	const SPtr<DataStream> fileStream = FileSystem::OpenFile(file);
	if(!fileStream)
		return {};

	GUIStyleSheetParser parser;
	parser.Parse(B3DMakeShared<SourceCode>(fileStream->GetAsString()));
	
	// TODO
	return {};
}

GUIStyleSheetStateStyle GUIStyleSheet::FindStyle(const String& elementType, const String& elementId, GUIElementState state)
{
	// TODO - Cache the style

	// TODO:
	// - Find element type style, narrow down inhertiance chain to the particular state
	// - Then find id style, override everything from previous step

	auto fnLookupStateStyle = [](const GUIStyleSheetStyle& style, GUIElementState state)
	{
		GUIStyleSheetStateStyle stateStyle = style.Normal;

		// TODO - Handle this case:
		// - Red color is always used on hover, but the border radius is used from the focus state
		// - Unless border-radius is also specified in 'hover', in which case it is used

		//input:focus
		//	{
		//		background - color : lightblue;
		//		border - radius : 25px;
		//	}

		//input:hover
		//	{
		//		background - color : red;
		//	}

		// normal -> focused -> hover
		// normal -> disabled -> hover

		switch(state)
		{
		case GUIElementState::Normal: break;
		case GUIElementState::Hover:
			if(style.Hover.has_value())
				stateStyle.Override(*style.Hover);
			break;
		case GUIElementState::Active:
			if(style.Active.has_value())
				stateStyle.Override(*style.Active);
			break;
		case GUIElementState::Focused:

			break;
		case GUIElementState::FocusedHover: break;
		case GUIElementState::NormalOn: break;
		case GUIElementState::HoverOn: break;
		case GUIElementState::ActiveOn: break;
		case GUIElementState::FocusedOn: break;
		case GUIElementState::FocusedHoverOn: break;
		case GUIElementState::TypeMask: break;
		case GUIElementState::OnFlag: break;
		default: ;
		}

		return stateStyle;

	};

	GUIStyleSheetStateStyle style;

	//if(auto it = mElementStyles.find(elementType); it != mElementStyles.end())
	//{
	//	style = it->second;
	//}

	return style;
}

