//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"

namespace bs
{
	/** @addtogroup Localization
	 *  @{
	 */

	/**
	 * A set of all languages that localized strings can be translated to. Loosely based on ISO 639-1 two letter language
	 * codes.
	 */
	enum class BS_SCRIPT_EXPORT(m:Localization) Language
	{
		Afar, 
		Abkhazian, 
		Avestan, 
		Afrikaans, 
		Akan, 
		Amharic, 
		Aragonese, 
		Arabic, 
		Assamese, 
		Avaric, 
		Aymara, 
		Azerbaijani, 
		Bashkir, 
		Belarusian, 
		Bulgarian, 
		Bihari, 
		Bislama, 
		Bambara, 
		Bengali, 
		Tibetan, 
		Breton, 
		Bosnian, 
		Catalan, 
		Chechen, 
		Chamorro, 
		Corsican, 
		Cree, 
		Czech, 
		ChurchSlavic,
		Chuvash, 
		Welsh, 
		Danish, 
		German, 
		Maldivian, 
		Bhutani, 
		Ewe, 		
		Greek, 
		EnglishUK, 
		EnglishUS,
		Esperanto, 
		Spanish, 
		Estonian, 
		Basque, 
		Persian, 
		Fulah, 
		Finnish, 
		Fijian, 
		Faroese, 
		French, 
		WesternFrisian, 
		Irish, 
		ScottishGaelic, 
		Galician, 
		Guarani, 
		Gujarati, 
		Manx, 
		Hausa, 
		Hebrew, 
		Hindi, 
		HiriMotu, 
		Croatian, 
		Haitian, 
		Hungarian, 
		Armenian, 
		Herero, 
		Interlingua, 
		Indonesian, 
		Interlingue, 
		Igbo, 
		SichuanYi, 
		Inupiak, 
		Ido, 
		Icelandic, 
		Italian, 
		Inuktitut, 
		Japanese, 
		Javanese, 
		Georgian, 
		Kongo, 
		Kikuyu, 
		Kuanyama, 
		Kazakh, 
		Kalaallisut, 
		Cambodian, 
		Kannada, 
		Korean, 
		Kanuri, 
		Kashmiri, 
		Kurdish, 
		Komi, 
		Cornish, 
		Kirghiz, 
		Latin, 
		Luxembourgish, 
		Ganda, 
		Limburgish,
		Lingala, 
		Laotian, 
		Lithuanian, 
		LubaKatanga, 
		Latvian,
		Malagasy, 
		Marshallese, 
		Maori, 
		Macedonian, 
		Malayalam, 
		Mongolian, 
		Moldavian, 
		Marathi, 
		Malay, 
		Maltese, 
		Burmese, 
		Nauru, 
		NorwegianBokmal, 
		Ndebele, 
		Nepali, 
		Ndonga, 
		Dutch, 
		NorwegianNynorsk, 
		Norwegian, 
		Navaho, 
		Nyanja, 
		Provencal, 
		Ojibwa, 
		Oromo, 
		Oriya, 
		Ossetic, 
		Punjabi, 
		Pali, 
		Polish, 
		Pushto, 
		Portuguese, 
		Quechua, 
		Romansh, 
		Kirundi, 
		Romanian, 
		Russian, 
		Kinyarwanda, 
		Sanskrit, 
		Sardinian, 
		Sindhi, 
		NorthernSami, 
		Sangro, 
		Sinhalese, 
		Slovak, 
		Slovenian, 
		Samoan, 
		Shona, 
		Somali, 
		Albanian, 
		Serbian, 
		Swati,
		Sesotho,
		Sundanese, 
		Swedish, 
		Swahili, 
		Tamil, 
		Telugu, 
		Tajik, 
		Thai, 
		Tigrinya, 
		Turkmen, 
		Tagalog, 
		Setswana, 
		Tonga, 
		Turkish, 
		Tsonga, 
		Tatar,
		Twi, 
		Tahitian, 
		Uighur, 
		Ukrainian, 
		Urdu, 
		Uzbek, 
		Venda, 
		Vietnamese, 
		Volapuk, 
		Walloon, 
		Wolof, 
		Xhosa, 
		Yiddish, 
		Yoruba, 
		Zhuang,
		Chinese,
		Zulu,
		Count // Number of entries
	};

