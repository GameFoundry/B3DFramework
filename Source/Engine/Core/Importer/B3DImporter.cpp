//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/B3DImporter.h"

#include "B3DApplication.h"
#include "Resources/B3DResource.h"
#include "FileSystem/B3DFileSystem.h"
#include "Importer/B3DSpecificImporter.h"
#include "Importer/B3DShaderIncludeImporter.h"
#include "Importer/B3DImportOptions.h"
#include "Debug/B3DDebug.h"
#include "FileSystem/B3DDataStream.h"
#include "Utility/B3DUUID.h"
#include "Resources/B3DResources.h"
#include "Threading/B3DSingleConsumerQueue.h"
#include "Threading/B3DThreadPool.h"

using namespace b3d;

Importer::Importer()
{
	RegisterAssetImporter(B3DNew<ShaderIncludeImporter>());
}

Importer::~Importer()
{
	for(auto iter = mAssetImporters.begin(); iter != mAssetImporters.end(); ++iter)
	{
		if((*iter) != nullptr)
			B3DDelete(*iter);
	}

	mAssetImporters.clear();
}

bool Importer::SupportsFileType(const String& extension) const
{
	for(auto iter = mAssetImporters.begin(); iter != mAssetImporters.end(); ++iter)
	{
		if(*iter != nullptr && (*iter)->IsExtensionSupported(extension))
			return true;
	}

	return false;
}

bool Importer::SupportsFileType(const u8* magicNumber, u32 magicNumberSize) const
{
	for(auto iter = mAssetImporters.begin(); iter != mAssetImporters.end(); ++iter)
	{
		if(*iter != nullptr && (*iter)->IsMagicNumberSupported(magicNumber, magicNumberSize))
			return true;
	}

	return false;
}

HResource Importer::Import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions, const UUID& UUID)
{
	SPtr<Resource> importedResource = ImportInternal(inputFilePath, importOptions);

	if(UUID.Empty())
		return GetResources().CreateResourceHandle(importedResource);

	return GetResources().CreateResourceHandle(importedResource, UUID);
}

TAsyncOp<HResource> Importer::ImportAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions, const UUID& UUID)
{
	TAsyncOp<HResource> output;

	SpecificImporter* importer = PrepareForImport(inputFilePath, importOptions);
	if(!importer)
	{
		output.CompleteOperation(HResource());
		return output;
	}

	QueueForImport(importer, inputFilePath, importOptions, UUID, output);
	return output;
}

