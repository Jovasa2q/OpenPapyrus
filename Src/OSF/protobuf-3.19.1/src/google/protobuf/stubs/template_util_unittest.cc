// Copyright 2005 Google Inc. All rights reserved.
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
// ----
// Author: lar@google.com (Laramie Leavitt)
//
// These tests are really compile time tests.
// If you try to step through this in a debugger
// you will not see any evaluations, merely that
// value is assigned true or false sequentially.

#include <protobuf-internal.h>
#pragma hdrstop
#include <google/protobuf/stubs/template_util.h>
#include <google/protobuf/testing/googletest.h>
#include <gtest/gtest.h>

namespace GOOGLE_NAMESPACE = google::protobuf::internal;

namespace google {
namespace protobuf {
namespace internal {
namespace {

TEST(TemplateUtilTest, TestSize) {
  EXPECT_GT(sizeof(GOOGLE_NAMESPACE::big_), sizeof(GOOGLE_NAMESPACE::small_));
}

TEST(TemplateUtilTest, TestIntegralConstants) {
  // test the built-in types.
  EXPECT_TRUE(true_type::value);
  EXPECT_FALSE(false_type::value);

  typedef integral_constant<int, 1> one_type;
  EXPECT_EQ(1, one_type::value);
}

TEST(TemplateUtilTest, TestTemplateIf) {
  typedef if_<true, true_type, false_type>::type if_true;
  EXPECT_TRUE(if_true::value);

  typedef if_<false, true_type, false_type>::type if_false;
  EXPECT_FALSE(if_false::value);
}

TEST(TemplateUtilTest, TestTemplateTypeEquals) {
  // Check that the TemplateTypeEquals works correctly.
  bool value = false;

  // Test the same type is true.
  value = type_equals_<int, int>::value;
  EXPECT_TRUE(value);

  // Test different types are false.
  value = type_equals_<float, int>::value;
  EXPECT_FALSE(value);

  // Test type aliasing.
  typedef const int foo;
  value = type_equals_<const foo, const int>::value;
  EXPECT_TRUE(value);
}

TEST(TemplateUtilTest, TestTemplateAndOr) {
  // Check that the TemplateTypeEquals works correctly.
  bool value = false;

  // Yes && Yes == true.
  value = and_<true_, true_>::value;
  EXPECT_TRUE(value);
  // Yes && No == false.
  value = and_<true_, false_>::value;
  EXPECT_FALSE(value);
  // No && Yes == false.
  value = and_<false_, true_>::value;
  EXPECT_FALSE(value);
  // No && No == false.
  value = and_<false_, false_>::value;
  EXPECT_FALSE(value);

  // Yes || Yes == true.
  value = or_<true_, true_>::value;
  EXPECT_TRUE(value);
  // Yes || No == true.
  value = or_<true_, false_>::value;
  EXPECT_TRUE(value);
  // No || Yes == true.
  value = or_<false_, true_>::value;
  EXPECT_TRUE(value);
  // No || No == false.
  value = or_<false_, false_>::value;
  EXPECT_FALSE(value);
}

TEST(TemplateUtilTest, TestIdentity) {
  EXPECT_TRUE(
      (type_equals_<GOOGLE_NAMESPACE::identity_<int>::type, int>::value));
  EXPECT_TRUE(
      (type_equals_<GOOGLE_NAMESPACE::identity_<void>::type, void>::value));
}

}  // anonymous namespace
}  // namespace internal
}  // namespace protobuf
}  // namespace google
