// Copyright (C) 2009 The Libphonenumber Authors
// @license Apache License 2.0
// Author: Philippe Liard
//
#include <libphonenumber-internal.h>
#pragma hdrstop
//#include "phonemetadata.pb.h"
//#include "phonenumber.pb.h"

namespace i18n {
	namespace phonenumbers {
		bool ExactlySameAs(const PhoneNumber& first_number, const PhoneNumber& second_number) 
		{
			if(first_number.has_country_code() != second_number.has_country_code() || first_number.country_code() != second_number.country_code()) {
				return false;
			}
			if(first_number.has_national_number() != second_number.has_national_number() || first_number.national_number() != second_number.national_number()) {
				return false;
			}
			if(first_number.has_extension() != second_number.has_extension() || first_number.extension() != second_number.extension()) {
				return false;
			}
			if(first_number.has_italian_leading_zero() != second_number.has_italian_leading_zero() || first_number.italian_leading_zero() != second_number.italian_leading_zero()) {
				return false;
			}
			if(first_number.has_number_of_leading_zeros() != second_number.has_number_of_leading_zeros() || first_number.number_of_leading_zeros() != second_number.number_of_leading_zeros()) {
				return false;
			}
			if(first_number.has_raw_input() != second_number.has_raw_input() || first_number.raw_input() != second_number.raw_input()) {
				return false;
			}
			if(first_number.has_country_code_source() != second_number.has_country_code_source() || first_number.country_code_source() != second_number.country_code_source()) {
				return false;
			}
			if(first_number.has_preferred_domestic_carrier_code() != second_number.has_preferred_domestic_carrier_code() || 
				first_number.preferred_domestic_carrier_code() != second_number.preferred_domestic_carrier_code()) {
				return false;
			}
			return true;
		}

		bool ExactlySameAs(const PhoneNumberDesc& first_number_desc, const PhoneNumberDesc& second_number_desc) 
		{
			if(first_number_desc.has_national_number_pattern() != second_number_desc.has_national_number_pattern() || 
				first_number_desc.national_number_pattern() != second_number_desc.national_number_pattern()) {
				return false;
			}
			if(first_number_desc.has_example_number() != second_number_desc.has_example_number() ||
				first_number_desc.example_number() != second_number_desc.example_number()) {
				return false;
			}
			if(first_number_desc.possible_length_size() != second_number_desc.possible_length_size()) {
				return false;
			}
			else {
				for(int i = 0; i < first_number_desc.possible_length_size(); ++i) {
					if(first_number_desc.possible_length(i) != second_number_desc.possible_length(i)) {
						return false;
					}
				}
			}
			if(first_number_desc.possible_length_local_only_size() != second_number_desc.possible_length_local_only_size()) {
				return false;
			}
			else {
				for(int i = 0; i < first_number_desc.possible_length_local_only_size(); ++i) {
					if(first_number_desc.possible_length_local_only(i) != second_number_desc.possible_length_local_only(i)) {
						return false;
					}
				}
			}
			return true;
		}
	}
}
