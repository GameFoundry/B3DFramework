//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsRenderAPIFactory.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Manager that handles render system start up. */
	class BS_CORE_EXPORT RenderAPIManager : public Module<RenderAPIManager>
	{
	public:
		RenderAPIManager();
		~RenderAPIManager();

		/**
		 * Starts the render API with the provided name and creates the primary render window.
		 *
		 * @param[in]	name				Name of the render system to start. Factory for this render system must be 
		 *									previously registered.
		 * @param[in]	primaryWindowDesc	Contains options used for creating the primary window.
		 * @return							Created render window if initialization is successful, null otherwise.
		 */
		SPtr<RenderWindow> initialize(const String& name, RENDER_WINDOW_DESC& primaryWindowDesc);

		/**	Registers a new render API factory responsible for creating a specific render system type. */
		void registerFactory(SPtr<RenderAPIFactory> factory);
	private:
		Vector<SPtr<RenderAPIFactory>> mAvailableFactories;
		bool mRenderAPIInitialized;
	};

	/** @} */
}

