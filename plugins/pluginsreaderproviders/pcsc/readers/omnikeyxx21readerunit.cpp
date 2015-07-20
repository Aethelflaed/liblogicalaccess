/**
 * \file omnikeyxx21readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX21 reader unit.
 */

#include "../readers/omnikeyxx21readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/windowsregistry.hpp"

#include "../readers/omnikeyxx21readerunitconfiguration.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include "logicalaccess/myexception.hpp"

#if defined(__unix__)
#include <PCSC/reader.h>
#elif defined(__APPLE__)

#ifndef SCARD_CTL_CODE
#define SCARD_CTL_CODE(code) (0x42000000 + (code))
#endif

#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#endif

#define CM_IOCTL_GET_SET_RFID_BAUDRATE SCARD_CTL_CODE(3215)

namespace logicalaccess
{
    std::map<std::string, OmnikeyXX21ReaderUnit::SecureModeStatus>
        OmnikeyXX21ReaderUnit::secure_connection_status_;

    OmnikeyXX21ReaderUnit::OmnikeyXX21ReaderUnit(const std::string &name)
        : OmnikeyReaderUnit(name)
    {
        d_readerUnitConfig.reset(new OmnikeyXX21ReaderUnitConfiguration());
    }

    OmnikeyXX21ReaderUnit::~OmnikeyXX21ReaderUnit()
    {
    }

    PCSCReaderUnitType OmnikeyXX21ReaderUnit::getPCSCType() const
    {
        return PCSC_RUT_OMNIKEY_XX21;
    }

    bool OmnikeyXX21ReaderUnit::waitRemoval(unsigned int maxwait)
    {
        bool removed = OmnikeyReaderUnit::waitRemoval(maxwait);
        if (removed)
        {
            setSecureConnectionStatus(SecureModeStatus::DISABLED);
        }

        return removed;
    }

    std::shared_ptr<ReaderCardAdapter>
        OmnikeyXX21ReaderUnit::getReaderCardAdapter(std::string /*type*/)
    {
        return getDefaultReaderCardAdapter();
    }

    void OmnikeyXX21ReaderUnit::changeReaderKey(
        std::shared_ptr<ReaderMemoryKeyStorage> keystorage,
        const std::vector<unsigned char> &key)
    {
        EXCEPTION_ASSERT_WITH_LOG(keystorage, std::invalid_argument,
                                  "Key storage must be defined.");
        EXCEPTION_ASSERT_WITH_LOG(key.size() > 0, std::invalid_argument,
                                  "key cannot be empty.");

        std::shared_ptr<PCSCReaderCardAdapter> rca =
            getDefaultPCSCReaderCardAdapter();
        // rca.reset(new OmnikeyHIDiClassReaderCardAdapter());
        // rca->setReaderUnit(shared_from_this());

        // setIsSecureConnectionMode(true);
        // rca->initSecureModeSession(0xFF);
        rca->sendAPDUCommand(0x84, 0x82,
                             (keystorage->getVolatile() ? 0x00 : 0x20),
                             keystorage->getKeySlot(),
                             static_cast<unsigned char>(key.size()), key);
        // rca->closeSecureModeSession();
        // setIsSecureConnectionMode(false);
    }

    void OmnikeyXX21ReaderUnit::getT_CL_ISOType(bool &isTypeA, bool &isTypeB)
    {
        unsigned char outBuffer[64];
        DWORD dwOutBufferSize;
        unsigned char inBuffer[2];
        DWORD dwInBufferSize;
        DWORD dwBytesReturned;
        DWORD dwControlCode = CM_IOCTL_GET_SET_RFID_BAUDRATE;
        DWORD dwStatus;

        memset(outBuffer, 0x00, sizeof(outBuffer));
        dwOutBufferSize = sizeof(outBuffer);
        dwInBufferSize  = sizeof(inBuffer);
        dwBytesReturned = 0;

        inBuffer[0] = 0x01; // Version of command
        inBuffer[1] = 0x00; // Get asymmetric baud rate information

        isTypeA = false;
        isTypeB = false;

        dwStatus = SCardControl(getHandle(), dwControlCode, (LPVOID)inBuffer,
                                dwInBufferSize, (LPVOID)outBuffer,
                                dwOutBufferSize, &dwBytesReturned);
        if (dwStatus == SCARD_S_SUCCESS && dwBytesReturned >= 10)
        {
            switch (outBuffer[9])
            {
                case 0x04:
                    isTypeA = true;
                    break;

                case 0x08:
                    isTypeB = true;
                    break;
            }
        }
    }

