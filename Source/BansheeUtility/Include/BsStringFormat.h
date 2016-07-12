//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
namespace BansheeEngine
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup String-Internal
	 *  @{
	 */

	/** Helper class used for string formatting operations. */
	class StringFormat
	{
	private:
		/**
		 * Data structure used during string formatting. It holds information about parameter identifiers to replace with
		 * actual parameters.
		 */
		struct FormatParamRange
		{
			FormatParamRange() { }
			FormatParamRange(UINT32 start, UINT32 identifierSize, UINT32 paramIdx)
				:start(start), identifierSize(identifierSize), paramIdx(paramIdx)
			{ }

			UINT32 start;
			UINT32 identifierSize;
			UINT32 paramIdx;
		};

		/** Structure that holds value of a parameter during string formatting. */
		template<class T>
		struct ParamData
		{
			T* buffer;
			UINT32 size;
		};

	public:
		/**
		 * Formats the provided string by replacing the identifiers with the provided parameters. The identifiers are 
		 * represented like "{0}, {1}" in the source string, where the number represents the position of the parameter 
		 * that will be used for replacing the identifier.
		 *			
		 * @note	
		 * You may use "\" to escape identifier brackets.
		 * @note
		 * Maximum identifier number is 19 (for a total of 20 unique identifiers. for example {20} won't be recognized as
		 * an identifier).
		 * @note	
		 * Total number of parameters that can be referenced is 200.
		 */
		template<class T, class... Args>
		static BasicString<T> format(const T* source, Args&& ...args)
		{
			UINT32 strLength = getLength(source);

			ParamData<T> parameters[MAX_PARAMS];
			memset(parameters, 0, sizeof(parameters));
			getParams(parameters, 0U, std::forward<Args>(args)...);

			T bracketChars[MAX_IDENTIFIER_SIZE + 1];
			UINT32 bracketWriteIdx = 0;

			FormatParamRange paramRanges[MAX_PARAM_REFERENCES];
			memset(paramRanges, 0, sizeof(paramRanges));
			UINT32 paramRangeWriteIdx = 0;

			// Determine parameter positions
			INT32 lastBracket = -1;
			bool escaped = false;
			UINT32 charWriteIdx = 0;
			for (UINT32 i = 0; i < strLength; i++)
			{
				if (source[i] == '\\' && !escaped && paramRangeWriteIdx < MAX_PARAM_REFERENCES)
				{
					escaped = true;
					paramRanges[paramRangeWriteIdx++] = FormatParamRange(charWriteIdx, 1, (UINT32)-1);
					continue;
				}

				if (lastBracket == -1)
				{
					// If current char is non-escaped opening bracket start parameter definition
					if (source[i] == '{' && !escaped)
						lastBracket = i;
					else
						charWriteIdx++;
				}
				else
				{
					if (isdigit(source[i]) && bracketWriteIdx < MAX_IDENTIFIER_SIZE)
						bracketChars[bracketWriteIdx++] = source[i];
					else
					{
						// If current char is non-escaped closing bracket end parameter definition
						UINT32 numParamChars = bracketWriteIdx;
						bool processedBracket = false;
						if (source[i] == '}' && numParamChars > 0 && !escaped)
						{
							bracketChars[bracketWriteIdx] = '\0';
							UINT32 paramIdx = strToInt(bracketChars);
							if (paramIdx < MAX_PARAMS && paramRangeWriteIdx < MAX_PARAM_REFERENCES) // Check if exceeded maximum parameter limit
							{
								paramRanges[paramRangeWriteIdx++] = FormatParamRange(charWriteIdx, numParamChars + 2, paramIdx);
								charWriteIdx += parameters[paramIdx].size;

								processedBracket = true;
							}
						}

						if (!processedBracket)
						{
							// Last bracket wasn't really a parameter
							for (UINT32 j = lastBracket; j <= i; j++)
								charWriteIdx++;
						}

						lastBracket = -1;
						bracketWriteIdx = 0;
					}
				}

				escaped = false;
			}

			// Copy the clean string into output buffer
			UINT32 finalStringSize = charWriteIdx;

			T* outputBuffer = (T*)bs_alloc(finalStringSize * sizeof(T));
			UINT32 copySourceIdx = 0;
			UINT32 copyDestIdx = 0;
			for (UINT32 i = 0; i < paramRangeWriteIdx; i++)
			{
				const FormatParamRange& rangeInfo = paramRanges[i];
				UINT32 copySize = rangeInfo.start - copyDestIdx;
				
				memcpy(outputBuffer + copyDestIdx, source + copySourceIdx, copySize * sizeof(T));
				copySourceIdx += copySize + rangeInfo.identifierSize;
				copyDestIdx += copySize;

				if (rangeInfo.paramIdx == (UINT32)-1)
					continue;

				UINT32 paramSize = parameters[rangeInfo.paramIdx].size;
				memcpy(outputBuffer + copyDestIdx, parameters[rangeInfo.paramIdx].buffer, paramSize * sizeof(T));
				copyDestIdx += paramSize;
			}

			memcpy(outputBuffer + copyDestIdx, source + copySourceIdx, (finalStringSize - copyDestIdx) * sizeof(T));

			BasicString<T> outputStr(outputBuffer, finalStringSize);
			bs_free(outputBuffer);

			for (UINT32 i = 0; i < MAX_PARAMS; i++)
			{
				if (parameters[i].buffer != nullptr)
					bs_free(parameters[i].buffer);
			}

			return outputStr;
		}

	private:
		/**
		 * Set of methods that can be specialized so we have a generalized way for retrieving length of strings of 
		 * different types.
		 */
		static UINT32 getLength(const char* source) { return (UINT32)strlen(source); }

		/**
		 * Set of methods that can be specialized so we have a generalized way for retrieving length of strings of 
		 * different types.
		 */
		static UINT32 getLength(const wchar_t* source) { return (UINT32)wcslen(source); }

		/** Parses the string and returns an integer value extracted from string characters. */
		static UINT32 strToInt(const char* buffer)
		{
			return (UINT32)strtoul(buffer, nullptr, 10);
		}

		/** Parses the string and returns an integer value extracted from string characters. */
		static UINT32 strToInt(const wchar_t* buffer)
		{
			return (UINT32)wcstoul(buffer, nullptr, 10);
		}

		/**	Helper method for converting any data type to a narrow string. */
		template<class T> static std::string toString(const T& param) { return std::to_string(param); }

		/**	Helper method that "converts" a narrow string to a narrow string (simply a pass through). */
		static std::string toString(const std::string& param) { return param; }

		/**	Helper method that converts a Banshee narrow string to a standard narrow string. */
		static std::string toString(const String& param)
		{
			return std::string(param.c_str());
		}

		/**	Helper method that converts a narrow character array to a narrow string. */
		template<class T> static std::string toString(T* param) { static_assert("Invalid pointer type."); }

		/**	Helper method that converts a narrow character array to a narrow string. */
		static std::string toString(const char* param)
		{
			if (param == nullptr)
				return std::string();

			return std::string(param);
		}

		/**	Helper method that converts a narrow character array to a narrow string. */
		static std::string toString(char* param)
		{
			if (param == nullptr)
				return std::string();

			return std::string(param);
		}

		/**	Helper method for converting any data type to a wide string. */
		template<class T> static std::wstring toWString(const T& param) { return std::to_wstring(param); }

		/**	Helper method that "converts" a wide string to a wide string (simply a pass through). */
		static std::wstring toWString(const std::wstring& param) { return param; }

		/**	Helper method that converts a Banshee wide string to a standard wide string. */
		static std::wstring toWString(const WString& param)
		{
			return std::wstring(param.c_str());
		}

		/**	Helper method that converts a wide character array to a wide string. */
		template<class T> static std::wstring toWString(T* param) { static_assert("Invalid pointer type."); }

		/**	Helper method that converts a wide character array to a wide string. */
		static std::wstring toWString(const wchar_t* param)
		{
			if (param == nullptr)
				return std::wstring();

			return std::wstring(param);
		}

		/**	Helper method that converts a wide character array to a wide string. */
		static std::wstring toWString(wchar_t* param)
		{
			if (param == nullptr)
				return std::wstring();

			return std::wstring(param);
		}

		/**
		 * Converts all the provided parameters into string representations and populates the provided @p parameters array.
		 */
		template<class P, class... Args>
		static void getParams(ParamData<char>* parameters, UINT32 idx, P&& param, Args&& ...args)
		{
			if (idx >= MAX_PARAMS)
				return;

			std::basic_string<char> sourceParam = toString(param);
			parameters[idx].buffer = (char*)bs_alloc((UINT32)sourceParam.size() * sizeof(char));
			parameters[idx].size = (UINT32)sourceParam.size();

			sourceParam.copy(parameters[idx].buffer, parameters[idx].size, 0);
			
			getParams(parameters, idx + 1, std::forward<Args>(args)...);
		}

		/**
		 * Converts all the provided parameters into string representations and populates the provided @p parameters array.
		 */
		template<class P, class... Args>
		static void getParams(ParamData<wchar_t>* parameters, UINT32 idx, P&& param, Args&& ...args)
		{
			if (idx >= MAX_PARAMS)
				return;

			std::basic_string<wchar_t> sourceParam = toWString(param);
			parameters[idx].buffer = (wchar_t*)bs_alloc((UINT32)sourceParam.size() * sizeof(wchar_t));
			parameters[idx].size = (UINT32)sourceParam.size();
			
			sourceParam.copy(parameters[idx].buffer, parameters[idx].size, 0);

			getParams(parameters, idx + 1, std::forward<Args>(args)...);
		}

		/** Helper method for parameter size calculation. Used as a stopping point in template recursion. */
		static void getParams(ParamData<char>* parameters, UINT32 idx)
		{
			// Do nothing
		}

		/**	Helper method for parameter size calculation. Used as a stopping point in template recursion. */
		static void getParams(ParamData<wchar_t>* parameters, UINT32 idx)
		{
			// Do nothing
		}

		static const UINT32 MAX_PARAMS = 20;
		static const UINT32 MAX_IDENTIFIER_SIZE = 2;
		static const UINT32 MAX_PARAM_REFERENCES = 200;
	};

	/** @} */
	/** @} */
}