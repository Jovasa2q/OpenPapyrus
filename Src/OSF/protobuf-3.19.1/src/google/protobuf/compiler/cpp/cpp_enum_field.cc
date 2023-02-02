// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
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
// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <protobuf-internal.h>
#pragma hdrstop
#include <google/protobuf/compiler/cpp/cpp_enum_field.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {
namespace {
void SetEnumVariables(const FieldDescriptor* descriptor,
    std::map<std::string, std::string>* variables,
    const Options& options) {
	SetCommonFieldVariables(descriptor, variables, options);
	const EnumValueDescriptor* default_value = descriptor->default_value_enum();
	(*variables)["type"] = QualifiedClassName(descriptor->enum_type(), options);
	(*variables)["default"] = Int32ToString(default_value->number());
	(*variables)["full_name"] = descriptor->full_name();
}
}  // namespace

// ===================================================================

EnumFieldGenerator::EnumFieldGenerator(const FieldDescriptor* descriptor,
    const Options& options)
	: FieldGenerator(descriptor, options) {
	SetEnumVariables(descriptor, &variables_, options);
}

EnumFieldGenerator::~EnumFieldGenerator() {
}

void EnumFieldGenerator::GeneratePrivateMembers(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("int $name$_;\n");
}

void EnumFieldGenerator::GenerateAccessorDeclarations(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("$deprecated_attr$$type$ ${1$$name$$}$() const;\n"
		"$deprecated_attr$void ${1$set_$name$$}$($type$ value);\n"
		"private:\n"
		"$type$ ${1$_internal_$name$$}$() const;\n"
		"void ${1$_internal_set_$name$$}$($type$ value);\n"
		"public:\n", descriptor_);
}

void EnumFieldGenerator::GenerateInlineAccessorDefinitions(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("inline $type$ $classname$::_internal_$name$() const { return static_cast< $type$ >($name$_); }\n"
		"inline $type$ $classname$::$name$() const {\n"
		"$annotate_get$"
		"  // @@protoc_insertion_point(field_get:$full_name$)\n"
		"  return _internal_$name$();\n"
		"}\n"
		"inline void $classname$::_internal_set_$name$($type$ value) {\n");
	if(!HasPreservingUnknownEnumSemantics(descriptor_)) {
		format("  assert($type$_IsValid(value));\n");
	}
	format(
		"  $set_hasbit$\n"
		"  $name$_ = value;\n"
		"}\n"
		"inline void $classname$::set_$name$($type$ value) {\n"
		"  _internal_set_$name$(value);\n"
		"$annotate_set$"
		"  // @@protoc_insertion_point(field_set:$full_name$)\n"
		"}\n");
}

void EnumFieldGenerator::GenerateClearingCode(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format("$name$_ = $default$;\n");
}

void EnumFieldGenerator::GenerateMergingCode(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format("_internal_set_$name$(from._internal_$name$());\n");
}

void EnumFieldGenerator::GenerateSwappingCode(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format("swap($name$_, other->$name$_);\n");
}

void EnumFieldGenerator::GenerateConstructorCode(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format("$name$_ = $default$;\n");
}

void EnumFieldGenerator::GenerateCopyConstructorCode(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format("$name$_ = from.$name$_;\n");
}

void EnumFieldGenerator::GenerateSerializeWithCachedSizesToArray(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format(
		"target = stream->EnsureSpace(target);\n"
		"target = ::$proto_ns$::internal::WireFormatLite::WriteEnumToArray($number$, this->_internal_$name$(), target);\n");
}

void EnumFieldGenerator::GenerateByteSize(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format(
		"total_size += $tag_size$ +\n"
		"  "
		"::$proto_ns$::internal::WireFormatLite::EnumSize(this->_internal_$name$("
		"));\n");
}

void EnumFieldGenerator::GenerateConstinitInitializer(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format("$name$_($default$)\n");
}

// ===================================================================

EnumOneofFieldGenerator::EnumOneofFieldGenerator(const FieldDescriptor* descriptor, const Options& options)
	: EnumFieldGenerator(descriptor, options) {
	SetCommonOneofFieldVariables(descriptor, &variables_);
}

EnumOneofFieldGenerator::~EnumOneofFieldGenerator() {
}

