//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTypeMetaData.h"
#include "B3DMonoManager.h"
#include "B3DMonoClass.h"
#include "B3DMonoField.h"

using namespace b3d;

ScriptTypeMetaData::ScriptTypeMetaData(const char* assembly, const char* nameSpace, const char* typeName, std::function<void()> setupScriptBindingsCallback)
	: SetupScriptBindingsCallback(std::move(setupScriptBindingsCallback))
{
	Identifier = MonoTypeIdentifier::Parse(typeName);

	if(assembly != nullptr)
		Identifier.Assembly = assembly;

	if(nameSpace != nullptr)
		Identifier.Namespace = nameSpace;
}

/**
 * Parses a type containing generic arguments, and returns the generic arguments.
 * e.g. for MyType<Arg1, Arg2<Arg3>> returns an array containing two entries [Arg1, Arg2<Arg3>].
 */
static TInlineArray<String, 2> ParseGenericArguments(const String& type)
{
	TInlineArray<String, 2> genericArguments;

	u32 level = 0;
	size_t start = 0;
	for(size_t i = 0; i < type.size(); ++i)
	{
		if(type[i] == '<')
			++level;
		else if(type[i] == '>')
			--level;
		else if(type[i] == ',' && level == 0)
		{
			String genericArgument = StringUtility::Trim(type.substr(start, i - start));
			genericArguments.Add(genericArgument);

			start = i + 1;
		}
	}

	if(start < type.size())
	{
		String genericArgument = StringUtility::Trim(type.substr(start));

		genericArguments.Add(genericArgument);
	}

	return genericArguments;
}

/** Parses a type name in format namespace::type, and outputs them separately. */
static void ParseNamespace(const String& type, String& outNamespace, String& outTypeName)
{
	size_t namespaceStartPosition = type.find(':');
	if(namespaceStartPosition == String::npos)
	{
		outTypeName = type;
		return;
	}

	if((namespaceStartPosition + 2) >= type.length() || type[namespaceStartPosition + 1] != ':')
	{
		outTypeName = type;
		return;
	}

	outNamespace = type.substr(0, namespaceStartPosition);
	outTypeName = type.substr(namespaceStartPosition + 2);
}

String MonoTypeIdentifier::GetTypeName(bool includeNamespace) const
{
	StringStream stream;

	auto fnAppendTypeName = [&stream, includeNamespace](const MonoTypeIdentifier& identifier, auto fnAppendTypeName) -> void
	{
		if(includeNamespace)
			stream << identifier.Namespace << "::";

		stream << identifier.TypeName;

		if(!identifier.GenericTypeParameters.Empty())
		{
			stream << "<";

			for(const auto& entry : identifier.GenericTypeParameters)
			{
				fnAppendTypeName(entry, fnAppendTypeName);
			}

			stream << ">";
		}
	};

	fnAppendTypeName(*this, fnAppendTypeName);
	return stream.str();
}

MonoTypeIdentifier MonoTypeIdentifier::Parse(const String& type)
{
	MonoTypeIdentifier result;

	size_t templateStartPosition = type.find('<');
	if(templateStartPosition != String::npos)
	{
		const String typeName = type.substr(0, templateStartPosition);
		const String unparsedArguments = type.substr(templateStartPosition + 1, type.size() - templateStartPosition - 2);

		TInlineArray<String, 2> parsedArguments = ParseGenericArguments(unparsedArguments);

		ParseNamespace(typeName, result.Namespace, result.TypeName);
		result.GenericTypeParameters.Reserve(parsedArguments.Size());

		for(const auto& genericType : parsedArguments)
			result.GenericTypeParameters.Add(Parse(genericType));

		return result;
	}

	ParseNamespace(type, result.Namespace, result.TypeName);
	return result;
}
