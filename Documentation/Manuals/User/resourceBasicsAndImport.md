Resource basics and import					{#resourceBasicsAndImport}
===============
[TOC]

Resources represent data that you can load from disk and use in your application. These may be images, meshes, fonts, and others. Normally these resources originate from other content creation programs like Photoshop or Maya. 

# Import {#resourceBasicsAndImport_a}
Before you can use such resources in the engine, you must first import them, converting them from their original format (e.g. ".jpg") into an engine object (e.g. a **Texture**).

You can import resources from its source format (e.g. ".jpg") into engine by using the @ref bs::Importer "Importer" module, accessible globally through @ref bs::gImporter() "gImporter()". Lets see an example of importing a **Texture** resource:

~~~~~~~~~~~~~{.cpp}
// Import a texture named "myTexture.jpg" from the disk
HTexture texture = gImporter().import<Texture>("myTexture.jpg");
~~~~~~~~~~~~~

We will touch upon different resource types like meshes and textures in later chapters. For now don't worry about what **Texture** is or how it works, nor what other resources types exist, and focus instead on the more general resource logic.

# Handles {#resourceBasicsAndImport_b}
Similar to scene objects and components, resources are also represented using handles. Resource handles are prefixed with an "H", followed by the resource class name (e.g. **HTexture** for the **Texture** resource, as seen above).

You may treat the handles as pointers, using "->" to access their members, comparing them for equality or with *nullptr* to check their validity. 

# Customizing import {#resourceBasicsAndImport_c}
Sometimes you need more control over import. In which case you can provide an additional **ImportOptions** object to the @ref bs::Importer::import<T> "Importer::import<T>()" method. See the example below on how we change the texture format on import, using **TextureImportOptions**.

~~~~~~~~~~~~~{.cpp}
// Create an import options object specific to textures
auto importOptions = TextureImportOptions::create();

// Specify we wish to import the texture as an uncompressed 32-bit RGBA format
importOptions->setFormat(PF_R8G8B8A8); 

// Import a texture using the specified import options
HTexture texture = gImporter().import<Texture>("myTexture.jpg", importOptions);
~~~~~~~~~~~~~

We'll touch more on import options as we talk about specific resource types in later chapters.

> Import option class names always start with the name of their resource, followed by "ImportOptions". e.g. **TextureImportOptions** for the **Texture** resource. However not all resource types have an import options object, in which case you have no choice but to import them in the default way.