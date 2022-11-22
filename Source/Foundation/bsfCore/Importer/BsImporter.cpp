//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsImporter.h"
#include "Resources/BsResource.h"
#include "FileSystem/BsFileSystem.h"
#include "Importer/BsSpecificImporter.h"
#include "Importer/BsShaderIncludeImporter.h"
#include "Importer/BsImportOptions.h"
#include "Debug/BsDebug.h"
#include "FileSystem/BsDataStream.h"
#include "Error/BsException.h"
#include "Utility/BsUUID.h"
#include "Resources/BsResources.h"
#include "Threading/BsThreadPool.h"
#include "Threading/BsTaskScheduler.h"

using namespace bs;

Importer::Importer()
{
	mAsyncOpSyncData = B3DMakeShared<AsyncOpSyncData>();

	RegisterAssetImporterInternal(B3DNew<ShaderIncludeImporter>());
}

Importer::~Importer()
{
	for(auto i = mAssetImporters.begin(); i != mAssetImporters.end(); ++i)
	{
		if((*i) != nullptr)
			B3DDelete(*i);
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

bool Importer::SupportsFileType(const u8* magicNumber, u32 magicNumSize) const
{
	for(auto iter = mAssetImporters.begin(); iter != mAssetImporters.end(); ++iter)
	{
		if(*iter != nullptr && (*iter)->IsMagicNumberSupported(magicNumber, magicNumSize))
			return true;
	}

	return false;
}

HResource Importer::Import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions, const UUID& UUID)
{
	SPtr<Resource> importedResource = ImportInternal(inputFilePath, importOptions);

	if(UUID.Empty())
		return GetResources().CreateResourceHandleInternal(importedResource);

	return GetResources().CreateResourceHandleInternal(importedResource, UUID);
}

TAsyncOp<HResource> Importer::ImportAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions, const UUID& UUID)
{
	TAsyncOp<HResource> output(mAsyncOpSyncData);

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
		HResource handle = GetResources().CreateResourceHandleInternal(entry.Value);
		output.push_back({ entry.Name, handle });
	}

	return B3DMakeShared<MultiResource>(output);
}

TAsyncOp<SPtr<MultiResource>> Importer::ImportAllAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
{
	TAsyncOp<SPtr<MultiResource>> output(mAsyncOpSyncData);

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

	const u64 taskId = WaitForAsync(importer);
	SPtr<Resource> output = importer->Import(inputFilePath, importOptions);

	if(importer->GetAsyncMode() == ImporterAsyncMode::Single)
	{
		Lock lock(mLastTaskMutex);
		auto iterFind = mLastQueuedTask.find(importer);
		if(iterFind != mLastQueuedTask.end())
		{
			if(iterFind->second.Id == taskId)
				mLastQueuedTask.erase(iterFind);

			mTaskCompleted.notify_one();
		}
	}

	return output;
}

