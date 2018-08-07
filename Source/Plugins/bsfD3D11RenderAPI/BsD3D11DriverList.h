//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Contains a list of all available drivers. */
	class D3D11DriverList
	{
	public:
		/**	Constructs a new driver list from an existing DXGI factory object. */
		D3D11DriverList(IDXGIFactory1* dxgiFactory);
		~D3D11DriverList();

		/**	Returns the number of available drivers. */
		UINT32 count() const;

		/**	Returns a driver at the specified index. */
		D3D11Driver* item(UINT32 idx) const;

		/**	Returns a driver with the specified name, or null if it cannot be found. */
		D3D11Driver* item(const String &name) const;

	private:
		/**	Enumerates the DXGI factory object and constructs a list of available drivers. */
		void enumerate(IDXGIFactory1* dxgiFactory);

		Vector<D3D11Driver*> mDriverList;
	};

	/** @} */
}}