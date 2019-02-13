//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Serializable class that contains UUID <-> file path mapping for resources.
	 * 			
	 * @note	
	 * This class allows you to reference resources between sessions. At the end of a session save the resource manifest, 
	 * and then restore it at the start of a new session. This way ensures that resource UUIDs stay consistent and anything 
	 * referencing them can find the resources.
	 * @note
	 * Thread safe.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Resources,api:bsf) ResourceManifest : public IReflectable
	{
		struct ConstructPrivately {};
	public:
		explicit ResourceManifest(const ConstructPrivately& dummy);
		ResourceManifest(const String& name);

		/**	Returns an unique name of the resource manifest. */
		BS_SCRIPT_EXPORT(pr:getter,n:Name)
		const String& getName() const { return mName; }

		/**	Registers a new resource in the manifest. */
		BS_SCRIPT_EXPORT()
		void registerResource(const UUID& uuid, const Path& filePath);

		/**	Removes a resource from the manifest. */
		BS_SCRIPT_EXPORT()
		void unregisterResource(const UUID& uuid);

		/**
		 * Attempts to find a resource with the provided UUID and outputs the path to the resource if found. Returns true
		 * if UUID was found, false otherwise.
		 */
		BS_SCRIPT_EXPORT()
		bool uuidToFilePath(const UUID& uuid, Path& filePath) const;

		/**
		 * Attempts to find a resource with the provided path and outputs the UUID to the resource if found. Returns true
		 * if path was found, false otherwise.
		 */
		BS_SCRIPT_EXPORT()
		bool filePathToUUID(const Path& filePath, UUID& outUUID) const;

		/**	Checks if provided UUID exists in the manifest. */
		BS_SCRIPT_EXPORT()
		bool uuidExists(const UUID& uuid) const;

		/**	Checks if the provided path exists in the manifest. */
		BS_SCRIPT_EXPORT()
		bool filePathExists(const Path& filePath) const;

		/**
		 * Saves the resource manifest to the specified location.
		 *
		 * @param[in]	manifest		Manifest to save.
		 * @param[in]	path			Full pathname of the file to save the manifest in.
		 * @param[in]	relativePath	If not empty, all pathnames in the manifest will be stored as if relative to this 
		 *								path.
		 */
		BS_SCRIPT_EXPORT()
		static void save(const SPtr<ResourceManifest>& manifest, const Path& path, const Path& relativePath);

		/**
		 * Loads the resource manifest from the specified location.
		 *
		 * @param[in]	path			Full pathname of the file to load the manifest from.
		 * @param[in]	relativePath	If not empty, all loaded pathnames will have this path prepended.
		 */
		BS_SCRIPT_EXPORT()
		static SPtr<ResourceManifest> load(const Path& path, const Path& relativePath);

		/** Creates a new empty resource manifest. Provided name should be unique among manifests. */
		BS_SCRIPT_EXPORT(ec:ResourceManifest)
		static SPtr<ResourceManifest> create(const String& name);

	private:
		String mName;
		UnorderedMap<UUID, Path> mUUIDToFilePath;
		UnorderedMap<Path, UUID> mFilePathToUUID;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty resource manifest. */
		static SPtr<ResourceManifest> createEmpty();

	public:
		friend class ResourceManifestRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}