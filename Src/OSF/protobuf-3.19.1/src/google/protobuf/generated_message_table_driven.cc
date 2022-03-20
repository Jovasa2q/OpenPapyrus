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
#include <protobuf-internal.h>
#pragma hdrstop
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/stubs/casts.h>
#include <google/protobuf/generated_message_table_driven_lite.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/wire_format_lite.h>

namespace google {
namespace protobuf {
namespace internal {
namespace {
UnknownFieldSet* MutableUnknownFields(MessageLite* msg, int64_t arena_offset) {
	return Raw<InternalMetadata>(msg, arena_offset)
	       ->mutable_unknown_fields<UnknownFieldSet>();
}

struct UnknownFieldHandler {
	// TODO(mvels): consider renaming UnknownFieldHandler to (TableDrivenTraits?),
	// and conflating InternalMetaData into it, simplifying the template.
	static constexpr bool IsLite() { return false; }
	static bool Skip(MessageLite* msg, const ParseTable& table, io::CodedInputStream* input, int tag) 
	{
		GOOGLE_DCHECK(table.unknown_field_set);
		return WireFormat::SkipField(input, tag, MutableUnknownFields(msg, table.arena_offset));
	}

	static void Varint(MessageLite* msg, const ParseTable& table, int tag, int value) 
	{
		GOOGLE_DCHECK(table.unknown_field_set);
		MutableUnknownFields(msg, table.arena_offset)->AddVarint(WireFormatLite::GetTagFieldNumber(tag), value);
	}

	static bool ParseExtension(MessageLite* msg, const ParseTable& table,
	    io::CodedInputStream* input, int tag) {
		ExtensionSet* extensions = GetExtensionSet(msg, table.extension_offset);
		if(extensions == nullptr) {
			return false;
		}
		const Message* prototype = down_cast<const Message*>(table.default_instance());
		GOOGLE_DCHECK(prototype != nullptr);
		GOOGLE_DCHECK(table.unknown_field_set);
		UnknownFieldSet* unknown_fields = MutableUnknownFields(msg, table.arena_offset);
		return extensions->ParseField(tag, input, prototype, unknown_fields);
	}
};
}  // namespace

bool MergePartialFromCodedStream(MessageLite* msg, const ParseTable& table, io::CodedInputStream* input) 
{
	return MergePartialFromCodedStreamImpl<UnknownFieldHandler>(msg, table, input);
}
}  // namespace internal
}  // namespace protobuf
}  // namespace google
