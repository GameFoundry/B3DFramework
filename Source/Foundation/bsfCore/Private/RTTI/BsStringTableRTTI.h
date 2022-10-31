//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "Localization/BsStringTable.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT StringTableRTTI : public RTTIType<StringTable, Resource, StringTableRTTI>
	{
	private:
		Language& GetActiveLanguage(StringTable* obj) { return obj->mActiveLanguage; }

		void SetActiveLanguage(StringTable* obj, Language& val) { obj->mActiveLanguage = val; }

		LanguageData& GetLanguageData(StringTable* obj, u32 idx) { return obj->mAllLanguages[idx]; }

		void SetLanguageData(StringTable* obj, u32 idx, LanguageData& val) { obj->mAllLanguages[idx] = val; }

		u32 GetNumLanguages(StringTable* obj) { return (u32)Language::Count; }

		void SetNumLanguages(StringTable* obj, u32 val)
		{ /* Do nothing */
		}

		UnorderedSet<String>& GetIdentifiers(StringTable* obj) { return obj->mIdentifiers; }

		void SetIdentifiers(StringTable* obj, UnorderedSet<String>& val) { obj->mIdentifiers = val; }

	public:
		StringTableRTTI()
		{
			AddPlainField("mActiveLanguage", 0, &StringTableRTTI::GetActiveLanguage, &StringTableRTTI::SetActiveLanguage);
			AddPlainArrayField("mLanguageData", 1, &StringTableRTTI::GetLanguageData, &StringTableRTTI::GetNumLanguages, &StringTableRTTI::SetLanguageData, &StringTableRTTI::SetNumLanguages);
			AddPlainField("mIdentifiers", 2, &StringTableRTTI::GetIdentifiers, &StringTableRTTI::SetIdentifiers);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			StringTable* stringTable = static_cast<StringTable*>(obj);
			stringTable->SetActiveLanguage(stringTable->mActiveLanguage);
		}

		const String& GetRttiName()
		{
			static String name = "StringTable";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_StringTable;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return StringTable::CreatePtrInternal();
		}
	};

	/**
	 * RTTIPlainType for LanguageData.
	 *
	 * @see		RTTIPlainType
	 */
	template <>
	struct RTTIPlainType<LanguageData>
	{
		enum
		{
			id = TID_LanguageData
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const LanguageData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				auto numElements = (uint32_t)data.Strings.size();
				size += rtti_write(numElements, stream);

				for (auto& entry : data.Strings)
				{
					size += rtti_write(entry.first, stream);
					size += rtti_write(*entry.second, stream);
				}

				return size; });
		}

		static BitLength FromMemory(LanguageData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t numElements = 0;
			rtti_read(numElements, stream);

			data.Strings.clear();
			for(uint32_t i = 0; i < numElements; i++)
			{
				String identifier;
				rtti_read(identifier, stream);

				SPtr<LocalizedStringData> entryData = B3DMakeShared<LocalizedStringData>();
				rtti_read(*entryData, stream);

				data.Strings[identifier] = entryData;
			}

			return size;
		}

		static BitLength GetSize(const LanguageData& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(auto& entry : data.Strings)
			{
				dataSize += rtti_size(entry.first);
				dataSize += rtti_size(*entry.second);
			}

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for LocalizedStringData.
	 *
	 * @see		RTTIPlainType
	 */
	template <>
	struct RTTIPlainType<LocalizedStringData>
	{
		enum
		{
			id = TID_LocalizedStringData
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const LocalizedStringData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				size += rtti_write(data.String, stream);
				size += rtti_write(data.NumParameters, stream);

				for (uint32_t i = 0; i < data.NumParameters; i++)
					size += rtti_write(data.ParameterOffsets[i], stream);

				return size; });
		}

		static BitLength FromMemory(LocalizedStringData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(data.ParameterOffsets != nullptr)
				B3DDeleteMultiple(data.ParameterOffsets, data.NumParameters);

			BitLength size;
			rtti_read_size_header(stream, compress, size);

			rtti_read(data.String, stream);
			rtti_read(data.NumParameters, stream);

			data.ParameterOffsets = B3DNewMultiple<LocalizedStringData::ParamOffset>(data.NumParameters);
			for(uint32_t i = 0; i < data.NumParameters; i++)
				rtti_read(data.ParameterOffsets[i], stream);

			return size;
		}

		static BitLength GetSize(const LocalizedStringData& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;

			dataSize += rtti_size(data.String);
			dataSize += rtti_size(data.NumParameters);

			for(uint32_t i = 0; i < data.NumParameters; i++)
				dataSize = rtti_size(data.ParameterOffsets[i]);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	BS_ALLOW_MEMCPY_SERIALIZATION(LocalizedStringData::ParamOffset);

	/** @} */
	/** @endcond */
} // namespace bs
