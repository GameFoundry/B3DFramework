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

	bool Importer::supportsFileType(const WString& extension) const
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

	AsyncOp Importer::importAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions, const UUID& UUID,
		bool handle)
	{
		AsyncOp output;

		SpecificImporter* importer = prepareForImport(inputFilePath, importOptions);
		if(!importer)
		{
			output._completeOperation(HResource());
			return output;
		}

		// Fall back on sync if not supported
		ImporterAsyncMode asyncMode = importer->getAsyncMode();
		if(asyncMode == ImporterAsyncMode::None)
		{
			HResource resource = import(inputFilePath, importOptions, UUID);
			output._completeOperation(resource);

			return output;
		}

		queueForImport(importer, inputFilePath, importOptions, false, UUID, handle, output);
		return output;
	}

	Vector<SubResource> Importer::importAll(const Path& inputFilePath, SPtr<const ImportOptions> importOptions)
	{
		Vector<SubResource> output;

		Vector<SubResourceRaw> importedResource = _importAll(inputFilePath, importOptions);
		for(auto& entry : importedResource)
		{
			HResource handle = gResources()._createResourceHandle(entry.value);
			output.push_back({ entry.name, handle });
		}

		return output;
	}

	AsyncOp Importer::importAllAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions, bool handle)
	{
		AsyncOp output;

		SpecificImporter* importer = prepareForImport(inputFilePath, importOptions);
		if(!importer)
		{
			output._completeOperation(Vector<SubResource>());
			return output;
		}

		// Fall back on sync if not supported
		ImporterAsyncMode asyncMode = importer->getAsyncMode();
		if(asyncMode == ImporterAsyncMode::None)
		{
			Vector<SubResource> resources = importAll(inputFilePath, importOptions);
			output._completeOperation(resources);

			return output;
		}

		queueForImport(importer, inputFilePath, importOptions, true, UUID::EMPTY, handle, output);
		return output;
	}

	SPtr<Resource> Importer::_import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions) const
	{
		SpecificImporter* importer = prepareForImport(inputFilePath, importOptions);
		if(importer == nullptr)
			return nullptr;

		return importer->import(inputFilePath, importOptions);
	}
	
	Vector<SubResourceRaw> Importer::_importAll(const Path& inputFilePath, SPtr<const ImportOptions> importOptions) const
	{
		SpecificImporter* importer = prepareForImport(inputFilePath, importOptions);
		if(!importer)
			return Vector<SubResourceRaw>();

		return importer->importAll(inputFilePath, importOptions);
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

	void Importer::queueForImport(SpecificImporter* importer, const Path& inputFilePath, 
		SPtr<const ImportOptions> importOptions, bool importAll, const UUID& uuid, bool handle, AsyncOp& op)
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

		QueuedOperation queuedOp(importer, inputFilePath, importOptions, importAll, uuid, handle, op);
		SPtr<Task> task = Task::create("ImportWorker", 
		[this, taskId, queuedOp] 
		{ 
			AsyncOp op = queuedOp.op;
			if (queuedOp.importAll)
			{
				Vector<SubResourceRaw> rawSubresources = queuedOp.importer->importAll(queuedOp.filePath,
					queuedOp.importOptions);

				if(queuedOp.handle)
				{
					Vector<SubResource> subresources;
					for (auto& entry : rawSubresources)
					{
						HResource handle = gResources()._createResourceHandle(entry.value);
						subresources.push_back({ entry.name, handle });
					}

					op._completeOperation(subresources);
				}
				else
					op._completeOperation(rawSubresources);
			}
			else
			{
				SPtr<Resource> resourcePtr = queuedOp.importer->import(queuedOp.filePath, queuedOp.importOptions);

				if(queuedOp.handle)
				{
					HResource resource;
					if (queuedOp.uuid.empty())
						resource = gResources()._createResourceHandle(resourcePtr);
					else
						resource = gResources()._createResourceHandle(resourcePtr, queuedOp.uuid);

					op._completeOperation(resource);
				}
				else
					op._completeOperation(resourcePtr);
			}

			// Clear itself from the task list so we don't unnecessarily keep a reference. But first make sure we are the
			// last task by comparing the ids.
			Lock lock(mLastTaskMutex);
			auto iterFind = mLastQueuedTask.find(queuedOp.importer);
			if(iterFind != mLastQueuedTask.end())
			{
				if(iterFind->second.id == taskId)
					mLastQueuedTask.erase(iterFind);
			}

		}, TaskPriority::Normal, dependency);

		if(asyncMode == ImporterAsyncMode::Single)
		{
			mLastQueuedTask[importer] = QueuedTask(task, taskId);
			mLastTaskMutex.unlock();
		}

		TaskScheduler::instance().addTask(task);
	}

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
		WString ext = inputFilePath.getWExtension();
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
