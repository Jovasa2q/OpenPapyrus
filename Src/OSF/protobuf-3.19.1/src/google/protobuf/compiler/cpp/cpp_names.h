// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
#ifndef GOOGLE_PROTOBUF_COMPILER_CPP_NAMES_H__
#define GOOGLE_PROTOBUF_COMPILER_CPP_NAMES_H__

//#include <string>
#include <google/protobuf/port_def.inc>

namespace google {
namespace protobuf {

class Descriptor;
class EnumDescriptor;
class EnumValueDescriptor;
class FieldDescriptor;

namespace compiler {
namespace cpp {

// Returns the unqualified C++ name.
//
// For example, if you had:
//   package foo.bar;
//   message Baz { message Qux {} }
// Then the non-qualified version would be:
//   Baz_Qux
std::string ClassName(const Descriptor* descriptor);
std::string ClassName(const EnumDescriptor* enum_descriptor);

// Returns the fully qualified C++ name.
//
// For example, if you had:
//   package foo.bar;
//   message Baz { message Qux {} }
// Then the qualified ClassName for Qux would be:
//   ::foo::bar::Baz_Qux
std::string QualifiedClassName(const Descriptor* d);
std::string QualifiedClassName(const EnumDescriptor* d);
std::string QualifiedExtensionName(const FieldDescriptor* d);

// Get the (unqualified) name that should be used for this field in C++ code.
// The name is coerced to lower-case to emulate proto1 behavior.  People
// should be using lowercase-with-underscores style for proto field names
// anyway, so normally this just returns field->name().
std::string FieldName(const FieldDescriptor* field);

// Requires that this field is in a oneof. Returns the (unqualified) case
// constant for this field.
std::string OneofCaseConstantName(const FieldDescriptor* field);
// Returns the quafilied case constant for this field.
std::string QualifiedOneofCaseConstantName(const FieldDescriptor* field);

// Get the (unqualified) name that should be used for this enum value in C++
// code.
std::string EnumValueName(const EnumValueDescriptor* enum_value);

// Strips ".proto" or ".protodevel" from the end of a filename.
PROTOC_EXPORT std::string StripProto(const std::string& filename);

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#include <google/protobuf/port_undef.inc>

#endif  // GOOGLE_PROTOBUF_COMPILER_CPP_NAMES_H__
