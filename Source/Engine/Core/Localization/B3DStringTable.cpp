//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Localization/B3DStringTable.h"
#include "Resources/B3DResources.h"
#include "RTTI/B3DStringTableRTTI.h"

using namespace b3d;

const Language StringTable::kDefaultLanguage = Language::EnglishUS;

LocalizedStringData::~LocalizedStringData()
{
	if(ParameterOffsets != nullptr)
		B3DDeleteMultiple(ParameterOffsets, NumParameters);
}

void LocalizedStringData::ConcatenateString(b3d::String& outputString, b3d::String* parameters, u32 parameterValueCount) const
{
	// A safeguard in case translated strings have different number of parameters
	u32 actualParameterCount = std::min(parameterValueCount, NumParameters);

	if(parameters != nullptr)
	{
		u32 totalCharacterCount = 0;
		u32 previousIndex = 0;
		for(u32 parameterIndex = 0; parameterIndex < actualParameterCount; parameterIndex++)
		{
			totalCharacterCount += (ParameterOffsets[parameterIndex].Location - previousIndex) + (u32)parameters[ParameterOffsets[parameterIndex].ParamIdx].size();
			;

			previousIndex = ParameterOffsets[parameterIndex].Location;
		}

		totalCharacterCount += (u32)String.size() - previousIndex;

		outputString.resize(totalCharacterCount);
		char* stringData = &outputString[0]; // String contiguity required by C++11, but this should work elsewhere as well

		previousIndex = 0;
		for(u32 parameterIndex = 0; parameterIndex < actualParameterCount; parameterIndex++)
		{
			u32 stringSize = ParameterOffsets[parameterIndex].Location - previousIndex;
			memcpy(stringData, &String[previousIndex], stringSize * sizeof(char));
			stringData += stringSize;

			b3d::String& parameter = parameters[ParameterOffsets[parameterIndex].ParamIdx];
			memcpy(stringData, &parameter[0], parameter.size() * sizeof(char));
			stringData += parameter.size();

			previousIndex = ParameterOffsets[parameterIndex].Location;
		}

		memcpy(stringData, &String[previousIndex], (String.size() - previousIndex) * sizeof(char));
	}
	else
	{
		outputString.resize(String.size());
		char* stringData = &outputString[0]; // String contiguity required by C++11, but this should work elsewhere as well

		memcpy(stringData, &String[0], String.size() * sizeof(char));
	}
}

void LocalizedStringData::UpdateString(const b3d::String& _string)
{
	if(ParameterOffsets != nullptr)
		B3DDeleteMultiple(ParameterOffsets, NumParameters);

	Vector<ParamOffset> paramOffsets;

	i32 lastBracket = -1;
	StringStream bracketChars;
	StringStream cleanString;
	bool escaped = false;
	u32 removedCharacterCount = 0;
	for(u32 charIndex = 0; charIndex < (u32)_string.size(); charIndex++)
	{
		if(_string[charIndex] == '^' && !escaped)
		{
			removedCharacterCount++;
			escaped = true;
			continue;
		}

		if(lastBracket == -1)
		{
			// If current char is non-escaped opening bracket start parameter definition
			if(_string[charIndex] == '{' && !escaped)
				lastBracket = charIndex;
			else
				cleanString << _string[charIndex];
		}
		else
		{
			if(isdigit(_string[charIndex]))
				bracketChars << _string[charIndex];
			else
			{
				// If current char is non-escaped closing bracket end parameter definition
				u32 parameterCharacterCount = (u32)bracketChars.tellp();
				if(_string[charIndex] == '}' && parameterCharacterCount > 0 && !escaped)
				{
					removedCharacterCount += parameterCharacterCount + 2; // +2 for open and closed brackets

					u32 parameterIndex = ParseU32(bracketChars.str());
					paramOffsets.push_back(ParamOffset(parameterIndex, charIndex + 1 - removedCharacterCount));
				}
				else
				{
					// Last bracket wasn't really a parameter
					for(u32 bracketCharIndex = lastBracket; bracketCharIndex <= charIndex; bracketCharIndex++)
						cleanString << _string[bracketCharIndex];
				}

				lastBracket = -1;

				bracketChars.str(u8"");
				bracketChars.clear();
			}
		}

		escaped = false;
	}

	String = cleanString.str();
	NumParameters = (u32)paramOffsets.size();

	// Try to find out of order param offsets and fix them
	std::sort(begin(paramOffsets), end(paramOffsets), [](const ParamOffset& a, const ParamOffset& b)
	{
		return a.ParamIdx < b.ParamIdx;
	});

	if(paramOffsets.size() > 0)
	{
		u32 sequentialIndex = 0;
		u32 lastParameterIndex = paramOffsets[0].ParamIdx;
		for(u32 offsetIndex = 0; offsetIndex < NumParameters; offsetIndex++)
		{
			if(paramOffsets[offsetIndex].ParamIdx == lastParameterIndex)
			{
				paramOffsets[offsetIndex].ParamIdx = sequentialIndex;
				continue;
			}

			lastParameterIndex = paramOffsets[offsetIndex].ParamIdx;
			sequentialIndex++;

			paramOffsets[offsetIndex].ParamIdx = sequentialIndex;
		}
	}

	// Re-sort based on location since we find that more useful at runtime
	std::sort(begin(paramOffsets), end(paramOffsets), [](const ParamOffset& a, const ParamOffset& b)
	{
		return a.Location < b.Location;
	});

	ParameterOffsets = B3DNewMultiple<ParamOffset>(NumParameters);
	for(u32 offsetIndex = 0; offsetIndex < NumParameters; offsetIndex++)
		ParameterOffsets[offsetIndex] = paramOffsets[offsetIndex];
}

