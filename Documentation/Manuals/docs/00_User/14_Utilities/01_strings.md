Strings 						{#strings}
===============
[TOC]

Strings are represented with @ref bs::String "String" and @ref bs::WString "WString" types. These are wrappers for the standard C++ strings and have the same interface and behaviour.

~~~~~~~~~~~~~{.cpp}
String narrow = "NarrowString";
WString wide = L"WideString";
~~~~~~~~~~~~~

# String encoding {#strings_a}
When using the standard string operator "" note that your string will use platform-specific string encoding. On Windows this will be a single byte non-Unicode locale specific encoding limited to 255 characters, while on macOS and Linux this will be a multi-byte UTF8 encoding. Therefore on Windows you cannot use this operator to encode full range on Unicode values.

~~~~~~~~~~~~~{.cpp}
// On Windows only valid for 255 characters of the current locale
String narrow = "NarrowString";

// On Windows this will not be encoded properly as these characters are unlikely to all be present
// in the current locale
String invalidNarrow = "Š¶¤ÞÐ";
~~~~~~~~~~~~~

Therefore if you need to support a whole range of Unicode characters make sure to either use **WString** and "L" prefix, or even better **String** and "u8" prefix. Otherwise you risk that your character wont be encoded properly for all platforms.

~~~~~~~~~~~~~{.cpp}
// Wide strings will always properly encode Unicode, but use unnecessarily large 32-bit UTF32 on Linux/macOS
WString validWide = L"Š¶¤ÞÐ";

// Best option is to use narrow strings and force UTF8 encoding
String validNarrow = u8"Š¶¤ÞÐ";
~~~~~~~~~~~~~

# Converting between encodings {#strings_b}
bs::f provides a variety of methods to convert between most common string encodings. This functionality is provided in the @ref bs::UTF8 "UTF8" class. For example use @ref bs::UTF8::fromANSI "UTF8::fromANSI()" to convert from locale-specific encoding to UTF8, and @ref bs::UTF8::toANSI "UTF8::toANSI()" for other way around. Conversions for UTF-16 and UTF-32 are also provided.

~~~~~~~~~~~~~{.cpp}
// Assuming Windows platform

// Locale specific ANSI encoding
String strANSI = "NarrowString";

// Convert to UTF-8
String strUTF8 = UTF8::fromANSI(strANSI);

// And back to ANSI
strANSI = UTF8::toANSI(strUTF8);
~~~~~~~~~~~~~

# Converting data types {#strings_c}
You can convert most primitive data types to strings by using the @ref bs::toString "toString" or @ref bs::toWString "toWString" functions.

~~~~~~~~~~~~~{.cpp}
bool v1 = false;
int v2 = 244;

String str1 = toString(v1);
String str2 = toString(v2);
~~~~~~~~~~~~~

You can also do an opposite conversion, converting from a string to a primitive data type by calling one of the *parse* functions.

~~~~~~~~~~~~~{.cpp}
String str1 = "false";
String str2 = "244";

bool v1 = parseBool(str1, false);
int v2 = parseINT32(str2, 0);
~~~~~~~~~~~~~

If the system cannot properly parse the string, it will instead assign the default value provided.

# Manipulating strings {#strings_d}
Various forms of string manipulations can be performed via @ref bs::StringUtil "StringUtil", including but not limited to: making a string upper or lower case, replacing string elements, matching string elements, splitting strings based on delimiters and more.

~~~~~~~~~~~~~{.cpp}
String string = "124,355,banana,954";

// Split string into entries separated by ,
Vector<String> entries = StringUtil::split(string, ",");

// Replace all occurrences of "banana" within the string, with "643"
string = StringUtil::replaceAll(string, "banana", "643");
~~~~~~~~~~~~~

# Formatting strings {#strings_e}
Often you need to construct larger strings from other strings. Use @ref bs::StringUtil::format "StringUtil::format()" to construct such strings by providing a template string, which contains special identifiers for inserting other strings. The identifiers are represented like "{0}, {1}" in the source string, where the number represents the position of the parameter that will be used for replacing the identifier.

~~~~~~~~~~~~~{.cpp}
String templateStr = "Hello, my name is {0}.";
String str = StringUtil::format(templateStr, "bs::f");

// str now contains the string "Hello, my name is bs::f."
~~~~~~~~~~~~~