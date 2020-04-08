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

#include <string>

#include "QXPTypes.h"

namespace test
{

using libqxp::Color;

using std::string;

class QXPTypesTest : public CPPUNIT_NS::TestFixture
{
public:
  virtual void setUp() override;
  virtual void tearDown() override;

private:
  CPPUNIT_TEST_SUITE(QXPTypesTest);
  CPPUNIT_TEST(testColorShade);
  CPPUNIT_TEST_SUITE_END();

private:
  void testColorShade();
};

void QXPTypesTest::setUp()
{
}

void QXPTypesTest::tearDown()
{
}

void QXPTypesTest::testColorShade()
{
  CPPUNIT_ASSERT_EQUAL(string(Color(1, 160, 198).toString().cstr()), string(Color(1, 160, 198).applyShade(1.0).toString().cstr()));
  CPPUNIT_ASSERT_EQUAL(string(Color(255, 255, 255).toString().cstr()), string(Color(1, 160, 198).applyShade(0.0).toString().cstr()));
  CPPUNIT_ASSERT_EQUAL(string(Color(179, 227, 238).toString().cstr()), string(Color(1, 160, 198).applyShade(0.3).toString().cstr()));
  CPPUNIT_ASSERT_EQUAL(string(Color(1, 160, 198).toString().cstr()), string(Color(1, 160, 198).applyShade(99.8).toString().cstr()));
  CPPUNIT_ASSERT_EQUAL(string(Color(1, 160, 198).toString().cstr()), string(Color(1, 160, 198).applyShade(-99.8).toString().cstr()));
}

CPPUNIT_TEST_SUITE_REGISTRATION(QXPTypesTest);

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
