//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "Localization/B3DStringTable.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT StringTableRTTI : public TRTTIType<StringTable, Resource, StringTableRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mActiveLanguage, 0)
			B3D_RTTI_MEMBER_CONTAINER(mAllLanguages, 1)
			B3D_RTTI_MEMBER_CONTAINER(mIdentifiers, 2)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(StringTable& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.SetActiveLanguage(object.mActiveLanguage);
			}
		}

		const String& GetRttiName()
		{
			static String name = "StringTable";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_StringTable;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return StringTable::CreateShared();
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
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				auto numElements = (uint32_t)data.Strings.size();
				size += B3DRTTIWrite(numElements, stream);

				for (auto& entry : data.Strings)
				{
					size += B3DRTTIWrite(entry.first, stream);
					size += B3DRTTIWrite(*entry.second, stream);
				}

				return size; });
		}

		static BitLength FromMemory(LanguageData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t numElements = 0;
			B3DRTTIRead(numElements, stream);

			data.Strings.clear();
			for(uint32_t i = 0; i < numElements; i++)
			{
				String identifier;
				B3DRTTIRead(identifier, stream);

				SPtr<LocalizedStringData> entryData = B3DMakeShared<LocalizedStringData>();
				B3DRTTIRead(*entryData, stream);

				data.Strings[identifier] = entryData;
			}

			return size;
		}

		static BitLength GetSize(const LanguageData& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(auto& entry : data.Strings)
			{
				dataSize += B3DRTTISize(entry.first);
				dataSize += B3DRTTISize(*entry.second);
			}

			B3DRTTIAddHeaderSize(dataSize, compress);
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
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				size += B3DRTTIWrite(data.String, stream);
				size += B3DRTTIWrite(data.NumParameters, stream);

				for (uint32_t i = 0; i < data.NumParameters; i++)
					size += B3DRTTIWrite(data.ParameterOffsets[i], stream);

				return size; });
		}

		static BitLength FromMemory(LocalizedStringData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(data.ParameterOffsets != nullptr)
				B3DDeleteMultiple(data.ParameterOffsets, data.NumParameters);

			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			B3DRTTIRead(data.String, stream);
			B3DRTTIRead(data.NumParameters, stream);

			data.ParameterOffsets = B3DNewMultiple<LocalizedStringData::ParamOffset>(data.NumParameters);
			for(uint32_t i = 0; i < data.NumParameters; i++)
				B3DRTTIRead(data.ParameterOffsets[i], stream);

			return size;
		}

		static BitLength GetSize(const LocalizedStringData& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;

			dataSize += B3DRTTISize(data.String);
			dataSize += B3DRTTISize(data.NumParameters);

			for(uint32_t i = 0; i < data.NumParameters; i++)
				dataSize = B3DRTTISize(data.ParameterOffsets[i]);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	B3D_ALLOW_MEMCPY_SERIALIZATION(LocalizedStringData::ParamOffset, TID_LocalizedStringParameterOffset);

	/** @} */
	/** @endcond */
} // namespace b3d
