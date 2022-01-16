// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/absl-internal.h"
#pragma hdrstop
#include "absl/random/internal/randen_traits.h"

// This file contains only the round keys for randen.
//
// "Nothing up my sleeve" numbers from the first hex digits of Pi, obtained
// from http://hexpi.sourceforge.net/. The array was generated by following
// Python script:

/*
   python >tmp.cc << EOF
   """Generates Randen round keys array from pi-hex.62500.txt file."""
   import binascii

   KEYS = 17 * 8

   def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i + n]

   def pairwise(t):
    """Transforms sequence into sequence of pairs."""
    it = iter(t)
    return zip(it,it)

   def digits_from_pi():
   """Reads digits from hexpi.sourceforge.net file."""
   with open("pi-hex.62500.txt") as file:
    return file.read()

   def digits_from_urandom():
   """Reads digits from /dev/urandom."""
   with open("/dev/urandom") as file:
    return binascii.hexlify(file.read(KEYS * 16))

   def print_row(b)
   print("  0x{0}, 0x{1}, 0x{2}, 0x{3}, 0x{4}, 0x{5}, 0x{6}, 0x{7}, 0x{8}, 0x{9},
   0x{10}, 0x{11}, 0x{12}, 0x{13}, 0x{14}, 0x{15},".format(*b))


   digits = digits_from_pi()
 #digits = digits_from_urandom()

   print("namespace {")
   print("static constexpr size_t kKeyBytes = {0};\n".format(KEYS * 16))
   print("}")

   print("alignas(16) const unsigned char kRandenRoundKeysBE[kKeyBytes] = {")

   for i, u16 in zip(range(KEYS), chunks(digits, 32)):
   b = list(chunks(u16, 2))
   print_row(b)

   print("};")

   print("alignas(16) const unsigned char kRandenRoundKeys[kKeyBytes] = {")

   for i, u16 in zip(range(KEYS), chunks(digits, 32)):
   b = list(chunks(u16, 2))
   b.reverse()
   print_row(b)

   print("};")

   EOF

 */

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace random_internal {
namespace {
static constexpr size_t kKeyBytes = 2176;
}

alignas(16) const unsigned char kRandenRoundKeysBE[kKeyBytes] = {
	0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3, 0x08, 0xD3, 0x13, 0x19, 0x8A, 0x2E,
	0x03, 0x70, 0x73, 0x44, 0xA4, 0x09, 0x38, 0x22, 0x29, 0x9F, 0x31, 0xD0,
	0x08, 0x2E, 0xFA, 0x98, 0xEC, 0x4E, 0x6C, 0x89, 0x45, 0x28, 0x21, 0xE6,
	0x38, 0xD0, 0x13, 0x77, 0xBE, 0x54, 0x66, 0xCF, 0x34, 0xE9, 0x0C, 0x6C,
	0xC0, 0xAC, 0x29, 0xB7, 0xC9, 0x7C, 0x50, 0xDD, 0x3F, 0x84, 0xD5, 0xB5,
	0xB5, 0x47, 0x09, 0x17, 0x92, 0x16, 0xD5, 0xD9, 0x89, 0x79, 0xFB, 0x1B,
	0xD1, 0x31, 0x0B, 0xA6, 0x98, 0xDF, 0xB5, 0xAC, 0x2F, 0xFD, 0x72, 0xDB,
	0xD0, 0x1A, 0xDF, 0xB7, 0xB8, 0xE1, 0xAF, 0xED, 0x6A, 0x26, 0x7E, 0x96,
	0xBA, 0x7C, 0x90, 0x45, 0xF1, 0x2C, 0x7F, 0x99, 0x24, 0xA1, 0x99, 0x47,
	0xB3, 0x91, 0x6C, 0xF7, 0x08, 0x01, 0xF2, 0xE2, 0x85, 0x8E, 0xFC, 0x16,
	0x63, 0x69, 0x20, 0xD8, 0x71, 0x57, 0x4E, 0x69, 0xA4, 0x58, 0xFE, 0xA3,
	0xF4, 0x93, 0x3D, 0x7E, 0x0D, 0x95, 0x74, 0x8F, 0x72, 0x8E, 0xB6, 0x58,
	0x71, 0x8B, 0xCD, 0x58, 0x82, 0x15, 0x4A, 0xEE, 0x7B, 0x54, 0xA4, 0x1D,
	0xC2, 0x5A, 0x59, 0xB5, 0x9C, 0x30, 0xD5, 0x39, 0x2A, 0xF2, 0x60, 0x13,
	0xC5, 0xD1, 0xB0, 0x23, 0x28, 0x60, 0x85, 0xF0, 0xCA, 0x41, 0x79, 0x18,
	0xB8, 0xDB, 0x38, 0xEF, 0x8E, 0x79, 0xDC, 0xB0, 0x60, 0x3A, 0x18, 0x0E,
	0x6C, 0x9E, 0x0E, 0x8B, 0xB0, 0x1E, 0x8A, 0x3E, 0xD7, 0x15, 0x77, 0xC1,
	0xBD, 0x31, 0x4B, 0x27, 0x78, 0xAF, 0x2F, 0xDA, 0x55, 0x60, 0x5C, 0x60,
	0xE6, 0x55, 0x25, 0xF3, 0xAA, 0x55, 0xAB, 0x94, 0x57, 0x48, 0x98, 0x62,
	0x63, 0xE8, 0x14, 0x40, 0x55, 0xCA, 0x39, 0x6A, 0x2A, 0xAB, 0x10, 0xB6,
	0xB4, 0xCC, 0x5C, 0x34, 0x11, 0x41, 0xE8, 0xCE, 0xA1, 0x54, 0x86, 0xAF,
	0x7C, 0x72, 0xE9, 0x93, 0xB3, 0xEE, 0x14, 0x11, 0x63, 0x6F, 0xBC, 0x2A,
	0x2B, 0xA9, 0xC5, 0x5D, 0x74, 0x18, 0x31, 0xF6, 0xCE, 0x5C, 0x3E, 0x16,
	0x9B, 0x87, 0x93, 0x1E, 0xAF, 0xD6, 0xBA, 0x33, 0x6C, 0x24, 0xCF, 0x5C,
	0x7A, 0x32, 0x53, 0x81, 0x28, 0x95, 0x86, 0x77, 0x3B, 0x8F, 0x48, 0x98,
	0x6B, 0x4B, 0xB9, 0xAF, 0xC4, 0xBF, 0xE8, 0x1B, 0x66, 0x28, 0x21, 0x93,
	0x61, 0xD8, 0x09, 0xCC, 0xFB, 0x21, 0xA9, 0x91, 0x48, 0x7C, 0xAC, 0x60,
	0x5D, 0xEC, 0x80, 0x32, 0xEF, 0x84, 0x5D, 0x5D, 0xE9, 0x85, 0x75, 0xB1,
	0xDC, 0x26, 0x23, 0x02, 0xEB, 0x65, 0x1B, 0x88, 0x23, 0x89, 0x3E, 0x81,
	0xD3, 0x96, 0xAC, 0xC5, 0x0F, 0x6D, 0x6F, 0xF3, 0x83, 0xF4, 0x42, 0x39,
	0x2E, 0x0B, 0x44, 0x82, 0xA4, 0x84, 0x20, 0x04, 0x69, 0xC8, 0xF0, 0x4A,
	0x9E, 0x1F, 0x9B, 0x5E, 0x21, 0xC6, 0x68, 0x42, 0xF6, 0xE9, 0x6C, 0x9A,
	0x67, 0x0C, 0x9C, 0x61, 0xAB, 0xD3, 0x88, 0xF0, 0x6A, 0x51, 0xA0, 0xD2,
	0xD8, 0x54, 0x2F, 0x68, 0x96, 0x0F, 0xA7, 0x28, 0xAB, 0x51, 0x33, 0xA3,
	0x6E, 0xEF, 0x0B, 0x6C, 0x13, 0x7A, 0x3B, 0xE4, 0xBA, 0x3B, 0xF0, 0x50,
	0x7E, 0xFB, 0x2A, 0x98, 0xA1, 0xF1, 0x65, 0x1D, 0x39, 0xAF, 0x01, 0x76,
	0x66, 0xCA, 0x59, 0x3E, 0x82, 0x43, 0x0E, 0x88, 0x8C, 0xEE, 0x86, 0x19,
	0x45, 0x6F, 0x9F, 0xB4, 0x7D, 0x84, 0xA5, 0xC3, 0x3B, 0x8B, 0x5E, 0xBE,
	0xE0, 0x6F, 0x75, 0xD8, 0x85, 0xC1, 0x20, 0x73, 0x40, 0x1A, 0x44, 0x9F,
	0x56, 0xC1, 0x6A, 0xA6, 0x4E, 0xD3, 0xAA, 0x62, 0x36, 0x3F, 0x77, 0x06,
	0x1B, 0xFE, 0xDF, 0x72, 0x42, 0x9B, 0x02, 0x3D, 0x37, 0xD0, 0xD7, 0x24,
	0xD0, 0x0A, 0x12, 0x48, 0xDB, 0x0F, 0xEA, 0xD3, 0x49, 0xF1, 0xC0, 0x9B,
	0x07, 0x53, 0x72, 0xC9, 0x80, 0x99, 0x1B, 0x7B, 0x25, 0xD4, 0x79, 0xD8,
	0xF6, 0xE8, 0xDE, 0xF7, 0xE3, 0xFE, 0x50, 0x1A, 0xB6, 0x79, 0x4C, 0x3B,
	0x97, 0x6C, 0xE0, 0xBD, 0x04, 0xC0, 0x06, 0xBA, 0xC1, 0xA9, 0x4F, 0xB6,
	0x40, 0x9F, 0x60, 0xC4, 0x5E, 0x5C, 0x9E, 0xC2, 0x19, 0x6A, 0x24, 0x63,
	0x68, 0xFB, 0x6F, 0xAF, 0x3E, 0x6C, 0x53, 0xB5, 0x13, 0x39, 0xB2, 0xEB,
	0x3B, 0x52, 0xEC, 0x6F, 0x6D, 0xFC, 0x51, 0x1F, 0x9B, 0x30, 0x95, 0x2C,
	0xCC, 0x81, 0x45, 0x44, 0xAF, 0x5E, 0xBD, 0x09, 0xBE, 0xE3, 0xD0, 0x04,
	0xDE, 0x33, 0x4A, 0xFD, 0x66, 0x0F, 0x28, 0x07, 0x19, 0x2E, 0x4B, 0xB3,
	0xC0, 0xCB, 0xA8, 0x57, 0x45, 0xC8, 0x74, 0x0F, 0xD2, 0x0B, 0x5F, 0x39,
	0xB9, 0xD3, 0xFB, 0xDB, 0x55, 0x79, 0xC0, 0xBD, 0x1A, 0x60, 0x32, 0x0A,
	0xD6, 0xA1, 0x00, 0xC6, 0x40, 0x2C, 0x72, 0x79, 0x67, 0x9F, 0x25, 0xFE,
	0xFB, 0x1F, 0xA3, 0xCC, 0x8E, 0xA5, 0xE9, 0xF8, 0xDB, 0x32, 0x22, 0xF8,
	0x3C, 0x75, 0x16, 0xDF, 0xFD, 0x61, 0x6B, 0x15, 0x2F, 0x50, 0x1E, 0xC8,
	0xAD, 0x05, 0x52, 0xAB, 0x32, 0x3D, 0xB5, 0xFA, 0xFD, 0x23, 0x87, 0x60,
	0x53, 0x31, 0x7B, 0x48, 0x3E, 0x00, 0xDF, 0x82, 0x9E, 0x5C, 0x57, 0xBB,
	0xCA, 0x6F, 0x8C, 0xA0, 0x1A, 0x87, 0x56, 0x2E, 0xDF, 0x17, 0x69, 0xDB,
	0xD5, 0x42, 0xA8, 0xF6, 0x28, 0x7E, 0xFF, 0xC3, 0xAC, 0x67, 0x32, 0xC6,
	0x8C, 0x4F, 0x55, 0x73, 0x69, 0x5B, 0x27, 0xB0, 0xBB, 0xCA, 0x58, 0xC8,
	0xE1, 0xFF, 0xA3, 0x5D, 0xB8, 0xF0, 0x11, 0xA0, 0x10, 0xFA, 0x3D, 0x98,
	0xFD, 0x21, 0x83, 0xB8, 0x4A, 0xFC, 0xB5, 0x6C, 0x2D, 0xD1, 0xD3, 0x5B,
	0x9A, 0x53, 0xE4, 0x79, 0xB6, 0xF8, 0x45, 0x65, 0xD2, 0x8E, 0x49, 0xBC,
	0x4B, 0xFB, 0x97, 0x90, 0xE1, 0xDD, 0xF2, 0xDA, 0xA4, 0xCB, 0x7E, 0x33,
	0x62, 0xFB, 0x13, 0x41, 0xCE, 0xE4, 0xC6, 0xE8, 0xEF, 0x20, 0xCA, 0xDA,
	0x36, 0x77, 0x4C, 0x01, 0xD0, 0x7E, 0x9E, 0xFE, 0x2B, 0xF1, 0x1F, 0xB4,
	0x95, 0xDB, 0xDA, 0x4D, 0xAE, 0x90, 0x91, 0x98, 0xEA, 0xAD, 0x8E, 0x71,
	0x6B, 0x93, 0xD5, 0xA0, 0xD0, 0x8E, 0xD1, 0xD0, 0xAF, 0xC7, 0x25, 0xE0,
	0x8E, 0x3C, 0x5B, 0x2F, 0x8E, 0x75, 0x94, 0xB7, 0x8F, 0xF6, 0xE2, 0xFB,
	0xF2, 0x12, 0x2B, 0x64, 0x88, 0x88, 0xB8, 0x12, 0x90, 0x0D, 0xF0, 0x1C,
	0x4F, 0xAD, 0x5E, 0xA0, 0x68, 0x8F, 0xC3, 0x1C, 0xD1, 0xCF, 0xF1, 0x91,
	0xB3, 0xA8, 0xC1, 0xAD, 0x2F, 0x2F, 0x22, 0x18, 0xBE, 0x0E, 0x17, 0x77,
	0xEA, 0x75, 0x2D, 0xFE, 0x8B, 0x02, 0x1F, 0xA1, 0xE5, 0xA0, 0xCC, 0x0F,
	0xB5, 0x6F, 0x74, 0xE8, 0x18, 0xAC, 0xF3, 0xD6, 0xCE, 0x89, 0xE2, 0x99,
	0xB4, 0xA8, 0x4F, 0xE0, 0xFD, 0x13, 0xE0, 0xB7, 0x7C, 0xC4, 0x3B, 0x81,
	0xD2, 0xAD, 0xA8, 0xD9, 0x16, 0x5F, 0xA2, 0x66, 0x80, 0x95, 0x77, 0x05,
	0x93, 0xCC, 0x73, 0x14, 0x21, 0x1A, 0x14, 0x77, 0xE6, 0xAD, 0x20, 0x65,
	0x77, 0xB5, 0xFA, 0x86, 0xC7, 0x54, 0x42, 0xF5, 0xFB, 0x9D, 0x35, 0xCF,
	0xEB, 0xCD, 0xAF, 0x0C, 0x7B, 0x3E, 0x89, 0xA0, 0xD6, 0x41, 0x1B, 0xD3,
	0xAE, 0x1E, 0x7E, 0x49, 0x00, 0x25, 0x0E, 0x2D, 0x20, 0x71, 0xB3, 0x5E,
	0x22, 0x68, 0x00, 0xBB, 0x57, 0xB8, 0xE0, 0xAF, 0x24, 0x64, 0x36, 0x9B,
	0xF0, 0x09, 0xB9, 0x1E, 0x55, 0x63, 0x91, 0x1D, 0x59, 0xDF, 0xA6, 0xAA,
	0x78, 0xC1, 0x43, 0x89, 0xD9, 0x5A, 0x53, 0x7F, 0x20, 0x7D, 0x5B, 0xA2,
	0x02, 0xE5, 0xB9, 0xC5, 0x83, 0x26, 0x03, 0x76, 0x62, 0x95, 0xCF, 0xA9,
	0x11, 0xC8, 0x19, 0x68, 0x4E, 0x73, 0x4A, 0x41, 0xB3, 0x47, 0x2D, 0xCA,
	0x7B, 0x14, 0xA9, 0x4A, 0x1B, 0x51, 0x00, 0x52, 0x9A, 0x53, 0x29, 0x15,
	0xD6, 0x0F, 0x57, 0x3F, 0xBC, 0x9B, 0xC6, 0xE4, 0x2B, 0x60, 0xA4, 0x76,
	0x81, 0xE6, 0x74, 0x00, 0x08, 0xBA, 0x6F, 0xB5, 0x57, 0x1B, 0xE9, 0x1F,
	0xF2, 0x96, 0xEC, 0x6B, 0x2A, 0x0D, 0xD9, 0x15, 0xB6, 0x63, 0x65, 0x21,
	0xE7, 0xB9, 0xF9, 0xB6, 0xFF, 0x34, 0x05, 0x2E, 0xC5, 0x85, 0x56, 0x64,
	0x53, 0xB0, 0x2D, 0x5D, 0xA9, 0x9F, 0x8F, 0xA1, 0x08, 0xBA, 0x47, 0x99,
	0x6E, 0x85, 0x07, 0x6A, 0x4B, 0x7A, 0x70, 0xE9, 0xB5, 0xB3, 0x29, 0x44,
	0xDB, 0x75, 0x09, 0x2E, 0xC4, 0x19, 0x26, 0x23, 0xAD, 0x6E, 0xA6, 0xB0,
	0x49, 0xA7, 0xDF, 0x7D, 0x9C, 0xEE, 0x60, 0xB8, 0x8F, 0xED, 0xB2, 0x66,
	0xEC, 0xAA, 0x8C, 0x71, 0x69, 0x9A, 0x18, 0xFF, 0x56, 0x64, 0x52, 0x6C,
	0xC2, 0xB1, 0x9E, 0xE1, 0x19, 0x36, 0x02, 0xA5, 0x75, 0x09, 0x4C, 0x29,
	0xA0, 0x59, 0x13, 0x40, 0xE4, 0x18, 0x3A, 0x3E, 0x3F, 0x54, 0x98, 0x9A,
	0x5B, 0x42, 0x9D, 0x65, 0x6B, 0x8F, 0xE4, 0xD6, 0x99, 0xF7, 0x3F, 0xD6,
	0xA1, 0xD2, 0x9C, 0x07, 0xEF, 0xE8, 0x30, 0xF5, 0x4D, 0x2D, 0x38, 0xE6,
	0xF0, 0x25, 0x5D, 0xC1, 0x4C, 0xDD, 0x20, 0x86, 0x84, 0x70, 0xEB, 0x26,
	0x63, 0x82, 0xE9, 0xC6, 0x02, 0x1E, 0xCC, 0x5E, 0x09, 0x68, 0x6B, 0x3F,
	0x3E, 0xBA, 0xEF, 0xC9, 0x3C, 0x97, 0x18, 0x14, 0x6B, 0x6A, 0x70, 0xA1,
	0x68, 0x7F, 0x35, 0x84, 0x52, 0xA0, 0xE2, 0x86, 0xB7, 0x9C, 0x53, 0x05,
	0xAA, 0x50, 0x07, 0x37, 0x3E, 0x07, 0x84, 0x1C, 0x7F, 0xDE, 0xAE, 0x5C,
	0x8E, 0x7D, 0x44, 0xEC, 0x57, 0x16, 0xF2, 0xB8, 0xB0, 0x3A, 0xDA, 0x37,
	0xF0, 0x50, 0x0C, 0x0D, 0xF0, 0x1C, 0x1F, 0x04, 0x02, 0x00, 0xB3, 0xFF,
	0xAE, 0x0C, 0xF5, 0x1A, 0x3C, 0xB5, 0x74, 0xB2, 0x25, 0x83, 0x7A, 0x58,
	0xDC, 0x09, 0x21, 0xBD, 0xD1, 0x91, 0x13, 0xF9, 0x7C, 0xA9, 0x2F, 0xF6,
	0x94, 0x32, 0x47, 0x73, 0x22, 0xF5, 0x47, 0x01, 0x3A, 0xE5, 0xE5, 0x81,
	0x37, 0xC2, 0xDA, 0xDC, 0xC8, 0xB5, 0x76, 0x34, 0x9A, 0xF3, 0xDD, 0xA7,
	0xA9, 0x44, 0x61, 0x46, 0x0F, 0xD0, 0x03, 0x0E, 0xEC, 0xC8, 0xC7, 0x3E,
	0xA4, 0x75, 0x1E, 0x41, 0xE2, 0x38, 0xCD, 0x99, 0x3B, 0xEA, 0x0E, 0x2F,
	0x32, 0x80, 0xBB, 0xA1, 0x18, 0x3E, 0xB3, 0x31, 0x4E, 0x54, 0x8B, 0x38,
	0x4F, 0x6D, 0xB9, 0x08, 0x6F, 0x42, 0x0D, 0x03, 0xF6, 0x0A, 0x04, 0xBF,
	0x2C, 0xB8, 0x12, 0x90, 0x24, 0x97, 0x7C, 0x79, 0x56, 0x79, 0xB0, 0x72,
	0xBC, 0xAF, 0x89, 0xAF, 0xDE, 0x9A, 0x77, 0x1F, 0xD9, 0x93, 0x08, 0x10,
	0xB3, 0x8B, 0xAE, 0x12, 0xDC, 0xCF, 0x3F, 0x2E, 0x55, 0x12, 0x72, 0x1F,
	0x2E, 0x6B, 0x71, 0x24, 0x50, 0x1A, 0xDD, 0xE6, 0x9F, 0x84, 0xCD, 0x87,
	0x7A, 0x58, 0x47, 0x18, 0x74, 0x08, 0xDA, 0x17, 0xBC, 0x9F, 0x9A, 0xBC,
	0xE9, 0x4B, 0x7D, 0x8C, 0xEC, 0x7A, 0xEC, 0x3A, 0xDB, 0x85, 0x1D, 0xFA,
	0x63, 0x09, 0x43, 0x66, 0xC4, 0x64, 0xC3, 0xD2, 0xEF, 0x1C, 0x18, 0x47,
	0x32, 0x15, 0xD8, 0x08, 0xDD, 0x43, 0x3B, 0x37, 0x24, 0xC2, 0xBA, 0x16,
	0x12, 0xA1, 0x4D, 0x43, 0x2A, 0x65, 0xC4, 0x51, 0x50, 0x94, 0x00, 0x02,
	0x13, 0x3A, 0xE4, 0xDD, 0x71, 0xDF, 0xF8, 0x9E, 0x10, 0x31, 0x4E, 0x55,
	0x81, 0xAC, 0x77, 0xD6, 0x5F, 0x11, 0x19, 0x9B, 0x04, 0x35, 0x56, 0xF1,
	0xD7, 0xA3, 0xC7, 0x6B, 0x3C, 0x11, 0x18, 0x3B, 0x59, 0x24, 0xA5, 0x09,
	0xF2, 0x8F, 0xE6, 0xED, 0x97, 0xF1, 0xFB, 0xFA, 0x9E, 0xBA, 0xBF, 0x2C,
	0x1E, 0x15, 0x3C, 0x6E, 0x86, 0xE3, 0x45, 0x70, 0xEA, 0xE9, 0x6F, 0xB1,
	0x86, 0x0E, 0x5E, 0x0A, 0x5A, 0x3E, 0x2A, 0xB3, 0x77, 0x1F, 0xE7, 0x1C,
	0x4E, 0x3D, 0x06, 0xFA, 0x29, 0x65, 0xDC, 0xB9, 0x99, 0xE7, 0x1D, 0x0F,
	0x80, 0x3E, 0x89, 0xD6, 0x52, 0x66, 0xC8, 0x25, 0x2E, 0x4C, 0xC9, 0x78,
	0x9C, 0x10, 0xB3, 0x6A, 0xC6, 0x15, 0x0E, 0xBA, 0x94, 0xE2, 0xEA, 0x78,
	0xA6, 0xFC, 0x3C, 0x53, 0x1E, 0x0A, 0x2D, 0xF4, 0xF2, 0xF7, 0x4E, 0xA7,
	0x36, 0x1D, 0x2B, 0x3D, 0x19, 0x39, 0x26, 0x0F, 0x19, 0xC2, 0x79, 0x60,
	0x52, 0x23, 0xA7, 0x08, 0xF7, 0x13, 0x12, 0xB6, 0xEB, 0xAD, 0xFE, 0x6E,
	0xEA, 0xC3, 0x1F, 0x66, 0xE3, 0xBC, 0x45, 0x95, 0xA6, 0x7B, 0xC8, 0x83,
	0xB1, 0x7F, 0x37, 0xD1, 0x01, 0x8C, 0xFF, 0x28, 0xC3, 0x32, 0xDD, 0xEF,
	0xBE, 0x6C, 0x5A, 0xA5, 0x65, 0x58, 0x21, 0x85, 0x68, 0xAB, 0x97, 0x02,
	0xEE, 0xCE, 0xA5, 0x0F, 0xDB, 0x2F, 0x95, 0x3B, 0x2A, 0xEF, 0x7D, 0xAD,
	0x5B, 0x6E, 0x2F, 0x84, 0x15, 0x21, 0xB6, 0x28, 0x29, 0x07, 0x61, 0x70,
	0xEC, 0xDD, 0x47, 0x75, 0x61, 0x9F, 0x15, 0x10, 0x13, 0xCC, 0xA8, 0x30,
	0xEB, 0x61, 0xBD, 0x96, 0x03, 0x34, 0xFE, 0x1E, 0xAA, 0x03, 0x63, 0xCF,
	0xB5, 0x73, 0x5C, 0x90, 0x4C, 0x70, 0xA2, 0x39, 0xD5, 0x9E, 0x9E, 0x0B,
	0xCB, 0xAA, 0xDE, 0x14, 0xEE, 0xCC, 0x86, 0xBC, 0x60, 0x62, 0x2C, 0xA7,
	0x9C, 0xAB, 0x5C, 0xAB, 0xB2, 0xF3, 0x84, 0x6E, 0x64, 0x8B, 0x1E, 0xAF,
	0x19, 0xBD, 0xF0, 0xCA, 0xA0, 0x23, 0x69, 0xB9, 0x65, 0x5A, 0xBB, 0x50,
	0x40, 0x68, 0x5A, 0x32, 0x3C, 0x2A, 0xB4, 0xB3, 0x31, 0x9E, 0xE9, 0xD5,
	0xC0, 0x21, 0xB8, 0xF7, 0x9B, 0x54, 0x0B, 0x19, 0x87, 0x5F, 0xA0, 0x99,
	0x95, 0xF7, 0x99, 0x7E, 0x62, 0x3D, 0x7D, 0xA8, 0xF8, 0x37, 0x88, 0x9A,
	0x97, 0xE3, 0x2D, 0x77, 0x11, 0xED, 0x93, 0x5F, 0x16, 0x68, 0x12, 0x81,
	0x0E, 0x35, 0x88, 0x29, 0xC7, 0xE6, 0x1F, 0xD6, 0x96, 0xDE, 0xDF, 0xA1,
	0x78, 0x58, 0xBA, 0x99, 0x57, 0xF5, 0x84, 0xA5, 0x1B, 0x22, 0x72, 0x63,
	0x9B, 0x83, 0xC3, 0xFF, 0x1A, 0xC2, 0x46, 0x96, 0xCD, 0xB3, 0x0A, 0xEB,
	0x53, 0x2E, 0x30, 0x54, 0x8F, 0xD9, 0x48, 0xE4, 0x6D, 0xBC, 0x31, 0x28,
	0x58, 0xEB, 0xF2, 0xEF, 0x34, 0xC6, 0xFF, 0xEA, 0xFE, 0x28, 0xED, 0x61,
	0xEE, 0x7C, 0x3C, 0x73, 0x5D, 0x4A, 0x14, 0xD9, 0xE8, 0x64, 0xB7, 0xE3,
	0x42, 0x10, 0x5D, 0x14, 0x20, 0x3E, 0x13, 0xE0, 0x45, 0xEE, 0xE2, 0xB6,
	0xA3, 0xAA, 0xAB, 0xEA, 0xDB, 0x6C, 0x4F, 0x15, 0xFA, 0xCB, 0x4F, 0xD0,
	0xC7, 0x42, 0xF4, 0x42, 0xEF, 0x6A, 0xBB, 0xB5, 0x65, 0x4F, 0x3B, 0x1D,
	0x41, 0xCD, 0x21, 0x05, 0xD8, 0x1E, 0x79, 0x9E, 0x86, 0x85, 0x4D, 0xC7,
	0xE4, 0x4B, 0x47, 0x6A, 0x3D, 0x81, 0x62, 0x50, 0xCF, 0x62, 0xA1, 0xF2,
	0x5B, 0x8D, 0x26, 0x46, 0xFC, 0x88, 0x83, 0xA0, 0xC1, 0xC7, 0xB6, 0xA3,
	0x7F, 0x15, 0x24, 0xC3, 0x69, 0xCB, 0x74, 0x92, 0x47, 0x84, 0x8A, 0x0B,
	0x56, 0x92, 0xB2, 0x85, 0x09, 0x5B, 0xBF, 0x00, 0xAD, 0x19, 0x48, 0x9D,
	0x14, 0x62, 0xB1, 0x74, 0x23, 0x82, 0x0D, 0x00, 0x58, 0x42, 0x8D, 0x2A,
	0x0C, 0x55, 0xF5, 0xEA, 0x1D, 0xAD, 0xF4, 0x3E, 0x23, 0x3F, 0x70, 0x61,
	0x33, 0x72, 0xF0, 0x92, 0x8D, 0x93, 0x7E, 0x41, 0xD6, 0x5F, 0xEC, 0xF1,
	0x6C, 0x22, 0x3B, 0xDB, 0x7C, 0xDE, 0x37, 0x59, 0xCB, 0xEE, 0x74, 0x60,
	0x40, 0x85, 0xF2, 0xA7, 0xCE, 0x77, 0x32, 0x6E, 0xA6, 0x07, 0x80, 0x84,
	0x19, 0xF8, 0x50, 0x9E, 0xE8, 0xEF, 0xD8, 0x55, 0x61, 0xD9, 0x97, 0x35,
	0xA9, 0x69, 0xA7, 0xAA, 0xC5, 0x0C, 0x06, 0xC2, 0x5A, 0x04, 0xAB, 0xFC,
	0x80, 0x0B, 0xCA, 0xDC, 0x9E, 0x44, 0x7A, 0x2E, 0xC3, 0x45, 0x34, 0x84,
	0xFD, 0xD5, 0x67, 0x05, 0x0E, 0x1E, 0x9E, 0xC9, 0xDB, 0x73, 0xDB, 0xD3,
	0x10, 0x55, 0x88, 0xCD, 0x67, 0x5F, 0xDA, 0x79, 0xE3, 0x67, 0x43, 0x40,
	0xC5, 0xC4, 0x34, 0x65, 0x71, 0x3E, 0x38, 0xD8, 0x3D, 0x28, 0xF8, 0x9E,
	0xF1, 0x6D, 0xFF, 0x20, 0x15, 0x3E, 0x21, 0xE7, 0x8F, 0xB0, 0x3D, 0x4A,
	0xE6, 0xE3, 0x9F, 0x2B, 0xDB, 0x83, 0xAD, 0xF7, 0xE9, 0x3D, 0x5A, 0x68,
	0x94, 0x81, 0x40, 0xF7, 0xF6, 0x4C, 0x26, 0x1C, 0x94, 0x69, 0x29, 0x34,
	0x41, 0x15, 0x20, 0xF7, 0x76, 0x02, 0xD4, 0xF7, 0xBC, 0xF4, 0x6B, 0x2E,
	0xD4, 0xA1, 0x00, 0x68, 0xD4, 0x08, 0x24, 0x71, 0x33, 0x20, 0xF4, 0x6A,
	0x43, 0xB7, 0xD4, 0xB7, 0x50, 0x00, 0x61, 0xAF, 0x1E, 0x39, 0xF6, 0x2E,
	0x97, 0x24, 0x45, 0x46,
};

alignas(16) const unsigned char kRandenRoundKeys[kKeyBytes] = {
	0x44, 0x73, 0x70, 0x03, 0x2E, 0x8A, 0x19, 0x13, 0xD3, 0x08, 0xA3, 0x85,
	0x88, 0x6A, 0x3F, 0x24, 0x89, 0x6C, 0x4E, 0xEC, 0x98, 0xFA, 0x2E, 0x08,
	0xD0, 0x31, 0x9F, 0x29, 0x22, 0x38, 0x09, 0xA4, 0x6C, 0x0C, 0xE9, 0x34,
	0xCF, 0x66, 0x54, 0xBE, 0x77, 0x13, 0xD0, 0x38, 0xE6, 0x21, 0x28, 0x45,
	0x17, 0x09, 0x47, 0xB5, 0xB5, 0xD5, 0x84, 0x3F, 0xDD, 0x50, 0x7C, 0xC9,
	0xB7, 0x29, 0xAC, 0xC0, 0xAC, 0xB5, 0xDF, 0x98, 0xA6, 0x0B, 0x31, 0xD1,
	0x1B, 0xFB, 0x79, 0x89, 0xD9, 0xD5, 0x16, 0x92, 0x96, 0x7E, 0x26, 0x6A,
	0xED, 0xAF, 0xE1, 0xB8, 0xB7, 0xDF, 0x1A, 0xD0, 0xDB, 0x72, 0xFD, 0x2F,
	0xF7, 0x6C, 0x91, 0xB3, 0x47, 0x99, 0xA1, 0x24, 0x99, 0x7F, 0x2C, 0xF1,
	0x45, 0x90, 0x7C, 0xBA, 0x69, 0x4E, 0x57, 0x71, 0xD8, 0x20, 0x69, 0x63,
	0x16, 0xFC, 0x8E, 0x85, 0xE2, 0xF2, 0x01, 0x08, 0x58, 0xB6, 0x8E, 0x72,
	0x8F, 0x74, 0x95, 0x0D, 0x7E, 0x3D, 0x93, 0xF4, 0xA3, 0xFE, 0x58, 0xA4,
	0xB5, 0x59, 0x5A, 0xC2, 0x1D, 0xA4, 0x54, 0x7B, 0xEE, 0x4A, 0x15, 0x82,
	0x58, 0xCD, 0x8B, 0x71, 0xF0, 0x85, 0x60, 0x28, 0x23, 0xB0, 0xD1, 0xC5,
	0x13, 0x60, 0xF2, 0x2A, 0x39, 0xD5, 0x30, 0x9C, 0x0E, 0x18, 0x3A, 0x60,
	0xB0, 0xDC, 0x79, 0x8E, 0xEF, 0x38, 0xDB, 0xB8, 0x18, 0x79, 0x41, 0xCA,
	0x27, 0x4B, 0x31, 0xBD, 0xC1, 0x77, 0x15, 0xD7, 0x3E, 0x8A, 0x1E, 0xB0,
	0x8B, 0x0E, 0x9E, 0x6C, 0x94, 0xAB, 0x55, 0xAA, 0xF3, 0x25, 0x55, 0xE6,
	0x60, 0x5C, 0x60, 0x55, 0xDA, 0x2F, 0xAF, 0x78, 0xB6, 0x10, 0xAB, 0x2A,
	0x6A, 0x39, 0xCA, 0x55, 0x40, 0x14, 0xE8, 0x63, 0x62, 0x98, 0x48, 0x57,
	0x93, 0xE9, 0x72, 0x7C, 0xAF, 0x86, 0x54, 0xA1, 0xCE, 0xE8, 0x41, 0x11,
	0x34, 0x5C, 0xCC, 0xB4, 0xF6, 0x31, 0x18, 0x74, 0x5D, 0xC5, 0xA9, 0x2B,
	0x2A, 0xBC, 0x6F, 0x63, 0x11, 0x14, 0xEE, 0xB3, 0x5C, 0xCF, 0x24, 0x6C,
	0x33, 0xBA, 0xD6, 0xAF, 0x1E, 0x93, 0x87, 0x9B, 0x16, 0x3E, 0x5C, 0xCE,
	0xAF, 0xB9, 0x4B, 0x6B, 0x98, 0x48, 0x8F, 0x3B, 0x77, 0x86, 0x95, 0x28,
	0x81, 0x53, 0x32, 0x7A, 0x91, 0xA9, 0x21, 0xFB, 0xCC, 0x09, 0xD8, 0x61,
	0x93, 0x21, 0x28, 0x66, 0x1B, 0xE8, 0xBF, 0xC4, 0xB1, 0x75, 0x85, 0xE9,
	0x5D, 0x5D, 0x84, 0xEF, 0x32, 0x80, 0xEC, 0x5D, 0x60, 0xAC, 0x7C, 0x48,
	0xC5, 0xAC, 0x96, 0xD3, 0x81, 0x3E, 0x89, 0x23, 0x88, 0x1B, 0x65, 0xEB,
	0x02, 0x23, 0x26, 0xDC, 0x04, 0x20, 0x84, 0xA4, 0x82, 0x44, 0x0B, 0x2E,
	0x39, 0x42, 0xF4, 0x83, 0xF3, 0x6F, 0x6D, 0x0F, 0x9A, 0x6C, 0xE9, 0xF6,
	0x42, 0x68, 0xC6, 0x21, 0x5E, 0x9B, 0x1F, 0x9E, 0x4A, 0xF0, 0xC8, 0x69,
	0x68, 0x2F, 0x54, 0xD8, 0xD2, 0xA0, 0x51, 0x6A, 0xF0, 0x88, 0xD3, 0xAB,
	0x61, 0x9C, 0x0C, 0x67, 0xE4, 0x3B, 0x7A, 0x13, 0x6C, 0x0B, 0xEF, 0x6E,
	0xA3, 0x33, 0x51, 0xAB, 0x28, 0xA7, 0x0F, 0x96, 0x76, 0x01, 0xAF, 0x39,
	0x1D, 0x65, 0xF1, 0xA1, 0x98, 0x2A, 0xFB, 0x7E, 0x50, 0xF0, 0x3B, 0xBA,
	0xB4, 0x9F, 0x6F, 0x45, 0x19, 0x86, 0xEE, 0x8C, 0x88, 0x0E, 0x43, 0x82,
	0x3E, 0x59, 0xCA, 0x66, 0x73, 0x20, 0xC1, 0x85, 0xD8, 0x75, 0x6F, 0xE0,
	0xBE, 0x5E, 0x8B, 0x3B, 0xC3, 0xA5, 0x84, 0x7D, 0x06, 0x77, 0x3F, 0x36,
	0x62, 0xAA, 0xD3, 0x4E, 0xA6, 0x6A, 0xC1, 0x56, 0x9F, 0x44, 0x1A, 0x40,
	0x48, 0x12, 0x0A, 0xD0, 0x24, 0xD7, 0xD0, 0x37, 0x3D, 0x02, 0x9B, 0x42,
	0x72, 0xDF, 0xFE, 0x1B, 0x7B, 0x1B, 0x99, 0x80, 0xC9, 0x72, 0x53, 0x07,
	0x9B, 0xC0, 0xF1, 0x49, 0xD3, 0xEA, 0x0F, 0xDB, 0x3B, 0x4C, 0x79, 0xB6,
	0x1A, 0x50, 0xFE, 0xE3, 0xF7, 0xDE, 0xE8, 0xF6, 0xD8, 0x79, 0xD4, 0x25,
	0xC4, 0x60, 0x9F, 0x40, 0xB6, 0x4F, 0xA9, 0xC1, 0xBA, 0x06, 0xC0, 0x04,
	0xBD, 0xE0, 0x6C, 0x97, 0xB5, 0x53, 0x6C, 0x3E, 0xAF, 0x6F, 0xFB, 0x68,
	0x63, 0x24, 0x6A, 0x19, 0xC2, 0x9E, 0x5C, 0x5E, 0x2C, 0x95, 0x30, 0x9B,
	0x1F, 0x51, 0xFC, 0x6D, 0x6F, 0xEC, 0x52, 0x3B, 0xEB, 0xB2, 0x39, 0x13,
	0xFD, 0x4A, 0x33, 0xDE, 0x04, 0xD0, 0xE3, 0xBE, 0x09, 0xBD, 0x5E, 0xAF,
	0x44, 0x45, 0x81, 0xCC, 0x0F, 0x74, 0xC8, 0x45, 0x57, 0xA8, 0xCB, 0xC0,
	0xB3, 0x4B, 0x2E, 0x19, 0x07, 0x28, 0x0F, 0x66, 0x0A, 0x32, 0x60, 0x1A,
	0xBD, 0xC0, 0x79, 0x55, 0xDB, 0xFB, 0xD3, 0xB9, 0x39, 0x5F, 0x0B, 0xD2,
	0xCC, 0xA3, 0x1F, 0xFB, 0xFE, 0x25, 0x9F, 0x67, 0x79, 0x72, 0x2C, 0x40,
	0xC6, 0x00, 0xA1, 0xD6, 0x15, 0x6B, 0x61, 0xFD, 0xDF, 0x16, 0x75, 0x3C,
	0xF8, 0x22, 0x32, 0xDB, 0xF8, 0xE9, 0xA5, 0x8E, 0x60, 0x87, 0x23, 0xFD,
	0xFA, 0xB5, 0x3D, 0x32, 0xAB, 0x52, 0x05, 0xAD, 0xC8, 0x1E, 0x50, 0x2F,
	0xA0, 0x8C, 0x6F, 0xCA, 0xBB, 0x57, 0x5C, 0x9E, 0x82, 0xDF, 0x00, 0x3E,
	0x48, 0x7B, 0x31, 0x53, 0xC3, 0xFF, 0x7E, 0x28, 0xF6, 0xA8, 0x42, 0xD5,
	0xDB, 0x69, 0x17, 0xDF, 0x2E, 0x56, 0x87, 0x1A, 0xC8, 0x58, 0xCA, 0xBB,
	0xB0, 0x27, 0x5B, 0x69, 0x73, 0x55, 0x4F, 0x8C, 0xC6, 0x32, 0x67, 0xAC,
	0xB8, 0x83, 0x21, 0xFD, 0x98, 0x3D, 0xFA, 0x10, 0xA0, 0x11, 0xF0, 0xB8,
	0x5D, 0xA3, 0xFF, 0xE1, 0x65, 0x45, 0xF8, 0xB6, 0x79, 0xE4, 0x53, 0x9A,
	0x5B, 0xD3, 0xD1, 0x2D, 0x6C, 0xB5, 0xFC, 0x4A, 0x33, 0x7E, 0xCB, 0xA4,
	0xDA, 0xF2, 0xDD, 0xE1, 0x90, 0x97, 0xFB, 0x4B, 0xBC, 0x49, 0x8E, 0xD2,
	0x01, 0x4C, 0x77, 0x36, 0xDA, 0xCA, 0x20, 0xEF, 0xE8, 0xC6, 0xE4, 0xCE,
	0x41, 0x13, 0xFB, 0x62, 0x98, 0x91, 0x90, 0xAE, 0x4D, 0xDA, 0xDB, 0x95,
	0xB4, 0x1F, 0xF1, 0x2B, 0xFE, 0x9E, 0x7E, 0xD0, 0xE0, 0x25, 0xC7, 0xAF,
	0xD0, 0xD1, 0x8E, 0xD0, 0xA0, 0xD5, 0x93, 0x6B, 0x71, 0x8E, 0xAD, 0xEA,
	0x64, 0x2B, 0x12, 0xF2, 0xFB, 0xE2, 0xF6, 0x8F, 0xB7, 0x94, 0x75, 0x8E,
	0x2F, 0x5B, 0x3C, 0x8E, 0x1C, 0xC3, 0x8F, 0x68, 0xA0, 0x5E, 0xAD, 0x4F,
	0x1C, 0xF0, 0x0D, 0x90, 0x12, 0xB8, 0x88, 0x88, 0x77, 0x17, 0x0E, 0xBE,
	0x18, 0x22, 0x2F, 0x2F, 0xAD, 0xC1, 0xA8, 0xB3, 0x91, 0xF1, 0xCF, 0xD1,
	0xE8, 0x74, 0x6F, 0xB5, 0x0F, 0xCC, 0xA0, 0xE5, 0xA1, 0x1F, 0x02, 0x8B,
	0xFE, 0x2D, 0x75, 0xEA, 0xB7, 0xE0, 0x13, 0xFD, 0xE0, 0x4F, 0xA8, 0xB4,
	0x99, 0xE2, 0x89, 0xCE, 0xD6, 0xF3, 0xAC, 0x18, 0x05, 0x77, 0x95, 0x80,
	0x66, 0xA2, 0x5F, 0x16, 0xD9, 0xA8, 0xAD, 0xD2, 0x81, 0x3B, 0xC4, 0x7C,
	0x86, 0xFA, 0xB5, 0x77, 0x65, 0x20, 0xAD, 0xE6, 0x77, 0x14, 0x1A, 0x21,
	0x14, 0x73, 0xCC, 0x93, 0xA0, 0x89, 0x3E, 0x7B, 0x0C, 0xAF, 0xCD, 0xEB,
	0xCF, 0x35, 0x9D, 0xFB, 0xF5, 0x42, 0x54, 0xC7, 0x5E, 0xB3, 0x71, 0x20,
	0x2D, 0x0E, 0x25, 0x00, 0x49, 0x7E, 0x1E, 0xAE, 0xD3, 0x1B, 0x41, 0xD6,
	0x1E, 0xB9, 0x09, 0xF0, 0x9B, 0x36, 0x64, 0x24, 0xAF, 0xE0, 0xB8, 0x57,
	0xBB, 0x00, 0x68, 0x22, 0x7F, 0x53, 0x5A, 0xD9, 0x89, 0x43, 0xC1, 0x78,
	0xAA, 0xA6, 0xDF, 0x59, 0x1D, 0x91, 0x63, 0x55, 0xA9, 0xCF, 0x95, 0x62,
	0x76, 0x03, 0x26, 0x83, 0xC5, 0xB9, 0xE5, 0x02, 0xA2, 0x5B, 0x7D, 0x20,
	0x4A, 0xA9, 0x14, 0x7B, 0xCA, 0x2D, 0x47, 0xB3, 0x41, 0x4A, 0x73, 0x4E,
	0x68, 0x19, 0xC8, 0x11, 0xE4, 0xC6, 0x9B, 0xBC, 0x3F, 0x57, 0x0F, 0xD6,
	0x15, 0x29, 0x53, 0x9A, 0x52, 0x00, 0x51, 0x1B, 0x1F, 0xE9, 0x1B, 0x57,
	0xB5, 0x6F, 0xBA, 0x08, 0x00, 0x74, 0xE6, 0x81, 0x76, 0xA4, 0x60, 0x2B,
	0xB6, 0xF9, 0xB9, 0xE7, 0x21, 0x65, 0x63, 0xB6, 0x15, 0xD9, 0x0D, 0x2A,
	0x6B, 0xEC, 0x96, 0xF2, 0xA1, 0x8F, 0x9F, 0xA9, 0x5D, 0x2D, 0xB0, 0x53,
	0x64, 0x56, 0x85, 0xC5, 0x2E, 0x05, 0x34, 0xFF, 0x44, 0x29, 0xB3, 0xB5,
	0xE9, 0x70, 0x7A, 0x4B, 0x6A, 0x07, 0x85, 0x6E, 0x99, 0x47, 0xBA, 0x08,
	0x7D, 0xDF, 0xA7, 0x49, 0xB0, 0xA6, 0x6E, 0xAD, 0x23, 0x26, 0x19, 0xC4,
	0x2E, 0x09, 0x75, 0xDB, 0xFF, 0x18, 0x9A, 0x69, 0x71, 0x8C, 0xAA, 0xEC,
	0x66, 0xB2, 0xED, 0x8F, 0xB8, 0x60, 0xEE, 0x9C, 0x29, 0x4C, 0x09, 0x75,
	0xA5, 0x02, 0x36, 0x19, 0xE1, 0x9E, 0xB1, 0xC2, 0x6C, 0x52, 0x64, 0x56,
	0x65, 0x9D, 0x42, 0x5B, 0x9A, 0x98, 0x54, 0x3F, 0x3E, 0x3A, 0x18, 0xE4,
	0x40, 0x13, 0x59, 0xA0, 0xF5, 0x30, 0xE8, 0xEF, 0x07, 0x9C, 0xD2, 0xA1,
	0xD6, 0x3F, 0xF7, 0x99, 0xD6, 0xE4, 0x8F, 0x6B, 0x26, 0xEB, 0x70, 0x84,
	0x86, 0x20, 0xDD, 0x4C, 0xC1, 0x5D, 0x25, 0xF0, 0xE6, 0x38, 0x2D, 0x4D,
	0xC9, 0xEF, 0xBA, 0x3E, 0x3F, 0x6B, 0x68, 0x09, 0x5E, 0xCC, 0x1E, 0x02,
	0xC6, 0xE9, 0x82, 0x63, 0x86, 0xE2, 0xA0, 0x52, 0x84, 0x35, 0x7F, 0x68,
	0xA1, 0x70, 0x6A, 0x6B, 0x14, 0x18, 0x97, 0x3C, 0x5C, 0xAE, 0xDE, 0x7F,
	0x1C, 0x84, 0x07, 0x3E, 0x37, 0x07, 0x50, 0xAA, 0x05, 0x53, 0x9C, 0xB7,
	0x0D, 0x0C, 0x50, 0xF0, 0x37, 0xDA, 0x3A, 0xB0, 0xB8, 0xF2, 0x16, 0x57,
	0xEC, 0x44, 0x7D, 0x8E, 0xB2, 0x74, 0xB5, 0x3C, 0x1A, 0xF5, 0x0C, 0xAE,
	0xFF, 0xB3, 0x00, 0x02, 0x04, 0x1F, 0x1C, 0xF0, 0xF6, 0x2F, 0xA9, 0x7C,
	0xF9, 0x13, 0x91, 0xD1, 0xBD, 0x21, 0x09, 0xDC, 0x58, 0x7A, 0x83, 0x25,
	0xDC, 0xDA, 0xC2, 0x37, 0x81, 0xE5, 0xE5, 0x3A, 0x01, 0x47, 0xF5, 0x22,
	0x73, 0x47, 0x32, 0x94, 0x0E, 0x03, 0xD0, 0x0F, 0x46, 0x61, 0x44, 0xA9,
	0xA7, 0xDD, 0xF3, 0x9A, 0x34, 0x76, 0xB5, 0xC8, 0x2F, 0x0E, 0xEA, 0x3B,
	0x99, 0xCD, 0x38, 0xE2, 0x41, 0x1E, 0x75, 0xA4, 0x3E, 0xC7, 0xC8, 0xEC,
	0x08, 0xB9, 0x6D, 0x4F, 0x38, 0x8B, 0x54, 0x4E, 0x31, 0xB3, 0x3E, 0x18,
	0xA1, 0xBB, 0x80, 0x32, 0x79, 0x7C, 0x97, 0x24, 0x90, 0x12, 0xB8, 0x2C,
	0xBF, 0x04, 0x0A, 0xF6, 0x03, 0x0D, 0x42, 0x6F, 0x10, 0x08, 0x93, 0xD9,
	0x1F, 0x77, 0x9A, 0xDE, 0xAF, 0x89, 0xAF, 0xBC, 0x72, 0xB0, 0x79, 0x56,
	0x24, 0x71, 0x6B, 0x2E, 0x1F, 0x72, 0x12, 0x55, 0x2E, 0x3F, 0xCF, 0xDC,
	0x12, 0xAE, 0x8B, 0xB3, 0x17, 0xDA, 0x08, 0x74, 0x18, 0x47, 0x58, 0x7A,
	0x87, 0xCD, 0x84, 0x9F, 0xE6, 0xDD, 0x1A, 0x50, 0xFA, 0x1D, 0x85, 0xDB,
	0x3A, 0xEC, 0x7A, 0xEC, 0x8C, 0x7D, 0x4B, 0xE9, 0xBC, 0x9A, 0x9F, 0xBC,
	0x08, 0xD8, 0x15, 0x32, 0x47, 0x18, 0x1C, 0xEF, 0xD2, 0xC3, 0x64, 0xC4,
	0x66, 0x43, 0x09, 0x63, 0x51, 0xC4, 0x65, 0x2A, 0x43, 0x4D, 0xA1, 0x12,
	0x16, 0xBA, 0xC2, 0x24, 0x37, 0x3B, 0x43, 0xDD, 0x55, 0x4E, 0x31, 0x10,
	0x9E, 0xF8, 0xDF, 0x71, 0xDD, 0xE4, 0x3A, 0x13, 0x02, 0x00, 0x94, 0x50,
	0x6B, 0xC7, 0xA3, 0xD7, 0xF1, 0x56, 0x35, 0x04, 0x9B, 0x19, 0x11, 0x5F,
	0xD6, 0x77, 0xAC, 0x81, 0xFA, 0xFB, 0xF1, 0x97, 0xED, 0xE6, 0x8F, 0xF2,
	0x09, 0xA5, 0x24, 0x59, 0x3B, 0x18, 0x11, 0x3C, 0xB1, 0x6F, 0xE9, 0xEA,
	0x70, 0x45, 0xE3, 0x86, 0x6E, 0x3C, 0x15, 0x1E, 0x2C, 0xBF, 0xBA, 0x9E,
	0xFA, 0x06, 0x3D, 0x4E, 0x1C, 0xE7, 0x1F, 0x77, 0xB3, 0x2A, 0x3E, 0x5A,
	0x0A, 0x5E, 0x0E, 0x86, 0x25, 0xC8, 0x66, 0x52, 0xD6, 0x89, 0x3E, 0x80,
	0x0F, 0x1D, 0xE7, 0x99, 0xB9, 0xDC, 0x65, 0x29, 0x78, 0xEA, 0xE2, 0x94,
	0xBA, 0x0E, 0x15, 0xC6, 0x6A, 0xB3, 0x10, 0x9C, 0x78, 0xC9, 0x4C, 0x2E,
	0x3D, 0x2B, 0x1D, 0x36, 0xA7, 0x4E, 0xF7, 0xF2, 0xF4, 0x2D, 0x0A, 0x1E,
	0x53, 0x3C, 0xFC, 0xA6, 0xB6, 0x12, 0x13, 0xF7, 0x08, 0xA7, 0x23, 0x52,
	0x60, 0x79, 0xC2, 0x19, 0x0F, 0x26, 0x39, 0x19, 0x83, 0xC8, 0x7B, 0xA6,
	0x95, 0x45, 0xBC, 0xE3, 0x66, 0x1F, 0xC3, 0xEA, 0x6E, 0xFE, 0xAD, 0xEB,
	0xA5, 0x5A, 0x6C, 0xBE, 0xEF, 0xDD, 0x32, 0xC3, 0x28, 0xFF, 0x8C, 0x01,
	0xD1, 0x37, 0x7F, 0xB1, 0x3B, 0x95, 0x2F, 0xDB, 0x0F, 0xA5, 0xCE, 0xEE,
	0x02, 0x97, 0xAB, 0x68, 0x85, 0x21, 0x58, 0x65, 0x70, 0x61, 0x07, 0x29,
	0x28, 0xB6, 0x21, 0x15, 0x84, 0x2F, 0x6E, 0x5B, 0xAD, 0x7D, 0xEF, 0x2A,
	0x96, 0xBD, 0x61, 0xEB, 0x30, 0xA8, 0xCC, 0x13, 0x10, 0x15, 0x9F, 0x61,
	0x75, 0x47, 0xDD, 0xEC, 0x39, 0xA2, 0x70, 0x4C, 0x90, 0x5C, 0x73, 0xB5,
	0xCF, 0x63, 0x03, 0xAA, 0x1E, 0xFE, 0x34, 0x03, 0xA7, 0x2C, 0x62, 0x60,
	0xBC, 0x86, 0xCC, 0xEE, 0x14, 0xDE, 0xAA, 0xCB, 0x0B, 0x9E, 0x9E, 0xD5,
	0xCA, 0xF0, 0xBD, 0x19, 0xAF, 0x1E, 0x8B, 0x64, 0x6E, 0x84, 0xF3, 0xB2,
	0xAB, 0x5C, 0xAB, 0x9C, 0xB3, 0xB4, 0x2A, 0x3C, 0x32, 0x5A, 0x68, 0x40,
	0x50, 0xBB, 0x5A, 0x65, 0xB9, 0x69, 0x23, 0xA0, 0x99, 0xA0, 0x5F, 0x87,
	0x19, 0x0B, 0x54, 0x9B, 0xF7, 0xB8, 0x21, 0xC0, 0xD5, 0xE9, 0x9E, 0x31,
	0x77, 0x2D, 0xE3, 0x97, 0x9A, 0x88, 0x37, 0xF8, 0xA8, 0x7D, 0x3D, 0x62,
	0x7E, 0x99, 0xF7, 0x95, 0xD6, 0x1F, 0xE6, 0xC7, 0x29, 0x88, 0x35, 0x0E,
	0x81, 0x12, 0x68, 0x16, 0x5F, 0x93, 0xED, 0x11, 0x63, 0x72, 0x22, 0x1B,
	0xA5, 0x84, 0xF5, 0x57, 0x99, 0xBA, 0x58, 0x78, 0xA1, 0xDF, 0xDE, 0x96,
	0x54, 0x30, 0x2E, 0x53, 0xEB, 0x0A, 0xB3, 0xCD, 0x96, 0x46, 0xC2, 0x1A,
	0xFF, 0xC3, 0x83, 0x9B, 0xEA, 0xFF, 0xC6, 0x34, 0xEF, 0xF2, 0xEB, 0x58,
	0x28, 0x31, 0xBC, 0x6D, 0xE4, 0x48, 0xD9, 0x8F, 0xE3, 0xB7, 0x64, 0xE8,
	0xD9, 0x14, 0x4A, 0x5D, 0x73, 0x3C, 0x7C, 0xEE, 0x61, 0xED, 0x28, 0xFE,
	0xEA, 0xAB, 0xAA, 0xA3, 0xB6, 0xE2, 0xEE, 0x45, 0xE0, 0x13, 0x3E, 0x20,
	0x14, 0x5D, 0x10, 0x42, 0xB5, 0xBB, 0x6A, 0xEF, 0x42, 0xF4, 0x42, 0xC7,
	0xD0, 0x4F, 0xCB, 0xFA, 0x15, 0x4F, 0x6C, 0xDB, 0xC7, 0x4D, 0x85, 0x86,
	0x9E, 0x79, 0x1E, 0xD8, 0x05, 0x21, 0xCD, 0x41, 0x1D, 0x3B, 0x4F, 0x65,
	0x46, 0x26, 0x8D, 0x5B, 0xF2, 0xA1, 0x62, 0xCF, 0x50, 0x62, 0x81, 0x3D,
	0x6A, 0x47, 0x4B, 0xE4, 0x92, 0x74, 0xCB, 0x69, 0xC3, 0x24, 0x15, 0x7F,
	0xA3, 0xB6, 0xC7, 0xC1, 0xA0, 0x83, 0x88, 0xFC, 0x9D, 0x48, 0x19, 0xAD,
	0x00, 0xBF, 0x5B, 0x09, 0x85, 0xB2, 0x92, 0x56, 0x0B, 0x8A, 0x84, 0x47,
	0xEA, 0xF5, 0x55, 0x0C, 0x2A, 0x8D, 0x42, 0x58, 0x00, 0x0D, 0x82, 0x23,
	0x74, 0xB1, 0x62, 0x14, 0x41, 0x7E, 0x93, 0x8D, 0x92, 0xF0, 0x72, 0x33,
	0x61, 0x70, 0x3F, 0x23, 0x3E, 0xF4, 0xAD, 0x1D, 0x60, 0x74, 0xEE, 0xCB,
	0x59, 0x37, 0xDE, 0x7C, 0xDB, 0x3B, 0x22, 0x6C, 0xF1, 0xEC, 0x5F, 0xD6,
	0x9E, 0x50, 0xF8, 0x19, 0x84, 0x80, 0x07, 0xA6, 0x6E, 0x32, 0x77, 0xCE,
	0xA7, 0xF2, 0x85, 0x40, 0xC2, 0x06, 0x0C, 0xC5, 0xAA, 0xA7, 0x69, 0xA9,
	0x35, 0x97, 0xD9, 0x61, 0x55, 0xD8, 0xEF, 0xE8, 0x84, 0x34, 0x45, 0xC3,
	0x2E, 0x7A, 0x44, 0x9E, 0xDC, 0xCA, 0x0B, 0x80, 0xFC, 0xAB, 0x04, 0x5A,
	0xCD, 0x88, 0x55, 0x10, 0xD3, 0xDB, 0x73, 0xDB, 0xC9, 0x9E, 0x1E, 0x0E,
	0x05, 0x67, 0xD5, 0xFD, 0xD8, 0x38, 0x3E, 0x71, 0x65, 0x34, 0xC4, 0xC5,
	0x40, 0x43, 0x67, 0xE3, 0x79, 0xDA, 0x5F, 0x67, 0x4A, 0x3D, 0xB0, 0x8F,
	0xE7, 0x21, 0x3E, 0x15, 0x20, 0xFF, 0x6D, 0xF1, 0x9E, 0xF8, 0x28, 0x3D,
	0xF7, 0x40, 0x81, 0x94, 0x68, 0x5A, 0x3D, 0xE9, 0xF7, 0xAD, 0x83, 0xDB,
	0x2B, 0x9F, 0xE3, 0xE6, 0xF7, 0xD4, 0x02, 0x76, 0xF7, 0x20, 0x15, 0x41,
	0x34, 0x29, 0x69, 0x94, 0x1C, 0x26, 0x4C, 0xF6, 0x6A, 0xF4, 0x20, 0x33,
	0x71, 0x24, 0x08, 0xD4, 0x68, 0x00, 0xA1, 0xD4, 0x2E, 0x6B, 0xF4, 0xBC,
	0x46, 0x45, 0x24, 0x97, 0x2E, 0xF6, 0x39, 0x1E, 0xAF, 0x61, 0x00, 0x50,
	0xB7, 0xD4, 0xB7, 0x43,
};
}  // namespace random_internal
ABSL_NAMESPACE_END
}  // namespace absl
