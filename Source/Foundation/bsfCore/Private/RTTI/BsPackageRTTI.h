//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2023 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsPackage.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsUUIDRTTI.h"
#include "RTTI/BsPathRTTI.h"
#include "RTTI/BsFlagsRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsStringRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT PackageMetaDataRTTI : public RTTIType<PackageMetaData, IReflectable, PackageMetaDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
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

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PackageMetaData>();
		}
	};

	class B3D_CORE_EXPORT PackageResourceMetaDataRTTI : public RTTIType<PackageResourceMetaData, IReflectable, PackageResourceMetaDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER_PLAIN(Path, 0)
		B3D_RTTI_MEMBER_PLAIN(Id, 1)
		B3D_RTTI_MEMBER_PLAIN(TypeId, 2)
		B3D_RTTI_MEMBER_PLAIN(Dependencies, 3)
		B3D_RTTI_MEMBER_PLAIN(CompressionType, 4)
		B3D_RTTI_MEMBER_PLAIN(Flags, 5)
		B3D_RTTI_MEMBER_REFLPTR(UserMetaData, 6)
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

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PackageResourceMetaData>();
		}
	};

	class B3D_CORE_EXPORT PackageResourceUserMetaDataRTTI : public RTTIType<PackageResourceUserMetaData, IReflectable, PackageResourceUserMetaDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_END_MEMBERS

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

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PackageResourceUserMetaData>();
		}
	};

	class B3D_CORE_EXPORT PackageRTTI : public RTTIType<Package, IReflectable, PackageRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER_PLAIN(mName, 0)
		B3D_RTTI_MEMBER_PLAIN(mId, 1)
		B3D_RTTI_MEMBER_REFLPTR(mPackageMetaData, 2)
		B3D_RTTI_END_MEMBERS

		SPtr<PackageResourceMetaData> GetResourceMetaData(Package* object, u32 index) { return mResourceMetaData[index]; }
		void SetResourceMetaData(Package* object, u32 index, SPtr<PackageResourceMetaData> data) { mResourceMetaData[index] = std::move(data); }
		u32 GetResourceMetaDataCount(Package* object) { return (u32)mResourceMetaData.size(); }
		void SetResourceMetaDataCount(Package* object, u32 size) { mResourceMetaData.resize(size); }

	public:
		PackageRTTI()
		{
			AddReflectablePtrArrayField("mResourceMetaData", 3, &PackageRTTI::GetResourceMetaData, &PackageRTTI::GetResourceMetaDataCount, &PackageRTTI::SetResourceMetaData, &PackageRTTI::SetResourceMetaDataCount);
		}

		void OnSerializationStarted(IReflectable* object, RTTIOperationContext* context) override
		{
			const Package* const package = static_cast<Package*>(object);

			for (const auto& entry : package->mResourceInformationByUUID)
				mResourceMetaData.push_back(entry.second->MetaData);
		}

		void OnDeserializationEnded(IReflectable* object, RTTIOperationContext* context) override
		{
			Package* const package = static_cast<Package*>(object);

			for (const auto& entry : mResourceMetaData)
			{
				UPtr<Package::ResourceInformation> resourceInformation = B3DMakeUnique<Package::ResourceInformation>();
				resourceInformation->MetaData = entry;

				package->mResourceInformationByPath[entry->Path] = resourceInformation.get();
				package->mResourceInformationByUUID[entry->Id] = std::move(resourceInformation);
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

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<Package>();
		}

	private:
		Vector<SPtr<PackageResourceMetaData>> mResourceMetaData;
	};

	/** @} */
	/** @endcond */
} // namespace bs
