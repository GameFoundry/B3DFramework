//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DPackage.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DResourceMetaDataRTTI.h"
#include "RTTI/B3DUUIDRTTI.h"
#include "RTTI/B3DPathRTTI.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "RTTI/B3DStringRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT PackageMetaDataRTTI : public TRTTIType<PackageMetaData, IReflectable, PackageMetaDataRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(IncludePackageNameInVirtualPath, 0)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "PackageMetaData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PackageMetaData;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PackageMetaData>();
		}
	};

	class B3D_EXPORT PackageResourceMetaDataRTTI : public TRTTIType<PackageResourceMetaData, IReflectable, PackageResourceMetaDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Path, 0)
			B3D_RTTI_MEMBER(Id, 1)
			B3D_RTTI_MEMBER(TypeId, 2)
			B3D_RTTI_MEMBER(Dependencies, 3)
			B3D_RTTI_MEMBER(CompressionType, 4)
			B3D_RTTI_MEMBER(Flags, 5)
			B3D_RTTI_MEMBER(AdditionalMetaData, 6)
			B3D_RTTI_MEMBER(ResourceMetaData, 7)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "PackageResourceMetaData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PackageResourceMetaData;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PackageResourceMetaData>();
		}
	};

	class B3D_EXPORT PackageResourceUserMetaDataRTTI : public TRTTIType<PackageResourceUserMetaData, IReflectable, PackageResourceUserMetaDataRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "PackageResourceUserMetaData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PackageResourceUserMetaData;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PackageResourceUserMetaData>();
		}
	};

	class B3D_EXPORT PackageRTTI : public TRTTIType<Package, IReflectable, PackageRTTI>
	{
		Vector<TShared<PackageResourceMetaData>> mResourceMetaData;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mName, 0)
			B3D_RTTI_MEMBER(mId, 1)
			B3D_RTTI_MEMBER(mPackageMetaData, 2)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mResourceMetaData, 3)
			B3D_RTTI_MEMBER(mMetaDataPaddingByteCount, 4)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(Package& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				for (const auto& entry : object.mResourceInformationByUUID)
					mResourceMetaData.push_back(entry.second->MetaData);
			}
		}

		void OnOperationEnded(Package& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				for (const auto& entry : mResourceMetaData)
				{
					UPtr<Package::ResourceInformation> resourceInformation = B3DMakeUnique<Package::ResourceInformation>();
					resourceInformation->MetaData = entry;

					object.mResourceInformationByPath[entry->Path] = resourceInformation.get();
					object.mResourceInformationByUUID[entry->Id] = std::move(resourceInformation);
				}
			}
		}

		const String& GetRttiName() override
		{
			static String name = "Package";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Package;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<Package>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
