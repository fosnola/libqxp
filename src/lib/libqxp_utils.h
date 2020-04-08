/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LIBQXP_UTILS_H
#define INCLUDED_LIBQXP_UTILS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
#include <memory>
#include <string>

#include <boost/cstdint.hpp>

#include <librevenge-stream/librevenge-stream.h>
#include <librevenge/librevenge.h>

#define QXP_EPSILON 1E-6
#define QXP_ALMOST_ZERO(m) (std::fabs(m) <= QXP_EPSILON)

#if defined(HAVE_FUNC_ATTRIBUTE_FORMAT)
#define QXP_ATTRIBUTE_PRINTF(fmt, arg) __attribute__((format(printf, fmt, arg)))
#else
#define QXP_ATTRIBUTE_PRINTF(fmt, arg)
#endif

#if defined(HAVE_CLANG_ATTRIBUTE_FALLTHROUGH)
#  define QXP_FALLTHROUGH [[clang::fallthrough]]
#elif defined(HAVE_GCC_ATTRIBUTE_FALLTHROUGH)
#  define QXP_FALLTHROUGH __attribute__((fallthrough))
#else
#  define QXP_FALLTHROUGH ((void) 0)
#endif

// do nothing with debug messages in a release compile
#ifdef DEBUG
namespace libqxp
{
void debugPrint(const char *format, ...) QXP_ATTRIBUTE_PRINTF(1, 2);
}

#define QXP_DEBUG_MSG(M) libqxp::debugPrint M
#define QXP_DEBUG(M) M
#else
#define QXP_DEBUG_MSG(M)
#define QXP_DEBUG(M)
#endif

#define QXP_NUM_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

namespace libqxp
{

struct QXPDummyDeleter
{
  void operator()(void *) {}
};

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

uint8_t readU8(librevenge::RVNGInputStream *input, bool = false);
uint16_t readU16(librevenge::RVNGInputStream *input, bool bigEndian=false);
uint32_t readU32(librevenge::RVNGInputStream *input, bool bigEndian=false);
uint64_t readU64(librevenge::RVNGInputStream *input, bool bigEndian=false);
int16_t readS16(librevenge::RVNGInputStream *input, bool bigEndian=false);
int32_t readS32(librevenge::RVNGInputStream *input, bool bigEndian=false);
double readFloat16(librevenge::RVNGInputStream *input, bool bigEndian=false);
double readFraction(librevenge::RVNGInputStream *input, bool bigEndian=false);

const unsigned char *readNBytes(librevenge::RVNGInputStream *input, unsigned long numBytes);

std::string readCString(librevenge::RVNGInputStream *input);
std::string readPascalString(librevenge::RVNGInputStream *input);
std::string readString(librevenge::RVNGInputStream *input, const unsigned length);
std::string readPlatformString(librevenge::RVNGInputStream *input, bool bigEndian=false);

void skip(librevenge::RVNGInputStream *input, unsigned long numBytes);

void seek(librevenge::RVNGInputStream *input, unsigned long pos);
void seekRelative(librevenge::RVNGInputStream *input, long pos);

unsigned long getRemainingLength(librevenge::RVNGInputStream *input);

uint8_t readU8(std::shared_ptr<librevenge::RVNGInputStream> input, bool = false);
uint16_t readU16(std::shared_ptr<librevenge::RVNGInputStream> input, bool bigEndian=false);
uint32_t readU32(std::shared_ptr<librevenge::RVNGInputStream> input, bool bigEndian=false);
uint64_t readU64(std::shared_ptr<librevenge::RVNGInputStream> input, bool bigEndian=false);
int16_t readS16(std::shared_ptr<librevenge::RVNGInputStream> input, bool bigEndian=false);
int32_t readS32(std::shared_ptr<librevenge::RVNGInputStream> input, bool bigEndian=false);
double readFloat16(std::shared_ptr<librevenge::RVNGInputStream> input, bool bigEndian=false);
double readFraction(std::shared_ptr<librevenge::RVNGInputStream> input, bool bigEndian=false);

const unsigned char *readNBytes(std::shared_ptr<librevenge::RVNGInputStream> input, unsigned long numBytes);

std::string readCString(std::shared_ptr<librevenge::RVNGInputStream> input);
std::string readPascalString(std::shared_ptr<librevenge::RVNGInputStream> input);
std::string readString(std::shared_ptr<librevenge::RVNGInputStream> input, const unsigned length);
std::string readPlatformString(std::shared_ptr<librevenge::RVNGInputStream> input, bool bigEndian=false);

void skip(std::shared_ptr<librevenge::RVNGInputStream> input, unsigned long numBytes);

void seek(std::shared_ptr<librevenge::RVNGInputStream> input, unsigned long pos);
void seekRelative(std::shared_ptr<librevenge::RVNGInputStream> input, long pos);

unsigned long getRemainingLength(const std::shared_ptr<librevenge::RVNGInputStream> &input);

double deg2rad(double value);
double normalizeDegAngle(double degAngle);
double normalizeRadAngle(double radAngle);

void appendCharacters(librevenge::RVNGString &text, const char *characters, const size_t size,
                      const char *encoding);

class EndOfStreamException
{
public:
  EndOfStreamException();
};

class GenericException
{
};

// parser exceptions

class FileAccessError
{
};

class ParseError
{
};

class UnsupportedFormat
{
};

} // namespace libqxp

#endif // INCLUDED_LIBQXP_UTILS_H

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
