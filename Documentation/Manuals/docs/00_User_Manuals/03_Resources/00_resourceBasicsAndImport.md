---
title: Resource basics and import
---

Resources represent data that you can load from disk and use in your application. These may be images, meshes, fonts, audio files, and many others. Normally these resources originate from other content creation programs like Photoshop, Blender, or Audacity.

# Import
Before you can use such resources in the engine, you must first import them, converting them from their original format (e.g. ".jpg") into an engine resource (e.g. a **Texture**).

You can import resources from their source format (e.g. ".jpg") using the @b3d::Importer module, accessible globally through @b3d::GetImporter(). Let's see an example of importing a **Texture** resource:

~~~~~~~~~~~~~{.cpp}
// Import a texture named "myTexture.jpg" from the disk
HTexture texture = GetImporter().Import<Texture>("myTexture.jpg");
~~~~~~~~~~~~~

We will cover different resource types like meshes and textures in later chapters. For now don't worry about what **Texture** is or how it works, nor what other resource types exist, and focus instead on the more general resource logic.

# Handles
Similar to scene objects and components, resources are also represented using handles. Resource handles are prefixed with an "H", followed by the resource class name (e.g. **HTexture** for the **Texture** resource, as seen above).

You may treat the handles as pointers, using "->" to access their members, comparing them for equality or with *nullptr* to check their validity.

~~~~~~~~~~~~~{.cpp}
// Check if a handle is valid
if (texture != nullptr)
{
	// Use the texture
	u32 width = texture->GetProperties().Width;
}

// Compare two handles
HTexture texture2 = GetImporter().Import<Texture>("anotherTexture.jpg");
if (texture == texture2)
{
	// They are the same resource
}
~~~~~~~~~~~~~

# Customizing import
Sometimes you need more control over import. In which case you can provide an additional **ImportOptions** object to the @b3d::Importer::Import method. You can create import options using @b3d::Importer::CreateImportOptions, which will automatically detect the correct options type based on the file extension.

~~~~~~~~~~~~~{.cpp}
// Create import options for a texture file
auto importOptions = GetImporter().CreateImportOptions<TextureImportOptions>("myTexture.jpg");

// Specify we wish to import the texture as an uncompressed 32-bit RGBA format
importOptions->Format = PF_RGBA8;

// Import a texture using the specified import options
HTexture texture = GetImporter().Import<Texture>("myTexture.jpg", importOptions);
~~~~~~~~~~~~~

> Import option class names always start with the name of their resource, followed by "ImportOptions". e.g. **TextureImportOptions** for the **Texture** resource. However not all resource types have an import options object, in which case you have no choice but to import them in the default way.

# Asynchronous import
By default, importing is performed synchronously and blocks the calling thread until complete. For large resources like meshes or textures, this can cause noticeable delays. You can import resources asynchronously using @b3d::Importer::ImportAsync:

~~~~~~~~~~~~~{.cpp}
// Start async import - returns immediately
TAsyncOp<HTexture> asyncOp = GetImporter().ImportAsync<Texture>("largeTexture.jpg");

// Do other work...

// Check if import is complete
if (asyncOp.HasCompleted())
{
	HTexture texture = asyncOp.GetReturnValue();
}

// Or block until complete
HTexture texture = asyncOp.BlockUntilComplete();
~~~~~~~~~~~~~

# Multi-resource import
Some file formats can contain multiple resources. For example, an FBX file might contain a mesh, multiple animations, and textures. By default, @b3d::Importer::Import only returns the primary resource (usually the mesh). To import all resources, use @b3d::Importer::ImportAll:

~~~~~~~~~~~~~{.cpp}
// Import all resources from an FBX file
TShared<MultiResource> multiResource = GetImporter().ImportAll("character.fbx");

// Access individual resources
for (const SubResource& subResource : multiResource->Entries)
{
	B3D_LOG(Info, LogGeneric, "Imported resource: {0}", subResource.Name);

	// Each sub-resource has a name and a handle
	HResource resource = subResource.Value;
}
~~~~~~~~~~~~~

# Supported formats
The importer automatically detects the file type based on extension and uses the appropriate importer. You can check if a file type is supported:

~~~~~~~~~~~~~{.cpp}
// Check if JPG files are supported
if (GetImporter().SupportsFileType("jpg"))
{
	// Import the file
}
~~~~~~~~~~~~~

Common supported formats include:
- **Images**: JPG, PNG, TGA, BMP, PSD, HDR
- **3D Models**: FBX, OBJ, DAE (Collada)
- **Audio**: WAV, OGG, FLAC, MP3
- **Fonts**: TTF, OTF

The exact list of supported formats depends on which importers are registered with the system.
