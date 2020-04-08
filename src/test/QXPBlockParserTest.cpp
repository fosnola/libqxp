/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <string>
#include <map>
#include <memory>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <librevenge-stream/librevenge-stream.h>

#include "libqxp_utils.h"
#include "QXPDetector.h"
#include "QXPHeader.h"
#include "QXPBlockParser.h"

#if !defined TEST_DATA_DIR
#error TEST_DATA_DIR not defined, cannot test
#endif

namespace test
{

using libqxp::QXPDetector;
using libqxp::QXPHeader;
using libqxp::QXPBlockParser;
using libqxp::readString;
using libqxp::getRemainingLength;

using librevenge::RVNGInputStream;
using std::string;
using std::map;
using std::make_shared;
using std::shared_ptr;

namespace
{

shared_ptr<QXPBlockParser> createParser(const string &name)
{
  const shared_ptr<RVNGInputStream> input(new librevenge::RVNGFileStream((string(TEST_DATA_DIR) + "/" + name).c_str()));
  QXPDetector detector;
  detector.detect(input);
  CPPUNIT_ASSERT(detector.header());
  CPPUNIT_ASSERT(detector.input());
  CPPUNIT_ASSERT(detector.header()->load(detector.input()));
  return make_shared<QXPBlockParser>(detector.input(), detector.header());
}

}

class QXPBlockParserTest : public CPPUNIT_NS::TestFixture
{
public:
  virtual void setUp() override;
  virtual void tearDown() override;

private:
  CPPUNIT_TEST_SUITE(QXPBlockParserTest);
  CPPUNIT_TEST(testGetTextBlock);
  CPPUNIT_TEST(testGetDocChain);
  CPPUNIT_TEST_SUITE_END();

private:
  void testGetTextBlock();
  void testGetDocChain();
  void testUnsupported();
};

void QXPBlockParserTest::setUp()
{
}

void QXPBlockParserTest::tearDown()
{
}

void QXPBlockParserTest::testGetTextBlock()
{
  string expectedBlock1 = "123" + string(252, 't') + "4";
  string expectedBlock2 = "56";
  map<string, unsigned> fileBlockMap =
  {
    { "qxp33mac_text", 0xC },
    { "qxp33win_text.qxd", 0x11 },
    { "qxp4mac_text", 0x40 },
    { "qxp4win_text.qxd", 0x45 },
  };

  for (const auto &item : fileBlockMap)
  {
    const auto name = item.first;
    const auto index = item.second;

    auto parser = createParser(name);
    auto stream = parser->getBlock(index);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name + " block 1", expectedBlock1, readString(stream, 256));
    stream = parser->getBlock(index + 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name + " block 2", expectedBlock2, readString(stream, 2));
  }
}

void QXPBlockParserTest::testGetDocChain()
{
  map<string, unsigned long> fileDocSizeMap =
  {
    { "qxp33mac_text", 3786 },
    { "qxp33win_text.qxd", 5582 },
    { "qxp4mac_text", 17780 },
    { "qxp4win_text.qxd", 18800 },
  };

  for (const auto &item : fileDocSizeMap)
  {
    const auto name = item.first;
    const auto expectedSize = item.second;

    auto parser = createParser(name);
    auto stream = parser->getChain(3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name, expectedSize, getRemainingLength(stream));
  }
}

CPPUNIT_TEST_SUITE_REGISTRATION(QXPBlockParserTest);

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
