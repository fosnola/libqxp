/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "libqxp_utils.h"
#include "QXP33Deobfuscator.h"
#include "QXP4Deobfuscator.h"

namespace test
{

using libqxp::QXP33Deobfuscator;
using libqxp::QXP4Deobfuscator;

class QXPDeobfuscatorTest : public CPPUNIT_NS::TestFixture
{
public:
  virtual void setUp() override;
  virtual void tearDown() override;

private:
  CPPUNIT_TEST_SUITE(QXPDeobfuscatorTest);
  CPPUNIT_TEST(test33Deobfuscation);
  CPPUNIT_TEST(test4Deobfuscation);
  CPPUNIT_TEST_SUITE_END();

private:
  void test33Deobfuscation();
  void test4Deobfuscation();
};

void QXPDeobfuscatorTest::setUp()
{
}

void QXPDeobfuscatorTest::tearDown()
{
}

void QXPDeobfuscatorTest::test33Deobfuscation()
{
  QXP33Deobfuscator deobfuscate(0x337c, 0x3797);

  uint8_t type = 0x7f;
  uint16_t content = 0x337c;
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), deobfuscate(type));
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), deobfuscate(content));
  deobfuscate.next();

  type = 0x10;
  content = 0x6b03;
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), deobfuscate(type));
  CPPUNIT_ASSERT_EQUAL(uint16_t(0x10), deobfuscate(content));
  deobfuscate.next();

  type = 0xa7;
  content = 0xa2aa;
  CPPUNIT_ASSERT_EQUAL(uint8_t(13), deobfuscate(type));
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), deobfuscate(content));
  deobfuscate.next();

  type = 0x42;
  content = 0xda41;
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), deobfuscate(type));
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), deobfuscate(content));
  deobfuscate.next();

  type = 0xdb;
  content = 0x11ca;
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), deobfuscate(type));
  CPPUNIT_ASSERT_EQUAL(uint16_t(0x12), deobfuscate(content));
  deobfuscate.next();

  type = 0x6c;
  content = 0x496f;
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), deobfuscate(type));
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), deobfuscate(content));
  deobfuscate.next();
}

void QXPDeobfuscatorTest::test4Deobfuscation()
{
  QXP4Deobfuscator deobfuscate(0x3c3e, 0xb3b7);

  uint16_t objectsCount = 0x3c3e;
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), deobfuscate(objectsCount));
  deobfuscate.nextRev();

  objectsCount = 0x8886;
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), deobfuscate(objectsCount));
  deobfuscate.nextRev();

  objectsCount = 0xd4cf;
  CPPUNIT_ASSERT_EQUAL(uint16_t(1), deobfuscate(objectsCount));
  deobfuscate.nextRev();

  uint8_t contentType = 0x15;
  uint16_t content = 0xc422;
  uint8_t shapeType = 0x27;
  contentType = deobfuscate(contentType);
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), contentType);
  deobfuscate.nextShift(contentType);
  content = deobfuscate(content);
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), content);
  CPPUNIT_ASSERT_EQUAL(uint8_t(5), deobfuscate(shapeType));
  deobfuscate.next(content);

  objectsCount = 0x77da;
  CPPUNIT_ASSERT_EQUAL(uint16_t(3), deobfuscate(objectsCount));
  deobfuscate.nextRev();

  contentType = 0x22;
  content = 0xf8c0;
  shapeType = 0x8c;
  contentType = deobfuscate(contentType);
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), contentType);
  deobfuscate.nextShift(contentType);
  content = deobfuscate(content);
  CPPUNIT_ASSERT_EQUAL(uint16_t(0x44), content);
  CPPUNIT_ASSERT_EQUAL(uint8_t(8), deobfuscate(shapeType));
  deobfuscate.next(content);

  contentType = 0x38;
  content = 0xf5c1;
  shapeType = 0x8f;
  contentType = deobfuscate(contentType);
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), contentType);
  deobfuscate.nextShift(contentType);
  content = deobfuscate(content);
  CPPUNIT_ASSERT_EQUAL(uint16_t(0x46), content);
  CPPUNIT_ASSERT_EQUAL(uint8_t(8), deobfuscate(shapeType));
  deobfuscate.next(content);

  contentType = 0xc1;
  content = 0xfe50;
  shapeType = 0x1d;
  contentType = deobfuscate(contentType);
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), contentType);
  deobfuscate.nextShift(contentType);
  content = deobfuscate(content);
  CPPUNIT_ASSERT_EQUAL(uint16_t(0x48), content);
  CPPUNIT_ASSERT_EQUAL(uint8_t(5), deobfuscate(shapeType));
  deobfuscate.next(content);

  objectsCount = 0xfe00;
  CPPUNIT_ASSERT_EQUAL(uint16_t(4), deobfuscate(objectsCount));
  deobfuscate.nextRev();

  contentType = 0x04;
  content = 0xfe04;
  shapeType = 0x06;
  contentType = deobfuscate(contentType);
  CPPUNIT_ASSERT_EQUAL(uint8_t(0), contentType);
  deobfuscate.nextShift(contentType);
  content = deobfuscate(content);
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), content);
  CPPUNIT_ASSERT_EQUAL(uint8_t(2), deobfuscate(shapeType));
  deobfuscate.next(content);

  contentType = 0x07;
  content = 0xffe0;
  shapeType = 0xe6;
  contentType = deobfuscate(contentType);
  CPPUNIT_ASSERT_EQUAL(uint8_t(4), contentType);
  deobfuscate.nextShift(contentType);
  content = deobfuscate(content);
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), content);
  CPPUNIT_ASSERT_EQUAL(uint8_t(6), deobfuscate(shapeType));
  deobfuscate.next(content);

  contentType = 0xdc;
  content = 0xfffb;
  shapeType = 0xf3;
  contentType = deobfuscate(contentType);
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), contentType);
  deobfuscate.nextShift(contentType);
  content = deobfuscate(content);
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), content);
  CPPUNIT_ASSERT_EQUAL(uint8_t(8), deobfuscate(shapeType));
  deobfuscate.next(content);

  contentType = 0xf9;
  content = 0xffff;
  shapeType = 0xfa;
  contentType = deobfuscate(contentType);
  CPPUNIT_ASSERT_EQUAL(uint8_t(3), contentType);
  deobfuscate.nextShift(contentType);
  content = deobfuscate(content);
  CPPUNIT_ASSERT_EQUAL(uint16_t(0), content);
  CPPUNIT_ASSERT_EQUAL(uint8_t(5), deobfuscate(shapeType));
  deobfuscate.next(content);
}

CPPUNIT_TEST_SUITE_REGISTRATION(QXPDeobfuscatorTest);

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
