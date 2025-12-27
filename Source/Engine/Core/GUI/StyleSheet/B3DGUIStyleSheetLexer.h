//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/StyleSheet/B3DGUIStyleSheetToken.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Scans style sheet code and outputs token that can then by used by the parser to parse style sheet code. */
	class B3D_EXPORT GUIStyleSheetLexer
	{
		using Token = GUIStyleSheetToken;
		using TokenType = GUIStyleSheetTokenTypes;
	public:
		GUIStyleSheetLexer();

		/** Starts scanning the provided code. Calling this method starts (or restarts scanning). Returns true if the scanning started successfully. */
		bool StartScanning(const SPtr<SourceCode>& sourceCode);

		/**
		 * Returns the current token and advances to the next token. EndOfStream token will be returned once end of stream has been reached. 
		 * Returns null if token scanning failed. Error that caused the failure can be read from GetErrors().
		 */
		TOptional<GUIStyleSheetToken> ScanNextToken(bool skipWhitespace = true);

		/** Returns errors that occurred during scanning, if any. */
		const String& GetErrors() const { return mErrors; }
		
	private:
		/** Checks is the current character a newline character. */
		bool IsCurrentCharacterNewLine() const { return (mCurrentCharacter == '\n' || mCurrentCharacter == '\r'); }

		/** Checks is the current character the provided character. */
		bool IsCurrentCharacter(char character) const { return (mCurrentCharacter == character); }

		/** Returns the current character. */
		char GetCurrentCharacter() const { return mCurrentCharacter; }

		/** Returns the current character and advances to the next character. */
		char GetCurrentCharacterAndAdvance();

		/**
		 * Reads the current character and compares it to the expected value. If the value matches the character is output in @p outCharacter, stream
		 * is advanced to the next character and the method returns true.
		 *
		 * If the value does not match the expected character an error is logged and false is returned.
		 */
		bool GetCurrentCharacterAndAdvance(char expected, char& outCharacter);

		/**
		 *  Constructs a new token. 
		 *
		 * @param type				Type of the token to create.
		 * @param takeCharacter		If true, the current character is consumed and the stream advanced. The token spelling will be the taken character.
		 * @return					Created token.
		 */
		GUIStyleSheetToken CreateToken(const TokenType& type, bool takeCharacter = false);

		/**
		 *  Constructs a new token. 
		 *
		 * @param type				Type of the token to create.
		 * @param spelling			Text that was scanned matching the token.
		 * @param takeCharacter		If true, the current character is consumed and the stream advanced. The token spelling will be appended with the taken character.
		 * @return					Created token.
		 */
		GUIStyleSheetToken CreateToken(const TokenType& type, String& spelling, bool takeCharacter = false);

		/**
		 *  Constructs a new token. 
		 *
		 * @param type					Type of the token to create.
		 * @param spelling				Text that was scanned matching the token.
		 * @param sourceCodePosition	Explicit source code position at which the token is located.
		 * @param takeCharacter			If true, the current character is consumed and the stream advanced. The token spelling will be appended with the taken character.
		 * @return						Created token.
		 */
		GUIStyleSheetToken CreateToken(const TokenType& type, String& spelling, const SourceCodePosition& sourceCodePosition, bool takeCharacter = false);

		/** Advances the stream as long as the characters are matching the provided predicate (predicate returns true). */
		void SkipMatching(const Function<bool(char)>& predicate);

		/**
		 * Advances the stream until the current character is not a whitespace.
		 *
		 * @param	includeNewLines		If true, newline are counted as whitespace and will also be skipped.
		 */
		void SkipWhiteSpaces(bool includeNewLines = true);

		/** Saves current source code position to be used for any following call using the source code position (e.g. error reporting). */
		void SaveCurrentSourcePosition();

		/** Tries to scan the next token and returns the token if scanning is successful. Token type will be automatically determined based on the current character. */
		TOptional<Token> ScanToken();

		/** Tries to scan the next token as an element selector identifier or a hex color (both starting with #). Returns null if the scanning failed. */
		TOptional<Token> ScanElementSelectorOrHexColor();

		/** Tries to scan the next token as an identifier. Returns null if the scanning failed. */
		TOptional<Token> ScanIdentifier(bool isStartingWithDot);

		/** Tries to scan the next token as a string literal (text surrounded by ""). Returns null if the scanning failed. */
		TOptional<Token> ScanStringLiteral();

		/** Tries to scan the next token as a number or a class selector. Assumes the current token is a '.'. Returns null if the scanning failed. */
		TOptional<Token> ScanNumberOrClassSelector();

		/** Tries to scan the next token as a number. Returns null if the scanning failed. */
		TOptional<Token> ScanNumber(bool isStartingWithDot);

		/** Records an error message and returns null. */
		TOptional<Token> Error(const String& message);

		/** Records an error message that the current character is unexpected and returns null. */
		TOptional<Token> ErrorUnexpected();

		/** Records an error message that the current character doesn't match @p expectedCharacter and returns null. */
		TOptional<Token> ErrorUnexpected(char expectedCharacter);

		SPtr<SourceCode> mSourceCode;
		char mCurrentCharacter = 0;
		SourceCodePosition mCurrentPosition;
		String mErrors;

		UnorderedMap<String, TokenType> mPropertyKeywords;
	};


	/** @} */
} // namespace b3d
