---
title: Modules
---

@b3d::Module is a specialized singleton pattern used throughout the framework for managing global systems and services. Unlike standard singletons, modules require explicit startup and shutdown, providing better control over initialization order and resource management.

# Basic usage

To create a module, inherit from **Module** and provide your class as the template parameter:

~~~~~~~~~~~~~{.cpp}
class MyModule : public Module<MyModule>
{
public:
	void DoSomething()
	{
		B3D_LOG(Info, LogGeneric, "Module is doing something!");
	}

	int GetValue() const { return mValue; }

private:
	int mValue = 42;
};
~~~~~~~~~~~~~

Use @b3d::Module::StartUp to initialize the module. Once started, access it through @b3d::Module::Instance. When done, call @b3d::Module::ShutDown to release it:

~~~~~~~~~~~~~{.cpp}
// Start up the module
MyModule::StartUp();

// Access the module instance
MyModule::Instance().DoSomething();
int value = MyModule::Instance().GetValue();

// Shut down the module when done
MyModule::ShutDown();
~~~~~~~~~~~~~

# Initialization order

Modules can be started in any order, but you should be careful about dependencies between modules. If one module depends on another, start the dependency first:

~~~~~~~~~~~~~{.cpp}
// Start dependencies first
FileSystem::StartUp();
ResourceManager::StartUp();

// Then start dependent modules
SceneManager::StartUp();
Renderer::StartUp();

// Shutdown in reverse order
Renderer::ShutDown();
SceneManager::ShutDown();
ResourceManager::ShutDown();
FileSystem::ShutDown();
~~~~~~~~~~~~~

# Accessing modules

Once a module is started, you can access it from anywhere in your code:

~~~~~~~~~~~~~{.cpp}
// Check if module is started
if(MyModule::IsStarted())
{
	// Access the module
	MyModule::Instance().DoSomething();
}
~~~~~~~~~~~~~

The @b3d::Module::IsStarted method returns `true` if the module has been initialized.

# Module with initialization parameters

Modules can accept initialization parameters through their constructor. These parameters are passed to StartUp():

~~~~~~~~~~~~~{.cpp}
class ConfigurableModule : public Module<ConfigurableModule>
{
public:
	ConfigurableModule(const String& configPath, bool enableLogging)
		: mConfigPath(configPath), mLoggingEnabled(enableLogging)
	{
		B3D_LOG(Info, LogGeneric, "Module initialized with config: {0}", mConfigPath);
	}

	void Process()
	{
		if(mLoggingEnabled)
			B3D_LOG(Info, LogGeneric, "Processing...");
	}

private:
	String mConfigPath;
	bool mLoggingEnabled;
};

// Start up with parameters
ConfigurableModule::StartUp("config.json", true);
ConfigurableModule::Instance().Process();
ConfigurableModule::ShutDown();
~~~~~~~~~~~~~

# OnStartUp and OnShutDown callbacks

Modules can override @b3d::Module::OnStartUp and @b3d::Module::OnShutDown to perform initialization and cleanup:

~~~~~~~~~~~~~{.cpp}
class ResourceModule : public Module<ResourceModule>
{
protected:
	void OnStartUp() override
	{
		// Initialize resources
		mResourcePool.Reserve(1000);
		B3D_LOG(Info, LogGeneric, "Resource module started");
	}

	void OnShutDown() override
	{
		// Clean up resources
		mResourcePool.Clear(true);
		B3D_LOG(Info, LogGeneric, "Resource module shut down");
	}

private:
	Vector<Resource*> mResourcePool;
};
~~~~~~~~~~~~~