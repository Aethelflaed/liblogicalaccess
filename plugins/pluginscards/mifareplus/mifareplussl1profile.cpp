/**
 * \file mifareplussl1profile.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus card profiles.
 */

#include <logicalaccess/logs.hpp>
#include "mifareplussl1profile.hpp"

namespace logicalaccess
{
    MifarePlusSL1Profile::MifarePlusSL1Profile()
        : MifarePlusProfile()
    {
        d_sectorKeys = new std::shared_ptr<MifarePlusKey>[(getNbSectors() + 1) * 2];
        for (unsigned int i = 0; i <= getNbSectors(); i++)
        {
            d_sectorKeys[i * 2] = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
            d_sectorKeys[i * 2 + 1] = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
        }
        d_switchL2Key = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        d_switchL3Key = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        d_SL1AesKey = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));

        clearKeys();
    }

    MifarePlusSL1Profile::~MifarePlusSL1Profile()
    {
        delete[] d_sectorKeys;
    }

    void MifarePlusSL1Profile::clearKeys()
    {
        for (unsigned int i = 0; i <= getNbSectors(); i++)
        {
            d_sectorKeys[i * 2].reset();
            d_sectorKeys[i * 2 + 1].reset();
        }
        d_switchL2Key.reset();
        d_switchL3Key.reset();
        d_SL1AesKey.reset();
    }

    std::shared_ptr<MifarePlusKey> MifarePlusSL1Profile::getKey(int index, MifarePlusKeyType keytype) const
    {
        if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
        {
            return std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
        }

        switch (keytype)
        {
        case KT_KEY_CRYPTO1_A:
        {
            return d_sectorKeys[index * 2];
        }
        case KT_KEY_CRYPTO1_B:
        {
            return d_sectorKeys[index * 2 + 1];
        }
        case KT_KEY_SWITCHL2:
        {
            return d_switchL2Key;
        }
        case KT_KEY_SWITCHL3:
        {
            return d_switchL3Key;
        }
        case KT_KEY_AESSL1:
        {
            return d_SL1AesKey;
        }
        default:
        {
            return std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        }
        }
    }

    void MifarePlusSL1Profile::setKey(int index, MifarePlusKeyType keytype, std::shared_ptr<MifarePlusKey> /*key*/)
    {
        if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
        }

        /*		switch (keytype)
        {
        case KT_KEY_CRYPTO1_A:
        {
        d_sectorKeys[index * 2] = key;

        break;
        }
        case KT_KEY_CRYPTO1_B:
        {
        d_sectorKeys[index * 2 + 1] = key;

        break;
        }
        case KT_KEY_SWITCHL2:
        {
        d_switchL2Key = key;

        break;
        }
        case KT_KEY_SWITCHL3:
        {
        d_switchL3Key = key;

        break;
        }
        case KT_KEY_AESSL1:
        {
        d_SL1AesKey = key;

        break;
        }
        }*/
    }

    bool MifarePlusSL1Profile::getKeyUsage(int index, MifarePlusKeyType keytype) const
    {
        if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
        }

        switch (keytype)
        {
        case KT_KEY_CRYPTO1_A:
        {
            return bool(d_sectorKeys[index * 2]);
        }
        case KT_KEY_CRYPTO1_B:
        {
            return bool(d_sectorKeys[index * 2 + 1]);
        }
        case KT_KEY_SWITCHL2:
        {
            return bool(d_switchL2Key);
        }
        case KT_KEY_SWITCHL3:
        {
            return bool(d_switchL3Key);
        }
        case KT_KEY_AESSL1:
        {
            return bool(d_SL1AesKey);
        }
        default:
        {
            return false;
        }
        }
    }

    void MifarePlusSL1Profile::setKeyUsage(int index, MifarePlusKeyType keytype, bool used)
    {
        if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
        }

       /* std::shared_ptr<MifarePlusKey> key;
        if (used && (keytype == KT_KEY_CRYPTO1_A || keytype == KT_KEY_CRYPTO1_B))
        {
            key = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
        }
        else if (used)
        {
            key = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        }

        switch (keytype)
        {
        case KT_KEY_CRYPTO1_A:
        {
        d_sectorKeys[index * 2] = key;
        }
        case KT_KEY_CRYPTO1_B:
        {
        d_sectorKeys[index * 2 + 1] = key;
        }
        case KT_KEY_SWITCHL2:
        {
        d_switchL2Key = key;
        }
        case KT_KEY_SWITCHL3:
        {
        d_switchL3Key = key;
        }
        case KT_KEY_AESSL1:
        {
        d_SL1AesKey = key;
        }
        }*/
    }

    void MifarePlusSL1Profile::setDefaultKeysAt(int index)
    {
        d_sectorKeys[index * 2].reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_CRYPTO1KEY, MIFARE_PLUS_CRYPTO1_KEY_SIZE, MIFARE_PLUS_CRYPTO1_KEY_SIZE));

        d_sectorKeys[index * 2 + 1].reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_CRYPTO1KEY, MIFARE_PLUS_CRYPTO1_KEY_SIZE, MIFARE_PLUS_CRYPTO1_KEY_SIZE));
    }

    void MifarePlusSL1Profile::setDefaultKeys()
    {
        for (unsigned int i = 0; i < getNbSectors(); i++)
        {
            setDefaultKeysAt(i);
        }
        d_switchL2Key.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
        d_switchL3Key.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
        d_SL1AesKey.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
    }

    void MifarePlusSL1Profile::setKeyAt(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> AccessInfo)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(AccessInfo, std::invalid_argument, "AccessInfo cannot be null.");

        std::shared_ptr<MifarePlusLocation> dfLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        std::shared_ptr<MifarePlusAccessInfo> dfAi = std::dynamic_pointer_cast<MifarePlusAccessInfo>(AccessInfo);

        EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a MifareLocation.");
        EXCEPTION_ASSERT_WITH_LOG(dfAi, std::invalid_argument, "AccessInfo must be a MifareAccessInfo.");

        if (!dfAi->keyA->isEmpty())
        {
            setKey(dfLocation->sector, KT_KEY_CRYPTO1_A, dfAi->keyA);
        }
        if (!dfAi->keyB->isEmpty())
        {
            setKey(dfLocation->sector, KT_KEY_CRYPTO1_B, dfAi->keyB);
        }
        if (!dfAi->keySwitchL2->isEmpty())
        {
            setKey(0, KT_KEY_SWITCHL2, dfAi->keySwitchL2);
        }
        if (!dfAi->keySwitchL3->isEmpty())
        {
            setKey(0, KT_KEY_SWITCHL3, dfAi->keySwitchL3);
        }
        if (!dfAi->keyAuthenticateSL1AES->isEmpty())
        {
            setKey(0, KT_KEY_AESSL1, dfAi->keyAuthenticateSL1AES);
        }
    }

    std::shared_ptr<AccessInfo> MifarePlusSL1Profile::createAccessInfo() const
    {
        std::shared_ptr<MifarePlusAccessInfo> ret;
        ret.reset(new MifarePlusAccessInfo(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
        return ret;
    }
}