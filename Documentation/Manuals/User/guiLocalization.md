Localization 						{#guiLocalization}
===============
[TOC]

So far we haven't mentioned why all GUI elements use the @ref bs::HString "HString" type for holding string information, instead of the raw **String** or **WString** types.

**HString** is a localizable string, meaning the actual value of the string can be changed by changing the active language. This ensures you can easily create translations for GUI elements.

# Localizing strings {#guiLocalization_a}
When creating a **HString** it takes an identifier as input, which must be unique.

~~~~~~~~~~~~~{.cpp}
HString myLocalizedString("_myStringId");
~~~~~~~~~~~~~

You can then assign values to that identifier through a @ref bs::StringTable "StringTable" resource. You create a **StringTable** by calling @ref bs::StringTable::create "StringTable::create()".

~~~~~~~~~~~~~{.cpp}
HStringTable stringTable = StringTable::create();
~~~~~~~~~~~~~

You can then use the string table to assign actual language strings for your localized string. This is done by calling @ref bs::StringTable::setString "StringTable::setString()". 

~~~~~~~~~~~~~{.cpp}
stringTable->setString("_myStringId", Language::EnglishUS, "Hello!");
stringTable->setString("_myStringId", Language::German, "Hallo!");
stringTable->setString("_myStringId", Language::Spanish, "!Hola!");
~~~~~~~~~~~~~

Finally, you need to register the string table with @ref bs::StringTableManager "StringTableManager" by calling @ref bs::StringTableManager::setTable "StringTableManager::setTable()". **StringTableManager** is accessible globally through @ref bs::gStringTableManager "gStringTableManager()".

~~~~~~~~~~~~~{.cpp}
gStringTableManager().setTable(0, stringTable);
~~~~~~~~~~~~~

> Note: Multiple string tables are supported by giving them different identifiers. By default all **HString**%s will use the 0th string table, so it is suggest to always set that one for most common localizations.

After the string table is set you can call @ref bs::StringTableManager::setActiveLanguage "StringTableManager::setActiveLanguage()" to change the current language. If the string table has a localization for the specified language, it will be used by any GUI elements referencing the localized string. The default language is English.

~~~~~~~~~~~~~{.cpp}
gStringTableManager().setActiveLanguage(Language::German);
~~~~~~~~~~~~~

# Default localization {#guiLocalization_b}
If you skip the step of creating the string table and assigning it to **StringTableManager**, the **HString** will use its identifier as the display string. In cases where you don't need localization you can just use actual display strings in the identifier field (as we have been doing so far).

~~~~~~~~~~~~~{.cpp}
HString myLocalizedString("Hello!");
~~~~~~~~~~~~~

> You can still localize this string. It will use "Hello!" as its value for the default language (English), which is also its identifier and can be used for assigning other values for it in a string table.

# Parameters {#guiLocalization_c}
Localized strings can use parameters as placeholders to insert other data. This is useful when a localized string needs to contain information like numbers or other non-localized information. Use identifiers like "{0}", "{1}", etc. to specify parameters.

~~~~~~~~~~~~~{.cpp}
HString myLocalizedString("Hello my name is {0}, and I am {1} years old.");
~~~~~~~~~~~~~

Parameters can then be assigned by calling @ref bs::HString::setParameter "HString::setParameter()".

~~~~~~~~~~~~~{.cpp}
myLocalizedString.setParameter(0, "John");
myLocalizedString.setParameter(1, "30");
~~~~~~~~~~~~~

> Note that translations in all languages should share the same number of parameters.