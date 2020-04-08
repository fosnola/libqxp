/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <librevenge-stream/librevenge-stream.h>

#include "libqxp_utils.h"

namespace test
{

using libqxp::readFloat16;
using libqxp::readFraction;

using librevenge::RVNGStringStream;
using std::make_shared;

class UtilsTest : public CPPUNIT_NS::TestFixture
{
public:
  virtual void setUp() override;
  virtual void tearDown() override;

private:
  CPPUNIT_TEST_SUITE(UtilsTest);
  CPPUNIT_TEST(testFloat16);
  CPPUNIT_TEST(testReadFraction);
  CPPUNIT_TEST(testGetRemainingLength);
  CPPUNIT_TEST_SUITE_END();

private:
  void testFloat16();
  void testReadFraction();
  void testGetRemainingLength();
};

void UtilsTest::setUp()
{
}

void UtilsTest::tearDown()
{
}

void UtilsTest::testFloat16()
{
  uint8_t data[] =
  {
    0x0, 0x0,
    0xff, 0xff,
    0x88, 0xc6,
    0xc6, 0x88
  };
  auto stream = make_shared<RVNGStringStream>(data, sizeof(data));

  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, readFloat16(stream), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, readFloat16(stream), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.776, readFloat16(stream, false), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.776, readFloat16(stream, true), 0.001);
}

void UtilsTest::testReadFraction()
{
  uint8_t data[] =
  {
    0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x24, 0x0,
    0x0, 0x24, 0x0, 0x0,
    0x0, 0x80, 0xb0, 0x01,
    0x01, 0xb0, 0x80, 0x0,
    0x0, 0x80, 0xf7, 0xff,
    0xff, 0xf7, 0x80, 0x0,
  };
  auto stream = make_shared<RVNGStringStream>(data, sizeof(data));

  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, readFraction(stream), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(36.0, readFraction(stream, false), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(36.0, readFraction(stream, true), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(432.5, readFraction(stream, false), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(432.5, readFraction(stream, true), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-8.5, readFraction(stream, false), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-8.5, readFraction(stream, true), 0.001);
}

void UtilsTest::testGetRemainingLength()
{
  uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  auto stream = make_shared<RVNGStringStream>(data, sizeof(data));

  using libqxp::getRemainingLength;
  CPPUNIT_ASSERT_EQUAL(sizeof(data), size_t(getRemainingLength(stream)));
  stream->seek(2, librevenge::RVNG_SEEK_CUR);
  CPPUNIT_ASSERT_EQUAL(sizeof(data) - 2, size_t(getRemainingLength(stream)));
  stream->seek(2, librevenge::RVNG_SEEK_CUR);
  CPPUNIT_ASSERT_EQUAL(sizeof(data) - 4, size_t(getRemainingLength(stream)));
  stream->seek(0, librevenge::RVNG_SEEK_END);
  CPPUNIT_ASSERT_EQUAL(0ul, getRemainingLength(stream));
}

CPPUNIT_TEST_SUITE_REGISTRATION(UtilsTest);

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
