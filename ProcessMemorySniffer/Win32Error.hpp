#pragma once

#include <Windows.h>

#include <stdexcept>
#include <string>

/// <summary>
/// Exception type that represents a Win32 error, deriving from std::runtime_error and storing the associated error code. 
/// Since this inheriits from std::runtime_error it can be caught as a std::exception, std::runtime_error, or Win32Error.
/// </summary>
class Win32Error : public std::runtime_error
{
public:
	/// <summary>
	/// Constructs a Win32Error exception containing a context message and a Windows error code.
	/// </summary>
	/// <param name="context">A context string describing the operation or location; it's combined into the exception message (std::runtime_error::what()).</param>
	/// <param name="errorCode">The Win32 error code (DWORD) to store. If not supplied, ::GetLastError() is used as the default.</param>
	explicit Win32Error(const std::string& context, DWORD errorCode = ::GetLastError())
		: std::runtime_error(context + "(error " + std::to_string(errorCode) + ")"), code_(errorCode)
	{ }

	/// <summary>
	/// Returns the stored DWORD code value. The method is const, noexcept, and marked [[nodiscard]] to discourage ignoring the result.
	/// </summary>
	/// <returns>The stored code_ value as a DWORD.</returns>
	[[nodiscard]] DWORD code() const noexcept
	{
		return code_;
	}

private:
	/// <summary>
	/// Contains the associated Win32 error code.
	/// </summary>
	DWORD code_;
};