//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderAPIFactory.h"
#include "BsRenderAPIManager.h"
#include "BsD3D11RenderAPI.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	extern const char* SystemName;

	/**	Handles creation of the DX11 render system. */
	class D3D11RenderAPIFactory : public RenderAPIFactory
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
					newFactory = bs_shared_ptr_new<D3D11RenderAPIFactory>();
					RenderAPIManager::instance().registerFactory(newFactory);
				}
			}
		};

		static InitOnStart initOnStart; // Makes sure factory is registered on program start
	};

	/** @} */
}