void EnumOneofFieldGenerator::GenerateInlineAccessorDefinitions(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format(
		"inline $type$ $classname$::_internal_$name$() const\n{\n"
		"\tif(_internal_has_$name$()) {\n"
		"\t\treturn static_cast< $type$ >($field_member$);\n"
		"\t}\n"
		"\treturn static_cast< $type$ >($default$);\n"
		"}\n"
		"inline $type$ $classname$::$name$() const\n{\n"
		"$annotate_get$"
		"\t// @@protoc_insertion_point(field_get:$full_name$)\n"
		"\treturn _internal_$name$();\n"
		"}\n"
		"inline void $classname$::_internal_set_$name$($type$ value)\n{\n");
	if(!HasPreservingUnknownEnumSemantics(descriptor_)) {
		format("\tassert($type$_IsValid(value));\n");
	}
	format(
		"\tif(!_internal_has_$name$()) {\n"
		"\t\tclear_$oneof_name$();\n"
		"\t\tset_has_$name$();\n"
		"\t}\n"
		"\t$field_member$ = value;\n"
		"}\n"
		"inline void $classname$::set_$name$($type$ value)\n{\n"
		"\t_internal_set_$name$(value);\n"
		"$annotate_set$"
		"\t// @@protoc_insertion_point(field_set:$full_name$)\n"
		"}\n");
}

void EnumOneofFieldGenerator::GenerateClearingCode(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("$field_member$ = $default$;\n");
}

void EnumOneofFieldGenerator::GenerateSwappingCode(io::Printer* printer) const {
	// Don't print any swapping code. Swapping the union will swap this field.
}

void EnumOneofFieldGenerator::GenerateConstructorCode(io::Printer* printer) const {
	Formatter format(printer, variables_);
	format("$ns$::_$classname$_default_instance_.$name$_ = $default$;\n");
}

// ===================================================================

RepeatedEnumFieldGenerator::RepeatedEnumFieldGenerator(const FieldDescriptor* descriptor, const Options& options) : FieldGenerator(descriptor, options) 
{
	SetEnumVariables(descriptor, &variables_, options);
}

RepeatedEnumFieldGenerator::~RepeatedEnumFieldGenerator() {
}

void RepeatedEnumFieldGenerator::GeneratePrivateMembers(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("::$proto_ns$::RepeatedField<int> $name$_;\n");
	if(descriptor_->is_packed() && HasGeneratedMethods(descriptor_->file(), options_)) {
		format("mutable std::atomic<int> _$name$_cached_byte_size_;\n");
	}
}

void RepeatedEnumFieldGenerator::GenerateAccessorDeclarations(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("private:\n"
		"$type$ ${1$_internal_$name$$}$(int index) const;\n"
		"void ${1$_internal_add_$name$$}$($type$ value);\n"
		"::$proto_ns$::RepeatedField<int> * ${1$_internal_mutable_$name$$}$();\n"
		"public:\n"
		"$deprecated_attr$$type$ ${1$$name$$}$(int index) const;\n"
		"$deprecated_attr$void ${1$set_$name$$}$(int index, $type$ value);\n"
		"$deprecated_attr$void ${1$add_$name$$}$($type$ value);\n"
		"$deprecated_attr$const ::$proto_ns$::RepeatedField<int> & ${1$$name$$}$() const;\n"
		"$deprecated_attr$::$proto_ns$::RepeatedField<int> * ${1$mutable_$name$$}$();\n",
		descriptor_);
}

