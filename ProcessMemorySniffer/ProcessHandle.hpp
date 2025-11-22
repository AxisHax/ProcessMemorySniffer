#pragma once

#include <Windows.h>

#include <optional>
#include <utility>

/// <summary>
/// RAII wrapper for a Windows process HANDLE that manages it's lifetime and provides safe move semantics.
/// </summary>
class ProcessHandle
{
public:
	/// <summary>
	/// Default constructor for ProcessHandle. Initializes the object to its default state and is noexcept (guaranteed not to throw).
	/// </summary>
	ProcessHandle() noexcept = default;

	/// <summary>
	/// Explicit constructor that initializes a ProcessHandle with the given native HANDLE and prevents implicit conversions.
	/// </summary>
	/// <param name="handle">The native HANDLE representing the process to be stored by the ProcessHandle.</param>
	explicit ProcessHandle(HANDLE handle) : handle_(handle)
	{ }

	
	ProcessHandle(const ProcessHandle&) = delete;
	ProcessHandle& operator=(const ProcessHandle&) = delete;

	/// <summary>
	/// Move constructor that acquires the internal handle from another ProcessHandle.
	/// </summary>
	/// <param name="other">Rvalue reference to the source ProcessHandle to move from. Ownership of the internal handle is transferred to the newly constructed object and the source's handle is set to nullptr.</param>
	ProcessHandle(ProcessHandle&& other) noexcept : handle_(std::exchange(other.handle_, nullptr))
	{ }

	/// <summary>
	/// Move-assignment operator that transfers ownership of a process handle from another ProcessHandle. The current handle (if any) is closed, and ownership of other's handle is taken. Operation is noexcept.
	/// </summary>
	/// <param name="other">Rvalue reference to the source ProcessHandle. Its handle is moved to this object and other.handle_ is set to nullptr; the moved-from object is left in a valid, empty state.</param>
	/// <returns>Reference to this ProcessHandle after the move assignment.</returns>
	ProcessHandle& operator=(ProcessHandle&& other) noexcept
	{
		if (this != &other)
		{
			close();
			handle_ = std::exchange(other.handle_, nullptr);
		}

		return *this;
	}

	/// <summary>
	/// Destructor for ProcessHandle that releases associated resources by calling close(). It is noexcept.
	/// </summary>
	~ProcessHandle() noexcept
	{
		close();
	}

	/// <summary>
	/// Attempts to open a handle to the process identified by pid, requesting PROCESS_QUERY_INFORMATION and PROCESS_VM_READ access.
	/// </summary>
	/// <param name="pid">The process identifier (DWORD) of the target process to open.</param>
	/// <returns>A std::optional containing a ProcessHandle that wraps the native HANDLE on success; std::nullopt if the process handle could not be opened. The function is marked [[nodiscard]] so the result should not be ignored.</returns>
	[[nodiscard]] static std::optional<ProcessHandle> open(DWORD pid) noexcept
	{
		HANDLE handle = ::OpenProcess(
			PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE,
			pid
		);

		if (!handle)
		{
			return std::nullopt;
		}

		return ProcessHandle(handle);
	}

	/// <summary>
	/// Returns the stored native HANDLE.
	/// </summary>
	/// <returns>The stored native HANDLE value.</returns>
	[[nodiscard]] HANDLE get() const noexcept
	{
		return handle_;
	}

	/// <summary>
	/// Explicit bool conversion that indicates whether the object holds a non-null handle.
	/// </summary>
	explicit operator bool() const noexcept
	{
		return handle_ != nullptr;
	}

private:
	/// <summary>
	/// Closes the underlying handle, if set, by calling ::CloseHandle and resets the handle to nullptr. The function is noexcept and does not throw exceptions.
	/// </summary>
	void close() noexcept
	{
		if (handle_)
		{
			::CloseHandle(handle_);
			handle_ = nullptr;
		}
	}

	/// <summary>
	/// A HANDLE variable initialized to nullptr.
	/// </summary>
	HANDLE handle_{ nullptr };
};