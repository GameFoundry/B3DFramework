//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11Device.h"
#include "BsException.h"

namespace bs
{
	D3D11Device::D3D11Device() 
		:mD3D11Device(nullptr), mImmediateContext(nullptr), mClassLinkage(nullptr)
	{
	}

	D3D11Device::D3D11Device(ID3D11Device* device)
		: mD3D11Device(device)
		, mImmediateContext(nullptr)
		, mInfoQueue(nullptr)
		, mClassLinkage(nullptr)
	{
		assert(device != nullptr);

		if (device)
		{
			device->GetImmediateContext(&mImmediateContext);

#if BS_DEBUG_MODE
			// This interface is not available unless we created the device with debug layer
			HRESULT hr = mD3D11Device->QueryInterface(__uuidof(ID3D11InfoQueue), (LPVOID*)&mInfoQueue);

			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Unable to query D3D11InfoQueue");

			setExceptionsErrorLevel(D3D11ERR_ERROR);
#endif

			// If feature level is 11, create class linkage
			SAFE_RELEASE(mClassLinkage);
			if (mD3D11Device->GetFeatureLevel() == D3D_FEATURE_LEVEL_11_0)
			{
				HRESULT hr = mD3D11Device->CreateClassLinkage(&mClassLinkage);

				if (FAILED(hr))
					BS_EXCEPT(RenderingAPIException, "Unable to create class linkage.");
			}
		}	
	}

	D3D11Device::~D3D11Device()
	{
		shutdown();
	}

	void D3D11Device::shutdown()
	{
		if (mImmediateContext)
		{
			mImmediateContext->Flush();
			mImmediateContext->ClearState();
		}

		SAFE_RELEASE(mInfoQueue);
		SAFE_RELEASE(mD3D11Device);
		SAFE_RELEASE(mImmediateContext);
		SAFE_RELEASE(mClassLinkage);
	}

	String D3D11Device::getErrorDescription(bool doClearErrors)
	{
		if (mD3D11Device == nullptr)
			return "Null device.";

		String res;

		if (mInfoQueue != nullptr)
		{
			UINT64 numStoredMessages = mInfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
			for (UINT64 i = 0 ; i < numStoredMessages ; i++ )
			{
				// Get the size of the message
				SIZE_T messageLength = 0;
				mInfoQueue->GetMessage(i, nullptr, &messageLength);
				// Allocate space and get the message
				D3D11_MESSAGE* pMessage = (D3D11_MESSAGE*)malloc(messageLength);
				mInfoQueue->GetMessage(i, pMessage, &messageLength);
				res = res + pMessage->pDescription + "\n";
				free(pMessage);
			}
		}

		if(doClearErrors)
			clearErrors();

		return res;
	}

	bool D3D11Device::hasError() const
	{
		if (mInfoQueue != nullptr)
		{
			UINT64 numStoredMessages = mInfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();

			if (numStoredMessages > 0)
				return true;

			return false;
		}
		else
		{
			return false;
		}
	}

	void D3D11Device::clearErrors()
	{
		if (mD3D11Device != nullptr && mInfoQueue != nullptr)
		{
			mInfoQueue->ClearStoredMessages();
		}
	}

	void D3D11Device::setExceptionsErrorLevel(const BS_D3D11_ERROR_LEVEL exceptionsErrorLevel)
	{
		if(mInfoQueue == nullptr)
			return;

		mInfoQueue->ClearRetrievalFilter();
		mInfoQueue->ClearStorageFilter();

		D3D11_INFO_QUEUE_FILTER filter;
		ZeroMemory(&filter, sizeof(D3D11_INFO_QUEUE_FILTER));
		Vector<D3D11_MESSAGE_SEVERITY> severityList;

		switch(exceptionsErrorLevel)
		{
		case D3D11ERR_INFO:
			severityList.push_back(D3D11_MESSAGE_SEVERITY_INFO);
		case D3D11ERR_WARNING:
			severityList.push_back(D3D11_MESSAGE_SEVERITY_WARNING);
		case D3D11ERR_ERROR:
			severityList.push_back(D3D11_MESSAGE_SEVERITY_ERROR);
		case D3D11ERR_CORRUPTION:
			severityList.push_back(D3D11_MESSAGE_SEVERITY_CORRUPTION);
		case D3D11ERR_NO_EXCEPTION:
		default: 
			break;
		}

		if (severityList.size() > 0)
		{
			filter.AllowList.NumSeverities = (UINT)severityList.size();
			filter.AllowList.pSeverityList = &severityList[0];
		}

		mInfoQueue->AddStorageFilterEntries(&filter);
		mInfoQueue->AddRetrievalFilterEntries(&filter);
	}
}