Vector<SubResourceRaw> Importer::ImportAllInternal(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
{
	SpecificImporter* importer = PrepareForImport(inputFilePath, importOptions);
	if(!importer)
		return Vector<SubResourceRaw>();

	const u64 taskId = WaitForAsync(importer);
	Vector<SubResourceRaw> output = importer->ImportAll(inputFilePath, importOptions);

	if(importer->GetAsyncMode() == ImporterAsyncMode::Single)
	{
		Lock lock(mLastTaskMutex);
		auto iterFind = mLastQueuedTask.find(importer);
		if(iterFind != mLastQueuedTask.end())
		{
			if(iterFind->second.Id == taskId)
				mLastQueuedTask.erase(iterFind);

			mTaskCompleted.notify_one();
		}
	}
	return output;
}

SpecificImporter* Importer::PrepareForImport(const Path& filePath, SPtr<const ImportOptions>& importOptions) const
{
	if(!FileSystem::IsFile(filePath))
	{
		B3D_LOG(Warning, Importer, "Trying to import asset that doesn't exists. Asset path: {0}", filePath);
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
		if(importOptions->GetTypeId() != defaultImportOptions->GetTypeId())
		{
			B3D_EXCEPT(InvalidParametersException, "Provided import options is not of valid type. "
												  "Expected: " +
						  defaultImportOptions->GetTypeName() + ". Got: " + importOptions->GetTypeName() + ".");
		}
	}

	return importer;
}

u64 Importer::WaitForAsync(SpecificImporter* importer)
{
	u64 taskId = 0;

	const ImporterAsyncMode asyncMode = importer->GetAsyncMode();
	if(asyncMode == ImporterAsyncMode::Single)
	{
		Lock lock(mLastTaskMutex);

		// Wait for any existing async tasks to complete
		while(true)
		{
			const auto iterFind = mLastQueuedTask.find(importer);
			if(iterFind != mLastQueuedTask.end())
				mTaskCompleted.wait(lock);
			else
				break;
		}

		// Register a new task so other calls to this method know to wait
		taskId = mTaskId++;
		mLastQueuedTask[importer] = QueuedTask(nullptr, taskId);
	}

	return taskId;
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
		resource = GetResources().CreateResourceHandleInternal(resourcePtr);
	else
		resource = GetResources().CreateResourceHandleInternal(resourcePtr, uuid);

	op.CompleteOperation(resource);
}

template <>
void DoImport(TAsyncOp<SPtr<MultiResource>> op, SpecificImporter* importer, const Path& filePath, const UUID& uuid, const SPtr<const ImportOptions>& importOptions)
{
	Vector<SubResourceRaw> rawSubresources = importer->ImportAll(filePath, importOptions);

	Vector<SubResource> subresources;
	for(auto& entry : rawSubresources)
	{
		HResource handle = GetResources().CreateResourceHandleInternal(entry.Value);
		subresources.push_back({ entry.Name, handle });
	}

	op.CompleteOperation(B3DMakeShared<MultiResource>(subresources));
}

template <class ReturnType>
void Importer::QueueForImport(SpecificImporter* importer, const Path& inputFilePath, const SPtr<const ImportOptions>& importOptions, const UUID& uuid, TAsyncOp<ReturnType>& op)
{
	ImporterAsyncMode asyncMode = importer->GetAsyncMode();

	// If the importer only supports single thread import, the tasks need to be chained using dependencies so they get
	// executed in sequence
	u64 taskId = 0;
	SPtr<Task> dependency;
	if(asyncMode == ImporterAsyncMode::Single)
	{
		mLastTaskMutex.lock();
		taskId = mTaskId++;

		auto iterFind = mLastQueuedTask.find(importer);
		if(iterFind != mLastQueuedTask.end())
			dependency = iterFind->second.Task;
	}

	SPtr<Task> task = Task::Create(
		"ImportWorker",
		[this, importer, inputFilePath, importOptions, uuid, taskId, op]
		{
			DoImport(op, importer, inputFilePath, uuid, importOptions);

			// Clear itself from the task list so we don't unnecessarily keep a reference. But first make sure we are the
			// last task by comparing the ids.
			Lock lock(mLastTaskMutex);
			auto iterFind = mLastQueuedTask.find(importer);
			if(iterFind != mLastQueuedTask.end())
			{
				if(iterFind->second.Id == taskId)
					mLastQueuedTask.erase(iterFind);

				mTaskCompleted.notify_one();
			}
		},
		TaskPriority::Normal, dependency);

	if(asyncMode == ImporterAsyncMode::Single)
	{
		mLastQueuedTask[importer] = QueuedTask(task, taskId);
		mLastTaskMutex.unlock();
	}

	TaskScheduler::Instance().AddTask(task);
}

template void Importer::QueueForImport(SpecificImporter*, const Path&, const SPtr<const ImportOptions>&, const UUID&, TAsyncOp<HResource>&);

template void Importer::QueueForImport(SpecificImporter*, const Path&, const SPtr<const ImportOptions>&, const UUID&, TAsyncOp<SPtr<MultiResource>>&);

SPtr<ImportOptions> Importer::CreateImportOptions(const Path& inputFilePath)
{
	if(!FileSystem::IsFile(inputFilePath))
	{
		B3D_LOG(Warning, Importer, "Trying to import asset that doesn't exists. Asset path: {0}", inputFilePath);
		return nullptr;
	}

	SpecificImporter* importer = GetImporterForFile(inputFilePath);
	if(importer == nullptr)
		return nullptr;

	return importer->CreateImportOptions();
}

void Importer::RegisterAssetImporterInternal(SpecificImporter* importer)
{
	if(!importer)
	{
		B3D_LOG(Warning, Importer, "Trying to register a null asset importer!");
		return;
	}

	mAssetImporters.push_back(importer);
}

SpecificImporter* Importer::GetImporterForFile(const Path& inputFilePath) const
{
	String ext = inputFilePath.GetExtension();
	if(ext.empty())
		return nullptr;

	ext = ext.substr(1, ext.size() - 1); // Remove the .
	if(!SupportsFileType(ext))
	{
		B3D_LOG(Warning, Importer, "There is no importer for the provided file type. ({0})", inputFilePath);
		return nullptr;
	}

	for(auto iter = mAssetImporters.begin(); iter != mAssetImporters.end(); ++iter)
	{
		if(*iter != nullptr && (*iter)->IsExtensionSupported(ext))
		{
			return *iter;
		}
	}

	return nullptr;
}

namespace bs
{
B3D_CORE_EXPORT Importer& GetImporter()
{
	return Importer::Instance();
}
} // namespace bs