void RepeatedEnumFieldGenerator::GenerateInlineAccessorDefinitions(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("inline $type$ $classname$::_internal_$name$(int index) const { return static_cast< $type$ >($name$_.Get(index)); }\n"
		"inline $type$ $classname$::$name$(int index) const\n{\n"
		"$annotate_get$"
		"\t// @@protoc_insertion_point(field_get:$full_name$)\n"
		"\treturn _internal_$name$(index);\n"
		"}\n"
		"inline void $classname$::set_$name$(int index, $type$ value)\n{\n");
	if(!HasPreservingUnknownEnumSemantics(descriptor_)) {
		format("\tassert($type$_IsValid(value));\n");
	}
	format("\t$name$_.Set(index, value);\n"
		"$annotate_set$"
		"\t// @@protoc_insertion_point(field_set:$full_name$)\n"
		"}\n"
		"inline void $classname$::_internal_add_$name$($type$ value)\n{\n");
	if(!HasPreservingUnknownEnumSemantics(descriptor_)) {
		format("\tassert($type$_IsValid(value));\n");
	}
	format("\t$name$_.Add(value);\n"
		"}\n"
		"inline void $classname$::add_$name$($type$ value)\n{\n"
		"\t_internal_add_$name$(value);\n"
		"$annotate_add$"
		"\t// @@protoc_insertion_point(field_add:$full_name$)\n"
		"}\n"
		"inline const ::$proto_ns$::RepeatedField<int> & $classname$::$name$() const\n{\n"
		"$annotate_list$"
		"\t// @@protoc_insertion_point(field_list:$full_name$)\n"
		"\treturn $name$_;\n"
		"}\n"
		"inline ::$proto_ns$::RepeatedField<int> * $classname$::_internal_mutable_$name$()\n{\n"
		"\treturn &$name$_;\n"
		"}\n"
		"inline ::$proto_ns$::RepeatedField<int> * $classname$::mutable_$name$()\n{\n"
		"$annotate_mutable_list$"
		"\t// @@protoc_insertion_point(field_mutable_list:$full_name$)\n"
		"\treturn _internal_mutable_$name$();\n"
		"}\n");
}

void RepeatedEnumFieldGenerator::GenerateClearingCode(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("$name$_.Clear();\n");
}

void RepeatedEnumFieldGenerator::GenerateMergingCode(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("$name$_.MergeFrom(from.$name$_);\n");
}

void RepeatedEnumFieldGenerator::GenerateSwappingCode(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("$name$_.InternalSwap(&other->$name$_);\n");
}

void RepeatedEnumFieldGenerator::GenerateConstructorCode(io::Printer* printer) const {
	// Not needed for repeated fields.
}

void RepeatedEnumFieldGenerator::GenerateSerializeWithCachedSizesToArray(io::Printer* printer) const {
	Formatter format(printer, variables_);
	if(descriptor_->is_packed()) {
		// Write the tag and the size.
		format(
			"{\n"
			"\tint byte_size = _$name$_cached_byte_size_.load(std::memory_order_relaxed);\n"
			"\tif(byte_size > 0) {\n"
			"\t\ttarget = stream->WriteEnumPacked($number$, $name$_, byte_size, target);\n"
			"\t}\n"
			"}\n");
	}
	else {
		format(
			"for(int i = 0, n = this->_internal_$name$_size(); i < n; i++) {\n"
			"\ttarget = stream->EnsureSpace(target);\n"
			"\ttarget = ::$proto_ns$::internal::WireFormatLite::WriteEnumToArray($number$, this->_internal_$name$(i), target);\n"
			"}\n");
	}
}

void RepeatedEnumFieldGenerator::GenerateByteSize(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format(
		"{\n"
		"\tsize_t data_size = 0;\n"
		"\tunsigned int count = static_cast<unsigned int>(this->_internal_$name$_size());");
	format.Indent();
	format(
		"for(unsigned int i = 0; i < count; i++) {\n"
		"\tdata_size += ::$proto_ns$::internal::WireFormatLite::EnumSize(this->_internal_$name$(static_cast<int>(i)));\n}\n");

	if(descriptor_->is_packed()) {
		format("if(data_size > 0) {\n"
			"\ttotal_size += $tag_size$ + ::$proto_ns$::internal::WireFormatLite::Int32Size(static_cast<$int32$>(data_size));\n"
			"}\n"
			"int cached_size = ::$proto_ns$::internal::ToCachedSize(data_size);\n"
			"_$name$_cached_byte_size_.store(cached_size, std::memory_order_relaxed);\n"
			"total_size += data_size;\n");
	}
	else {
		format("total_size += ($tag_size$UL * count) + data_size;\n");
	}
	format.Outdent();
	format("}\n");
}

void RepeatedEnumFieldGenerator::GenerateConstinitInitializer(io::Printer* printer) const 
{
	Formatter format(printer, variables_);
	format("$name$_()");
	if(descriptor_->is_packed() && HasGeneratedMethods(descriptor_->file(), options_)) {
		format("\n, _$name$_cached_byte_size_(0)");
	}
}
}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
