---
title: Generating documentation
---

Framework's documentation is generated using [Doxygen](http://www.doxygen.nl/) for the API reference, and [daux.io](https://daux.io/) for the manuals. Before continuing make sure both are installed and can be accessed from the terminal/console.

# API reference
To generate the API reference, enter the `bsf/Documentation/Doxygen` folder and run Doxygen as such:
 - C++ API: `doxygen native.doxyconfig`
 - C# API: `doxygen csharp.doxyconfig`

Output will be placed in `bsf/Documentation/Generated/` folder, **native** sub-folder for C++ and **csharp** sub-folder for C#.

In order to get correct HTML style, you will also want to grab the relevant CSS and JS files from [https://data.banshee3d.com/SemanticUI.zip](https://data.banshee3d.com/SemanticUI.zip). This archive should be extracted in `native/html` and/or `csharp/html` folders.

# Manuals
To generate the manuals, first make sure that C++ API reference is generated as described above. The manuals contain links to the API reference and they wont be resolved unless API reference information is present.

Then enter the `bsf/Documentation/Manuals` folder and run `daux generate`. The manuals will be output to the `bsf/Documentation/Manuals/static` folder.

In order to get API reference links to resolve properly you will want to rename the `bsf/Documentation/Generated/native/html` folder to `api` and copy it to `bsf/Documentation/Manuals/static/` folder.