//************************************ B3D Framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DCommandLine.h"
#include "String/B3DString.h"
#include "FileSystem/B3DPath.h"
#include "Debug/B3DDebug.h"

namespace b3d
{
	Path CommandLine::sExecutablePath;
	String CommandLine::sCommandLine;
	TArray<String> CommandLine::sArguments;
	UnorderedMap<String, String> CommandLine::sParameters;

	void CommandLine::Initialize(int argc, char* argv[])
	{
		if (argc > 0)
			sExecutablePath = argv[0];

		// Build command-line string from arguments (excluding executable)
		TArray<String> allTokens;
		StringStream commandLineBuilder;
		for (i32 argumentIndex = 1; argumentIndex < argc; argumentIndex++)
		{
			if (argumentIndex > 1)
				commandLineBuilder <<" ";

			const String& argument = argv[argumentIndex];

			// Quote arguments that contain spaces
			if (argument.find(' ') != String::npos)
			{
				commandLineBuilder << "\"";
				commandLineBuilder << argument;
				commandLineBuilder << "\"";

				allTokens.Add("\"" + argument + "\"");
			}
			else
			{
				commandLineBuilder << argument;

				allTokens.Add(argument);
			}
		}

		sCommandLine = commandLineBuilder.str();
		Parse(allTokens);
	}

	void CommandLine::Initialize(const String& commandLine)
	{
		TArray<String> allTokens;
		StringStream currentTokenBuilder;
		bool inQuote = false;
		bool escaped = false;

		for (u32 charIndex = 0; charIndex < commandLine.size(); charIndex++)
		{
			const char character = commandLine[charIndex];

			if (escaped)
			{
				currentTokenBuilder << character;
				escaped = false;
			}
			else if (character == '\\')
			{
				// Check if this is escaping a quote
				if (charIndex + 1 < commandLine.size() && (commandLine[charIndex + 1] == '"' || commandLine[charIndex + 1] == '\\'))
					escaped = true;
				else
					currentTokenBuilder << character;
			}
			else if (character == '"')
			{
				inQuote = !inQuote;
			}
			else if ((character == ' ' || character == '\t') && !inQuote)
			{
				const String currentToken = currentTokenBuilder.str();
				if (!currentToken.empty())
				{
					allTokens.Add(currentToken);

					currentTokenBuilder.str("");
					currentTokenBuilder.clear();
				}
			}
			else
			{
				currentTokenBuilder << character;
			}
		}

		const String currentToken = currentTokenBuilder.str();
		if (!currentToken.empty())
			allTokens.Add(currentToken);

		// First token is executable path
		if (!allTokens.Empty())
		{
			sExecutablePath = allTokens[0];

			// Build command-line from remaining tokens
			StringStream commandLineBuilder;
			for (u32 tokenIndex = 1; tokenIndex < allTokens.size(); tokenIndex++)
			{
				if (tokenIndex > 1)
					commandLineBuilder << " ";

				commandLineBuilder << allTokens[tokenIndex];
			}

			sCommandLine = commandLineBuilder.str();
		}
		else
		{
			sCommandLine = "";
		}

		Parse(allTokens);
	}

	const String& CommandLine::GetArgument(u32 index)
	{
		if (index < sArguments.size())
			return sArguments[index];

		return StringUtil::kBlank;
	}

	bool CommandLine::HasParameter(const String& name)
	{
		String normalizedParam = name;
		StringUtil::ToLowerCase(normalizedParam);

		return sParameters.find(normalizedParam) != sParameters.end();
	}

	String CommandLine::GetParameterValue(const String& name, const String& defaultValue)
	{
		String normalizedParam = name;
		StringUtil::ToLowerCase(normalizedParam);

		auto iterator = sParameters.find(normalizedParam);
		if (iterator != sParameters.end())
			return iterator->second;

		return defaultValue;
	}

	i32 CommandLine::GetParameterValueAsInt(const String& name, i32 defaultValue)
	{
		String value = GetParameterValue(name, "");
		if (value.empty())
			return defaultValue;

		return ParseI32(value, defaultValue);
	}

	float CommandLine::GetParameterValueAsFloat(const String& name, float defaultValue)
	{
		String value = GetParameterValue(name, "");
		if (value.empty())
			return defaultValue;

		return ParseFloat(value, defaultValue);
	}

	bool CommandLine::GetParameterValueAsBool(const String& name, bool defaultValue)
	{
		String value = GetParameterValue(name, "");

		// If parameter exists but has no value (flag), return true
		if (HasParameter(name) && value.empty())
			return true;

		if (value.empty())
			return defaultValue;

		// Convert to lowercase for case-insensitive comparison
		String valueLower = value;
		StringUtil::ToLowerCase(valueLower);

		if (valueLower == "true" || valueLower == "1" || valueLower == "yes" || valueLower == "on")
			return true;
		else if (valueLower == "false" || valueLower == "0" || valueLower == "no" || valueLower == "off")
			return false;

		return defaultValue;
	}

	void CommandLine::Parse(const TArray<String>& tokens)
	{
		sArguments.clear();
		sParameters.clear();

		if (sCommandLine.empty())
			return;

		// Store all tokens as arguments
		sArguments = tokens;

		// Process parameters
		for (u32 tokenIndex = 0; tokenIndex < tokens.size(); tokenIndex++)
		{
			const String& token = tokens[tokenIndex];

			if (token.empty())
				continue;

			// Check if this is a parameter (starts with -, --, or /)
			bool isParameter = false;
			String parameterName;
			String parameterValue;
			u32 prefixLength = 0;

			if (token.size() >= 2 && token[0] == '-' && token[1] == '-')
			{
				isParameter = true;
				prefixLength = 2;
			}
			else if (token.size() >= 1 && (token[0] == '-' || token[0] == '/'))
			{
				isParameter = true;
				prefixLength = 1;
			}

			if (isParameter)
			{
				// Extract parameter name and value
				String parameterPart = token.substr(prefixLength);

				// Check for = or : separator
				size_t separatorPos = parameterPart.find('=');
				if (separatorPos == String::npos)
					separatorPos = parameterPart.find(':');

				if (separatorPos != String::npos)
				{
					// Format: -param=value or /param:value
					parameterName = parameterPart.substr(0, separatorPos);
					parameterValue = parameterPart.substr(separatorPos + 1);
				}
				else
				{
					// Format: -param (flag) or -param value (check next token)
					parameterName = parameterPart;

					// Check if next token is a value (doesn't start with -, --, /)
					if (tokenIndex + 1 < tokens.size())
					{
						String nextToken = tokens[tokenIndex + 1];
						if (!nextToken.empty() && nextToken[0] != '-' && nextToken[0] != '/')
						{
							parameterValue = nextToken;
							tokenIndex++; // Skip next token as it's been consumed as value
						}
						else
						{
							parameterValue = ""; // Flag with no value
						}
					}
					else
					{
						parameterValue = ""; // Flag with no value
					}
				}

				// Normalize parameter name
				StringUtil::ToLowerCase(parameterName);
				sParameters[parameterName] = parameterValue;
			}
		}
	}
}
