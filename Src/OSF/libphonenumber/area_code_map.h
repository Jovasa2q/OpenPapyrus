// Copyright (C) 2012 The Libphonenumber Authors
// Licensed under the Apache License, Version 2.0 (the "License");
//
// Author: Patrick Mezard

#ifndef I18N_PHONENUMBERS_AREA_CODE_MAP_H_
#define I18N_PHONENUMBERS_AREA_CODE_MAP_H_

#include "scoped_ptr.h"

namespace i18n {
namespace phonenumbers {
using std::map;
using std::string;

class DefaultMapStorage;
class PhoneNumber;
class PhoneNumberUtil;
struct PrefixDescriptions;

// A utility that maps phone number prefixes to a string describing the
// geographical area the prefix covers.
class AreaCodeMap {
public:
	AreaCodeMap();
	~AreaCodeMap();

	// Returns the description of the geographical area the number corresponds
	// to. This method distinguishes the case of an invalid prefix and a prefix
	// for which the name is not available in the current language. If the
	// description is not available in the current language an empty string is
	// returned. If no description was found for the provided number, null is
	// returned.
	const char* Lookup(const PhoneNumber& number) const;

	// Creates an AreaCodeMap initialized with area_codes. Note that the
	// underlying implementation of this method is expensive thus should
	// not be called by time-critical applications.
	//
	// area_codes maps phone number prefixes to geographical area description.
	void ReadAreaCodeMap(const PrefixDescriptions* descriptions);

private:
	// Does a binary search for value in the provided array from start to end
	// (inclusive). Returns the position if {@code value} is found; otherwise,
	// returns the position which has the largest value that is less than value.
	// This means if value is the smallest, -1 will be returned.
	int BinarySearch(int start, int end, int64 value) const;

	const PhoneNumberUtil& phone_util_;
	scoped_ptr<const DefaultMapStorage> storage_;

	DISALLOW_COPY_AND_ASSIGN(AreaCodeMap);
};
}  // namespace phonenumbers
}  // namespace i18n

#endif /* I18N_PHONENUMBERS_AREA_CODE_MAP_H_ */
