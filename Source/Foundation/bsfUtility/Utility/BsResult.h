//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** Common result codes for TResult/Result types. */
	enum class ResultStatus
	{
		Succeeded,
		Failed,
		FailedAlreadyExists,
		FailedInvalidInput,
		FailedInternalError,
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

		/** Returns true if the result is one of the success states. */
		bool IsSuccessful() const;

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
		static TResult Fail(const char* errorMessage, ResultStatus status = ResultStatus::Failed, String additionalErrorMessage = StringUtil::kBlank);

		ResultStatus Status = ResultStatus::Failed;
		const char* ErrorMessage = nullptr;
		String AdditionalErrorMessage;
		T Output;

	private:
		TResult(ResultStatus status, const char* errorMessage, String additionalErrorMessage = StringUtil::kBlank);
		TResult(ResultStatus status, const T& output);
		TResult(ResultStatus status, T&& output);
	};

	/** Same as TResult, but with no output object. */
	struct Result
	{
		/** Converts TResult to Result (discards the output field). */
		template<typename T> Result(const TResult<T>& other);

		/** Converts TResult to Result (discards the output field). */
		template<typename T> Result(TResult<T>&& other);

		/** Returns true if the result is one of the success states. */
		bool IsSuccessful() const;

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

		ResultStatus Status = ResultStatus::Failed;
		const char* ErrorMessage = nullptr;
		String AdditionalErrorMessage;

	private:
		Result(ResultStatus status, const char* errorMessage, String additionalErrorMessage = StringUtil::kBlank);
		Result(ResultStatus status);

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
	bool TResult<T>::IsSuccessful() const { return Status == ResultStatus::Succeeded; }

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
	TResult<T> TResult<T>::Fail(const char* errorMessage, ResultStatus status, String additionalErrorMessage)
	{
		return TResult(status, errorMessage, std::move(additionalErrorMessage));
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

	/** @} */
} // namespace bs
