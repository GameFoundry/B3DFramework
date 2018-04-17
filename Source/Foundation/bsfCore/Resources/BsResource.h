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
		virtual ~Resource() {};

		/**	Returns the name of the resource. */
		const String& getName() const;

		/**	Sets the name of the resource.  */
		void setName(const String& name);

		/**	Retrieves meta-data containing various information describing a resource. */
		SPtr<ResourceMetaData> getMetaData() const { return mMetaData; }

		/**	Returns whether or not this resource is allowed to be asynchronously loaded. */
		virtual bool allowAsyncLoading() const { return true; }

	protected:
		friend class Resources;
		friend class ResourceHandleBase;

		/**	Retrieves a list of all resources that this resource depends on. */
		virtual void getResourceDependencies(FrameVector<HResource>& dependencies) const { }

		/**	Checks if all the resources this object is dependent on are fully loaded. */
		bool areDependenciesLoaded() const;

		/** 
		 * Returns true if the resource can be compressed using a generic compression when saved on a storage device. 
		 * Certain resources already have their contents compressed (like audio files) and will not benefit from further 
		 * compression. Resources supporting streaming should never be compressed, instead such resources can handle
		 * compression/decompression locally through their streams.
		 */
		virtual bool isCompressible() const { return true; }

		UINT32 mSize;
		SPtr<ResourceMetaData> mMetaData;

		/** 
		 * Signal to the resource implementation if original data should be kept in memory. This is sometimes needed if
		 * the resource destroys original data during normal usage, but it might still be required for special purposes
		 * (like saving in the editor).
		 */
		bool mKeepSourceData; 
		
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