SPtr<MultiResource> Importer::ImportAll(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
{
	Vector<SubResource> output;

	Vector<SubResourceRaw> importedResource = ImportAllInternal(inputFilePath, importOptions);
	for(auto& entry : importedResource)
	{
		HResource handle = GetResources().CreateResourceHandle(entry.Value);
		output.push_back({ entry.Name, handle });
	}

	return B3DMakeShared<MultiResource>(output);
}

TAsyncOp<SPtr<MultiResource>> Importer::ImportAllAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
{
	TAsyncOp<SPtr<MultiResource>> output;

	SpecificImporter* importer = PrepareForImport(inputFilePath, importOptions);
	if(!importer)
	{
		output.CompleteOperation(B3DMakeShared<MultiResource>());
		return output;
	}

	QueueForImport(importer, inputFilePath, importOptions, UUID::kEmpty, output);
	return output;
}

SPtr<Resource> Importer::ImportInternal(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
{
	SpecificImporter* importer = PrepareForImport(inputFilePath, importOptions);
	if(importer == nullptr)
		return nullptr;

	WaitForAsync(importer);
	return importer->Import(inputFilePath, importOptions);
}

Vector<SubResourceRaw> Importer::ImportAllInternal(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
{
	SpecificImporter* importer = PrepareForImport(inputFilePath, importOptions);
	if(!importer)
		return Vector<SubResourceRaw>();

	WaitForAsync(importer);
	return importer->ImportAll(inputFilePath, importOptions);
}

TAsyncOp<Vector<SubResourceRaw>> Importer::ImportAllAsyncInternal(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
{
	TAsyncOp<Vector<SubResourceRaw>> output;

	SpecificImporter* importer = PrepareForImport(inputFilePath, importOptions);
	if(!importer)
	{
		output.CompleteOperation(Vector<SubResourceRaw>());
		return output;
	}

	QueueForImport(importer, inputFilePath, importOptions, UUID::kEmpty, output);
	return output;
}

SpecificImporter* Importer::PrepareForImport(const Path& filePath, SPtr<const ImportOptions>& importOptions) const
{
	if(!FileSystem::IsFile(filePath))
	{
		B3D_LOG(Warning, LogImporter, "Trying to import asset that doesn't exists. Asset path: {0}", filePath);
		return nullptr;
	}

	SpecificImporter* importer = GetImporterForFile(filePath);
	if(importer == nullptr)
		return nullptr;

	if(importOptions == nullptr)
		importOptions = importer->GetDefaultImportOptions();
	else
	{
		SPtr<const ImportOptions> defaultImportOptions = importer->GetDefaultImportOptions();
		if(!B3D_ENSURE_LOG(importOptions->GetTypeId() == defaultImportOptions->GetTypeId(), "Provided import options is not of valid type. Expected: {0}, Got: {1}", defaultImportOptions->GetTypeName(), importOptions->GetTypeName()))
			return nullptr;
	}

	return importer;
}

void Importer::WaitForAsync(SpecificImporter* importer)
{
	const ImporterAsyncMode asyncMode = importer->GetAsyncMode();
	if (asyncMode != ImporterAsyncMode::Single)
		return;

	SchedulerTicketQueue* perImporterQueue = nullptr;
	{
		Lock lock(mPerImporterQueueMutex);

		auto foundQueue = mPerImporterQueues.find(importer);
		if(foundQueue == mPerImporterQueues.end())
			return;

		perImporterQueue = foundQueue->second.get();

		if(!B3D_ENSURE(perImporterQueue != nullptr))
			return;
	}

	SchedulerTicket waitTicket =  perImporterQueue->TakeTicket();
	waitTicket.WaitUntilCalled();
}

template <class ReturnType>
void DoImport(TAsyncOp<ReturnType> op, SpecificImporter* importer, const Path& filePath, const UUID& uuid, const SPtr<const ImportOptions>& importOptions)
{
	B3D_ASSERT(false && "Invalid template instantiation called.");
}

template <>
void DoImport(TAsyncOp<HResource> op, SpecificImporter* importer, const Path& filePath, const UUID& uuid, const SPtr<const ImportOptions>& importOptions)
{
	SPtr<Resource> resourcePtr = importer->Import(filePath, importOptions);

	HResource resource;
	if(uuid.Empty())
		resource = GetResources().CreateResourceHandle(resourcePtr);
	else
		resource = GetResources().CreateResourceHandle(resourcePtr, uuid);

	op.CompleteOperation(resource);
}

template <>
void DoImport(TAsyncOp<SPtr<MultiResource>> op, SpecificImporter* importer, const Path& filePath, const UUID& uuid, const SPtr<const ImportOptions>& importOptions)
{
	Vector<SubResourceRaw> rawSubresources = importer->ImportAll(filePath, importOptions);

	Vector<SubResource> subresources;
	for(auto& entry : rawSubresources)
	{
		HResource handle = GetResources().CreateResourceHandle(entry.Value);
		subresources.push_back({ entry.Name, handle });
	}

	op.CompleteOperation(B3DMakeShared<MultiResource>(subresources));
}

template <>
void DoImport(TAsyncOp<Vector<SubResourceRaw>> op, SpecificImporter* importer, const Path& filePath, const UUID& uuid, const SPtr<const ImportOptions>& importOptions)
{
	Vector<SubResourceRaw> rawSubresources = importer->ImportAll(filePath, importOptions);

	op.CompleteOperation(rawSubresources);
}

template <class ReturnType>
void Importer::QueueForImport(SpecificImporter* importer, const Path& inputFilePath, const SPtr<const ImportOptions>& importOptions, const UUID& uuid, TAsyncOp<ReturnType>& op)
{
	ImporterAsyncMode asyncMode = importer->GetAsyncMode();
	if (asyncMode == ImporterAsyncMode::Multi)
	{
		auto fnDoImport = [this, importer, inputFilePath, importOptions, uuid, op]
		{
			DoImport(op, importer, inputFilePath, uuid, importOptions);
		};

		GetApplication().GetTaskScheduler().Post(SchedulerTask(std::move(fnDoImport), "ImportWorker", SchedulerTaskFlag::None, inputFilePath.ToString()));
	}
	else
	{
		SchedulerTicketQueue* perImporterQueue;
		{
			Lock lock(mPerImporterQueueMutex);

			if(const auto& foundQueue = mPerImporterQueues.find(importer); foundQueue != mPerImporterQueues.end())
				perImporterQueue = foundQueue->second.get();
			else
			{
				UPtr<SchedulerTicketQueue> newQueue = B3DMakeUnique<SchedulerTicketQueue>(GetApplication().GetTaskScheduler());
				perImporterQueue = newQueue.get();

				mPerImporterQueues[importer] = std::move(newQueue);
			}
		}

		SchedulerTicket ticket = perImporterQueue->TakeTicket();
		auto fnWaitForPreviousAndDoImport = [this, importer, inputFilePath, importOptions, uuid, op, ticket]
		{
			ticket.WaitUntilCalled();

			DoImport(op, importer, inputFilePath, uuid, importOptions);
		};

		GetApplication().GetTaskScheduler().Post(SchedulerTask(std::move(fnWaitForPreviousAndDoImport), "ImportWorker", SchedulerTaskFlag::None, inputFilePath.ToString()));
	}
}

template void Importer::QueueForImport(SpecificImporter*, const Path&, const SPtr<const ImportOptions>&, const UUID&, TAsyncOp<HResource>&);

template void Importer::QueueForImport(SpecificImporter*, const Path&, const SPtr<const ImportOptions>&, const UUID&, TAsyncOp<SPtr<MultiResource>>&);

SPtr<ImportOptions> Importer::CreateImportOptions(const Path& inputFilePath)
{
	if(!FileSystem::IsFile(inputFilePath))
	{
		B3D_LOG(Warning, LogImporter, "Trying to import asset that doesn't exists. Asset path: {0}", inputFilePath);
		return nullptr;
	}

	SpecificImporter* importer = GetImporterForFile(inputFilePath);
	if(importer == nullptr)
		return nullptr;

	return importer->CreateImportOptions();
}

void Importer::RegisterAssetImporter(SpecificImporter* importer)
{
	if(!importer)
	{
		B3D_LOG(Warning, LogImporter, "Trying to register a null asset importer!");
		return;
	}

	mAssetImporters.push_back(importer);
}

SpecificImporter* Importer::GetImporterForFile(const Path& inputFilePath) const
{
	String extension = inputFilePath.GetExtension();
	if(extension.empty())
		return nullptr;

	extension = extension.substr(1, extension.size() - 1); // Remove the .
	if(!SupportsFileType(extension))
	{
		B3D_LOG(Warning, LogImporter, "There is no importer for the provided file type. ({0})", inputFilePath);
		return nullptr;
	}

	for(auto iter = mAssetImporters.begin(); iter != mAssetImporters.end(); ++iter)
	{
		if(*iter != nullptr && (*iter)->IsExtensionSupported(extension))
		{
			return *iter;
		}
	}

	return nullptr;
}

namespace b3d
{
B3D_EXPORT Importer& GetImporter()
{
	return Importer::Instance();
}
} // namespace b3d
