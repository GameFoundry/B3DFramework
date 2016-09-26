//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsStringTable.h"

namespace BansheeEngine
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

		UnorderedSet<WString>& getIdentifiers(StringTable* obj) { return obj->mIdentifiers; }
		void setIdentifiers(StringTable* obj, UnorderedSet<WString>& val) { obj->mIdentifiers = val; }

	public:
		StringTableRTTI()
		{
			addPlainField("mActiveLanguage", 0, &StringTableRTTI::getActiveLanguage, &StringTableRTTI::setActiveLanguage);
			addPlainArrayField("mLanguageData", 1, &StringTableRTTI::getLanguageData, &StringTableRTTI::getNumLanguages, 
				&StringTableRTTI::setLanguageData, &StringTableRTTI::setNumLanguages);
			addPlainField("mIdentifiers", 2, &StringTableRTTI::getIdentifiers, &StringTableRTTI::setIdentifiers);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
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
		static void toMemory(const LanguageData& data, char* memory)
		{ 
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 numElements = (UINT32)data.strings.size();
			memory = rttiWriteElem(numElements, memory, size);

			for (auto& entry : data.strings)
			{
				memory = rttiWriteElem(entry.first, memory, size);
				memory = rttiWriteElem(*entry.second, memory, size);
			}
			
			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(LanguageData& data, char* memory)
		{ 
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			UINT32 numElements = 0;
			memory = rttiReadElem(numElements, memory);

			data.strings.clear();
			for (UINT32 i = 0; i < numElements; i++)
			{
				WString identifier;
				memory = rttiReadElem(identifier, memory);

				SPtr<LocalizedStringData> entryData = bs_shared_ptr_new<LocalizedStringData>();
				memory = rttiReadElem(*entryData, memory);

				data.strings[identifier] = entryData;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const LanguageData& data)
		{ 
			UINT64 dataSize = sizeof(UINT32) * 2;

			for (auto& entry : data.strings)
			{
				dataSize += rttiGetElemSize(entry.first);
				dataSize += rttiGetElemSize(*entry.second);
			}

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
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
		static void toMemory(const LocalizedStringData& data, char* memory)
		{ 
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			memory = rttiWriteElem(data.string, memory, size);
			memory = rttiWriteElem(data.numParameters, memory, size);

			for (UINT32 i = 0; i < data.numParameters; i++)
				memory = rttiWriteElem(data.parameterOffsets[i], memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(LocalizedStringData& data, char* memory)
		{ 
			if (data.parameterOffsets != nullptr)
				bs_deleteN(data.parameterOffsets, data.numParameters);

			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			memory = rttiReadElem(data.string, memory);
			memory = rttiReadElem(data.numParameters, memory);

			data.parameterOffsets = bs_newN<LocalizedStringData::ParamOffset>(data.numParameters);
			for (UINT32 i = 0; i < data.numParameters; i++)
				memory = rttiReadElem(data.parameterOffsets[i], memory);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const LocalizedStringData& data)
		{ 
			UINT64 dataSize = sizeof(UINT32);

			dataSize += rttiGetElemSize(data.string);
			dataSize += rttiGetElemSize(data.numParameters);

			for (UINT32 i = 0; i < data.numParameters; i++)
				dataSize = rttiGetElemSize(data.parameterOffsets[i]);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}	
	}; 

	BS_ALLOW_MEMCPY_SERIALIZATION(LocalizedStringData::ParamOffset);

	/** @} */
	/** @endcond */
}