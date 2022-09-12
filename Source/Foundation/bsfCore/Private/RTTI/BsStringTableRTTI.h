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

		LanguageData& GetLanguageData(StringTable* obj, UINT32 idx) { return obj->mAllLanguages[idx]; }
		void SetLanguageData(StringTable* obj, UINT32 idx, LanguageData& val) { obj->mAllLanguages[idx] = val; }
		UINT32 GetNumLanguages(StringTable* obj) { return (UINT32)Language::Count; }
		void SetNumLanguages(StringTable* obj, UINT32 val) { /* Do nothing */ }

		UnorderedSet<String>& GetIdentifiers(StringTable* obj) { return obj->mIdentifiers; }
		void SetIdentifiers(StringTable* obj, UnorderedSet<String>& val) { obj->mIdentifiers = val; }

	public:
		StringTableRTTI()
		{
			addPlainField("mActiveLanguage", 0, &StringTableRTTI::getActiveLanguage, &StringTableRTTI::setActiveLanguage);
			addPlainArrayField("mLanguageData", 1, &StringTableRTTI::getLanguageData, &StringTableRTTI::getNumLanguages,
				&StringTableRTTI::setLanguageData, &StringTableRTTI::setNumLanguages);
			addPlainField("mIdentifiers", 2, &StringTableRTTI::getIdentifiers, &StringTableRTTI::setIdentifiers);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			StringTable* stringTable = static_cast<StringTable*>(obj);
			stringTable->SetActiveLanguage(stringTable->mActiveLanguage);
		}

		const String& GetRTTIName() override
		{
			static String name = "StringTable";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_StringTable;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return StringTable::_createPtr();
		}
	};

	/**
	 * RTTIPlainType for LanguageData.
	 * 			
	 * @see		RTTIPlainType
	 */
	template<>
	struct RTTIPlainType<LanguageData>
	{	
		enum { id = TID_LanguageData }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength ToMemory(const LanguageData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				auto numElements = (uint32_t)data.strings.Size();
				size += rtti_write(numElements, stream);

				for (auto& entry : data.strings)
				{
					size += rtti_write(entry.first, stream);
					size += rtti_write(*entry.second, stream);
				}

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength FromMemory(LanguageData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t numElements = 0;
			rtti_read(numElements, stream);

			data.strings.Clear();
			for (uint32_t i = 0; i < numElements; i++)
			{
				String identifier;
				rtti_read(identifier, stream);

				SPtr<LocalizedStringData> entryData = bs_shared_ptr_new<LocalizedStringData>();
				rtti_read(*entryData, stream);

				data.strings[identifier] = entryData;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength GetSize(const LanguageData& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for (auto& entry : data.strings)
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
	template<>
	struct RTTIPlainType<LocalizedStringData>
	{	
		enum { id = TID_LocalizedStringData }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength ToMemory(const LocalizedStringData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				size += rtti_write(data.string, stream);
				size += rtti_write(data.numParameters, stream);

				for (uint32_t i = 0; i < data.numParameters; i++)
					size += rtti_write(data.parameterOffsets[i], stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength FromMemory(LocalizedStringData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (data.parameterOffsets != nullptr)
				bs_deleteN(data.parameterOffsets, data.numParameters);

			BitLength size;
			rtti_read_size_header(stream, compress, size);

			rtti_read(data.string, stream);
			rtti_read(data.numParameters, stream);

			data.parameterOffsets = bs_newN<LocalizedStringData::ParamOffset>(data.numParameters);
			for (uint32_t i = 0; i < data.numParameters; i++)
				rtti_read(data.parameterOffsets[i], stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength GetSize(const LocalizedStringData& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;

			dataSize += rtti_size(data.string);
			dataSize += rtti_size(data.numParameters);

			for (uint32_t i = 0; i < data.numParameters; i++)
				dataSize = rtti_size(data.parameterOffsets[i]);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}	
	};

	BS_ALLOW_MEMCPY_SERIALIZATION(LocalizedStringData::ParamOffset);

	/** @} */
	/** @endcond */
}
