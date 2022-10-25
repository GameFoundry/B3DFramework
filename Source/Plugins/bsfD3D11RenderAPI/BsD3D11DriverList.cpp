//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11DriverList.h"
#include "BsD3D11Driver.h"
#include "Error/BsException.h"

namespace bs
{
	namespace ct
	{
		D3D11DriverList::D3D11DriverList(IDXGIFactory1* dxgiFactory)
		{
			Enumerate(dxgiFactory);
		}

		D3D11DriverList::~D3D11DriverList(void)
		{
			for(size_t i = 0; i < mDriverList.size(); i++)
			{
				bs_delete(mDriverList[i]);
			}

			mDriverList.clear();
		}

		void D3D11DriverList::Enumerate(IDXGIFactory1* dxgiFactory)
		{
			u32 adapterIdx = 0;
			IDXGIAdapter* dxgiAdapter = nullptr;
			HRESULT hr;

			while((hr = dxgiFactory->EnumAdapters(adapterIdx, &dxgiAdapter)) != DXGI_ERROR_NOT_FOUND)
			{
				if(FAILED(hr))
				{
					SAFE_RELEASE(dxgiAdapter);
					BS_EXCEPT(InternalErrorException, "Enumerating adapters failed.");
				}

				mDriverList.push_back(bs_new<D3D11Driver>(adapterIdx, dxgiAdapter));

				SAFE_RELEASE(dxgiAdapter);
				adapterIdx++;
			}
		}

		u32 D3D11DriverList::Count() const
		{
			return (u32)mDriverList.size();
		}

		D3D11Driver* D3D11DriverList::Item(u32 idx) const
		{
			return mDriverList.at(idx);
		}

		D3D11Driver* D3D11DriverList::Item(const String& name) const
		{
			for(auto it = mDriverList.begin(); it != mDriverList.end(); ++it)
			{
				if((*it)->GetDriverDescription() == name)
					return (*it);
			}

			BS_EXCEPT(InvalidParametersException, "Cannot find video mode with the specified name.");
			return nullptr;
		}
	} // namespace ct
} // namespace bs
