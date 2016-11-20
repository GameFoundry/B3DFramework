//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11Driver.h"
#include "BsD3D11VideoModeInfo.h"
#include "BsException.h"

namespace bs
{
	D3D11Driver::D3D11Driver(const D3D11Driver &ob) 
	{
		mAdapterNumber = ob.mAdapterNumber;
		mAdapterIdentifier = ob.mAdapterIdentifier;
		mDXGIAdapter = ob.mDXGIAdapter;

		if(mDXGIAdapter)
			mDXGIAdapter->AddRef();

		construct();
	}

	D3D11Driver::D3D11Driver(UINT32 adapterNumber, IDXGIAdapter* pDXGIAdapter)
	{
		mAdapterNumber = adapterNumber;
		mDXGIAdapter = pDXGIAdapter;

		if(mDXGIAdapter)
			mDXGIAdapter->AddRef();

		pDXGIAdapter->GetDesc(&mAdapterIdentifier);

		construct();
	}

	D3D11Driver::~D3D11Driver()
	{
		SAFE_RELEASE(mDXGIAdapter);
	}

	void D3D11Driver::construct()
	{
		assert(mDXGIAdapter != nullptr);

		UINT32 outputIdx = 0;
		IDXGIOutput* output = nullptr;
		while(mDXGIAdapter->EnumOutputs(outputIdx, &output) != DXGI_ERROR_NOT_FOUND)
		{
			outputIdx++;
			SAFE_RELEASE(output);
		}

		mNumOutputs = outputIdx;

		mVideoModeInfo = bs_shared_ptr_new<D3D11VideoModeInfo>(mDXGIAdapter);
	}

	D3D11Driver& D3D11Driver::operator=(const D3D11Driver& ob)
	{
		mAdapterNumber = ob.mAdapterNumber;
		mAdapterIdentifier = ob.mAdapterIdentifier;

		if(ob.mDXGIAdapter)
			ob.mDXGIAdapter->AddRef();

		SAFE_RELEASE(mDXGIAdapter);
		mDXGIAdapter = ob.mDXGIAdapter;

		return *this;
	}

	String D3D11Driver::getDriverName() const
	{
		size_t size = wcslen(mAdapterIdentifier.Description);
		char* str = (char*)bs_alloc((UINT32)(size + 1));

		wcstombs(str, mAdapterIdentifier.Description, size);
		str[size] = '\0';
		String Description = str;
		
		bs_free(str);
		return String(Description );
	}

	String D3D11Driver::getDriverDescription() const
	{
		size_t size = wcslen(mAdapterIdentifier.Description);
		char* str = (char*)bs_alloc((UINT32)(size + 1));

		wcstombs(str, mAdapterIdentifier.Description, size);
		str[size] = '\0';
		String driverDescription = str;

		bs_free(str);
		StringUtil::trim(driverDescription);

		return driverDescription;
	}

	DXGI_OUTPUT_DESC D3D11Driver::getOutputDesc(UINT32 adapterOutputIdx) const
	{
		DXGI_OUTPUT_DESC desc;
		
		IDXGIOutput* output = nullptr;
		if(mDXGIAdapter->EnumOutputs(adapterOutputIdx, &output) == DXGI_ERROR_NOT_FOUND)
		{
			BS_EXCEPT(InvalidParametersException, "Cannot find output with the specified index: " + toString(adapterOutputIdx));
		}

		output->GetDesc(&desc);

		SAFE_RELEASE(output);

		return desc;
	}
}