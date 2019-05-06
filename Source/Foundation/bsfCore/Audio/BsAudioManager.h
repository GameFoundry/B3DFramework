//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup Audio-Internal
	 *  @{
	 */

	/** Creates and destroys a specific audio system implementation. */
	class BS_CORE_EXPORT AudioFactory
	{
	public:
		virtual ~AudioFactory() = default;

		/** Initializes the audio system. */
		virtual void startUp() = 0;

		/** Shuts down the audio system. */
		virtual void shutDown() = 0;
	};

	/** Takes care of loading, initializing and shutting down of a particular audio system implementation. */
	class BS_CORE_EXPORT AudioManager : public Module<AudioManager>
	{
	public:
		/** 
		 * Initializes the physics manager and a particular audio system implementation. 
		 *
		 * @param[in]	pluginName	Name of the plugin containing a audio system implementation.
		 */
		AudioManager(const String& pluginName);
		~AudioManager();

	private:
		DynLib* mPlugin = nullptr;
		AudioFactory* mFactory = nullptr;
	};

	/** @} */
}