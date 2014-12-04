/**
 * \file mifarepcsccommands.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare PC/SC commands.
 */

#include "../commands/mifarepcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifarechip.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

namespace logicalaccess
{
    MifarePCSCCommands::MifarePCSCCommands()
        : MifareCommands()
    {
    }

    MifarePCSCCommands::~MifarePCSCCommands()
    {
    }

    bool MifarePCSCCommands::loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol)
    {
        bool r = false;

        std::vector<unsigned char> result, vector_key((unsigned char*)key, (unsigned char*)key + keylen);

        result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x82, (vol ? 0x00 : 0x20), static_cast<char>(keyno), static_cast<unsigned char>(vector_key.size()), vector_key);
        if (!vol && (result[result.size() - 2] == 0x63) && (result[result.size() - 1] == 0x86))
        {
            if (keyno == 0)
            {
                r = loadKey(keyno, keytype, key, keylen, true);
            }
        }
        else
        {
            r = true;
        }

        return r;
    }

    void MifarePCSCCommands::loadKey(std::shared_ptr<Location> location, std::shared_ptr<Key> key, MifareKeyType keytype)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(key, std::invalid_argument, "key cannot be null.");

        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        std::shared_ptr<MifareKey> mKey = std::dynamic_pointer_cast<MifareKey>(key);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
        EXCEPTION_ASSERT_WITH_LOG(mKey, std::invalid_argument, "key must be a MifareKey.");

        std::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();

        if (std::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
        {
            loadKey(0, keytype, key->getData(), key->getLength());
        }
        else if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
        {
            // Don't load the key when reader memory, except if specified
            if (!key->isEmpty())
            {
                std::shared_ptr<ReaderMemoryKeyStorage> rmKs = std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
                loadKey(rmKs->getKeySlot(), keytype, key->getData(), key->getLength(), rmKs->getVolatile());
            }
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
        }
    }

    void MifarePCSCCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
    {
        std::vector<unsigned char> command;

        command.push_back(static_cast<unsigned char>(keytype));
        command.push_back(keyno);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x88, 0x00, blockno, command);
    }

    void MifarePCSCCommands::authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
    {
        if (std::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
        {
            authenticate(blockno, 0, keytype);
        }
        else if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
        {
            std::shared_ptr<ReaderMemoryKeyStorage> rmKs = std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
            authenticate(blockno, rmKs->getKeySlot(), keytype);
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
        }
    }

    std::vector<unsigned char> MifarePCSCCommands::readBinary(unsigned char blockno, size_t len)
    {
        if (len >= 256)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad len parameter.");
        }

        std::vector<unsigned char> result;

        result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xB0, 0x00, blockno, static_cast<unsigned char>(len));
        return std::vector<unsigned char>(result.begin(), result.end() - 2);
    }

    void MifarePCSCCommands::updateBinary(unsigned char blockno, const std::vector<unsigned char>& buf)
    {
        if (buf.size() >= 256)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buf size parameter.");
        }

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD6, 0x00, blockno, static_cast<unsigned char>(buf.size()), buf);
    }
}