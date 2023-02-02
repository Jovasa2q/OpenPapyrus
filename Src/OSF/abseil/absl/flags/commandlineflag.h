// commandlineflag.h
// Copyright 2020 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at https://www.apache.org/licenses/LICENSE-2.0
//
// This header file defines the `CommandLineFlag`, which acts as a type-erased
// handle for accessing metadata about the Abseil Flag in question.
//
// Because an actual Abseil flag is of an unspecified type, you should not
// manipulate or interact directly with objects of that type. Instead, use the
// CommandLineFlag type as an intermediary.
#ifndef ABSL_FLAGS_COMMANDLINEFLAG_H_
#define ABSL_FLAGS_COMMANDLINEFLAG_H_

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace flags_internal {
	class PrivateHandleAccessor;
	class FlagStateInterface; // @sobolev
	enum FlagSettingMode; // @sobolev
	enum ValueSource; // @sobolev
}  // namespace flags_internal

// CommandLineFlag
//
// This type acts as a type-erased handle for an instance of an Abseil Flag and
// holds reflection information pertaining to that flag. Use CommandLineFlag to
// access a flag's name, location, help string etc.
//
// To obtain an absl::CommandLineFlag, invoke `absl::FindCommandLineFlag()`
// passing it the flag name string.
//
// Example:
//
//   // Obtain reflection handle for a flag named "flagname".
//   const absl::CommandLineFlag* my_flag_data =
//        absl::FindCommandLineFlag("flagname");
//
//   // Now you can get flag info from that reflection handle.
//   std::string flag_location = my_flag_data->Filename();
//   ...
class CommandLineFlag {
public:
	constexpr CommandLineFlag() = default;
	// Not copyable/assignable.
	CommandLineFlag(const CommandLineFlag&) = delete;
	CommandLineFlag& operator = (const CommandLineFlag&) = delete;
	//
	// Return true iff flag has type T.
	//
	template <typename T> inline bool IsOfType() const { return TypeId() == base_internal::FastTypeId<T>(); }
	//
	// Attempts to retrieve the flag value. Returns value on success, absl::nullopt otherwise.
	//
	template <typename T> absl::optional<T> TryGet() const 
	{
		if(IsRetired() || !IsOfType<T>()) {
			return absl::nullopt;
		}
		// Implementation notes:
		//
		// We are wrapping a union around the value of `T` to serve three purposes:
		//
		//  1. `U.value` has correct size and alignment for a value of type `T`
		//  2. The `U.value` constructor is not invoked since U's constructor does
		//     not do it explicitly.
		//  3. The `U.value` destructor is invoked since U's destructor does it
		//     explicitly. This makes `U` a kind of RAII wrapper around non default
		//     constructible value of T, which is destructed when we leave the
		//     scope. We do need to destroy U.value, which is constructed by
		//     CommandLineFlag::Read even though we left it in a moved-from state
		//     after std::move.
		//
		// All of this serves to avoid requiring `T` being default constructible.
		union U {
			T value;
			U() 
			{
			}
			~U() 
			{
				value.~T();
			}
		};

		U u;

		Read(&u.value);
		// allow retired flags to be "read", so we can report invalid access.
		return IsRetired() ? absl::nullopt : std::move(u.value);
	}
	virtual absl::string_view Name() const = 0; // Returns name of this flag.
	virtual std::string Filename() const = 0; // Returns name of the file where this flag is defined.
	virtual std::string Help() const = 0; // Returns help message associated with this flag.
	virtual bool IsRetired() const; // Returns true iff this object corresponds to retired flag.
	virtual std::string DefaultValue() const = 0; // Returns the default value for this flag.
	virtual std::string CurrentValue() const = 0; // Returns the current value for this flag.
	//
	// Sets the value of the flag based on specified string `value`. If the flag
	// was successfully set to new value, it returns true. Otherwise, sets `error`
	// to indicate the error, leaves the flag unchanged, and returns false.
	//
	bool ParseFrom(absl::string_view value, std::string* error);
protected:
	~CommandLineFlag() = default;
private:
	friend class flags_internal::PrivateHandleAccessor;
	// Sets the value of the flag based on specified string `value`. If the flag
	// was successfully set to new value, it returns true. Otherwise, sets `error`
	// to indicate the error, leaves the flag unchanged, and returns false. There
	// are three ways to set the flag's value:
	//  * Update the current flag value
	//  * Update the flag's default value
	//  * Update the current flag value if it was never set before
	// The mode is selected based on `set_mode` parameter.
	virtual bool ParseFrom(absl::string_view value, flags_internal::FlagSettingMode set_mode, flags_internal::ValueSource source, std::string & error) = 0;
	// Returns id of the flag's value type.
	virtual flags_internal::FlagFastTypeId TypeId() const = 0;
	// Interface to save flag to some persistent state. Returns current flag state
	// or nullptr if flag does not support saving and restoring a state.
	virtual std::unique_ptr<flags_internal::FlagStateInterface> SaveState() = 0;
	// Copy-construct a new value of the flag's type in a memory referenced by
	// the dst based on the current flag's value.
	virtual void Read(void* dst) const = 0;
	// To be deleted. Used to return true if flag's current value originated from command line.
	virtual bool IsSpecifiedOnCommandLine() const = 0;
	// Validates supplied value usign validator or parseflag routine
	virtual bool ValidateInputValue(absl::string_view value) const = 0;
	// Checks that flags default value can be converted to string and back to the
	// flag's value type.
	virtual void CheckDefaultValueParsingRoundtrip() const = 0;
};

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_FLAGS_COMMANDLINEFLAG_H_