	/** @} */
	/** @addtogroup Localization-Internal
	 *  @{
	 */

	/**
	 * Internal data used for representing a localized string instance. for example a specific instance of a localized
	 * string using specific parameters.
	 */
	struct LocalizedStringData
	{
		struct ParamOffset
		{
			ParamOffset()
				:paramIdx(0), location(0)
			{ }

			ParamOffset(UINT32 _paramIdx, UINT32 _location)
				:paramIdx(_paramIdx), location(_location)
			{ }

			UINT32 paramIdx;
			UINT32 location;
		};

		LocalizedStringData();
		~LocalizedStringData();

		String string;
		UINT32 numParameters;
		ParamOffset* parameterOffsets; 

		void concatenateString(String& outputString, String* parameters, UINT32 numParameterValues) const;
		void updateString(const String& string);
	};

	/** Data for a single language in the string table. */
	struct LanguageData
	{
		UnorderedMap<String, SPtr<LocalizedStringData>> strings;
	};

	/** @} */
	/** @addtogroup Localization
	 *  @{
	 */

	/** Used for string localization. Stores strings and their translations in various languages. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Localization) StringTable : public Resource
	{
		// TODO - When editing string table I will need to ensure that all languages of the same string have the same number of parameters

	public:
		StringTable();
		~StringTable();

		/**
		 * Checks does the string table contain the provided identifier.
		 *
		 * @param[in]	identifier		Identifier to look for.
		 * @return						True if the identifier exists in the table, false otherwise.
		 */
		BS_SCRIPT_EXPORT()
		bool contains(const String& identifier);

		/** Returns a total number of strings in the table. */
		BS_SCRIPT_EXPORT(n:NumStrings,pr:getter)
		UINT32 getNumStrings() const { return (UINT32)mIdentifiers.size(); }

		/** Returns all identifiers that the string table contains localized strings for. */
		BS_SCRIPT_EXPORT(n:Identifiers,pr:getter)
		Vector<String> getIdentifiers() const;

		/**	Adds or modifies string translation for the specified language. */
		BS_SCRIPT_EXPORT()
		void setString(const String& identifier, Language language, const String& value);

		/**	Returns a string translation for the specified language. Returns the identifier itself if one doesn't exist. */
		BS_SCRIPT_EXPORT()
		String getString(const String& identifier, Language language);

		/** Removes the string described by identifier, from all languages. */
		BS_SCRIPT_EXPORT()
		void removeString(const String& identifier);

		/**
		 * Gets a string data for the specified string identifier and currently active language.
		 *
		 * @param[in]	identifier		   	Unique string identifier.
		 * @param[in]	insertIfNonExisting	If true, a new string data for the specified identifier and language will be 
		 *									added to the table if data doesn't already exist. The data will use the 
		 *									identifier as the translation string.
		 * @return							The string data. Don't store reference to this data as it may get deleted.
		 */
		SPtr<LocalizedStringData> getStringData(const String& identifier, bool insertIfNonExisting = true);

		/**
		 * Gets a string data for the specified string identifier and language.
		 *
		 * @param[in]	identifier		   	Unique string identifier.
		 * @param[in]	language		   	Language.
		 * @param[in]	insertIfNonExisting	If true, a new string data for the specified identifier and language will be 
		 *									added to the table if data doesn't already exist. The data will use the 
		 *									identifier as the translation string.
		 * @return							The string data. Don't store reference to this data as it may get deleted.
		 */
		SPtr<LocalizedStringData> getStringData(const String& identifier, Language language, bool insertIfNonExisting = true);

		/** Creates a new empty string table resource. */
		BS_SCRIPT_EXPORT(ec:StringTable)
		static HStringTable create();

		static const Language DEFAULT_LANGUAGE;
	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates a new empty string table resource.
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<StringTable> _createPtr();

		/** @} */
	private:
		friend class HString;
		friend class StringTableManager;

		/** Gets the currently active language. */
		Language getActiveLanguage() const { return mActiveLanguage; }

		/** Changes the currently active language. Any newly created strings will use this value. */
		void setActiveLanguage(Language language);

		Language mActiveLanguage;
		LanguageData* mActiveLanguageData;
		LanguageData* mDefaultLanguageData;

		LanguageData* mAllLanguages;

		UnorderedSet<String> mIdentifiers;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class StringTableRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}