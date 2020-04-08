/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <librevenge-stream/librevenge-stream.h>

#include <libqxp/libqxp.h>

#if !defined DETECTION_TEST_DIR
#error DETECTION_TEST_DIR not defined, cannot test
#endif

namespace test
{

using libqxp::QXPDocument;

using std::string;

namespace
{

QXPDocument::Type assertDetection(const string &name, const bool expectedSupported)
{
  librevenge::RVNGFileStream input((string(DETECTION_TEST_DIR) + "/" + name).c_str());
  QXPDocument::Type type = QXPDocument::TYPE_UNKNOWN;
  const bool supported = QXPDocument::isSupported(&input, &type);
  CPPUNIT_ASSERT_EQUAL_MESSAGE(name, expectedSupported, supported);
  return type;
}

void assertSupported(const string &name)
{
  auto type = assertDetection(name, true);
  CPPUNIT_ASSERT_EQUAL_MESSAGE(name, QXPDocument::TYPE_DOCUMENT, type);
}

void assertUnsupported(const string &name)
{
  assertDetection(name, false);
}

}

class QXPDocumentTest : public CPPUNIT_NS::TestFixture
{
public:
  virtual void setUp() override;
  virtual void tearDown() override;

private:
  CPPUNIT_TEST_SUITE(QXPDocumentTest);
  CPPUNIT_TEST(testDetectQXP);
  CPPUNIT_TEST(testUnsupported);
  CPPUNIT_TEST_SUITE_END();

private:
  void testDetectQXP();
  void testUnsupported();
};

void QXPDocumentTest::setUp()
{
}

void QXPDocumentTest::tearDown()
{
}

void QXPDocumentTest::testDetectQXP()
{
  assertSupported("qxp1.zip");
  assertSupported("qxp31mac");
  assertSupported("qxp31win.qxd");
  assertSupported("qxp33mac");
  assertSupported("qxp33win.qxd");
  assertSupported("qxp4mac");
  assertSupported("qxp4win.qxd");
}

void QXPDocumentTest::testUnsupported()
{
  assertUnsupported("unsupported.zip");
  assertUnsupported("qxp5.qxd");
  assertUnsupported("qxp6.qxd");
}

CPPUNIT_TEST_SUITE_REGISTRATION(QXPDocumentTest);

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
