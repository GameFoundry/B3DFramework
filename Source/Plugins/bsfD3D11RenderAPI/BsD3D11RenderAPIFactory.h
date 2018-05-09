//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Managers/BsRenderAPIFactory.h"
#include "Managers/BsRenderAPIManager.h"
#include "BsD3D11RenderAPI.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */
	/**	Handles creation of the DX11 render system. */
	class D3D11RenderAPIFactory : public RenderAPIFactory
	{
	public:
		static constexpr const char* SystemName = "bsfD3D11RenderAPI";

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
}}
