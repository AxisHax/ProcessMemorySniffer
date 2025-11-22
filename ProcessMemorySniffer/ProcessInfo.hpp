#pragma once

#include <Windows.h>

#include <string>
#include <cstddef>

#define WIN32_LEAN_AND_MEAN

/// <summary>
/// Defines Bytes as an alias for std::size_t.
/// </summary>
using Bytes = std::size_t;

/// <summary>
/// Data model for a process and it's memory usage information.
/// </summary>
struct ProcessInfo
{
	/// <summary>
	/// The PID of the process. Initialized to 0 by default.
	/// </summary>
	DWORD			pid{ 0 };

	/// <summary>
	/// The name of the process.
	/// </summary>
	std::wstring	name;

	/// <summary>
	/// The working set size of the process.
	/// </summary>
	Bytes			workingSetBytes{ 0 };

	/// <summary>
	/// The private bytes size of the process.
	/// </summary>
	Bytes			privateBytes{ 0 };
};