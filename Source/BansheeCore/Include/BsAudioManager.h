//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup Audio-Internal
	 *  @{
	 */

	/** Creates and destroys a specific audio system implementation. */
	class BS_CORE_EXPORT AudioFactory
	{
	public:
		virtual ~AudioFactory() { }

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
		DynLib* mPlugin;
		AudioFactory* mFactory;
	};

	/** @} */
}