//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGUIStyleSheetLexer.h"
#include "B3DGUIStyleSheet.h"
#include "Image/B3DColor.h"
#include "Utility/B3DBitfield.h"
#include "Utility/B3DRectOffset.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Parses a GUI style sheet source file and outputs a GUIStyleSheet. */
	class B3D_EXPORT GUIStyleSheetParser
	{
		using Token = GUIStyleSheetToken;
		using TokenType = GUIStyleSheetTokenTypes;
		using Lexer = GUIStyleSheetLexer;
	public:
		GUIStyleSheetParser();

		/** Attempts to parse the provided style sheet file and outputs the parsed style sheet, if successful. */
		SPtr<GUIStyleSheet> Parse(const SPtr<SourceCode>& sourceCode);

		/** Returns errors in case parsing failed. */
		const String& GetErrors() const { return mErrors; }

		/** Returns any warnings generated during the parse process. */
		String GetWarnings() const { return mWarnings.str(); }
		
	private:
		/** All possible types for property values. */
		enum class ValueType
		{
			Undefined,
			Integer,
			Pixel,
			Decimal,
			Percent,
			Color,
			String,
			URL,
			Icon,
			BorderStyle,
			TextAlign,
			VerticalAlign,
			WordWrap,
			Visibility,
			None,
			Multiple
		};

		/** Information about an icon. */
		struct IconValue
		{
			u32 NameStringId;
			float IconSize;
		};

		/** Contains a union storing all possible types of property values. */
		struct VariableValue
		{
			union
			{
				u32 UnsignedInteger;
				i32 SignedInteger;
				float Float;
				Color Color;
				IconValue Icon;
			};

			ValueType Type = ValueType::Undefined;

			VariableValue()
				: UnsignedInteger(0)
			{ }

			VariableValue(u32 value, ValueType type)
				: UnsignedInteger(value), Type(type)
			{ }

			VariableValue(i32 value, ValueType type)
				: SignedInteger(value), Type(type)
			{ }

			VariableValue(float value, ValueType type)
				: Float(value), Type(type)
			{ }

			VariableValue(const class Color& value, ValueType type)
				: Color(value), Type(type)
			{ }

			VariableValue(const IconValue& value, ValueType type)
				: Icon(value), Type(type)
			{ }


			void GetValue(u32& outValue) const
			{
				B3D_ASSERT(Type == ValueType::Integer || Type == ValueType::Pixel || Type == ValueType::String || Type == ValueType::URL || Type == ValueType::Decimal);

				if(Type == ValueType::Decimal)
					outValue = (u32)Float;
				else
					outValue = UnsignedInteger;
			}

			void GetValue(i32& outValue) const
			{
				B3D_ASSERT(Type == ValueType::Integer || Type == ValueType::Pixel || Type == ValueType::String || Type == ValueType::URL || Type == ValueType::Decimal);

				if(Type == ValueType::Decimal)
					outValue = (i32)Float;
				else
					outValue = SignedInteger;
			}

			void GetValue(float& outValue) const
			{
				B3D_ASSERT(Type == ValueType::Decimal || Type == ValueType::Percent || Type == ValueType::Integer);

				if(Type == ValueType::Integer)
					outValue = (float)SignedInteger;
				else
					outValue = Float;
			}

			void GetValue(class Color& outValue) const
			{
				B3D_ASSERT(Type == ValueType::Color);
				outValue = Color;
			}

			void GetValue(IconValue& outValue) const
			{
				B3D_ASSERT(Type == ValueType::Icon);
				outValue = Icon;
			}

			void GetValue(GUIBorderElementStyle& outValue)
			{
				if(Type == ValueType::BorderStyle)
					outValue = (GUIBorderElementStyle)UnsignedInteger;
				else if(Type == ValueType::None)
					outValue = GUIBorderElementStyle::None;
				else
					B3D_ASSERT(false);
			}

			void GetValue(GUIHorizontalTextAlignment& outValue)
			{
				B3D_ASSERT(Type == ValueType::TextAlign);
				outValue = (GUIHorizontalTextAlignment)UnsignedInteger;
			}

			void GetValue(GUIVerticalTextAlignment& outValue)
			{
				B3D_ASSERT(Type == ValueType::VerticalAlign);
				outValue = (GUIVerticalTextAlignment)UnsignedInteger;
			}

			void GetValue(GUIWordWrapMode& outValue)
			{
				if(Type == ValueType::WordWrap)
					outValue = (GUIWordWrapMode)UnsignedInteger;
				else if(Type == ValueType::None)
					outValue = GUIWordWrapMode::None;
				else
					B3D_ASSERT(false);
			}

			void GetValue(GUIElementVisibility& outValue)
			{
				B3D_ASSERT(Type == ValueType::Visibility);
				outValue = (GUIElementVisibility)UnsignedInteger;
			}
		};

		/** Stores a set of variables defined in a particular scope. */
		struct VariableContext
		{
			UnorderedMap<String, VariableValue> Variables;
		};

		/** Description of a single property. */
		struct PropertyInformation
		{
			PropertyInformation(GUIStyleSheetPropertyType propertyType = GUIStyleSheetPropertyType::Undefined, ValueType valueType = ValueType::Undefined)
				: PropertyType(propertyType), ValueType(valueType) {}

			GUIStyleSheetPropertyType PropertyType = GUIStyleSheetPropertyType::Undefined;
			ValueType ValueType = ValueType::Undefined;
		};

		/**
		 * @name Property value (Literals)
		 * @{
		 */

		/** Attempts to parse the next token as an integer literal. Integer literal is detected as a set of digits with no decimal point or suffix. If successful, returns true and outputs the parsed value. */
		bool TryParseIntegerLiteral(i32& outValue);

		/** Attempts to parse the next token as an integer literal. Integer literal is detected as a set of digits with no decimal point or suffix. If successful, returns true and outputs the parsed value. */
		bool TryParseIntegerLiteral(u32& outValue);

		/** Attempts to parse the next token as a pixel literal. Pixel literal is detected as a set of digits with no decimal point with 'px' suffix. If successful, returns true and outputs the parsed value. */
		bool TryParsePixelLiteral(i32& outValue);

		/** Attempts to parse the next token as a pixel literal. Pixel literal is detected as a set of digits with no decimal point with 'px' suffix. If successful, returns true and outputs the parsed value. */
		bool TryParsePixelLiteral(u32& outValue);

		/** Attempts to parse the next token as a decimal literal. Decimal literal is detected as a set of digits with an optional decimal point. If successful, returns true and outputs the parsed value. */
		bool TryParseDecimalLiteral(float& outValue);

		/** Attempts to parse the next token as a percent literal. Percent literal is detected as a set of digits with an optional decimal point, with a '%' suffix. If successful, returns true and outputs the parsed value. */
		bool TryParsePercentLiteral(float& outValue);

		/** Attempts to parse the next token as a string literal. String literal is detected as a pair of '"', with an optional text in-between. If successful, returns true and outputs the parsed value. */
		bool TryParseStringLiteral(String& outValue);

		/** Attempts to parse the next token as an element selector. If successful, returns true and outputs the parsed value. */
		bool TryParseElementSelector(String& outValue);

		/** Attempts to parse the next token as 'none' token and returns true if successful. */
		bool TryParseNone();

		/** @} */

		/**
		 * @name Property value (Complex)
		 * @{
		 */

		/**
		 * @page StyleSheetSides
		 *
		 * If one value is parsed, all sides are assigned the parsed value.
		 * If two values are parsed, first value is assigned to top/bottom and second value to left/right sides.
		 * If three values are parsed, first value is assigned to top, second value to left/right, and third value to bottom side.
		 * If four values are parsed, first value is assigned to top, second value to right, third value to bottom, and fourth value to left side.
		 */

		/** Attempts to parse the next token (or set of tokens) as a color. Can be a hex color (#FFFFFF, rgb(), hsl(), rgba() or hsla() construct). If successful, returns true and outputs the parsed value. */
		bool TryParseColor(Color& outValue);

		/** Attempts to parse the next tokens as url("some-url"). If successful, returns true and outputs the parsed value. */
		bool TryParseURL(String& outValue);

		/** Attempts to parse the next tokens as icon(icon-name, icon-size). If successful, returns true and outputs the parsed value. */
		bool TryParseIcon(String& outIconName, float& outIconSize);

		/** Attempts to parse the next token as an image based on provided path or icon name. If successful, returns true and outputs the parsed value. */
		bool TryParseImage(HSpriteImage& outValue);

		/** Attempts to parse the next token as an image loaded from the resource system based on provided font name. If successful, returns true and outputs the parsed value. */
		bool TryParseFont(HFont& outValue);

		/**
		 * Attempts to parse the next set of 1 to 4 pixel values and assigns them to the rectangle offset sides. If successful, returns true and outputs the parsed value.
		 * @copydoc StyleSheetSides
		 */
		bool TryParseRectOffset(RectOffset& outValue);

		/** Attempts to parse the next token as a border style. Token must be one of supported border style identifiers. If successful, returns true and outputs the parsed value. */
		bool TryParseBorderStyle(GUIBorderElementStyle& outValue);

		/**
		 * Attempts to parse the next set of 1 to 4 tokens as border styles for each side. Each token must be one of supported border style identifiers. If successful, returns true and outputs the parsed values.
		 * @copydoc StyleSheetSides
		 */
		bool TryParseBorderStyle(GUIBorderElementStyle& outTop, GUIBorderElementStyle& outRight, GUIBorderElementStyle& outBottom, GUIBorderElementStyle& outLeft);

		/**
		 * Attempts to parse the next set of 1 to 4 tokens as pixel literals for each side. If successful, returns true and outputs the parsed values.
		 * @copydoc StyleSheetSides
		 */
		bool TryParseBorderWidth(u32& outTop, u32& outRight, u32& outBottom, u32& outLeft);

		/**
		 * Attempts to parse the next set of 1 to 4 tokens as colors for each side. If successful, returns true and outputs the parsed values.
		 * @copydoc StyleSheetSides
		 */
		bool TryParseBorderColor(Color& outTop, Color& outRight, Color& outBottom, Color& outLeft);

		/** Attempts to parse the next token as a text alignment style. Token must be one of supported text alignment identifiers. If successful, returns true and outputs the parsed value. */
		bool TryParseTextAlign(GUIHorizontalTextAlignment& outValue);

		/** Attempts to parse the next token as a vertical alignment style. Token must be one of supported vertical alignment identifiers. If successful, returns true and outputs the parsed value. */
		bool TryParseVerticalAlign(GUIVerticalTextAlignment& outValue);

		/** Attempts to parse the next token as a word wrap mode. Token must be one of supported word wrap mode identifiers. If successful, returns true and outputs the parsed value. */
		bool TryParseWordWrapMode(GUIWordWrapMode& outValue);

		/** Attempts to parse the next token as a visibility mode. Token must be one of supported visibility mode identifiers. If successful, returns true and outputs the parsed value. */
		bool TryParseVisibility(GUIElementVisibility& outValue);

		/** Attempts to parse the next 1 to 3 tokens as style, width and color for the border. Tokens may be provided in any order, but cannot be duplicated. If successful, returns true and outputs the parsed value. */
		bool TryParseBorderElement(GUIStyleSheetBorderElement& outValue);

		/**
		 * Attempts to parse the next set of 1 to 4 tokens as colors for each corner. If successful, returns true and outputs the parsed values.
		 *
		 * If one value is parsed, all corners are assigned the parsed value.
		 * If two values are parsed, first value is assigned to top-left/bottom-right and second value to bottom-left/top-right corners.
		 * If three values are parsed, first value is assigned to top-left, second value to bottom-left/top-right, and third value to bottom-right corner.
		 * If four values are parsed, first value is assigned to top-left, second value to top-right, third value to bottom-right, and fourth value to bottom-left corner.

		 */
		bool TryParseBorderRadius(u32& outTopLeft, u32& outTopRight, u32& outBottomLeft, u32& outBottomRight);

		/**
		 * Attempts to parse the variable value of the specified type. Internally redirects to the correct parsing method depending on the value type. 
		 *
		 * @param expectedType		Variable type to parse.
		 * @param outValue			Output value.
		 * @return					True if parsing is successful, false otherwise.
		 */
		bool TryParseAndLookupVariableValue(ValueType expectedType, VariableValue& outValue);

		/** @} */

		/**
		 * @name Higher level constructs
		 * @{
		 */

		/** Attempts to parse a property and its value. If successful, returns true and writes the property data in the provided data structure. */
		bool TryParseProperty(GUIStyleSheetRules& inOutValue);

		/** Attempts to parse a variable declaration and its value. If successful, returns true and appends (or overwrites) the variable in the provided context. */
		bool TryParseVariable(VariableContext& inOutVariableContext);

		/**
		 * Attempts to parse all properties and/or variable declarations in a particular ruleset. If successful returns true and the rule information is added
		 * to the local ruleset array.
		 */
		bool TryParseRuleset();

		/** Parses all selectors and generates a selector list, if any are provided.  */
		TOptional<GUIStyleSheetSelectorList> TryParseSelectorList();

		/** @} */

		/**
		 * @name Parse helpers
		 * @{
		 */

		/** Helper that parses between 1-4 tokens needed, assigning the correct values to output variables depending on how many tokens were parsed. Follows the rules from @ref StyleSheetSides. */
		template<class T>
		bool TryParsePropertyValueSides(ValueType valueType, T& outTop, T& outRight, T& outBottom, T& outLeft);

		/**
		 * Attempts to parse the property value of the specified type. Internally redirects to the correct parsing method depending on the property value type. If the value is determined
		 * to be a variable, variable lookup is performed in the existing variable contexts.
		 *
		 * @param valueType		Property type to parse.
		 * @param outValue		Output value.
		 * @return				True if parsing is successful, false otherwise.
		 */
		template<class T>
		bool TryParsePropertyValue(ValueType valueType, T& outValue);

		/**
		 * Parses a variable identifier used as a property value, and looks up the value of the variable in the current variable contexts.
		 *
		 * @param	expectedType		Value type as expected by the property.
		 * @param	outValue			Variable value, if successful.
		 * @return						True if successful and variable was found and was of correct type, false otherwise.
		 */
		template<class T>
		bool TryParseAndLookupVariableValue(ValueType expectedType, T& outValue);

		/** @copydoc TryParseAndLookupVariableValue(ValueType, T&) */
		bool TryParseAndLookupVariableValue(ValueType expectedType, String& outValue);

		/** @copydoc TryParseAndLookupVariableValue(ValueType, T&) */
		bool TryParseAndLookupVariableValue(ValueType expectedType, HSpriteImage& outValue);

		/** @copydoc TryParseAndLookupVariableValue(ValueType, T&) */
		bool TryParseAndLookupVariableValue(ValueType expectedType, HFont& outValue);

		/** Attempts to parse an integer from the provided string. Returns true if successful, and outputs the parsed integer. */
		bool TryParseInteger(const StringView& toParse, i32& outValue) const;

		/** Attempts to parse a float from the provided string. Returns true if successful, and outputs the parsed float. */
		bool TryParseFloat(const StringView& toParse, float& outValue) const;

		/** Attempts to parse a color from the provided hex string (e. #FFFFFF or #FFFFFFAA). Returns true if successful, and outputs the parsed color. */
		bool TryParseHexColor(const StringView& toParse, Color& outValue) const;

		/** @} */

		/**
		 * @name Token iteration and lookup
		 * @{
		 */

		/** Returns true if the current token is of the specified type. */
		bool IsCurrentToken(TokenType type) const;

		/** Returns true if the current token is of the specified type and has the provided spelling. */
		bool IsCurrentToken(TokenType type, const String& spelling) const;

		/** Returns the type of the current token. */
		TokenType GetCurrentTokenType() const { return mCurrentToken.has_value() ? mCurrentToken->GetType() : GUIStyleSheetTokenTypes::Undefined; }

		/** Returns the current token. */
		TOptional<Token> GetCurrentToken() const { return mCurrentToken; }

		/** Returns the current token and advances to the next token. */
		TOptional<Token> GetCurrentTokenAndAdvance(bool skipWhitespace = true);

		/** Returns the current token if it matches the provided type, and advances to the next token. If the type is not matching and error is logged and null is returned. */
		TOptional<Token> GetCurrentTokenAndAdvance(TokenType expectedType);

		/** Returns the current token if it matches the provided type and spelling, and advances to the next token. If the type or spelling is not matching and error is logged and null is returned. */
		TOptional<Token> GetCurrentTokenAndAdvance(TokenType expectedType, const String& spelling);

		/** Skips the current token if it matches the provided type. */
		void SkipToken(TokenType type);

		/**
		 * @name Error handling
		 * @{
		 */

		/** Records a warning message. */
		void Warning(const String& message);

		/** Records an error message and returns null. */
		TOptional<Token> Error(const String& message);

		/** Records an error message that the current token is unexpected and returns null. */
		TOptional<Token> ErrorUnexpected();

		/** Records an error message that the current token doesn't match @p expectedTokenType and returns null. */
		TOptional<Token> ErrorUnexpected(TokenType expectedTokenType);

		/** Records an error message that the current token doesn't match @p expectedTokenSpelling and returns null. */
		TOptional<Token> ErrorUnexpected(const String& expectedTokenSpelling);

		/** Converts property value type into a human readable string. */
		static const char* ValueTypeToString(ValueType type);

		/** Returns true if the values of the provided types can be cast between each other safely. */
		static bool CanCastValue(ValueType expectedType, ValueType receivedType);

		/** @} */

		SPtr<SourceCode> mSourceCode;
		GUIStyleSheetLexer mLexer;
		TOptional<Token> mCurrentToken;
		TArray<GUIStyleSheetRuleset> mParsedRulesets;

		VariableContext mGlobalVariableContext;
		VariableContext mLocalVariableContext;
		Vector<String> mStringLiterals;

		String mErrors;
		StringStream mWarnings;

		UnorderedMap<String, PropertyInformation> mPropertyKeywords;
	};


	/** @} */
} // namespace b3d
