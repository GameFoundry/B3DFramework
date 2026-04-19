//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "FileSystem/B3DPath.h"

namespace b3d
{
	/** @addtogroup System
	 *  @{
	 */

	/**
	 * Provides access to command-line arguments passed to the application. Supports parsing parameters in various
	 * formats: -param, --param, /param, -param=value, --param=value, /param:value.
	 */
	class B3D_EXPORT CommandLine
	{
	public:
		/**
		 * Initialize the command-line from argc/argv (Unix-style).
		 *
		 * @param	argc	Argument count.
		 * @param	argv	Argument values.
		 */
		static void Initialize(int argc, char* argv[]);

		/**
		 * Initialize the command-line from a single string (Windows-style).
		 *
		 * @param	commandLine	The full command-line string including executable path.
		 */
		static void Initialize(const String& commandLine);

		/** Gets the full command-line string (excluding executable path). */
		static const String& Get() { return sCommandLine; }

		/**
		 * Get the executable path from the command-line.
		 *
		 * @return	The full path to the executable (e.g., "C:\Program Files\MyApp\MyApp.exe").
		 */
		static const Path& GetExecutablePath() { return sExecutablePath; }

		/**
		 * Get the number of arguments (excluding executable).
		 *
		 * @return	The argument count.
		 */
		static u32 GetArgumentCount() { return (u32)sArguments.size(); }

		/**
		 * Get argument at specific index (0-based, excluding executable).
		 *
		 * @param	index	The argument index (0-based).
		 * @return			The argument string, or empty string if index is out of bounds.
		 */
		static const String& GetArgument(u32 index);

		/**
		 * Check if a parameter exists on the command-line.
		 * Supports formats: -param, --param, /param (case-insensitive).
		 *
		 * @param	name	The parameter name.
		 * @return			True if the parameter exists, false otherwise.
		 */
		static bool HasParameter(const String& name);

		/**
		 * Get parameter value as string.
		 * Supports formats: -param=value, --param=value, /param:value, -param value.
		 *
		 * @param	name			The parameter name.
		 * @param	defaultValue	The default value if parameter doesn't exist.
		 * @return					The parameter value, or default value if not found.
		 */
		static String GetParameterValue(const String& name, const String& defaultValue = StringUtility::kBlank);

		/**
		 * Get parameter value as integer.
		 *
		 * @param	name			The parameter name.
		 * @param	defaultValue	The default value if parameter doesn't exist or isn't a valid integer.
		 * @return					The parameter value as integer.
		 */
		static i32 GetParameterValueAsInt(const String& name, i32 defaultValue = 0);

		/**
		 * Get parameter value as float.
		 *
		 * @param	name			The parameter name.
		 * @param	defaultValue	The default value if parameter doesn't exist or isn't a valid float.
		 * @return					The parameter value as float.
		 */
		static float GetParameterValueAsFloat(const String& name, float defaultValue = 0.0f);

		/**
		 * Get parameter value as boolean.
		 * Accepts: true/false, 1/0, yes/no, on/off (case-insensitive).
		 * Switches without values return true.
		 *
		 * @param	name			The parameter name.
		 * @param	defaultValue	The default value if parameter doesn't exist.
		 * @return					The parameter value as boolean.
		 */
		static bool GetParameterValueAsBool(const String& name, bool defaultValue = false);

		/**
		 * Get all command-line parameters as a map.
		 * Keys are normalized to lowercase.
		 *
		 * @return	Const reference to the parameters map.
		 */
		static const UnorderedMap<String, String>& GetAllParameters() { return sParameters; }

	private:
		/** Parse the command-line with proper quote and escape character handling. Populates sArguments and sParameters. */
		static void Parse(const TArray<String>& tokens);

		static Path sExecutablePath;
		static String sCommandLine;
		static TArray<String> sArguments;
		static UnorderedMap<String, String> sParameters;
	};

	/** @} */
}
