//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "CoreObject/B3DCoreObject.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Base class for all resources. */
	class B3D_EXPORT Resource : public IReflectable, public IScriptExportable, public CoreObject
	{
	public:
		Resource(bool createRenderProxy = true, const String& name = StringUtility::kBlank);
		virtual ~Resource() = default;

		/**	Returns the name of the resource. */
		const String& GetName() const { return mName; }

		/**	Sets the name of the resource.  */
		virtual void SetName(const String& name) { mName = name; }

		/** Returns a globally unique identifier of the resource. */
		const UUID& GetId() const { return mId; }

		/** Get a handle to this resource, if there is any associated. */
		TResourceHandle<Resource> GetHandle() const { return mSelfHandle.Lock(); }

		/**	Retrieves meta-data containing various information describing a resource. */
		TShared<ResourceMetaData> GetMetaData() const { return mMetaData; }

		/**	Returns whether or not this resource is allowed to be asynchronously loaded. */
		virtual bool AllowAsyncLoading() const { return true; }

		void Destroy() override;

		/**
		 * @name Internal
		 * @{
		 */

		/** Associates a handle with the resource. Should be called right after the handle for the resource is created, or right after resource load completes. */
		void SetHandle(const TWeakResourceHandle<Resource>& handle)
		{
			mSelfHandle = handle;
			SetId(mSelfHandle.GetId());
		}

		/** Associates a new id with the resource. If the resource can be referenced via a handle, the UUID must match the handle UUID. */
		void SetId(const UUID& id)
		{
			B3D_ASSERT(mSelfHandle == nullptr || mSelfHandle.GetId() == id);
			mId = id;
		}

		/** Call this on the resource after it has been duplicated (on the duplicated object). */
		void NotifyDidDuplicate() { OnDidDuplicate(); }

		/** @} */

	protected:
		friend class Resources;
		friend class ResourceHandle;
		friend class Package;

		/**
		 * Retrieves a list of all resources that this resource depends on.
		 *
		 * @note Thread safe.
		 */
		void GetResourceDependencies(FrameVector<HResource>& dependencies) const;

		/**	Checks if all the resources this object is dependent on are fully loaded. */
		bool AreDependenciesLoaded() const;

		/**
		 * Registers a new resource that this resource is dependent on.
		 *
		 * @note Thread safe.
		 */
		void AddResourceDependency(const HResource& resource);

		/**
		 * Unregisters a previously registered dependency.
		 *
		 * @note Thread safe.
		 */
		void RemoveResourceDependency(const HResource& resource);

		/** Called on the resource after it has been duplicated (called on the duplicated object). */
		virtual void OnDidDuplicate() { }

		/**
		 * Called on a loaded resource when a package containing its ID is about to have its file replaced, written to, moved or removed.
		 * If the resource is reading from that file it must stop holding the file open and keep whatever it still needs in memory, so it
		 * remains fully usable. Always followed by OnPackageFileDidChange().
		 *
		 * @param	filePath	Path to the file about to change. The resource might have been loaded from a different package containing
		 *						the same ID, in which case it has nothing to do.
		 */
		virtual void OnPackageFileWillChange(const Path& filePath) { }

		/**
		 * Called on a loaded resource after the change announced by OnPackageFileWillChange() is done, whether it succeeded or not.
		 *
		 * @param	filePath			Location of the package file after the change.
		 * @param	dataOffsetChange	How many bytes the resource's serialized data moved within the file. Its bytes are otherwise the same as
		 *								they were, so the resource may read from the file again. If not set, the file no longer contains the data
		 *								the resource was loaded from, because it was re-encoded, removed from the package, or the file itself was
		 *								removed.
		 */
		virtual void OnPackageFileDidChange(const Path& filePath, const TOptional<i64>& dataOffsetChange) { }

		/**
		 * Returns true if the resource can be compressed using a generic compression when saved on a storage device.
		 * Certain resources already have their contents compressed (like audio files) and will not benefit from further
		 * compression. Resources supporting streaming should never be compressed, instead such resources can handle
		 * compression/decompression locally through their streams.
		 */
		virtual bool IsCompressible() const { return true; }

		UUID mId;
		TWeakResourceHandle<Resource> mSelfHandle;

		String mName;
		TShared<ResourceMetaData> mMetaData;

		/**
		 * Signal to the resource implementation if original data should be kept in memory. This is sometimes needed if
		 * the resource destroys original data during normal usage, but it might still be required for special purposes
		 * (like saving in the editor).
		 */
		bool mKeepSourceData;

		/** A list of all other resources this resource depends on. */
		Vector<TWeakResourceHandle<Resource>> mDependencies;

		/** Mutex ensuring dependencies list updates and queries are thread safe. */
		mutable Mutex mDependenciesMutex;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ResourceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
