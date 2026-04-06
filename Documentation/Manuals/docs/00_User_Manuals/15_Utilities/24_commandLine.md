---
title: Command line
---

The @b3d::CommandLine class provides access to command-line arguments passed to the application. It is automatically initialized by the framework before **Application::StartUp**.

# Checking parameters

Use @b3d::CommandLine::HasParameter to check if a named parameter was provided, and the typed getter methods to retrieve values:

~~~~~~~~~~~~~{.cpp}
// Check if a parameter was provided
if(CommandLine::HasParameter("headless"))
{
	// Running without a window
}

// Get parameter values with defaults
String gpuName = CommandLine::GetParameterValue("gpu", "default");
i32 msaaLevel = CommandLine::GetParameterValueAsInt("msaa", 4);
float volume = CommandLine::GetParameterValueAsFloat("volume", 1.0f);
bool enableVSync = CommandLine::GetParameterValueAsBool("vsync", true);
~~~~~~~~~~~~~

Parameter lookup is case-insensitive. Multiple formats are supported: `-param=value`, `--param value`, `/param:value`.

# Accessing raw arguments

You can also access the raw argument list by index:

~~~~~~~~~~~~~{.cpp}
u32 argumentCount = CommandLine::GetArgumentCount();
for(u32 argumentIndex = 0; argumentIndex < argumentCount; ++argumentIndex)
{
	const String& argument = CommandLine::GetArgument(argumentIndex);
	// Process argument
}
~~~~~~~~~~~~~

@b3d::CommandLine::GetExecutablePath returns the path to the application executable. @b3d::CommandLine::Get returns the full command-line string (excluding the executable path). @b3d::CommandLine::GetAllParameters returns all parsed parameters as an **UnorderedMap**.
