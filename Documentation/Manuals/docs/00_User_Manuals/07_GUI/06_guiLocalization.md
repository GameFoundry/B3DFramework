---
title: Localization
---

So far we haven't mentioned why all GUI elements use the @b3d::HString type for holding string information, instead of the raw **String** type.

**HString** is a localizable string, meaning the actual value of the string can be changed by changing the active language. This ensures you can easily create translations for GUI elements.

# Localizing strings
When creating a **HString** it takes an identifier as input, which must be unique.

~~~~~~~~~~~~~{.cpp}
HString myLocalizedString("_myStringId");
~~~~~~~~~~~~~

You can then assign values to that identifier through a @b3d::StringTable resource. You create a **StringTable** by calling @b3d::StringTable::Create.

~~~~~~~~~~~~~{.cpp}
HStringTable stringTable = StringTable::Create();
~~~~~~~~~~~~~

You can then use the string table to assign actual language strings for your localized string. This is done by calling @b3d::StringTable::SetString.

~~~~~~~~~~~~~{.cpp}
stringTable->SetString("_myStringId", Language::EnglishUS, "Hello!");
stringTable->SetString("_myStringId", Language::German, "Hallo!");
stringTable->SetString("_myStringId", Language::Spanish, "!Hola!");
~~~~~~~~~~~~~

Finally, you need to register the string table with @b3d::StringTableManager by calling @b3d::StringTableManager::SetTable. **StringTableManager** is accessible globally through @b3d::GetStringTableManager.

~~~~~~~~~~~~~{.cpp}
GetStringTableManager().SetTable(0, stringTable);
~~~~~~~~~~~~~

> Note: Multiple string tables are supported by giving them different identifiers. By default all **HString**%s will use the 0th string table, so it is suggest to always set that one for most common localizations.

After the string table is set you can call @b3d::StringTableManager::SetActiveLanguage to change the current language. If the string table has a localization for the specified language, it will be used by any GUI elements referencing the localized string. The default language is English.

~~~~~~~~~~~~~{.cpp}
GetStringTableManager().SetActiveLanguage(Language::German);
~~~~~~~~~~~~~

# Default localization
If you skip the step of creating the string table and assigning it to **StringTableManager**, the **HString** will use its identifier as the display string. In cases where you don't need localization you can just use actual display strings in the identifier field (as we have been doing so far).

~~~~~~~~~~~~~{.cpp}
HString myLocalizedString("Hello!");
~~~~~~~~~~~~~

> You can still localize this string. It will use "Hello!" as its value for the default language (English), which is also its identifier and can be used for assigning other values for it in a string table.

# Parameters
Localized strings can use parameters as placeholders to insert other data. This is useful when a localized string needs to contain information like numbers or other non-localized information. Use identifiers like "{0}", "{1}", etc. to specify parameters.

~~~~~~~~~~~~~{.cpp}
HString myLocalizedString("Hello my name is {0}, and I am {1} years old.");
~~~~~~~~~~~~~

Parameters can then be assigned by calling @b3d::HString::SetParameter.

~~~~~~~~~~~~~{.cpp}
myLocalizedString.SetParameter(0, "John");
myLocalizedString.SetParameter(1, "30");
~~~~~~~~~~~~~

> Note that translations in all languages should share the same number of parameters.
