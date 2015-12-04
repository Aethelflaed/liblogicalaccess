#include <iostream>
#include <gtest/gtest.h>
#include "logicalaccess/lla_fwd.hpp"
#include "pluginsreaderproviders/pcsc/atrparser.hpp"

using namespace logicalaccess;
TEST(test_atr_parser, bla)
{
    ASSERT_EQ("DESFire", ATRParser::guessCardType(
                             ByteVector{0x3b, 0x81, 0x80, 0x1, 0x80, 0x80}));

    ASSERT_EQ("Mifare1K", ATRParser::guessCardType(ByteVector{
                              0x3b, 0x8f, 0x80, 0x1, 0x80, 0x4f, 0xc, 0xa0, 0, 0, 3,
                              6, 3, 0, 1, 0, 0, 0, 0, 0x6a}));

    ASSERT_EQ("HIDiClass16KS", ATRParser::guessCardType(ByteVector{
                                   0x3b, 0x8f, 0x80, 1, 0x80, 0x4f, 0xc, 0xa0, 0, 0,
                                   3, 6, 0xa, 0, 0x1a, 0, 0, 0, 0, 0x78}));
    ASSERT_EQ("HIDiClass16KS",
              ATRParser::guessCardType("3B8F8001804F0CA0000003060A001A0000000078"));

    ASSERT_EQ("HIDiClass2KS",
              ATRParser::guessCardType("3B8F8001804F0CA0000003060A0018000000007A"));

    ASSERT_EQ("ISO15693",
              ATRParser::guessCardType("3B8F8001804F0CA0000003060B00140000000077"));
    ASSERT_EQ("HIDiClass8x2KS",
              ATRParser::guessCardType("3B8F8001804F0CA0000003060A001C000000007E"));
}