StringTable::StringTable()
	: Resource(false), mActiveLanguageData(nullptr), mDefaultLanguageData(nullptr)
{
	mAllLanguages.Resize((u32)Language::Count);

	mDefaultLanguageData = &(mAllLanguages[(u32)kDefaultLanguage]);
	mActiveLanguageData = mDefaultLanguageData;
	mActiveLanguage = kDefaultLanguage;
}

void StringTable::SetActiveLanguage(Language language)
{
	if(language == mActiveLanguage)
		return;

	mActiveLanguageData = &(mAllLanguages[(u32)language]);
	mActiveLanguage = language;
}

bool StringTable::Contains(const String& identifier)
{
	return mIdentifiers.find(identifier) == mIdentifiers.end();
}

Vector<String> StringTable::GetIdentifiers() const
{
	Vector<String> output;
	for(auto& entry : mIdentifiers)
		output.push_back(entry);

	return output;
}

void StringTable::SetString(const String& identifier, Language language, const String& value)
{
	LanguageData* curLanguage = &(mAllLanguages[(u32)language]);

	auto found = curLanguage->Strings.find(identifier);

	SPtr<LocalizedStringData> stringData;
	if(found == curLanguage->Strings.end())
	{
		stringData = B3DMakeShared<LocalizedStringData>();
		curLanguage->Strings[identifier] = stringData;
	}
	else
	{
		stringData = found->second;
	}

	mIdentifiers.insert(identifier);
	stringData->UpdateString(value);
}

String StringTable::GetString(const String& identifier, Language language)
{
	LanguageData* curLanguage = &(mAllLanguages[(u32)language]);

	auto found = curLanguage->Strings.find(identifier);
	if(found != curLanguage->Strings.end())
		return found->second->String;

	return identifier;
}

void StringTable::RemoveString(const String& identifier)
{
	for(u32 languageIndex = 0; languageIndex < (u32)Language::Count; languageIndex++)
	{
		mAllLanguages[languageIndex].Strings.erase(identifier);
	}

	mIdentifiers.erase(identifier);
}

SPtr<LocalizedStringData> StringTable::GetStringData(const String& identifier, bool insertIfNonExisting)
{
	return GetStringData(identifier, mActiveLanguage, insertIfNonExisting);
}

SPtr<LocalizedStringData> StringTable::GetStringData(const String& identifier, Language language, bool insertIfNonExisting)
{
	LanguageData* curLanguage = &(mAllLanguages[(u32)language]);

	auto found = curLanguage->Strings.find(identifier);
	if(found != curLanguage->Strings.end())
		return found->second;

	auto defaultFound = mDefaultLanguageData->Strings.find(identifier);
	if(defaultFound != mDefaultLanguageData->Strings.end())
		return defaultFound->second;

	if(insertIfNonExisting)
	{
		SetString(identifier, kDefaultLanguage, identifier);

		auto defaultFound = mDefaultLanguageData->Strings.find(identifier);
		if(defaultFound != mDefaultLanguageData->Strings.end())
			return defaultFound->second;
	}

	B3D_ENSURE_LOG(false, "There is no string data for the provided identifier.");
	return nullptr;
}

HStringTable StringTable::Create()
{
	return B3DStaticResourceCast<StringTable>(GetResources().CreateResourceHandle(CreateShared()));
}

SPtr<StringTable> StringTable::CreateShared()
{
	SPtr<StringTable> stringTable = B3DMakeSharedFromExisting<StringTable>(new(B3DAllocate<StringTable>()) StringTable());
	stringTable->SetShared(stringTable);
	stringTable->Initialize();

	return stringTable;
}

RTTIType* StringTable::GetRttiStatic()
{
	return StringTableRTTI::Instance();
}

RTTIType* StringTable::GetRtti() const
{
	return StringTable::GetRttiStatic();
}
