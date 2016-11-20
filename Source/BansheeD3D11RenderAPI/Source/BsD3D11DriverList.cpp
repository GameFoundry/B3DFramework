//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11DriverList.h"
#include "BsD3D11Driver.h"
#include "BsException.h"

namespace bs
{
	D3D11DriverList::D3D11DriverList(IDXGIFactory* dxgiFactory) 
	{
		enumerate(dxgiFactory);
	}

	D3D11DriverList::~D3D11DriverList(void)
	{
		for(size_t i = 0; i < mDriverList.size(); i++)
		{
			bs_delete(mDriverList[i]);
		}

		mDriverList.clear();
	}

	void D3D11DriverList::enumerate(IDXGIFactory* dxgiFactory)
	{
		UINT32 adapterIdx = 0;
		IDXGIAdapter* dxgiAdapter = nullptr;
		HRESULT hr;

		while((hr = dxgiFactory->EnumAdapters(adapterIdx, &dxgiAdapter)) != DXGI_ERROR_NOT_FOUND)
		{
			if( FAILED(hr) )
			{
				SAFE_RELEASE(dxgiAdapter);
				BS_EXCEPT(InternalErrorException, "Enumerating adapters failed.");
			}

			mDriverList.push_back(bs_new<D3D11Driver>(adapterIdx, dxgiAdapter));

			SAFE_RELEASE(dxgiAdapter);
			adapterIdx++;
		}
	}

	UINT32 D3D11DriverList::count() const 
	{
		return (UINT32)mDriverList.size();
	}

	D3D11Driver* D3D11DriverList::item(UINT32 idx) const
	{
		return mDriverList.at(idx);
	}

	D3D11Driver* D3D11DriverList::item(const String &name) const
	{
		for (auto it = mDriverList.begin(); it != mDriverList.end(); ++it)
		{
			if ((*it)->getDriverDescription() == name)
				return (*it);
		}

		BS_EXCEPT(InvalidParametersException, "Cannot find video mode with the specified name.");
		return nullptr;
	}
}