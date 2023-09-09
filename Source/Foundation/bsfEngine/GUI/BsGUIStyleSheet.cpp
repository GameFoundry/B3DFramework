//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIStyleSheet.h"

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
	SourceCode(const SPtr<std::istream>& stream);

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

	SPtr<std::istream> mStream;
	String mCurrentLine;
	Vector<String> mReadLines;
	SourceCodePosition mPosition;
};

SourceCode::SourceCode(const SPtr<std::istream>& stream)
	: mStream(stream)
{
}

bool SourceCode::IsValid() const
{
	return (mStream != nullptr && mStream->good());
}

char SourceCode::GetNextCharacter()
{
	// Check if reader is at end-of-line
	while(mPosition.GetColumn() >= mCurrentLine.size())
	{
		// Check if end-of-file is reached.
		if(!IsValid() || mStream->eof())
			return 0;

		// Read new line in source file
		std::getline(*mStream, mCurrentLine);
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
	StateSelector, // active, hover, focus, checked, disabled

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
	String mSpelling;
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

	UnorderedMap<String, TokenType> mKeywords;
};

GUIStyleSheetLexer::GUIStyleSheetLexer()
{
	// Size properties
	mKeywords["width"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["height"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["min-width"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["min-height"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["max-width"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["max-height"] = GUIStyleSheetTokenTypes::Property;

	// Margin
	mKeywords["margin"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["margin-top"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["margin-bottom"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["margin-left"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["margin-right"] = GUIStyleSheetTokenTypes::Property;

	// Padding
	mKeywords["padding"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["padding-top"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["padding-bottom"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["padding-left"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["padding-right"] = GUIStyleSheetTokenTypes::Property;

	// Color properties
	mKeywords["color"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["opacity"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["background-color"] = GUIStyleSheetTokenTypes::Property;

	// Text properties
	mKeywords["text-align"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["vertical-align"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["font-family"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["font-size"] = GUIStyleSheetTokenTypes::Property;

	// Border properties
	mKeywords["border"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-style"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-width"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-color"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-top"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-top-style"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-top-width"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-top-color"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-bottom"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-bottom-style"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-bottom-width"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-bottom-color"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-left"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-left-style"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-left-width"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-left-color"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-right"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-right-style"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-right-width"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-right-color"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-radius"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-top-left-radius"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-top-right-radius"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-bottom-left-radius"] = GUIStyleSheetTokenTypes::Property;
	mKeywords["border-bottom-right-radius"] = GUIStyleSheetTokenTypes::Property;

	// Border styles
	mKeywords["none"] = GUIStyleSheetTokenTypes::BorderStyle;
	mKeywords["solid"] = GUIStyleSheetTokenTypes::BorderStyle;

	// Selector
	mKeywords["active"] = GUIStyleSheetTokenTypes::StateSelector;
	mKeywords["hover"] = GUIStyleSheetTokenTypes::StateSelector;
	mKeywords["focus"] = GUIStyleSheetTokenTypes::StateSelector;
	mKeywords["checked"] = GUIStyleSheetTokenTypes::StateSelector;
	mKeywords["disabled"] = GUIStyleSheetTokenTypes::StateSelector;

	// Keywords
	mKeywords["var"] = GUIStyleSheetTokenTypes::Variable;
	mKeywords["rgb"] = GUIStyleSheetTokenTypes::ColorRGB;
	mKeywords["hsl"] = GUIStyleSheetTokenTypes::ColorHSL;
	mKeywords["rgba"] = GUIStyleSheetTokenTypes::ColorRGBA;
	mKeywords["hsla"] = GUIStyleSheetTokenTypes::ColorHSLA;
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
	if(!isVariable)
		spelling += firstCharacter;
	else
	{
		char unused;
		if(!GetCurrentCharacterAndAdvance('-', unused))
			return {};
	}

	// First character of the name can be a letter, '_' or '-'. Special case for '-' as we already parsed it above in case this is not a variable or a name identifier.
	if(std::isalpha(GetCurrentCharacter()) || IsCurrentCharacter('_') || ((isVariable || isNameIdentifier) && IsCurrentCharacter('-')))
	{
		spelling += GetCurrentCharacterAndAdvance();

		while(std::isalnum(GetCurrentCharacter()) || IsCurrentCharacter('_') || IsCurrentCharacter('-'))
			spelling += GetCurrentCharacterAndAdvance();
	}

	if(auto it = mKeywords.find(spelling); it != mKeywords.end())
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

		if(IsCurrentCharacter('p'))
		{
			GetCurrentCharacterAndAdvance();
			if(IsCurrentCharacter('x'))
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
	bool StartParsing(const SPtr<SourceCode>& sourceCode);
	
private:
	bool IsCurrentToken(TokenType type) const;
	bool IsCurrentToken(TokenType type, const String& spelling) const;
	Optional<Token> GetCurrentToken() const { return mCurrentToken; }
	Optional<Token> GetCurrentTokenAndAdvance();
	Optional<Token> GetCurrentTokenAndAdvance(TokenType expectedType);
	Optional<Token> GetCurrentTokenAndAdvance(TokenType expectedType, const String& spelling);

	SPtr<SourceCode> mSourceCode;
	GUIStyleSheetLexer mLexer;
	Optional<Token> mCurrentToken;
};

