//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Contains a list of all available drivers. */
	class BS_D3D11_EXPORT D3D11DriverList
	{
	public:
		/**	Constructs a new driver list from an existing DXGI factory object. */
		D3D11DriverList(IDXGIFactory* dxgiFactory);
		~D3D11DriverList();

		/**	Returns the number of available drivers. */
		UINT32 count() const;

		/**	Returns a driver at the specified index. */
		D3D11Driver* item(UINT32 idx) const;

		/**	Returns a driver with the specified name, or null if it cannot be found. */
		D3D11Driver* item(const String &name) const;

	private:
		/**	Enumerates the DXGI factory object and constructs a list of available drivers. */
		void enumerate(IDXGIFactory* dxgiFactory);

		Vector<D3D11Driver*> mDriverList;
	};

	/** @} */
}