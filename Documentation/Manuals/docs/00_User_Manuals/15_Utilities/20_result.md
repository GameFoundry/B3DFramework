---
title: Result and TResult
---

@b3d::Result and @b3d::TResult provide a modern error handling mechanism that allows functions to return either a success value or detailed error information without using exceptions. This makes error handling explicit and helps prevent errors from being silently ignored.

# Basic usage

## Result (no return value)

Use @b3d::Result for operations that don't return a value but can fail:

~~~~~~~~~~~~~{.cpp}
Result SaveConfiguration(const String& filename)
{
	if(filename.empty())
		return Result::Fail("Filename cannot be empty");

	if(!FileSystem::Exists(filename.GetDirectory()))
		return Result::Fail("Directory does not exist", ResultStatus::FailedNotFound);

	// Perform save operation
	bool saveSucceeded = PerformSave(filename);

	if(!saveSucceeded)
		return Result::Fail("Failed to write file", ResultStatus::FailedInternalError);

	return Result::Success();
}

// Usage
Result result = SaveConfiguration("config.json");
if(result.IsSuccessful())
{
	B3D_LOG(Info, LogGeneric, "Configuration saved successfully");
}
else
{
	B3D_LOG(Error, LogGeneric, "Save failed: {0}", result.GetFullErrorMessage());
}
~~~~~~~~~~~~~

## TResult (with return value)

Use @b3d::TResult when an operation returns a value on success:

~~~~~~~~~~~~~{.cpp}
TResult<HTexture> LoadTexture(const Path& texturePath)
{
	if(!FileSystem::Exists(texturePath))
		return TResult<HTexture>::Fail("Texture file not found", ResultStatus::FailedNotFound);

	HTexture texture = GetImporter().Import<Texture>(texturePath);

	if(!texture)
		return TResult<HTexture>::Fail("Failed to import texture");

	return TResult<HTexture>::Success(texture);
}

// Usage
TResult<HTexture> result = LoadTexture("Textures/Player.png");
if(result.IsSuccessful())
{
	HTexture texture = result.Output;
	// Use the texture
}
else
{
	B3D_LOG(Error, LogGeneric, "Failed to load texture: {0}", result.GetFullErrorMessage());
}
~~~~~~~~~~~~~

# Result status codes

@b3d::ResultStatus provides standard status codes for different failure scenarios:

- **Succeeded** - Operation completed successfully
- **Failed** - General failure
- **FailedAlreadyExists** - Resource or entity already exists
- **FailedInvalidInput** - Invalid input parameters
- **FailedInternalError** - Internal error occurred
- **FailedNotFound** - Resource or entity not found
- **FailedReadOnly** - Attempted to modify read-only resource

# Error messages

Results can have two error messages:

- **ErrorMessage** - Primary error message (const char*, lightweight)
- **AdditionalErrorMessage** - Secondary message for detailed context (String)

~~~~~~~~~~~~~{.cpp}
TResult<HMesh> LoadMesh(const Path& meshPath)
{
	if(!FileSystem::Exists(meshPath))
	{
		String details = "Searched in: " + meshPath.GetDirectory().ToString();
		return TResult<HMesh>::Fail("Mesh file not found",
			ResultStatus::FailedNotFound, details);
	}

	HMesh mesh = GetImporter().Import<Mesh>(meshPath);
	return TResult<HMesh>::Success(mesh);
}

// Usage
TResult<HMesh> result = LoadMesh("Models/Character.fbx");
if(!result.IsSuccessful())
{
	// Get combined error message
	String fullError = result.GetFullErrorMessage();
	B3D_LOG(Error, LogGeneric, "Mesh loading failed: {0}", fullError);
}
~~~~~~~~~~~~~

# Chaining results

You can chain results by inheriting error status from child operations:

~~~~~~~~~~~~~{.cpp}
Result ValidateConfiguration(const ConfigData& config)
{
	if(config.Name.empty())
		return Result::Fail("Configuration name is required", ResultStatus::FailedInvalidInput);

	return Result::Success();
}

Result SaveConfiguration(const ConfigData& config)
{
	// Validate first
	Result validationResult = ValidateConfiguration(config);
	if(!validationResult.IsSuccessful())
	{
		// Chain the error, adding context
		return Result::Fail("Configuration validation failed",
			std::move(validationResult));
	}

	// Proceed with save
	bool saved = PerformSave(config);
	if(!saved)
		return Result::Fail("File write failed");

	return Result::Success();
}

// The error messages are chained together
Result result = SaveConfiguration(config);
if(!result.IsSuccessful())
{
	// Will show: "Configuration validation failed: Configuration name is required"
	B3D_LOG(Error, LogGeneric, "{0}", result.GetFullErrorMessage());
}
~~~~~~~~~~~~~