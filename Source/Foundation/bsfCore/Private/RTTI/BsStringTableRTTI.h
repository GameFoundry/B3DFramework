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
		Language& getActiveLanguage(StringTable* obj) { return obj->mActiveLanguage; }
		void setActiveLanguage(StringTable* obj, Language& val) { obj->mActiveLanguage = val; }

		LanguageData& getLanguageData(StringTable* obj, UINT32 idx) { return obj->mAllLanguages[idx]; }
		void setLanguageData(StringTable* obj, UINT32 idx, LanguageData& val) { obj->mAllLanguages[idx] = val; }
		UINT32 getNumLanguages(StringTable* obj) { return (UINT32)Language::Count; }
		void setNumLanguages(StringTable* obj, UINT32 val) { /* Do nothing */ }

		UnorderedSet<String>& getIdentifiers(StringTable* obj) { return obj->mIdentifiers; }
		void setIdentifiers(StringTable* obj, UnorderedSet<String>& val) { obj->mIdentifiers = val; }

	public:
		StringTableRTTI()
		{
			addPlainField("mActiveLanguage", 0, &StringTableRTTI::getActiveLanguage, &StringTableRTTI::setActiveLanguage);
			addPlainArrayField("mLanguageData", 1, &StringTableRTTI::getLanguageData, &StringTableRTTI::getNumLanguages,
				&StringTableRTTI::setLanguageData, &StringTableRTTI::setNumLanguages);
			addPlainField("mIdentifiers", 2, &StringTableRTTI::getIdentifiers, &StringTableRTTI::setIdentifiers);
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			StringTable* stringTable = static_cast<StringTable*>(obj);
			stringTable->setActiveLanguage(stringTable->mActiveLanguage);
		}

		const String& getRTTIName() override
		{
			static String name = "StringTable";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_StringTable;
		}

		SPtr<IReflectable> newRTTIObject() override
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
		static uint32_t toMemory(const LanguageData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;

				auto numElements = (uint32_t)data.strings.size();
				size += rttiWriteElem(numElements, stream);

				for (auto& entry : data.strings)
				{
					size += rttiWriteElem(entry.first, stream);
					size += rttiWriteElem(*entry.second, stream);
				}

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(LanguageData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size = 0;
			rttiReadElem(size, stream);

			uint32_t numElements = 0;
			rttiReadElem(numElements, stream);

			data.strings.clear();
			for (uint32_t i = 0; i < numElements; i++)
			{
				String identifier;
				rttiReadElem(identifier, stream);

				SPtr<LocalizedStringData> entryData = bs_shared_ptr_new<LocalizedStringData>();
				rttiReadElem(*entryData, stream);

				data.strings[identifier] = entryData;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const LanguageData& data)
		{
			uint64_t dataSize = sizeof(uint32_t) * 2;

			for (auto& entry : data.strings)
			{
				dataSize += rttiGetElemSize(entry.first);
				dataSize += rttiGetElemSize(*entry.second);
			}

			assert(dataSize <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)dataSize;
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
		static uint32_t toMemory(const LocalizedStringData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;

				size += rttiWriteElem(data.string, stream);
				size += rttiWriteElem(data.numParameters, stream);

				for (uint32_t i = 0; i < data.numParameters; i++)
					size += rttiWriteElem(data.parameterOffsets[i], stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(LocalizedStringData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			if (data.parameterOffsets != nullptr)
				bs_deleteN(data.parameterOffsets, data.numParameters);

			uint32_t size = 0;
			rttiReadElem(size, stream);

			rttiReadElem(data.string, stream);
			rttiReadElem(data.numParameters, stream);

			data.parameterOffsets = bs_newN<LocalizedStringData::ParamOffset>(data.numParameters);
			for (uint32_t i = 0; i < data.numParameters; i++)
				rttiReadElem(data.parameterOffsets[i], stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const LocalizedStringData& data)
		{
			uint64_t dataSize = sizeof(uint32_t);

			dataSize += rttiGetElemSize(data.string);
			dataSize += rttiGetElemSize(data.numParameters);

			for (uint32_t i = 0; i < data.numParameters; i++)
				dataSize = rttiGetElemSize(data.parameterOffsets[i]);

			assert(dataSize <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)dataSize;
		}	
	};

	BS_ALLOW_MEMCPY_SERIALIZATION(LocalizedStringData::ParamOffset);

	/** @} */
	/** @endcond */
}
