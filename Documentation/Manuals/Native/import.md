Creating custom importers						{#customImporters}
===============
[TOC]

Importers process a raw resource in a third-party format (like FBX mesh or a PNG image) into an engine-ready format (e.g. a @ref bs::Mesh "Mesh" or a @ref bs::Texture "Texture"). Banshee has an extensible importer system so you may easily add your own, either for existing resource types, or for new ones. If you are also interested in creating new resource types check out [this manual](@ref resources).

On the user-facing level resource import works through the @ref bs::Importer "Importer" module. You call its @ref bs::Importer::import "Importer::import" method which takes a path to the resource you want to import, and the system automatically finds the necessary importer plugin (if one is available) and returns the resource in an engine ready format. Optionally you can also supply the importer with @ref bs::ImportOptions "ImportOptions" to control resource import in more detail.

To implement your own importer you need to implement the @ref bs::SpecificImporter "SpecificImporter" interface.

# Implementing SpecificImporter # {#customImporters_a}
Implementing this interface involves implementation of the following methods:
 * @ref bs::SpecificImporter::isExtensionSupported "isExtensionSupported" - Receives an extension and returns true or false depending if the importer can process that file. Used by the importer to find which importer plugin to use for import of a specific file.
 * @ref bs::SpecificImporter::isMagicNumberSupported "isMagicNumberSupported" - Similar to the method above, but receives a magic number (first few bytes of a file) instead of the extension, as this is the more common way of identifying files on non-Windows systems.
 * @ref bs::SpecificImporter::import "import" - Receives a path to a file, as well as a set of import options. This is the meat of the importer where you will read the file and convert it into engine ready format. When done the method returns a @ref bs::Resource "Resource" of a valid type, or null if it failed. The method should take into account the import options it was provided (if your importer supports any).
 
You may also optionally implement the following methods:
 * @ref bs::SpecificImporter::createImportOptions "createImportOptions" - If your importer supports specific import options this should return a brand new empty instance of such import options. These will eventually be provided to the @ref bs::SpecificImporter::import "import" method. Import options must implement @ref bs::ImportOptions "ImportOptions" and you may include any necessary fields in your implementation. You must also [implement RTTI](@ref rtti) for your custom import options. You should strive to use same type of import options for same resource types (for example all textures use @ref bs::TextureImportOptions "TextureImportOptions").
 * @ref bs::SpecificImporter::importAll "importAll" - Sometimes a single third-party resource can contain multiple engine resources (for a example an .fbx may contain both a mesh and animation). In such cases you will want to implement this method, which allows you to return multiple resources, each with a unique identifier. One of the resources must always be considered primary, and that's the resource that should be returned by @ref bs::SpecificImporter::import "import" (others should be ignored). In this method the primary resource must have the "primary" identifier, while you are free to add custom identifiers for every other resource.
 
Once your importer is implemented you must register it with the global importer system. You may do this by calling @ref bs::Importer::_registerAssetImporter "_registerAssetImporter" with an instance of your importer. It should be allocated using the general allocator and will be freed automatically on system shutdown.

Optionally you can do this on the higher level by providing a list of importers to @ref bs::Application::startUp "Application::startUp" method. This methods expects a list of dynamic library file-names, which means you must implement your importer as a [plugin](@ref customPlugins).

See @ref bs::PlainTextImporter "PlainTextImporter" for an implementation of a very simple importer that just imports raw textual files.