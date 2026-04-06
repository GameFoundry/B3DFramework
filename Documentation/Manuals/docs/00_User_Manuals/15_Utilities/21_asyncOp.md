---
title: AsyncOp and TAsyncOp
---

@b3d::AsyncOp and @b3d::TAsyncOp represent asynchronous operations that may complete at some point in the future. They provide a safe way to check operation status, retrieve results, and register callbacks without blocking execution unnecessarily.

# Basic usage

## TAsyncOp (with return value)

Use @b3d::TAsyncOp when an asynchronous operation returns a value:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<HTexture> LoadTextureAsync(const Path& texturePath)
{
	TAsyncOp<HTexture> asyncOp;

	auto loadTask = [texturePath, asyncOp]() mutable
	{
		HTexture texture = GetResources().Load<Texture>(texturePath);
		asyncOp.CompleteOperation(texture);
	};

	Scheduler::Get()->Post(SchedulerTask(loadTask, "LoadTexture"));

	return asyncOp;
}

// Usage
TAsyncOp<HTexture> operation = LoadTextureAsync("Textures/Player.png");

// Check if completed
if(operation.HasCompleted())
{
	HTexture texture = operation.GetReturnValue();
	// Use the texture
}
~~~~~~~~~~~~~

## TAsyncOp<void> (no return value)

Use @b3d::TAsyncOp<void> for operations that complete without returning a value:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<void> SaveGameAsync(const SaveData& data)
{
	TAsyncOp<void> asyncOp;

	auto saveTask = [data, asyncOp]() mutable
	{
		FileStream file("save.dat", FileMode::Write);
		file.Write(data);
		file.Close();

		asyncOp.CompleteOperation();
	};

	ThreadPool::Instance().Run("SaveGame", saveTask);

	return asyncOp;
}

// Usage
TAsyncOp<void> operation = SaveGameAsync(saveData);

// Wait for completion
operation.BlockUntilComplete();

B3D_LOG(Info, LogGeneric, "Save completed");
~~~~~~~~~~~~~

# Checking completion status

Use @b3d::AsyncOp::HasCompleted to check if an operation has finished without blocking:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<HMesh> loadOperation = LoadMeshAsync("Models/Character.fbx");

// In your update loop
if(loadOperation.HasCompleted())
{
	HMesh mesh = loadOperation.GetReturnValue();
	B3D_LOG(Info, LogGeneric, "Mesh loaded successfully");
}
else
{
	B3D_LOG(Info, LogGeneric, "Still loading...");
}
~~~~~~~~~~~~~

# Blocking until completion

Use @b3d::AsyncOp::BlockUntilComplete to wait for an operation to finish:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<ConfigData> loadOperation = LoadConfigAsync("config.json");

// Block the current thread until the operation completes
loadOperation.BlockUntilComplete();

// Operation is now guaranteed to be complete
ConfigData config = loadOperation.GetReturnValue();
~~~~~~~~~~~~~

By default, @b3d::AsyncOp::BlockUntilComplete also waits for all registered callbacks to finish. You can pass `false` to only wait for the operation itself:

~~~~~~~~~~~~~{.cpp}
// Only wait for the operation, not callbacks
loadOperation.BlockUntilComplete(false);
~~~~~~~~~~~~~

# Completion callbacks

Use @b3d::AsyncOp::DoWhenComplete to register a callback that executes when the operation finishes:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<HTexture> loadOperation = LoadTextureAsync("Textures/UI_Button.png");

// Register callback
loadOperation.DoWhenComplete([loadOperation]()
{
	HTexture texture = loadOperation.GetReturnValue();
	B3D_LOG(Info, LogGeneric, "Texture loaded: {0}", texture->GetName());
});

// The callback will execute on the same thread that registered it
// Your code continues executing immediately
B3D_LOG(Info, LogGeneric, "Load initiated, callback registered");
~~~~~~~~~~~~~

Callbacks are guaranteed to execute on the thread that registered them. You can register multiple callbacks:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<HMesh> loadOperation = LoadMeshAsync("Models/Weapon.fbx");

// Register first callback for logging
loadOperation.DoWhenComplete([]()
{
	B3D_LOG(Info, LogGeneric, "Mesh load completed");
});

// Register second callback for processing
loadOperation.DoWhenComplete([loadOperation]()
{
	HMesh mesh = loadOperation.GetReturnValue();
	ProcessMesh(mesh);
});

// Both callbacks will execute when the operation completes
~~~~~~~~~~~~~

If the operation has already completed when you call @b3d::AsyncOp::DoWhenComplete, the callback executes immediately:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<int> operation = CalculateAsync(42);

// Wait for completion
operation.BlockUntilComplete();

// This callback executes immediately since the operation is already complete
operation.DoWhenComplete([]()
{
	B3D_LOG(Info, LogGeneric, "Callback executed immediately");
});
~~~~~~~~~~~~~

# Retrieving return values

Use @b3d::TAsyncOp::GetReturnValue to retrieve the result of a completed operation:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<Vector<String>> searchOperation = SearchFilesAsync("*.txt");

// Wait for completion
searchOperation.BlockUntilComplete();

// Get the result
Vector<String> files = searchOperation.GetReturnValue();

for(const String& file : files)
{
	B3D_LOG(Info, LogGeneric, "Found: {0}", file);
}
~~~~~~~~~~~~~

Calling @b3d::TAsyncOp::GetReturnValue before the operation completes produces an error in debug builds:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<int> operation = CalculateAsync(100);

// Always check completion first
if(operation.HasCompleted())
{
	int result = operation.GetReturnValue();
}
else
{
	// Don't call GetReturnValue() here - operation not complete yet!
}
~~~~~~~~~~~~~

# Copying and moving

@b3d::TAsyncOp objects can be copied and moved freely. All copies reference the same underlying operation:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<HTexture> operation1 = LoadTextureAsync("Textures/A.png");

// Copy the operation
TAsyncOp<HTexture> operation2 = operation1;

// Both refer to the same async operation
operation1.BlockUntilComplete();

// operation2 is also complete
B3D_ASSERT(operation2.HasCompleted());
~~~~~~~~~~~~~