// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
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

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.
//
// This test insures that net/proto2/proto/descriptor.pb.{h,cc} match exactly
// what would be generated by the protocol compiler.  These files are not
// generated automatically at build time because they are compiled into the
// protocol compiler itself.  So, if they were auto-generated, you'd have a
// chicken-and-egg problem.
//
// If this test fails, run the script
// "generate_descriptor_proto.sh" and add
// descriptor.pb.{h,cc} to your changelist.

#include <protobuf-internal.h>
#pragma hdrstop
#include <google/protobuf/testing/file.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/test_util2.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/testing/googletest.h>
#include <gtest/gtest.h>
#include <google/protobuf/stubs/map_util.h>
#include <google/protobuf/stubs/stl_util.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

namespace {

class MockErrorCollector : public MultiFileErrorCollector {
 public:
  MockErrorCollector() {}
  ~MockErrorCollector() {}

  std::string text_;

  // implements ErrorCollector ---------------------------------------
  void AddError(const std::string& filename, int line, int column,
                const std::string& message) {
    strings::SubstituteAndAppend(&text_, "$0:$1:$2: $3\n", filename, line, column,
                              message);
  }
};

class MockGeneratorContext : public GeneratorContext {
 public:
  void ExpectFileMatches(const std::string& virtual_filename,
                         const std::string& physical_filename) {
    auto it = files_.find(virtual_filename);
    ASSERT_TRUE(it != files_.end())
        << "Generator failed to generate file: " << virtual_filename;

    std::string expected_contents = *it->second;
    std::string actual_contents;
    GOOGLE_CHECK_OK(
        File::GetContents(TestUtil::TestSourceDir() + "/" + physical_filename,
                          &actual_contents, true))
        << physical_filename;
    CleanStringLineEndings(&actual_contents, false);

#ifdef WRITE_FILES  // Define to debug mismatched files.
    GOOGLE_CHECK_OK(File::SetContents("/tmp/expected.cc", expected_contents,
                               true));
    GOOGLE_CHECK_OK(
        File::SetContents("/tmp/actual.cc", actual_contents, true));
#endif

    ASSERT_EQ(expected_contents, actual_contents)
        << physical_filename
        << " needs to be regenerated.  Please run "
           "generate_descriptor_proto.sh. "
           "Then add this file to your CL.";
  }

  // implements GeneratorContext --------------------------------------

  virtual io::ZeroCopyOutputStream* Open(const std::string& filename) {
    auto& map_slot = files_[filename];
    map_slot.reset(new std::string);
    return new io::StringOutputStream(map_slot.get());
  }

 private:
  std::map<std::string, std::unique_ptr<std::string>> files_;
};

const char kDescriptorParameter[] = "dllexport_decl=PROTOBUF_EXPORT";
const char kPluginParameter[] = "dllexport_decl=PROTOC_EXPORT";


const char* test_protos[][2] = {
    {"google/protobuf/descriptor", kDescriptorParameter},
    {"google/protobuf/compiler/plugin", kPluginParameter},
};

TEST(BootstrapTest, GeneratedFilesMatch) {
  // We need a mapping from the actual file to virtual and actual path
  // of the data to compare to.
  std::map<std::string, std::string> vpath_map;
  std::map<std::string, std::string> rpath_map;
  rpath_map["third_party/protobuf/src/google/protobuf/test_messages_proto2"] =
      "net/proto2/z_generated_example/test_messages_proto2";
  rpath_map["third_party/protobuf/src/google/protobuf/test_messages_proto3"] =
      "net/proto2/z_generated_example/test_messages_proto3";
  rpath_map["net/proto2/internal/proto2_weak"] =
      "net/proto2/z_generated_example/proto2_weak";

  DiskSourceTree source_tree;
  source_tree.MapPath("", TestUtil::TestSourceDir());

  for (auto file_parameter : test_protos) {
    MockErrorCollector error_collector;
    Importer importer(&source_tree, &error_collector);
    const FileDescriptor* file =
        importer.Import(file_parameter[0] + std::string(".proto"));
    ASSERT_TRUE(file != nullptr)
        << "Can't import file " << file_parameter[0] + std::string(".proto")
        << "\n";
    EXPECT_EQ("", error_collector.text_);
    CppGenerator generator;
    MockGeneratorContext context;
#ifdef GOOGLE_PROTOBUF_RUNTIME_INCLUDE_BASE
    generator.set_opensource_runtime(true);
    generator.set_runtime_include_base(GOOGLE_PROTOBUF_RUNTIME_INCLUDE_BASE);
#endif
    std::string error;
    ASSERT_TRUE(generator.Generate(file, file_parameter[1], &context, &error));

    std::string vpath =
        FindWithDefault(vpath_map, file_parameter[0], file_parameter[0]);
    std::string rpath =
        FindWithDefault(rpath_map, file_parameter[0], file_parameter[0]);
    context.ExpectFileMatches(vpath + ".pb.cc", rpath + ".pb.cc");
    context.ExpectFileMatches(vpath + ".pb.h", rpath + ".pb.h");
  }
}

// test Generate in cpp_generator.cc
TEST(BootstrapTest, OptionNotExist) {
  cpp::CppGenerator generator;
  DescriptorPool pool;
  GeneratorContext* generator_context = nullptr;
  std::string parameter = "aaa";
  std::string error;
  ASSERT_FALSE(generator.Generate(
      pool.FindFileByName("google/protobuf/descriptor.proto"), parameter,
      generator_context, &error));
  EXPECT_EQ(error, "Unknown generator option: " + parameter);
}

}  // namespace

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
