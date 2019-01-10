//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Localization/BsStringTable.h"
#include "Error/BsException.h"
#include "Resources/BsResources.h"
#include "Private/RTTI/BsStringTableRTTI.h"

namespace bs
{
	const Language StringTable::DEFAULT_LANGUAGE = Language::EnglishUS;

	LocalizedStringData::~LocalizedStringData()
	{
		if(parameterOffsets != nullptr)
			bs_deleteN(parameterOffsets, numParameters);
	}

	void LocalizedStringData::concatenateString(String& outputString, String* parameters, UINT32 numParameterValues) const
	{
		// A safeguard in case translated strings have different number of parameters
		UINT32 actualNumParameters = std::min(numParameterValues, numParameters);
		
		if(parameters != nullptr)
		{
			UINT32 totalNumChars = 0;
			UINT32 prevIdx = 0;
			for(UINT32 i = 0; i < actualNumParameters; i++)
			{
				totalNumChars += (parameterOffsets[i].location - prevIdx) + (UINT32)parameters[parameterOffsets[i].paramIdx].size();;

				prevIdx = parameterOffsets[i].location;
			}

			totalNumChars += (UINT32)string.size() - prevIdx;

			outputString.resize(totalNumChars);
			char* strData = &outputString[0]; // String contiguity required by C++11, but this should work elsewhere as well

			prevIdx = 0;
			for(UINT32 i = 0; i < actualNumParameters; i++)
			{
				UINT32 strSize = parameterOffsets[i].location - prevIdx;
				memcpy(strData, &string[prevIdx], strSize * sizeof(char));
				strData += strSize;

				String& param = parameters[parameterOffsets[i].paramIdx];
				memcpy(strData, &param[0], param.size() * sizeof(char));
				strData += param.size();

				prevIdx = parameterOffsets[i].location;
			}

			memcpy(strData, &string[prevIdx], (string.size() - prevIdx) * sizeof(char));
		}
		else
		{
			outputString.resize(string.size());
			char* strData = &outputString[0]; // String contiguity required by C++11, but this should work elsewhere as well

			memcpy(strData, &string[0], string.size() * sizeof(char));
		}
	}

	void LocalizedStringData::updateString(const String& _string)
	{
		if(parameterOffsets != nullptr)
			bs_deleteN(parameterOffsets, numParameters);

		Vector<ParamOffset> paramOffsets;

		INT32 lastBracket = -1;
		StringStream bracketChars;
		StringStream cleanString;
		bool escaped = false;
		UINT32 numRemovedChars = 0;
		for(UINT32 i = 0; i < (UINT32)_string.size(); i++)
		{
			if(_string[i] == '^' && !escaped)
			{
				numRemovedChars++;
				escaped = true;
				continue;
			}

			if(lastBracket == -1)
			{
				// If current char is non-escaped opening bracket start parameter definition
				if(_string[i] == '{' && !escaped) 
					lastBracket = i;
				else
					cleanString<<_string[i];
			}
			else
			{
				if(isdigit(_string[i]))
					bracketChars<<_string[i];
				else
				{
					// If current char is non-escaped closing bracket end parameter definition
					UINT32 numParamChars = (UINT32)bracketChars.tellp();
					if(_string[i] == '}' && numParamChars > 0 && !escaped) 
					{
						numRemovedChars += numParamChars + 2; // +2 for open and closed brackets

						UINT32 paramIdx = parseUINT32(bracketChars.str());
						paramOffsets.push_back(ParamOffset(paramIdx, i + 1 - numRemovedChars));
					}
					else
					{
						// Last bracket wasn't really a parameter
						for(UINT32 j = lastBracket; j <= i; j++)
							cleanString<<_string[j];
					}

					lastBracket = -1;

					bracketChars.str(u8"");
					bracketChars.clear();
				}
			}

			escaped = false;
		}

		string = cleanString.str();
		numParameters = (UINT32)paramOffsets.size();

		// Try to find out of order param offsets and fix them
		std::sort(begin(paramOffsets), end(paramOffsets), 
			[&] (const ParamOffset& a, const ParamOffset& b) { return a.paramIdx < b.paramIdx; } );

		if(paramOffsets.size() > 0)
		{
			UINT32 sequentialIdx = 0;
			UINT32 lastParamIdx = paramOffsets[0].paramIdx;
			for(UINT32 i = 0; i < numParameters; i++)
			{
				if(paramOffsets[i].paramIdx == lastParamIdx)
				{
					paramOffsets[i].paramIdx = sequentialIdx;
					continue;
				}

				lastParamIdx = paramOffsets[i].paramIdx;
				sequentialIdx++;

				paramOffsets[i].paramIdx = sequentialIdx;
			}
		}

		// Re-sort based on location since we find that more useful at runtime
		std::sort(begin(paramOffsets), end(paramOffsets), 
			[&] (const ParamOffset& a, const ParamOffset& b) { return a.location < b.location; } );

		parameterOffsets = bs_newN<ParamOffset>(numParameters);
		for(UINT32 i = 0; i < numParameters; i++)
			parameterOffsets[i] = paramOffsets[i];
	}

