---
title: Strings
---

Strings are represented with @b3d::String and @b3d::WString types. These are wrappers for the standard C++ strings and have the same interface and behaviour.

~~~~~~~~~~~~~{.cpp}
String narrow = "NarrowString";
WString wide = L"WideString";
~~~~~~~~~~~~~

# String encoding
When using the standard string operator "" note that your string will use platform-specific string encoding. On Windows this will be a single byte non-Unicode locale specific encoding limited to 255 characters, while on macOS and Linux this will be a multi-byte UTF8 encoding. Therefore on Windows you cannot use this operator to encode full range on Unicode values.

~~~~~~~~~~~~~{.cpp}
// On Windows only valid for 255 characters of the current locale
String narrow = "NarrowString";

// On Windows this will not be encoded properly as these characters are unlikely to all be present
// in the current locale
String invalidNarrow = "�����";
~~~~~~~~~~~~~

Therefore if you need to support a whole range of Unicode characters make sure to either use **WString** and "L" prefix, or even better **String** and "u8" prefix. Otherwise you risk that your character wont be encoded properly for all platforms.

~~~~~~~~~~~~~{.cpp}
// Wide strings will always properly encode Unicode, but use unnecessarily large 32-bit UTF32 on Linux/macOS
WString validWide = L"�����";

// Best option is to use narrow strings and force UTF8 encoding
String validNarrow = u8"�����";
~~~~~~~~~~~~~

# Converting between encodings
The framework provides a variety of methods to convert between most common string encodings. This functionality is provided in the @b3d::UTF8 class. For example use @b3d::UTF8::FromANSI to convert from locale-specific encoding to UTF8, and @b3d::UTF8::ToANSI for other way around. Conversions for UTF-16 and UTF-32 are also provided.

~~~~~~~~~~~~~{.cpp}
// Assuming Windows platform

// Locale specific ANSI encoding
String ansiString = "NarrowString";

// Convert to UTF-8
String utf8String = UTF8::FromANSI(ansiString);

// And back to ANSI
ansiString = UTF8::ToANSI(utf8String);
~~~~~~~~~~~~~

# Converting data types
You can convert most primitive data types to strings by using the @b3d::ToString or @b3d::ToWString functions.

~~~~~~~~~~~~~{.cpp}
bool booleanValue = false;
i32 integerValue = 244;

String stringFromBoolean = ToString(booleanValue);
String stringFromInteger = ToString(integerValue);
~~~~~~~~~~~~~

You can also do an opposite conversion, converting from a string to a primitive data type by calling one of the *Parse* functions.

~~~~~~~~~~~~~{.cpp}
String booleanString = "false";
String integerString = "244";

bool parsedBoolean = ParseBool(booleanString, false);
i32 parsedInteger = Parsei32(integerString, 0);
~~~~~~~~~~~~~

If the system cannot properly parse the string, it will instead assign the default value provided.

# Manipulating strings
Various forms of string manipulations can be performed via @b3d::StringUtility, including but not limited to: making a string upper or lower case, replacing string elements, matching string elements, splitting strings based on delimiters and more.

~~~~~~~~~~~~~{.cpp}
String commaDelimitedString = "124,355,banana,954";

// Split string into entries separated by ,
Vector<String> stringEntries = StringUtility::Split(commaDelimitedString, ",");

// Replace all occurrences of "banana" within the string, with "643"
commaDelimitedString = StringUtility::ReplaceAll(commaDelimitedString, "banana", "643");
~~~~~~~~~~~~~

# Formatting strings
Often you need to construct larger strings from other strings. Use @b3d::StringUtility::Format to construct such strings by providing a template string, which contains special identifiers for inserting other strings. The identifiers are represented like "{0}, {1}" in the source string, where the number represents the position of the parameter that will be used for replacing the identifier.

~~~~~~~~~~~~~{.cpp}
String templateString = "Hello, my name is {0}.";
String formattedString = StringUtility::Format(templateString, "B3D Framework");

// formattedString now contains the string "Hello, my name is B3D Framework."
~~~~~~~~~~~~~
