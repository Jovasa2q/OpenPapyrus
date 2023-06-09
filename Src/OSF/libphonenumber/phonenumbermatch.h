// Copyright (C) 2011 The Libphonenumber Authors
// Licensed under the Apache License, Version 2.0 (the "License");
//
// Author: Tao Huang
//
// A mutable match of a phone number within a piece of text.
// Matches may be found using PhoneNumberUtil::FindNumbers.
//
// A match consists of the phone number as well as the start and end offsets of
// the corresponding subsequence of the searched text. Use raw_string() to
// obtain a copy of the matched subsequence.
//
// The following annotated example clarifies the relationship between the
// searched text, the match offsets, and the parsed number:
//
// string text = "Call me at +1 425 882-8080 for details.";
// const string country = "US";
//
// // Find the first phone number match:
// PhoneNumberMatcher matcher(text, country);
// if (matcher.HasNext()) {
//   PhoneNumberMatch match;
//   matcher.Next(&match);
// }
//
// // raw_string() contains the phone number as it appears in the text.
// "+1 425 882-8080" == match.raw_string();
//
// // start() and end() define the range of the matched subsequence.
// string subsequence = text.substr(match.start(), match.end());
// "+1 425 882-8080" == subsequence;
//
// // number() returns the the same result as PhoneNumberUtil::Parse()
// // invoked on raw_string().
// const PhoneNumberUtil& util = *PhoneNumberUtil::GetInstance();
// util.Parse(match.raw_string(), country).Equals(match.number());
//
// This class is a port of PhoneNumberMatch.java

#ifndef I18N_PHONENUMBERS_PHONENUMBERMATCH_H_
#define I18N_PHONENUMBERS_PHONENUMBERMATCH_H_

#include "phonenumber.pb.h"

namespace i18n {
	namespace phonenumbers {
		using std::string;

		class PhoneNumberMatch {
		public:
			// Creates a new match.
			// - start is the index into the target text.
			// - match is the matched string of the target text.
			// - number is the matched phone number.
			PhoneNumberMatch(int start,
				const string & raw_string,
				const PhoneNumber& number);

			// Default constructor.
			PhoneNumberMatch();
			~PhoneNumberMatch() 
			{
			}
			// Returns the phone number matched by the receiver.
			const PhoneNumber& number() const;
			// Returns the start index of the matched phone number within the searched text.
			int start() const;
			// Returns the exclusive end index of the matched phone number within the searched text.
			int end() const;
			// Returns the length of the text matched in the searched text.
			int length() const;
			// Returns the raw string matched as a phone number in the searched text.
			const string & raw_string() const;
			// Returns a string containing debug information.
			string ToString() const;
			void set_start(int start);
			void set_raw_string(const string & raw_string);
			void set_number(const PhoneNumber& number);
			bool Equals(const PhoneNumberMatch& number) const;
			void CopyFrom(const PhoneNumberMatch& number);
		private:
			int start_; // The start index into the text.
			string raw_string_; // The raw substring matched.
			PhoneNumber number_; // The matched phone number.
			DISALLOW_COPY_AND_ASSIGN(PhoneNumberMatch);
		};
	}  // namespace phonenumbers
}  // namespace i18n

#endif  // I18N_PHONENUMBERS_PHONENUMBERMATCH_H_
