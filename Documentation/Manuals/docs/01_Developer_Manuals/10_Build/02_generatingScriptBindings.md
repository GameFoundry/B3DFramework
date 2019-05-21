---
title: Generating script bindings
---

In case you are modifying C++ code and need to expose new methods to the script API, or change the existing one, you will need to run the script binding generator tool. This tool will parse all the C++ code and generate the relevant script binding and script files.

The tool must be compiled manually from its source: [https://github.com/BearishSun/BansheeSBGen](https://github.com/BearishSun/BansheeSBGen). You should follow instructions on its GitHub page on how to compile the tool.

Once compiled the tool should be placed in the `bsf/Dependencies/tools/BansheeSBGen` folder. 

You should then run CMake with `-DSCRIPT_BINDING_GENERATION=ON`. CMake will then check if the tool is present and report an error if it cannot find it. If CMake generation goes well, you will now have a `GenerateScriptBindings` target. Running the target will rebuild all the script binding code and generated C# code.

In order to learn how to actually expose your C++ code to the script API you can follow the [Exposing code to script API](../Scripting/scriptingAuto) manual.