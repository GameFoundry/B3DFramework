//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreObject/BsCoreObject.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Base class for all resources. */
	class B3D_CORE_EXPORT Resource : public IReflectable, public CoreObject
	{
	public:
		Resource(bool createRenderProxy = true, const String& name = StringUtil::kBlank);
		virtual ~Resource() = default;

		/**	Returns the name of the resource. */
		const String& GetName() const;

		/**	Sets the name of the resource.  */
		virtual void SetName(const String& name);

		/** Returns a globally unique identifier of the resource. */
		const UUID& GetId() const { return mId; }

		/** Get a handle to this resource, if there is any associated. */
		ResourceHandle<Resource> GetHandle() const { return mSelfHandle.Lock(); }

		/**	Retrieves meta-data containing various information describing a resource. */
		SPtr<ResourceMetaData> GetMetaData() const { return mMetaData; }

		/**	Returns whether or not this resource is allowed to be asynchronously loaded. */
		virtual bool AllowAsyncLoading() const { return true; }

		/**
		 * @name Internal
		 * @{
		 */

		/** Associates a handle with the resource. Should be called right after the handle for the resource is created, or right after resource load completes. */
		void SetHandle(const WeakResourceHandle<Resource>& handle)
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

		/** @} */

	protected:
		friend class Resources;
		friend class ResourceHandleBase;

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

		/**
		 * Returns true if the resource can be compressed using a generic compression when saved on a storage device.
		 * Certain resources already have their contents compressed (like audio files) and will not benefit from further
		 * compression. Resources supporting streaming should never be compressed, instead such resources can handle
		 * compression/decompression locally through their streams.
		 */
		virtual bool IsCompressible() const { return true; }

		UUID mId;
		WeakResourceHandle<Resource> mSelfHandle;

		u32 mSize;
		SPtr<ResourceMetaData> mMetaData;

		/**
		 * Signal to the resource implementation if original data should be kept in memory. This is sometimes needed if
		 * the resource destroys original data during normal usage, but it might still be required for special purposes
		 * (like saving in the editor).
		 */
		bool mKeepSourceData;

		/** A list of all other resources this resource depends on. */
		Vector<WeakResourceHandle<Resource>> mDependencies;

		/** Mutex ensuring dependencies list updates and queries are thread safe. */
		mutable Mutex mDependenciesMutex;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ResourceRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;
	};

	/** @} */
} // namespace bs
