/** @file
 *  @brief Map string to idf normalisation code
 */
/* Warning: This file is generated by weight\collate-idf-norm - do not modify directly! */
/* Copyright (C) 2020 Dipanshu Garg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_IDF_NORM_DISPATCH_H
#define XAPIAN_INCLUDED_IDF_NORM_DISPATCH_H

static const unsigned char idf_norm_tab[] = {
    23,

      1,   1,   1,  37,   0,   7,  15,   1,
      1,   1,  24,   1,   1,   1,  53,  70,
      1,   1,   1,   1,   1,   1,  88,

    (1 - 1),
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::TFIDF), 'T','F','I','D','F',

    (1 - 1),
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::SQUARE), 'S','Q','U','A','R','E',

    (1 - 1),
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::PIVOTED), 'P','I','V','O','T','E','D',

    (1 - 1),
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::GLOBAL_FREQ), 'G','L','O','B','A','L','_','F','R','E','Q',

    (3 - 1),
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::FREQ), 'F','R','E','Q',
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::NONE), 'N','O','N','E',
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::PROB), 'P','R','O','B',

    (1 - 1),
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::LOG_GLOBAL_FREQ), 'L','O','G','_','G','L','O','B','A','L','_','F','R','E','Q',

    (1 - 1),
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::SQRT_GLOBAL_FREQ), 'S','Q','R','T','_','G','L','O','B','A','L','_','F','R','E','Q',

    (1 - 1),
    static_cast<unsigned char>(Xapian::TfIdfWeight::idf_norm::INCREMENTED_GLOBAL_FREQ), 'I','N','C','R','E','M','E','N','T','E','D','_','G','L','O','B','A','L','_','F','R','E','Q'
};

#endif
