//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"

namespace bs
{
	/** @addtogroup Localization
	 *  @{
	 */

	/** Loosely based on ISO 639-1 two letter language codes */
	enum class Language
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

		WString string;
		UINT32 numParameters;
		ParamOffset* parameterOffsets; 

		void concatenateString(WString& outputString, WString* parameters, UINT32 numParameterValues) const;
		void updateString(const WString& string);
	};

	/** Data for a single language in the string table. */
	struct LanguageData
	{
		UnorderedMap<WString, SPtr<LocalizedStringData>> strings;
	};

	/** @} */
	/** @addtogroup Localization
	 *  @{
	 */

	/** Used for string localization. Stores strings and their translations in various languages. */
	class BS_CORE_EXPORT StringTable : public Resource
	{
		// TODO - When editing string table I will need to ensure that all languages of the same string have the same number of parameters

	public:
		StringTable();
		~StringTable();

		/** Returns all identifiers in the table. */
		const UnorderedSet<WString>& getIdentifiers() const { return mIdentifiers; }

		/**	Adds or modifies string translation for the specified language. */
		void setString(const WString& identifier, Language language, const WString& string);

		/**	Returns a string translation for the specified language. Returns the identifier itself if one doesn't exist. */
		WString getString(const WString& identifier, Language language);

		/** Removes the string described by identifier, from all languages. */
		void removeString(const WString& identifier);

		/**
		 * Gets a string data for the specified string identifier and currently active language.
		 *
		 * @param[in]	identifier		   	Unique string identifier.
		 * @param[in]	insertIfNonExisting	If true, a new string data for the specified identifier and language will be 
		 *									added to the table if data doesn't already exist. The data will use the 
		 *									identifier as the translation string.
		 * @return							The string data. Don't store reference to this data as it may get deleted.
		 */
		SPtr<LocalizedStringData> getStringData(const WString& identifier, bool insertIfNonExisting = true);

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
		SPtr<LocalizedStringData> getStringData(const WString& identifier, Language language, bool insertIfNonExisting = true);

		/** Creates a new empty string table resource. */
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

		UnorderedSet<WString> mIdentifiers;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class StringTableRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}