    void OmnikeyXX21ReaderUnit::setSecureConnectionStatus(
        OmnikeyXX21ReaderUnit::SecureModeStatus st)
    {
        auto itr = secure_connection_status_.find(getConnectedName());
        if (itr != secure_connection_status_.end())
            itr->second = st;
        else
            secure_connection_status_.insert(
                std::make_pair(getConnectedName(), st));
    }

    OmnikeyXX21ReaderUnit::SecureModeStatus
    OmnikeyXX21ReaderUnit::getSecureConnectionStatus()
    {
        const auto itr = secure_connection_status_.find(getConnectedName());
        if (itr == secure_connection_status_.end())
            return SecureModeStatus(SecureModeStatus::DISABLED);
        return itr->second;
    }

    void OmnikeyXX21ReaderUnit::setSecureConnectionStatus(int v)
    {
        setSecureConnectionStatus(SecureModeStatus(v));
    }

    void OmnikeyXX21ReaderUnit::setCardTechnologies(const TechnoBitset &bitset)
    {
#ifdef _WIN32
        // We are not (yet) able to do so on Linux.

        // The registry values are mapped to card techno.
        // This is the mapping for the Omnikey driver.
        //     ICLASS15693  -> 02
        //     ISO15693     -> 04
        //     ICODE1       -> 06
        //     STM14443B    -> 05
        //     ISO14443B    -> 03
        //     ISO14443A    -> 01

        std::vector<uint8_t> data;
        if (bitset & Techno::ICLASS_15693)
            data.push_back(0x02);
        if (bitset & Techno::ISO_15693)
            data.push_back(0x04);
        if (bitset & Techno::ICODE1)
            data.push_back(0x06);
        if (bitset & Techno::STM14443B)
            data.push_back(0x05);
        if (bitset & Techno::ISO_14443_B)
            data.push_back(0x03);
        if (bitset & Techno::ISO_14443_A)
            data.push_back(0x01);

        if (data.size() == 0) // Nothing means all techno are disabled,
                              // however no registry key
            data = {0, 0, 0,
                    0, 0, 0}; // means (for the driver) that all is enabled.
        if (!WindowsRegistry::writeBinary(
                "SYSTEM\\CurrentControlSet\\Control\\CardMan\\RFID",
                "CardSearch", data))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Cannot set card technologies.");
        }
#endif
    }


    TechnoBitset OmnikeyXX21ReaderUnit::getCardTechnologies()
    {
#ifdef _WIN32
        TechnoBitset bs = 0;
        std::vector<uint8_t> data;
        if (WindowsRegistry::readBinary(
                "SYSTEM\\CurrentControlSet\\Control\\CardMan\\RFID",
                "CardSearch", data))
        {
            if (std::find(data.begin(), data.end(), 0x02) != data.end())
                bs |= Techno::ICLASS_15693;
            if (std::find(data.begin(), data.end(), 0x04) != data.end())
                bs |= Techno::ISO_15693;
            if (std::find(data.begin(), data.end(), 0x06) != data.end())
                bs |= Techno::ICODE1;
            if (std::find(data.begin(), data.end(), 0x05) != data.end())
                bs |= Techno::STM14443B;
            if (std::find(data.begin(), data.end(), 0x03) != data.end())
                bs |= Techno::ISO_14443_B;
            if (std::find(data.begin(), data.end(), 0x01) != data.end())
                bs |= Techno::ISO_14443_A;
        }
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Cannot retrieve card technologies.");
        return bs;
#endif
    }

    LIBLOGICALACCESS_API std::ostream &
    operator<<(std::ostream &os,
               const OmnikeyXX21ReaderUnit::SecureModeStatus &s)
    {
        using sms = OmnikeyXX21ReaderUnit::SecureModeStatus;
        switch (s.value_)
        {
            case sms::READ:
                os << "READ ONLY";
                break;
            case sms::WRITE:
                os << "WRITE";
                break;
            case sms::DISABLED:
                os << "DISABLED";
                break;
        }
        return os;
    }
}
