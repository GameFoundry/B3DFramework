---
title: Configuration variables
---

The framework provides a typed configuration variable system through @b3d::TConfigVariable. Configuration variables are declared globally and provide a convenient way to expose settings that can be controlled from code, configuration files, or the command line.

# Declaring variables

Declare configuration variables at file scope. The constructor takes a name, description, default value, and optional flags:

~~~~~~~~~~~~~{.cpp}
TConfigVariable<bool> gVSync("render.vsync", "Enable vertical sync", true);
TConfigVariable<i32> gMSAA("render.msaa", "MSAA sample count", 4, ConfigVariableFlag::RenderThreadSafe);
TConfigVariable<float> gVolume("audio.volume", "Master volume", 1.0f);
TConfigVariable<u32> gMaxLights("render.maxLights", "Maximum light count", 128, ConfigVariableFlag::ReadOnly);
~~~~~~~~~~~~~

Supported value types are `bool`, `i32`, `u32`, and `float`.

# Reading and writing

Reading a configuration variable is a fast atomic load. Writing is done through @b3d::TConfigVariable::Set:

~~~~~~~~~~~~~{.cpp}
// Read a value
if(gVSync.Get())
{
	// VSync is enabled
}

// Implicit conversion is also supported
float volume = gVolume;

// Change a value at runtime
gMSAA.Set(8);

// Query the default value
i32 defaultMSAA = gMSAA.GetDefault();
~~~~~~~~~~~~~

# Value sources

Each variable tracks where its current value came from, via @b3d::ConfigVariableSource. The sources have a defined priority — a higher-priority source always overrides a lower one:

| Priority | Source | Description |
|---|---|---|
| Lowest | @b3d::ConfigVariableSource::Default | The compile-time default value |
| | @b3d::ConfigVariableSource::ConfigFile | Loaded from a configuration file |
| | @b3d::ConfigVariableSource::CommandLine | Set via a command-line parameter |
| Highest | @b3d::ConfigVariableSource::Runtime | Modified at runtime via **Set()** |

You can query the current source with @b3d::ConfigVariable::GetSource.

# Flags

@b3d::ConfigVariableFlag controls variable behavior:
 - @b3d::ConfigVariableFlag::RenderThreadSafe - Value updates are deferred to frame boundaries, ensuring the render thread always sees a consistent value throughout a frame. Use this for any variable read by the renderer.
 - @b3d::ConfigVariableFlag::ReadOnly - The variable cannot be changed after initialization. Useful for settings that should only come from the config file or command line.

# Configuration files

Configuration files use a simple `key=value` format and are loaded during application startup from `engine.ini` in the executable directory:

~~~~~~~~~~~~~
# Rendering settings
render.vsync=true
render.msaa=4

# Audio settings
audio.volume=0.8
~~~~~~~~~~~~~

Lines starting with `#` are comments. Variable names are case-insensitive. Command-line parameters with matching names automatically override configuration file values.
