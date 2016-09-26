//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsIReflectable.h"

namespace BansheeEngine
{
	/** @addtogroup Library
	 *  @{
	 */

	/**	Contains meta-data for a resource stored in the ProjectLibrary. */
	class BS_ED_EXPORT ProjectResourceMeta : public IReflectable
	{
	private:
		struct ConstructPrivately {};

	public:
		explicit ProjectResourceMeta(const ConstructPrivately&);

		/**
		 * Creates a new project library resource meta-data entry.
		 *
		 * @param[in]	name				Name of the resource, unique within the file containing the resource.
		 * @param[in]	uuid				UUID of the resource.
		 * @param[in]	typeId				RTTI type id of the resource.
		 * @param[in]	resourceMetaData	Non-project library specific meta-data.
		 * @return							New project library resource meta data instance.
		 */
		static SPtr<ProjectResourceMeta> create(const WString& name, const String& uuid, UINT32 typeId, 
			const SPtr<ResourceMetaData>& resourceMetaData);

		/** Returns the name of the resource, unique within the file containing the resource. */
		const WString& getUniqueName() const { return mName; }

		/**	Returns the UUID of the resource this meta data belongs to. */
		const String& getUUID() const { return mUUID; }

		/**	Returns the non-project library specific meta-data. */
		SPtr<ResourceMetaData> getResourceMetaData() const { return mResourceMeta; }

		/**	Returns the RTTI type ID of the resource this object is referencing. */
		UINT32 getTypeID() const { return mTypeId; }

		/** 
		 * Returns additional data attached to the resource meta by the user. This is non-specific data and can contain
		 * anything the user requires. 
		 */
		SPtr<IReflectable> getUserData() const { return mUserData; }

	private:
		friend class ProjectLibrary;

		WString mName;
		String mUUID;
		SPtr<ResourceMetaData> mResourceMeta;
		UINT32 mTypeId;
		SPtr<IReflectable> mUserData;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty meta-data instance. Used only for serialization purposes. */
		static SPtr<ProjectResourceMeta> createEmpty();

	public:
		friend class ProjectResourceMetaRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;	
	};

	/**	
	 * Contains meta-data for a file stored in the ProjectLibrary. A single file meta-data can contain one or multiple
	 * ProjectResourceMeta instances.
	 */
	class BS_ED_EXPORT ProjectFileMeta : public IReflectable
	{
	private:
		struct ConstructPrivately {};

	public:
		explicit ProjectFileMeta(const ConstructPrivately&);

		/**
		 * Creates a new project library file meta-data entry.
		 *
		 * @param[in]	importOptions		Import options used for importing the resource.
		 * @return							New project library file meta data instance.
		 */
		static SPtr<ProjectFileMeta> create(const SPtr<ImportOptions>& importOptions);

		/** Registers a new resource in the file meta-data. */
		void add(const SPtr<ProjectResourceMeta>& resourceMeta);

		/** 
		 * Registers an inactive resource in the file meta-data. Inactive meta-data is stored for resources that used
		 * to exist, but do not exist currently, in order to restore their handles if they get restored at a later date.
		 */
		void addInactive(const SPtr<ProjectResourceMeta>& resourceMeta);

		/** Returns meta-data for all active resources contained in the file represented by this meta-data object.  */
		const Vector<SPtr<ProjectResourceMeta>>& getResourceMetaData() const { return mResourceMetaData; }

		/** 
		 * Returns meta-data for all resources (both active and inactive) contained in the file represented by this
		 * meta-data object.  
		 */
		Vector<SPtr<ProjectResourceMeta>> getAllResourceMetaData() const;

		/** 
		 * Removes all resource meta-data stored by this object. This includes meta-data for both active and inactive
		 * resources. 
		 */
		void clearResourceMetaData() { mResourceMetaData.clear(); }

		/**	Returns the import options used for importing the resource this object is referencing. */
		const SPtr<ImportOptions>& getImportOptions() const { return mImportOptions; }

		/** Checks should this resource always be included in the build, regardless if it's being referenced or not. */
		bool getIncludeInBuild() const { return mIncludeInBuild; }

		/** Determines if this resource will always be included in the build, regardless if it's being referenced or not. */
		void setIncludeInBuild(bool include) { mIncludeInBuild = include; }

		/** Checks does the file contain a resource with the specified type id. */
		bool hasTypeId(UINT32 typeId) const;

		/** Checks does the file contain a resource with the specified UUID. */
		bool hasUUID(const String& uuid) const;

	private:
		friend class ProjectLibrary;

		Vector<SPtr<ProjectResourceMeta>> mResourceMetaData;
		Vector<SPtr<ProjectResourceMeta>> mInactiveResourceMetaData;
		SPtr<ImportOptions> mImportOptions;
		bool mIncludeInBuild;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty meta-data instance. Used only for serialization purposes. */
		static SPtr<ProjectFileMeta> createEmpty();

	public:
		friend class ProjectFileMetaRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;	
	};

	/** @} */
}