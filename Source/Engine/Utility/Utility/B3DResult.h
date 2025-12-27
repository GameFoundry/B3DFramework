//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/** Common result codes for TResult/Result types. */
	enum class B3D_SCRIPT_EXPORT() ResultStatus
	{
		Succeeded,
		Failed,
		FailedAlreadyExists,
		FailedInvalidInput,
		FailedInternalError,
		FailedNotFound,
		FailedReadOnly,
	};

	struct Result;

	/**
	 * Helper type to be used as a return value from methods/functions. Contains success/failure state, along with an optional error message in the failure case,
	 * or an output object in case of success.
	 */
	template<typename T>
	struct TResult
	{
		TResult(const Result& other);
		TResult(Result&& other);
		template<class OtherType> TResult(const TResult<OtherType>& other);
		template<class OtherType> TResult(TResult<OtherType>&& other);

		/** Returns true if the result is one of the success states. */
		bool IsSuccessful() const;

		/** Returns error message and additional error message as a combined string. */
		String GetFullErrorMessage() const;

		/**
		 * Creates the success result object.
		 *
		 * @param	output		Object to return from the function/method.
		 * @param	status		One of the successful result status code.
		 * @return				Newly created TResult object.
		 */
		static TResult Success(const T& output, ResultStatus status = ResultStatus::Succeeded);

		/**
		 * Creates the success result object.
		 *
		 * @param	output		Object to return from the function/method.
		 * @param	status		One of the successful result status code.
		 * @return				Newly created TResult object.
		 */
		static TResult Success(T&& output, ResultStatus status = ResultStatus::Succeeded);

		/**
		 * Creates the fail result object.
		 *
		 * @param	errorMessage			Error message describing the failure.
		 * @param	status					One of the failure result status code.
		 * @param	additionalErrorMessage	Additional error message for information that cannot easily be stored in @p errorMessage.
		 * @return							Newly created TResult object.
		 */
		static TResult Fail(const char* errorMessage, ResultStatus status = ResultStatus::Failed, const String& additionalErrorMessage = StringUtil::kBlank);

		/**
		 * Creates the fail result object by inheriting the status from another result object. Other result object error message is appended
		 * in the additional error message.
		 *
		 * @param	errorMessage			Error message describing the failure.
		 * @param	childResult				Result from which to inherit the failure status and append the error message.
		 * @param	additionalErrorMessage	Additional error message for information that cannot easily be stored in @p errorMessage. The other result's
		 *									error message will be appended to this error message.
		 * @return							Newly created TResult object.
		 */
		template<typename OtherType>
		static TResult Fail(const char* errorMessage, TResult<OtherType>&& childResult, const String& additionalErrorMessage = StringUtil::kBlank);

		/**
		 * Creates the fail result object by inheriting the status from another result object. Other result object error message is appended
		 * in the additional error message.
		 *
		 * @param	errorMessage			Error message describing the failure.
		 * @param	childResult				Result from which to inherit the failure status and append the error message.
		 * @param	additionalErrorMessage	Additional error message for information that cannot easily be stored in @p errorMessage. The other result's
		 *									error message will be appended to this error message.
		 * @return							Newly created TResult object.
		 */
		static TResult Fail(const char* errorMessage, Result&& childResult, const String& additionalErrorMessage = StringUtil::kBlank);

		ResultStatus Status = ResultStatus::Failed;
		const char* ErrorMessage = nullptr;
		String AdditionalErrorMessage;
		T Output;

	private:
		B3D_SCRIPT_EXPORT(Exclude(true))
		TResult(ResultStatus status, const char* errorMessage, String additionalErrorMessage = StringUtil::kBlank);

		B3D_SCRIPT_EXPORT(Exclude(true))
		TResult(ResultStatus status, const T& output);

		B3D_SCRIPT_EXPORT(Exclude(true))
		TResult(ResultStatus status, T&& output);
	};

	/** Same as TResult, but with no output object. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true)) Result
	{
		Result() = default;

		/** Converts TResult to Result (discards the output field). */
		template<typename T> Result(const TResult<T>& other);

		/** Converts TResult to Result (discards the output field). */
		template<typename T> Result(TResult<T>&& other);

		/** Returns true if the result is one of the success states. */
		bool IsSuccessful() const;

		/** Returns error message and additional error message as a combined string. */
		String GetFullErrorMessage() const { return CombineErrorMessage(ErrorMessage, AdditionalErrorMessage);}

		/**
		 * Creates the success result object.
		 *
		 * @param	status		One of the successful result status code.
		 * @return				Newly created TResult object.
		 */
		static Result Success(ResultStatus status = ResultStatus::Succeeded);

		/**
		 * Creates the fail result object.
		 *
		 * @param	errorMessage			Error message describing the failure.
		 * @param	status					One of the failure result status code.
		 * @param	additionalErrorMessage	Additional error message for information that cannot easily be stored in @p errorMessage.
		 * @return							Newly created TResult object.
		 */
		static Result Fail(const char* errorMessage, ResultStatus status = ResultStatus::Failed, String additionalErrorMessage = StringUtil::kBlank);

		/**
		 * Creates the fail result object by inheriting the status from another result object. Other result object error message is appended
		 * in the additional error message.
		 *
		 * @param	errorMessage			Error message describing the failure.
		 * @param	childResult				Result from which to inherit the failure status and append the error message.
		 * @param	additionalErrorMessage	Additional error message for information that cannot easily be stored in @p errorMessage. The other result's
		 *									error message will be appended to this error message.
		 * @return							Newly created TResult object.
		 */
		template<typename OtherType>
		static Result Fail(const char* errorMessage, TResult<OtherType>&& childResult, const String& additionalErrorMessage = StringUtil::kBlank);

		/**
		 * Creates the fail result object by inheriting the status from another result object. Other result object error message is appended
		 * in the additional error message.
		 *
		 * @param	errorMessage			Error message describing the failure.
		 * @param	childResult				Result from which to inherit the failure status and append the error message.
		 * @param	additionalErrorMessage	Additional error message for information that cannot easily be stored in @p errorMessage. The other result's
		 *									error message will be appended to this error message.
		 * @return							Newly created TResult object.
		 */
		static Result Fail(const char* errorMessage, Result&& childResult, const String& additionalErrorMessage = StringUtil::kBlank);

		ResultStatus Status = ResultStatus::Failed;
		const char* ErrorMessage = nullptr;
		String AdditionalErrorMessage;

	private:
		template <typename T> friend struct TResult;

		B3D_SCRIPT_EXPORT(Exclude(true))
		Result(ResultStatus status, const char* errorMessage, String additionalErrorMessage = StringUtil::kBlank);

		B3D_SCRIPT_EXPORT(Exclude(true))
		Result(ResultStatus status);

		/**
		 * Combines error messages from two result objects into a string that can be used as an additional string object for the new result object.
		 *
		 * @param	additionalErrorMessage		Additional error message for the primary result object.
		 * @param	childErrorMessage			Error message from the child result object. Can be null.
		 * @param	childAdditionalErrorMessage Additional error message from the child result object.
		 * @return								Combined string with all three error messages.
		 */
		static String CombineAdditionalErrorMessage(const String& additionalErrorMessage, const char* childErrorMessage, const String& childAdditionalErrorMessage);

		/** Combines the error message and additional error message into a single string. */
		static String CombineErrorMessage(const char* errorMessage, const String& additionalErrorMessage);
	};

	template<typename T>
	TResult<T>::TResult(ResultStatus status, const char* errorMessage, String additionalErrorMessage)
		: Status(status), ErrorMessage(errorMessage), AdditionalErrorMessage(std::move(additionalErrorMessage))
	{ }

	template<typename T>
	TResult<T>::TResult(ResultStatus status, const T& output)
		: Status(status), Output(output)
	{ }

	template<typename T>
	TResult<T>::TResult(ResultStatus status, T&& output)
		: Status(status), Output(std::move(output))
	{ }

	template<typename T>
	TResult<T>::TResult(const Result& other)
		: Status(other.Status), ErrorMessage(other.ErrorMessage), AdditionalErrorMessage(other.AdditionalErrorMessage)
	{ }

	template<typename T>
	TResult<T>::TResult(Result&& other)
		: Status(other.Status), ErrorMessage(other.ErrorMessage), AdditionalErrorMessage(std::move(other.AdditionalErrorMessage))
	{ }

	template<typename T>
	template<typename OtherType>
	TResult<T>::TResult(const TResult<OtherType>& other)
		: Status(other.Status), ErrorMessage(other.ErrorMessage), AdditionalErrorMessage(other.AdditionalErrorMessage)
	{ }

	template<typename T>
	template<typename OtherType>
	TResult<T>::TResult(TResult<OtherType>&& other)
		: Status(other.Status), ErrorMessage(other.ErrorMessage), AdditionalErrorMessage(std::move(other.AdditionalErrorMessage))
	{ }

	template<typename T>
	bool TResult<T>::IsSuccessful() const { return Status == ResultStatus::Succeeded; }

	template <typename T>
	String TResult<T>::GetFullErrorMessage() const
	{
		return Result::CombineErrorMessage(ErrorMessage, AdditionalErrorMessage);
	}

	template<typename T>
	TResult<T> TResult<T>::Success(const T& output, ResultStatus status)
	{
		return TResult(status, output);
	}

	template<typename T>
	TResult<T> TResult<T>::Success(T&& output, ResultStatus status)
	{
		return TResult(status, std::move(output));
	}

	template<typename T>
	TResult<T> TResult<T>::Fail(const char* errorMessage, ResultStatus status, const String& additionalErrorMessage)
	{
		return TResult(status, errorMessage, additionalErrorMessage);
	}

	template<typename T>
	template<typename OtherType>
	TResult<T> TResult<T>::Fail(const char* errorMessage, TResult<OtherType>&& childResult, const String& additionalErrorMessage)
	{
		String combinedAdditionalErrorMessage = Result::CombineAdditionalErrorMessage(additionalErrorMessage, childResult.ErrorMessage, childResult.AdditionalErrorMessage);
		return TResult(childResult.Status, errorMessage, std::move(combinedAdditionalErrorMessage));
	}

	template<typename T>
	TResult<T> TResult<T>::Fail(const char* errorMessage, Result&& childResult, const String& additionalErrorMessage)
	{
		String combinedAdditionalErrorMessage = Result::CombineAdditionalErrorMessage(additionalErrorMessage, childResult.ErrorMessage, childResult.AdditionalErrorMessage);
		return TResult(childResult.Status, errorMessage, std::move(combinedAdditionalErrorMessage));
	}

	inline Result::Result(ResultStatus status, const char* errorMessage, String additionalErrorMessage)
		: Status(status), ErrorMessage(errorMessage), AdditionalErrorMessage(std::move(additionalErrorMessage))
	{ }

	inline Result::Result(ResultStatus status)
		: Status(status)
	{ }

	template<typename T>
	Result::Result(const TResult<T>& other)
		: Status(other.Status), ErrorMessage(other.ErrorMessage), AdditionalErrorMessage(other.AdditionalErrorMessage)
	{ }

	template<typename T>
	Result::Result(TResult<T>&& other)
		: Status(other.Status), ErrorMessage(other.ErrorMessage), AdditionalErrorMessage(std::move(other.AdditionalErrorMessage))
	{ }

	inline bool Result::IsSuccessful() const { return Status == ResultStatus::Succeeded; }

	inline Result Result::Success(ResultStatus status)
	{
		return Result(status);
	}

	inline Result Result::Fail(const char* errorMessage, ResultStatus status, String additionalErrorMessage)
	{
		return Result(status, errorMessage, std::move(additionalErrorMessage));
	}

	template<typename OtherType>
	Result Result::Fail(const char* errorMessage, TResult<OtherType>&& childResult, const String& additionalErrorMessage)
	{
		String combinedAdditionalErrorMessage = Result::CombineAdditionalErrorMessage(additionalErrorMessage, childResult.ErrorMessage, childResult.AdditionalErrorMessage);
		return Result(childResult.Status, errorMessage, std::move(combinedAdditionalErrorMessage));
	}

	inline Result Result::Fail(const char* errorMessage, Result&& childResult, const String& additionalErrorMessage)
	{
		String combinedAdditionalErrorMessage = Result::CombineAdditionalErrorMessage(additionalErrorMessage, childResult.ErrorMessage, childResult.AdditionalErrorMessage);
		return Result(childResult.Status, errorMessage, std::move(combinedAdditionalErrorMessage));
	}

	inline String Result::CombineAdditionalErrorMessage(const String& additionalErrorMessage, const char* childErrorMessage, const String& childAdditionalErrorMessage)
	{
		StringStream combinedAdditionalErrorMessage;
		if(!additionalErrorMessage.empty())
			combinedAdditionalErrorMessage << additionalErrorMessage;

		if(!additionalErrorMessage.empty() && (childErrorMessage != nullptr || !childAdditionalErrorMessage.empty()))
			combinedAdditionalErrorMessage << "\n";

		if(childErrorMessage != nullptr)
			combinedAdditionalErrorMessage << "Child error: " << childErrorMessage << " " << childAdditionalErrorMessage;
		else if(!childAdditionalErrorMessage.empty())
			combinedAdditionalErrorMessage << "Child error: " << childAdditionalErrorMessage;

		return combinedAdditionalErrorMessage.str();
	}

	inline String Result::CombineErrorMessage(const char* errorMessage, const String& additionalErrorMessage)
	{
		StringStream combinedErrorMessage;

		if(errorMessage != nullptr)
			combinedErrorMessage << errorMessage;

		if(!additionalErrorMessage.empty())
			combinedErrorMessage << "\n" << additionalErrorMessage;

		return combinedErrorMessage.str();
		
	}


	/** @} */
} // namespace b3d
