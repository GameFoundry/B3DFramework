//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
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
			FormatParamRange() = default;

			FormatParamRange(u32 start, u32 identifierSize, u32 parameterIndex)
				: Start(start), IdentifierSize(identifierSize), ParameterIndex(parameterIndex)
			{}

			u32 Start = 0;
			u32 IdentifierSize = 0;
			u32 ParameterIndex = 0;
		};

		/** Structure that holds value of a parameter during string formatting. */
		template <class T>
		struct ParamData
		{
			T* Buffer = nullptr;
			u32 Size = 0;
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
		template <class T, class... Args>
		static BasicString<T> Format(const T* source, Args&&... args)
		{
			u32 strLength = GetLength(source);

			ParamData<T> parameters[kMaxParams];
			memset(parameters, 0, sizeof(parameters));
			GetParams(parameters, 0U, std::forward<Args>(args)...);

			T bracketChars[kMaxIdentifierSize + 1];
			u32 bracketWriteIdx = 0;

			FormatParamRange paramRanges[kMaxParamReferences];
			memset(paramRanges, 0, sizeof(paramRanges));
			u32 paramRangeWriteIdx = 0;

			// Determine parameter positions
			i32 lastBracket = -1;
			bool escaped = false;
			u32 charWriteIdx = 0;
			for(u32 i = 0; i < strLength; i++)
			{
				if(source[i] == '\\' && !escaped && paramRangeWriteIdx < kMaxParamReferences)
				{
					escaped = true;
					paramRanges[paramRangeWriteIdx++] = FormatParamRange(charWriteIdx, 1, (u32)-1);
					continue;
				}

				if(lastBracket == -1)
				{
					// If current char is non-escaped opening bracket start parameter definition
					if(source[i] == '{' && !escaped)
						lastBracket = i;
					else
						charWriteIdx++;
				}
				else
				{
					if(isdigit(source[i]) && bracketWriteIdx < kMaxIdentifierSize)
						bracketChars[bracketWriteIdx++] = source[i];
					else
					{
						// If current char is non-escaped closing bracket end parameter definition
						u32 numParamChars = bracketWriteIdx;
						bool processedBracket = false;
						if(source[i] == '}' && numParamChars > 0 && !escaped)
						{
							bracketChars[bracketWriteIdx] = '\0';
							u32 parameterIndex = StrToInt(bracketChars);
							if(parameterIndex < kMaxParams && paramRangeWriteIdx < kMaxParamReferences) // Check if exceeded maximum parameter limit
							{
								paramRanges[paramRangeWriteIdx++] = FormatParamRange(charWriteIdx, numParamChars + 2, parameterIndex);
								charWriteIdx += parameters[parameterIndex].Size;

								processedBracket = true;
							}
						}

						if(!processedBracket)
						{
							// Last bracket wasn't really a parameter
							for(u32 j = lastBracket; j <= i; j++)
								charWriteIdx++;
						}

						lastBracket = -1;
						bracketWriteIdx = 0;
					}
				}

				escaped = false;
			}

			// Copy the clean string into output buffer
			u32 finalStringSize = charWriteIdx;

			T* outputBuffer = (T*)B3DAllocate(finalStringSize * sizeof(T));
			u32 copySourceIdx = 0;
			u32 copyDestIdx = 0;
			for(u32 i = 0; i < paramRangeWriteIdx; i++)
			{
				const FormatParamRange& rangeInfo = paramRanges[i];
				u32 copySize = rangeInfo.Start - copyDestIdx;

				memcpy(outputBuffer + copyDestIdx, source + copySourceIdx, copySize * sizeof(T));
				copySourceIdx += copySize + rangeInfo.IdentifierSize;
				copyDestIdx += copySize;

				if(rangeInfo.ParameterIndex == (u32)-1)
					continue;

				u32 parameterSize = parameters[rangeInfo.ParameterIndex].Size;
				memcpy(outputBuffer + copyDestIdx, parameters[rangeInfo.ParameterIndex].Buffer, parameterSize * sizeof(T));
				copyDestIdx += parameterSize;
			}

			memcpy(outputBuffer + copyDestIdx, source + copySourceIdx, (finalStringSize - copyDestIdx) * sizeof(T));

			BasicString<T> outputStr(outputBuffer, finalStringSize);
			B3DFree(outputBuffer);

			for(u32 i = 0; i < kMaxParams; i++)
			{
				if(parameters[i].Buffer != nullptr)
					B3DFree(parameters[i].Buffer);
			}

			return outputStr;
		}

	private:
		/**
		 * Set of methods that can be specialized so we have a generalized way for retrieving length of strings of
		 * different types.
		 */
		static u32 GetLength(const char* source) { return (u32)strlen(source); }

		/**
		 * Set of methods that can be specialized so we have a generalized way for retrieving length of strings of
		 * different types.
		 */
		static u32 GetLength(const wchar_t* source) { return (u32)wcslen(source); }

		/** Parses the string and returns an integer value extracted from string characters. */
		static u32 StrToInt(const char* buffer)
		{
			return (u32)strtoul(buffer, nullptr, 10);
		}

		/** Parses the string and returns an integer value extracted from string characters. */
		static u32 StrToInt(const wchar_t* buffer)
		{
			return (u32)wcstoul(buffer, nullptr, 10);
		}

		/**	Helper method for converting any data type to a narrow string. */
		template <class T>
		static String ToString(const T& param)
		{
			return b3d::ToString(param);
		}

		/**	Helper method that "converts" a narrow string to a narrow string (simply a pass through). */
		static String ToString(const String& param) { return param; }

		/**	Helper method that converts a narrow character array to a narrow string. */
		template <class T>
		static String ToString(T* param)
		{
			static_assert(!std::is_same<T, T>::value, "Invalid pointer type.");
			return "";
		}

		/**	Helper method that converts a narrow character array to a narrow string. */
		static String ToString(const char* param)
		{
			if(param == nullptr)
				return String();

			return String(param);
		}

		/**	Helper method that converts a narrow character array to a narrow string. */
		static String ToString(char* param)
		{
			if(param == nullptr)
				return String();

			return String(param);
		}

		/**	Helper method that converts a StringView to a narrow string. */
		static String ToString(StringView param)
		{
			return String(param.data(), param.size());
		}

		/**	Helper method for converting any data type to a wide string. */
		template <class T>
		static WString ToWideString(const T& param)
		{
			return b3d::ToWideString(param);
		}

		/**	Helper method that "converts" a wide string to a wide string (simply a pass through). */
		static WString ToWideString(const WString& param) { return param; }

		/**	Helper method that converts a wide character array to a wide string. */
		template <class T>
		static WString ToWideString(T* param)
		{
			static_assert(!std::is_same<T, T>::value, "Invalid pointer type.");
			return L"";
		}

		/**	Helper method that converts a wide character array to a wide string. */
		static WString ToWideString(const wchar_t* param)
		{
			if(param == nullptr)
				return WString();

			return WString(param);
		}

		/**	Helper method that converts a wide character array to a wide string. */
		static WString ToWideString(wchar_t* param)
		{
			if(param == nullptr)
				return WString();

			return WString(param);
		}

		/**
		 * Converts all the provided parameters into string representations and populates the provided @p parameters array.
		 */
		template <class P, class... Args>
		static void GetParams(ParamData<char>* parameters, u32 idx, P&& param, Args&&... args)
		{
			if(idx >= kMaxParams)
				return;

			BasicString<char> sourceParam = ToString(param);
			parameters[idx].Buffer = (char*)B3DAllocate((u32)sourceParam.size() * sizeof(char));
			parameters[idx].Size = (u32)sourceParam.size();

			sourceParam.copy(parameters[idx].Buffer, parameters[idx].Size, 0);

			GetParams(parameters, idx + 1, std::forward<Args>(args)...);
		}

		/**
		 * Converts all the provided parameters into string representations and populates the provided @p parameters array.
		 */
		template <class P, class... Args>
		static void GetParams(ParamData<wchar_t>* parameters, u32 idx, P&& param, Args&&... args)
		{
			if(idx >= kMaxParams)
				return;

			BasicString<wchar_t> sourceParam = ToWideString(param);
			parameters[idx].Buffer = (wchar_t*)B3DAllocate((u32)sourceParam.size() * sizeof(wchar_t));
			parameters[idx].Size = (u32)sourceParam.size();

			sourceParam.copy(parameters[idx].Buffer, parameters[idx].Size, 0);

			GetParams(parameters, idx + 1, std::forward<Args>(args)...);
		}

		/** Helper method for parameter size calculation. Used as a stopping point in template recursion. */
		static void GetParams(ParamData<char>* parameters, u32 idx)
		{
			// Do nothing
		}

		/**	Helper method for parameter size calculation. Used as a stopping point in template recursion. */
		static void GetParams(ParamData<wchar_t>* parameters, u32 idx)
		{
			// Do nothing
		}

		static constexpr const u32 kMaxParams = 20;
		static constexpr const u32 kMaxIdentifierSize = 2;
		static constexpr const u32 kMaxParamReferences = 200;
	};

	/** @} */
	/** @} */
} // namespace b3d
