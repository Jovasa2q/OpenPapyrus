/*
 * Copyright 2016-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * https://www.openssl.org/source/license.html
 * or in the file LICENSE in the source distribution.
 */
#ifndef __FUZZER_H
#define __FUZZER_H

#include <slib.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/stat.h>
#include <openssl/crypto.h>
#include "internal/o_dir.h"

int FuzzerTestOneInput(const uint8_t *buf, size_t len);
int FuzzerInitialize(int *argc, char ***argv);
void FuzzerCleanup(void);
void FuzzerSetRand(void);
void FuzzerClearRand(void);

#endif // __FUZZER_H
