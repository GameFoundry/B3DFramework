Creating custom importers						{#customImporters}
===============
[TOC]

Importers process raw resources in a third-party format (like FBX mesh or a PNG image) into an engine-ready format (e.g. a **Mesh** or a **Texture**). bs::f has an extensible importer system so you may easily add your own importers, either for existing resource types or for new ones. This way you can add support for new third party file formats.

To implement your own importer you need to implement the @ref bs::SpecificImporter "SpecificImporter" interface.

# Implementing SpecificImporter {#customImporters_a}
Implementing this interface involves implementation of the following methods:
 * @ref bs::SpecificImporter::isExtensionSupported "SpecificImporter::isExtensionSupported()" - Receives a file extension and returns true or false depending if the importer can process that file. Used by the importer to find which importer plugin to use for import of a specific file.
 * @ref bs::SpecificImporter::isMagicNumberSupported "SpecificImporter::isMagicNumberSupported()" - Similar to the method above, but receives a magic number (first few bytes of a file) instead of the extension, as this is the more common way of identifying files on non-Windows systems.
 * @ref bs::SpecificImporter::import "SpecificImporter::import()" - Receives a path to a file, as well as a set of import options. This is the meat of the importer where you will read the file and convert it into engine ready format. When done the method returns a @ref bs::Resource "Resource" of a valid type, or null if it failed. The method should take into account the import options it was provided (if your importer supports any).
 
~~~~~~~~~~~~~{.cpp}
//	Simple importer for plain text types.
class PlainTextImporter : public SpecificImporter
{
public:
	bool isExtensionSupported(const String& ext) const override
	{
		String lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == "txt";
	}

	bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override
	{
		// Magic numbers don't make sense for plain text files, so we rely on extension checking
		return true;
	}

	SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override
	{
		SPtr<DataStream> stream = FileSystem::openFile(filePath);
		String textData = stream->getAsString();

		// ... initialize some resource with the text and return
	}
};
~~~~~~~~~~~~~ 
 
# Registering SpecificImporter {#customImporters_b}
To register your **SpecificImporter** implementation with the importer system you must call @ref bs::Importer::_registerAssetImporter "_registerAssetImporter()". You can do this after application start-up, or during by implementing your own **Application** class as described in the @ref nonComponentApproach manual.

~~~~~~~~~~~~~{.cpp}
Application::startUp(...);

PlainTextImporter* myImporter = bs_new<PlainTextImporter>();
gImporter()._registerAssetImporter(myImporter);
~~~~~~~~~~~~~ 

> Your importer must be allocated using a general purpose allocator (**bs_new**) because the importer system automatically frees it on shutdown, and it doesn't expect any special memory types.

Optionally you can do this on the higher level by providing a list of importers to @ref bs::Application::startUp "Application::startUp()" method. This method expects a list of dynamic library file-names, which means you must implement your importer as a plugin, as described in the @ref plugins manual.

# Optional features {#customImporters_c}
Your importer may also optionally implement any of the following features.

## Import options {#customImporters_c_a}
If you want to allow the user to control how is a file imported, you need to implement the @ref bs::ImportOptions "ImportOptions" class. The class has no special interface except the requirement to create a RTTI object for it.

~~~~~~~~~~~~~{.cpp}
// Import options for our PlainTextImporter. Contains a single option
// that allows the user to choose whether or not to convert all text
// to lowercase on import.
class PlainTextImportOptions : public ImportOptions
{
public:
	// Converts all text to lowercase on import
	bool convertToLowercase = false;
	
	friend class PlainTextImportOptionsRTTI;
	static RTTITypeBase* getRTTIStatic() { return PlainTextImportOptionsRTTI::instance(); }
	RTTITypeBase* getRTTI() const override { return getRTTIStatic(); }
};
~~~~~~~~~~~~~ 

The RTTI is implemented as normal, as described in the @ref serializingObjects manual.

You can then instantiate import options and provide them to the call of @ref bs::Importer::import "Importer::import()" and they will be passed through all the way to your **SpecificImporter** implementation. After that you can read the relevant options and perform the import accordingly.

~~~~~~~~~~~~~{.cpp}
class PlainTextImporter : public SpecificImporter
{
public:
	// ... other importer code

	SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override
	{
		const PlainTextImportOptions* myIO = static_cast<const PlainTextImportOptions*>(importOptions.get());
	
		if(myIO->convertToLowercase)
		{
			// Do something
		}
		else
		{
			// Do something else
		}
		
		// ... import and return a resource
	}
};
~~~~~~~~~~~~~ 

## Multiple resources {#customImporters_c_b}
If a single external file can be interpreted as multiple engine resources you should override the @ref bs::SpecificImporter::importAll "SpecificImporter::importAll()" method. For example this is the case with FBX format which can contain a mesh and one or multiple animations.

**SpecificImporter::importAll()** method will return a list of resources, each with a unique identifier which allows external code to know what those resources are. One of the resources must always be considered primary, and that's the resource that should be returned by **SpecificImporter::import()** (others should be ignored). The primary resource must have the "primary" identifier, while you are free to add custom identifiers for every other resource.
 
~~~~~~~~~~~~~{.cpp}
class PlainTextImporter : public SpecificImporter
{
public:
	// ... other importer code

	Vector<SubResourceRaw> importAll(const Path& filePath, SPtr<const ImportOptions> importOptions) override
	{
		Vector<SubResourceRaw> output;
	
		// ... read the file and generate resources
	
		output.push_back({ "primary", someResourceA });
		output.push_back({ "otherOne", someResourceB });
		output.push_back({ "anotherOne", someResourceC });
		return output;
	}
};
~~~~~~~~~~~~~ 

Then you can call @ref bs::Importer::importAll "Importer::importAll()" to import multiple resources from a file.