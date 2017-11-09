/**
 * \file mifareultralightcomnikeyxx21commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C - Omnikey xx21.
 */

#include <logicalaccess/plugins/readers/pcsc/commands/mifareultralightcomnikeyxx21commands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>
#include <logicalaccess/cards/computermemorykeystorage.hpp>
#include <logicalaccess/cards/readermemorykeystorage.hpp>
#include <logicalaccess/cards/samkeystorage.hpp>

namespace logicalaccess
{
MifareUltralightCOmnikeyXX21Commands::MifareUltralightCOmnikeyXX21Commands()
    : MifareUltralightCPCSCCommands(CMD_MIFAREULTRALIGHTCOMNIKEYXX21)
{
}

MifareUltralightCOmnikeyXX21Commands::MifareUltralightCOmnikeyXX21Commands(std::string ct)
    : MifareUltralightCPCSCCommands(ct)
{
}

MifareUltralightCOmnikeyXX21Commands::~MifareUltralightCOmnikeyXX21Commands()
{
}

void MifareUltralightCOmnikeyXX21Commands::startGenericSession()
{
    ByteVector data;
    data.push_back(0x01);
    data.push_back(0x00);
    data.push_back(0x01);
    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xA0, 0x00, 0x07, static_cast<unsigned char>(data.size()), data);
}

void MifareUltralightCOmnikeyXX21Commands::stopGenericSession()
{
    ByteVector data;
    data.push_back(0x01);
    data.push_back(0x00);
    data.push_back(0x02);
    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xA0, 0x00, 0x07, static_cast<unsigned char>(data.size()), data);
}

ByteVector
MifareUltralightCOmnikeyXX21Commands::sendGenericCommand(const ByteVector &data)
{
    ByteVector wdata;
    wdata.push_back(0x01);
    wdata.push_back(0x00);
    wdata.push_back(0xF3);
    wdata.push_back(0x00);
    wdata.push_back(0x00);
    wdata.push_back(0x64);
    wdata.insert(wdata.end(), data.begin(), data.end());

    ByteVector ret = getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xA0, 0x00, 0x05, static_cast<unsigned char>(wdata.size()), wdata, 0x00);
    // Should return 00 00 [data] 90 00, otherwise we return the raw received buffer
    if (ret.size() < 4)
    {
        return ret;
    }

    // Remove 00 00 starting bytes
    return ByteVector(ret.begin() + 2, ret.end());
}
}