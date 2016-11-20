//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include <string>
#include "BsRenderAPIFactory.h"
#include "BsRenderAPIManager.h"
#include "BsGLRenderAPI.h"

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	static const char* SystemName = "BansheeGLRenderSystem";

	/** Handles creation of the OpenGL render system. */
	class GLRenderAPIFactory : public RenderAPIFactory
	{
	public:
		/** @copydoc RenderAPIFactory::create */
		void create() override;

		/** @copydoc RenderAPIFactory::name */
		const char* name() const override { return SystemName; }

	private:
		/**	Registers the factory with the render system manager when constructed. */
		class InitOnStart
		{
		public:
			InitOnStart() 
			{ 
				static SPtr<RenderAPIFactory> newFactory;
				if(newFactory == nullptr)
				{
					newFactory = bs_shared_ptr_new<GLRenderAPIFactory>();
					RenderAPIManager::instance().registerFactory(newFactory);
				}
			}
		};

		static InitOnStart initOnStart; // Makes sure factory is registered on library load
	};

	/** @} */
}