	StringTable::StringTable()
		:Resource(false), mActiveLanguageData(nullptr), mDefaultLanguageData(nullptr), mAllLanguages(nullptr)
	{
		mAllLanguages = bs_newN<LanguageData>((UINT32)Language::Count);

		mDefaultLanguageData = &(mAllLanguages[(UINT32)DEFAULT_LANGUAGE]);
		mActiveLanguageData = mDefaultLanguageData;
		mActiveLanguage = DEFAULT_LANGUAGE;
	}
	
	StringTable::~StringTable()
	{
		bs_deleteN(mAllLanguages, (UINT32)Language::Count);
	}

	void StringTable::setActiveLanguage(Language language)
	{
		if(language == mActiveLanguage)
			return;

		mActiveLanguageData = &(mAllLanguages[(UINT32)language]);
		mActiveLanguage = language;
	}

	bool StringTable::contains(const String& identifier)
	{
		return mIdentifiers.find(identifier) == mIdentifiers.end();
	}

	Vector<String> StringTable::getIdentifiers() const
	{
		Vector<String> output;
		for (auto& entry : mIdentifiers)
			output.push_back(entry);

		return output;
	}

	void StringTable::setString(const String& identifier, Language language, const String& value)
	{
		LanguageData* curLanguage = &(mAllLanguages[(UINT32)language]);

		auto iterFind = curLanguage->strings.find(identifier);

		SPtr<LocalizedStringData> stringData;
		if(iterFind == curLanguage->strings.end())
		{
			stringData = bs_shared_ptr_new<LocalizedStringData>();
			curLanguage->strings[identifier] = stringData;
		}
		else
		{
			stringData = iterFind->second;
		}

		mIdentifiers.insert(identifier);
		stringData->updateString(value);
	}

	String StringTable::getString(const String& identifier, Language language)
	{
		LanguageData* curLanguage = &(mAllLanguages[(UINT32)language]);

		auto iterFind = curLanguage->strings.find(identifier);
		if (iterFind != curLanguage->strings.end())
			return iterFind->second->string;
			
		return identifier;
	}

	void StringTable::removeString(const String& identifier)
	{
		for(UINT32 i = 0; i < (UINT32)Language::Count; i++)
		{
			mAllLanguages[i].strings.erase(identifier);
		}

		mIdentifiers.erase(identifier);
	}

	SPtr<LocalizedStringData> StringTable::getStringData(const String& identifier, bool insertIfNonExisting)
	{
		return getStringData(identifier, mActiveLanguage, insertIfNonExisting);
	}

	SPtr<LocalizedStringData> StringTable::getStringData(const String& identifier, Language language, bool insertIfNonExisting)
	{
		LanguageData* curLanguage = &(mAllLanguages[(UINT32)language]);

		auto iterFind = curLanguage->strings.find(identifier);
		if(iterFind != curLanguage->strings.end())
			return iterFind->second;

		auto defaultIterFind = mDefaultLanguageData->strings.find(identifier);
		if(defaultIterFind != mDefaultLanguageData->strings.end())
			return defaultIterFind->second;

		if(insertIfNonExisting)
		{
			setString(identifier, DEFAULT_LANGUAGE, identifier);

			auto defaultIterFind = mDefaultLanguageData->strings.find(identifier);
			if(defaultIterFind != mDefaultLanguageData->strings.end())
				return defaultIterFind->second;
		}

		BS_EXCEPT(InvalidParametersException, "There is no string data for the provided identifier.");
		return nullptr;
	}

	HStringTable StringTable::create()
	{
		return static_resource_cast<StringTable>(gResources()._createResourceHandle(_createPtr()));
	}

	SPtr<StringTable> StringTable::_createPtr()
	{
		SPtr<StringTable> scriptCodePtr = bs_core_ptr<StringTable>(
			new (bs_alloc<StringTable>()) StringTable());
		scriptCodePtr->_setThisPtr(scriptCodePtr);
		scriptCodePtr->initialize();

		return scriptCodePtr;
	}

	RTTITypeBase* StringTable::getRTTIStatic()
	{
		return StringTableRTTI::instance();
	}

	RTTITypeBase* StringTable::getRTTI() const
	{
		return StringTable::getRTTIStatic();
	}
}