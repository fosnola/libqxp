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
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <librevenge-stream/librevenge-stream.h>

#include "libqxp_utils.h"
#include "QXPDetector.h"
#include "QXPHeader.h"
#include "QXPTextParser.h"
#include "QXPTypes.h"

#if !defined TEST_DATA_DIR
#error TEST_DATA_DIR not defined, cannot test
#endif

namespace test
{

using libqxp::QXPHeader;
using libqxp::QXPDetector;
using libqxp::QXPTextParser;
using libqxp::CharFormat;
using libqxp::ParagraphFormat;

using librevenge::RVNGInputStream;
using std::string;
using std::map;
using std::make_shared;
using std::shared_ptr;
using std::vector;

namespace
{

shared_ptr<QXPTextParser> createParser(const string &name)
{
  const shared_ptr<RVNGInputStream> input(new librevenge::RVNGFileStream((string(TEST_DATA_DIR) + "/" + name).c_str()));
  QXPDetector detector;
  detector.detect(input);
  CPPUNIT_ASSERT(detector.header());
  CPPUNIT_ASSERT(detector.input());
  CPPUNIT_ASSERT(detector.header()->load(detector.input()));
  return make_shared<QXPTextParser>(detector.input(), detector.header());
}

}

class QXPTextParserTest : public CPPUNIT_NS::TestFixture
{
public:
  virtual void setUp() override;
  virtual void tearDown() override;

private:
  CPPUNIT_TEST_SUITE(QXPTextParserTest);
  CPPUNIT_TEST(testParseText);
  CPPUNIT_TEST_SUITE_END();

private:
  void testParseText();
};

void QXPTextParserTest::setUp()
{
}

void QXPTextParserTest::tearDown()
{
}

void QXPTextParserTest::testParseText()
{
  string expectedText = "123" + string(252, 't') + "456";
  map<string, unsigned> fileBlockMap =
  {
    { "qxp33mac_text", 0xb },
    { "qxp33win_text.qxd", 0x10 },
    { "qxp4mac_text", 0x3f },
    { "qxp4win_text.qxd", 0x44 },
  };

  for (const auto &item : fileBlockMap)
  {
    const auto name = item.first;
    const auto index = item.second;

    vector<shared_ptr<CharFormat>> charFormats;
    charFormats.push_back(make_shared<CharFormat>(CharFormat()));
    vector<shared_ptr<ParagraphFormat>> paragraphFormats;
    paragraphFormats.push_back(make_shared<ParagraphFormat>(ParagraphFormat()));

    auto parser = createParser(name);
    auto text = parser->parseText(index, charFormats, paragraphFormats);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name, expectedText, text->text);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name, 258u, text->charFormats[0].length);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name, 0u, text->charFormats[0].startIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name, charFormats[0].get(), text->charFormats[0].format.get());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name, 258u, text->paragraphs[0].length);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name, 0u, text->paragraphs[0].startIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(name, paragraphFormats[0].get(), text->paragraphs[0].format.get());
  }
}

CPPUNIT_TEST_SUITE_REGISTRATION(QXPTextParserTest);

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
