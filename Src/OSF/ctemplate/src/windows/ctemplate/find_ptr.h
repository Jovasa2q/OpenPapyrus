// Copyright (c) 2012, Olaf van der Spek <olafvdspek@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ---
// Author: Olaf van der Spek <olafvdspek@gmail.com>

#ifndef TEMPLATE_FIND_PTR_H_
#define TEMPLATE_FIND_PTR_H_

#include <cstddef>

// NOTE: if you are statically linking the template library into your binary
// (rather than using the template .dll), set '/D CTEMPLATE_DLL_DECL='
// as a compiler flag in your project file to turn off the dllimports.
#ifndef CTEMPLATE_DLL_DECL
	#define CTEMPLATE_DLL_DECL  __declspec(dllimport)
#endif

namespace ctemplate {
	template <class T, class U> const typename T::value_type* find_ptr0(const T& c, U v)
	{
		typename T::const_iterator i = c.find(v);
		return i == c.end() ? NULL : &*i;
	}

	template <class T, class U> typename T::value_type::second_type* find_ptr(T& c, U v)
	{
		typename T::iterator i = c.find(v);
		return i == c.end() ? NULL : &i->second;
	}

	template <class T, class U> const typename T::value_type::second_type* find_ptr(const T& c, U v)
	{
		typename T::const_iterator i = c.find(v);
		return i == c.end() ? NULL : &i->second;
	}

	template <class T, class U> typename T::value_type::second_type find_ptr2(T& c, U v)
	{
		typename T::iterator i = c.find(v);
		return i == c.end() ? NULL : i->second;
	}

	template <class T, class U> const typename T::value_type::second_type find_ptr2(const T& c, U v)
	{
		typename T::const_iterator i = c.find(v);
		return i == c.end() ? NULL : i->second;
	}
}

#endif // TEMPLATE_FIND_PTR_H_
