//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Error
	 *  @{
	 */

	/**
	 * Base class for all BSF exceptions.
	 *
	 * @note	BSF doesn't perform exception handling, but these classes remain here in case others wish to enable them.
	 */
	class Exception : public std::exception
	{
	public:
		Exception(const char* type, const String& description, const String& source)
			: mTypeName(type), mDescription(description), mSource(source)
		{}

		Exception(const char* type, const String& description, const String& source, const char* file, long line)
			: mLine(line), mTypeName(type), mDescription(description), mSource(source), mFile(file)
		{}

		Exception(const Exception& rhs)
			: mLine(rhs.mLine), mTypeName(rhs.mTypeName), mDescription(rhs.mDescription), mSource(rhs.mSource), mFile(rhs.mFile)
		{}

		~Exception() noexcept = default;

		void operator=(const Exception& rhs)
		{
			mDescription = rhs.mDescription;
			mSource = rhs.mSource;
			mFile = rhs.mFile;
			mLine = rhs.mLine;
			mTypeName = rhs.mTypeName;
		}

		/**
		 * Returns a string with the full description of the exception.
		 *
		 * @note
		 * The description contains the error number, the description supplied by the thrower, what routine threw the
		 * exception, and will also supply extra platform-specific information where applicable.
		 */
		virtual const String& GetFullDescription() const
		{
			if(mFullDesc.empty())
			{
				StringStream desc;

				desc << "B3D Framework EXCEPTION(" << mTypeName << "): "
					 << mDescription
					 << " in " << mSource;

				if(mLine > 0)
				{
					desc << " at " << mFile << " (line " << mLine << ")";
				}

				mFullDesc = desc.str();
			}

			return mFullDesc;
		}

		/** Gets the source function that threw the exception. */
		virtual const String& GetSource() const { return mSource; }

		/** Gets the source file name in which the exception was thrown. */
		virtual const String& GetFile() const { return mFile; }

		/** Gets line number on which the exception was thrown. */
		virtual long GetLine() const { return mLine; }

		/** Gets a short description about the exception. */
		virtual const String& GetDescription(void) const { return mDescription; }

		/** Overriden std::exception::what. Returns the same value as getFullDescription(). */
		const char* what() const noexcept override { return GetFullDescription().c_str(); }

	protected:
		long mLine = 0;
		String mTypeName;
		String mDescription;
		String mSource;
		String mFile;
		mutable String mFullDesc;
	};

	/** Exception for signaling not implemented parts of the code. */
	class NotImplementedException : public Exception
	{
	public:
		NotImplementedException(const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception("NotImplementedException", inDescription, inSource, inFile, inLine) {}
	};

	/** Exception for signaling file system errors when file could not be found. */
	class FileNotFoundException : public Exception
	{
	public:
		FileNotFoundException(const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception("FileNotFoundException", inDescription, inSource, inFile, inLine) {}
	};

	/** Exception for signaling general IO errors.
	 *
	 * @note	An example being failed to open a file or a network connection.
	 */
	class IOException : public Exception
	{
	public:
		IOException(const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception("IOException", inDescription, inSource, inFile, inLine) {}
	};

	/** Exception for signaling not currently executing code in not in a valid state. */
	class InvalidStateException : public Exception
	{
	public:
		InvalidStateException(const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception("InvalidStateException", inDescription, inSource, inFile, inLine) {}
	};

	/** Exception for signaling not some parameters you have provided are not valid. */
	class InvalidParametersException : public Exception
	{
	public:
		InvalidParametersException(const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception("InvalidParametersException", inDescription, inSource, inFile, inLine) {}
	};

	/**
	 * Exception for signaling an internal error, normally something that shouldn't have happened or wasn't anticipated by
	 * the programmers of that system.
	 */
	class InternalErrorException : public Exception
	{
	public:
		InternalErrorException(const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception("InternalErrorException", inDescription, inSource, inFile, inLine) {}
	};

	/** Exception for signaling an error in a rendering API. */
	class RenderingAPIException : public Exception
	{
	public:
		RenderingAPIException(const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception("RenderingAPIException", inDescription, inSource, inFile, inLine) {}
	};

	/** Exception for signaling an error in an unit test. */
	class UnitTestException : public Exception
	{
	public:
		UnitTestException(const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception("UnitTestException", inDescription, inSource, inFile, inLine) {}
	};

	/**
	 * Macro for throwing exceptions that will automatically fill out function name, file name and line number of the
	 * exception.
	 */
	// BSF doesn't actually use exceptions, so we just emulate the unhandled exception handler by crashing the application.
#ifndef B3D_EXCEPT
#	define B3D_EXCEPT(type, desc)                                                                                                  \
		{                                                                                                                          \
			static_assert((std::is_base_of<b3d::Exception, type>::value), "Invalid exception type (" #type ") for B3D_EXCEPT macro." \
																		 " It needs to derive from b3d::Exception.");               \
			GetCrashHandler().ReportCrash(#type, desc, __PRETTY_FUNCTION__, __FILE__, __LINE__);                                     \
			PlatformUtility::Terminate(true);                                                                                      \
		}
#endif

	/** @} */
} // namespace b3d
