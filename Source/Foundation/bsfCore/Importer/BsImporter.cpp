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

namespace bs
{
	Importer::Importer()
	{
		mAsyncOpSyncData = bs_shared_ptr_new<AsyncOpSyncData>();

		_registerAssetImporter(bs_new<ShaderIncludeImporter>());
	}

	Importer::~Importer()
	{
		for(auto i = mAssetImporters.begin(); i != mAssetImporters.end(); ++i)
		{
			if((*i) != nullptr)
				bs_delete(*i);
		}

		mAssetImporters.clear();
	}

	bool Importer::supportsFileType(const String& extension) const
	{
		for(auto iter = mAssetImporters.begin(); iter != mAssetImporters.end(); ++iter)
		{
			if(*iter != nullptr && (*iter)->isExtensionSupported(extension))
				return true;
		}

		return false;
	}

	bool Importer::supportsFileType(const UINT8* magicNumber, UINT32 magicNumSize) const
	{
		for(auto iter = mAssetImporters.begin(); iter != mAssetImporters.end(); ++iter)
		{
			if(*iter != nullptr && (*iter)->isMagicNumberSupported(magicNumber, magicNumSize))
				return true;
		}

		return false;
	}

	HResource Importer::import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions, const UUID& UUID)
	{
		SPtr<Resource> importedResource = _import(inputFilePath, importOptions);

		if(UUID.empty())
			return gResources()._createResourceHandle(importedResource);

		return gResources()._createResourceHandle(importedResource, UUID);
	}

	TAsyncOp<HResource> Importer::importAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions, 
		const UUID& UUID)
	{
		TAsyncOp<HResource> output(mAsyncOpSyncData);

		SpecificImporter* importer = prepareForImport(inputFilePath, importOptions);
		if(!importer)
		{
			output._completeOperation(HResource());
			return output;
		}

		queueForImport(importer, inputFilePath, importOptions, UUID, output);
		return output;
	}

	SPtr<MultiResource> Importer::importAll(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
	{
		Vector<SubResource> output;

		Vector<SubResourceRaw> importedResource = _importAll(inputFilePath, importOptions);
		for(auto& entry : importedResource)
		{
			HResource handle = gResources()._createResourceHandle(entry.value);
			output.push_back({ entry.name, handle });
		}

		return bs_shared_ptr_new<MultiResource>(output);
	}

	TAsyncOp<SPtr<MultiResource>> Importer::importAllAsync(const Path& inputFilePath, 
		SPtr<const ImportOptions> importOptions)
	{
		TAsyncOp<SPtr<MultiResource>> output(mAsyncOpSyncData);

		SpecificImporter* importer = prepareForImport(inputFilePath, importOptions);
		if(!importer)
		{
			output._completeOperation(bs_shared_ptr_new<MultiResource>());
			return output;
		}

		queueForImport(importer, inputFilePath, importOptions, UUID::EMPTY, output);
		return output;
	}

	SPtr<Resource> Importer::_import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
	{
		SpecificImporter* importer = prepareForImport(inputFilePath, importOptions);
		if(importer == nullptr)
			return nullptr;

		const UINT64 taskId = waitForAsync(importer);
		SPtr<Resource> output = importer->import(inputFilePath, importOptions);
		
		if(importer->getAsyncMode() == ImporterAsyncMode::Single)
		{
			Lock lock(mLastTaskMutex);
			auto iterFind = mLastQueuedTask.find(importer);
			if (iterFind != mLastQueuedTask.end())
			{
				if (iterFind->second.id == taskId)
					mLastQueuedTask.erase(iterFind);

				mTaskCompleted.notify_one();
			}
		}

		return output;
	}

	Vector<SubResourceRaw> Importer::_importAll(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
	{
		SpecificImporter* importer = prepareForImport(inputFilePath, importOptions);
		if(!importer)
			return Vector<SubResourceRaw>();

		const UINT64 taskId = waitForAsync(importer);
		Vector<SubResourceRaw> output = importer->importAll(inputFilePath, importOptions);

		if(importer->getAsyncMode() == ImporterAsyncMode::Single)
		{
			Lock lock(mLastTaskMutex);
			auto iterFind = mLastQueuedTask.find(importer);
			if (iterFind != mLastQueuedTask.end())
			{
				if (iterFind->second.id == taskId)
					mLastQueuedTask.erase(iterFind);

				mTaskCompleted.notify_one();
			}
		}
		return output;
	}

	SpecificImporter* Importer::prepareForImport(const Path& filePath, SPtr<const ImportOptions>& importOptions) const
	{
		if (!FileSystem::isFile(filePath))
		{
			LOGWRN("Trying to import asset that doesn't exists. Asset path: " + filePath.toString());
			return nullptr;
		}

		SpecificImporter* importer = getImporterForFile(filePath);
		if (importer == nullptr)
			return nullptr;

		if (importOptions == nullptr)
			importOptions = importer->getDefaultImportOptions();
		else
		{
			SPtr<const ImportOptions> defaultImportOptions = importer->getDefaultImportOptions();
			if (importOptions->getTypeId() != defaultImportOptions->getTypeId())
			{
				BS_EXCEPT(InvalidParametersException, "Provided import options is not of valid type. " \
					"Expected: " + defaultImportOptions->getTypeName() + ". Got: " + importOptions->getTypeName() + ".");
			}
		}

		return importer;
	}

	UINT64 Importer::waitForAsync(SpecificImporter* importer)
	{
		UINT64 taskId = 0;

		const ImporterAsyncMode asyncMode = importer->getAsyncMode();
		if(asyncMode == ImporterAsyncMode::Single)
		{
			Lock lock(mLastTaskMutex);

			// Wait for any existing async tasks to complete
			while(true)
			{
				const auto iterFind = mLastQueuedTask.find(importer);
				if (iterFind != mLastQueuedTask.end())
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

	template<class ReturnType>
	void doImport(TAsyncOp<ReturnType> op, SpecificImporter* importer, const Path& filePath, const UUID& uuid, 
		const SPtr<const ImportOptions>& importOptions)
	{
		assert(false && "Invalid template instantiation called.");
	}

	template<>
	void doImport(TAsyncOp<HResource> op, SpecificImporter* importer, const Path& filePath, const UUID& uuid,
		const SPtr<const ImportOptions>& importOptions)
	{
		SPtr<Resource> resourcePtr = importer->import(filePath, importOptions);

		HResource resource;
		if (uuid.empty())
			resource = gResources()._createResourceHandle(resourcePtr);
		else
			resource = gResources()._createResourceHandle(resourcePtr, uuid);

		op._completeOperation(resource);
	}

	template<>
	void doImport(TAsyncOp<SPtr<MultiResource>> op, SpecificImporter* importer, const Path& filePath, const UUID& uuid,
		const SPtr<const ImportOptions>& importOptions)
	{
		Vector<SubResourceRaw> rawSubresources = importer->importAll(filePath, importOptions);

		Vector<SubResource> subresources;
		for (auto& entry : rawSubresources)
		{
			HResource handle = gResources()._createResourceHandle(entry.value);
			subresources.push_back({ entry.name, handle });
		}

		op._completeOperation(bs_shared_ptr_new<MultiResource>(subresources));
	}

	template<class ReturnType>
	void Importer::queueForImport(SpecificImporter* importer, const Path& inputFilePath, 
		const SPtr<const ImportOptions>& importOptions, const UUID& uuid, TAsyncOp<ReturnType>& op)
	{
		ImporterAsyncMode asyncMode = importer->getAsyncMode();

		// If the importer only supports single thread import, the tasks need to be chained using dependencies so they get
		// executed in sequence
		UINT64 taskId = 0;
		SPtr<Task> dependency;
		if(asyncMode == ImporterAsyncMode::Single)
		{
			mLastTaskMutex.lock();
			taskId = mTaskId++;

			auto iterFind = mLastQueuedTask.find(importer);
			if(iterFind != mLastQueuedTask.end())
				dependency = iterFind->second.task;
		}

		SPtr<Task> task = Task::create("ImportWorker", 
		[this, importer, inputFilePath, importOptions, uuid, taskId, op] 
		{ 
			doImport(op, importer, inputFilePath, uuid, importOptions);

			// Clear itself from the task list so we don't unnecessarily keep a reference. But first make sure we are the
			// last task by comparing the ids.
			Lock lock(mLastTaskMutex);
			auto iterFind = mLastQueuedTask.find(importer);
			if(iterFind != mLastQueuedTask.end())
			{
				if(iterFind->second.id == taskId)
					mLastQueuedTask.erase(iterFind);

				mTaskCompleted.notify_one();
			}

		}, TaskPriority::Normal, dependency);

		if(asyncMode == ImporterAsyncMode::Single)
		{
			mLastQueuedTask[importer] = QueuedTask(task, taskId);
			mLastTaskMutex.unlock();
		}

		TaskScheduler::instance().addTask(task);
	}

	template void Importer::queueForImport(SpecificImporter*, const Path&, const SPtr<const ImportOptions>&, const UUID&, 
		TAsyncOp<HResource>&);

	template void Importer::queueForImport(SpecificImporter*, const Path&, const SPtr<const ImportOptions>&, const UUID&, 
		TAsyncOp<SPtr<MultiResource>>&);

	SPtr<ImportOptions> Importer::createImportOptions(const Path& inputFilePath)
	{
		if(!FileSystem::isFile(inputFilePath))
		{
			LOGWRN("Trying to import asset that doesn't exists. Asset path: " + inputFilePath.toString());
			return nullptr;
		}

		SpecificImporter* importer = getImporterForFile(inputFilePath);
		if(importer == nullptr)
			return nullptr;

		return importer->createImportOptions();
	}

	void Importer::_registerAssetImporter(SpecificImporter* importer)
	{
		if(!importer)
		{
			LOGWRN("Trying to register a null asset importer!");
			return;
		}

		mAssetImporters.push_back(importer);
	}

	SpecificImporter* Importer::getImporterForFile(const Path& inputFilePath) const
	{
		String ext = inputFilePath.getExtension();
		if (ext.empty())
			return nullptr;

		ext = ext.substr(1, ext.size() - 1); // Remove the .
		if(!supportsFileType(ext))
		{
			LOGWRN("There is no importer for the provided file type. (" + inputFilePath.toString() + ")");
			return nullptr;
		}

		for(auto iter = mAssetImporters.begin(); iter != mAssetImporters.end(); ++iter)
		{
			if(*iter != nullptr && (*iter)->isExtensionSupported(ext))
			{
				return *iter;
			}
		}

		return nullptr;
	}

	BS_CORE_EXPORT Importer& gImporter()
	{
		return Importer::instance();
	}
}
