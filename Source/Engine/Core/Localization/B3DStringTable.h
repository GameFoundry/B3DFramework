//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup Localization
	 *  @{
	 */

	/**
	 * A set of all languages that localized strings can be translated to. Loosely based on ISO 639-1 two letter language
	 * codes.
	 */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Localization)) Language
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
			ParamOffset() = default;

			ParamOffset(u32 _parameterIndex, u32 _location)
				: ParamIdx(_parameterIndex), Location(_location)
			{}

			u32 ParamIdx = 0;
			u32 Location = 0;
		};

		LocalizedStringData() = default;
		~LocalizedStringData();

		String String;
		u32 NumParameters = 0;
		ParamOffset* ParameterOffsets = nullptr;

		void ConcatenateString(b3d::String& outputString, b3d::String* parameters, u32 parameterValueCount) const;
		void UpdateString(const b3d::String& string);
	};

	/** Data for a single language in the string table. */
	struct LanguageData
	{
		UnorderedMap<String, TShared<LocalizedStringData>> Strings;
	};

	/** @} */
	/** @addtogroup Localization
	 *  @{
	 */

	/** Used for string localization. Stores strings and their translations in various languages. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Localization)) StringTable : public Resource
	{
		// TODO - When editing string table I will need to ensure that all languages of the same string have the same number of parameters

	public:
		StringTable();

		/**
		 * Checks does the string table contain the provided identifier.
		 *
		 * @param	identifier		Identifier to look for.
		 * @return					True if the identifier exists in the table, false otherwise.
		 */
		B3D_SCRIPT_EXPORT()
		bool Contains(const String& identifier);

		/** Returns a total number of strings in the table. */
		B3D_SCRIPT_EXPORT(ExportName(NumStrings), Property(Getter))

		u32 GetNumStrings() const { return (u32)mIdentifiers.size(); }

		/** Returns all identifiers that the string table contains localized strings for. */
		B3D_SCRIPT_EXPORT(ExportName(Identifiers), Property(Getter))
		Vector<String> GetIdentifiers() const;

		/**	Adds or modifies string translation for the specified language. */
		B3D_SCRIPT_EXPORT()
		void SetString(const String& identifier, Language language, const String& value);

		/**	Returns a string translation for the specified language. Returns the identifier itself if one doesn't exist. */
		B3D_SCRIPT_EXPORT()
		String GetString(const String& identifier, Language language);

		/** Removes the string described by identifier, from all languages. */
		B3D_SCRIPT_EXPORT()
		void RemoveString(const String& identifier);

		/**
		 * Gets a string data for the specified string identifier and currently active language.
		 *
		 * @param	identifier		   	Unique string identifier.
		 * @param	insertIfNonExisting	If true, a new string data for the specified identifier and language will be
		 *								added to the table if data doesn't already exist. The data will use the
		 *								identifier as the translation string.
		 * @return						The string data. Don't store reference to this data as it may get deleted.
		 */
		TShared<LocalizedStringData> GetStringData(const String& identifier, bool insertIfNonExisting = true);

		/**
		 * Gets a string data for the specified string identifier and language.
		 *
		 * @param	identifier		   	Unique string identifier.
		 * @param	language		   	Language.
		 * @param	insertIfNonExisting	If true, a new string data for the specified identifier and language will be
		 *								added to the table if data doesn't already exist. The data will use the
		 *								identifier as the translation string.
		 * @return						The string data. Don't store reference to this data as it may get deleted.
		 */
		TShared<LocalizedStringData> GetStringData(const String& identifier, Language language, bool insertIfNonExisting = true);

		/** Creates a new empty string table resource. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(StringTable))
		static HStringTable Create();

		static const Language kDefaultLanguage;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates a new empty string table resource.
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static TShared<StringTable> CreateShared();

		/** @} */
	private:
		friend class HString;
		friend class StringTableManager;

		/** Gets the currently active language. */
		Language GetActiveLanguage() const { return mActiveLanguage; }

		/** Changes the currently active language. Any newly created strings will use this value. */
		void SetActiveLanguage(Language language);

		Language mActiveLanguage;
		LanguageData* mActiveLanguageData;
		LanguageData* mDefaultLanguageData;

		TArray<LanguageData> mAllLanguages;

		UnorderedSet<String> mIdentifiers;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class StringTableRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
