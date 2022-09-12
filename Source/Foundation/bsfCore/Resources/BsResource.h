//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Base class for all resources. */
	class BS_CORE_EXPORT Resource : public IReflectable, public CoreObject
	{
	public:
		Resource(bool requiresGpuInitialization = true);
		virtual ~Resource() = default;

		/**	Returns the name of the resource. */
		const String& GetName() const;

		/**	Sets the name of the resource.  */
		void SetName(const String& name);

		/**	Retrieves meta-Data containing various information describing a resource. */
		SPtr<ResourceMetaData> GetMetaData() const { return mMetaData; }

		/**	Returns whether or not this resource is allowed to be asynchronously loaded. */
		virtual bool AllowAsyncLoading() const { return true; }

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

		UINT32 mSize;
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
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
