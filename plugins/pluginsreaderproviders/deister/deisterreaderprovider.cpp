/**
 * \file deisterreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader Provider Deister.
 */

#include "deisterreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "deisterreaderunit.hpp"

namespace logicalaccess
{
    DeisterReaderProvider::DeisterReaderProvider() :
        ReaderProvider()
    {
    }

    boost::shared_ptr<DeisterReaderProvider> DeisterReaderProvider::getSingletonInstance()
    {
        static boost::shared_ptr<DeisterReaderProvider> instance;
        if (!instance)
        {
            instance.reset(new DeisterReaderProvider());
            instance->refreshReaderList();
        }

        return instance;
    }

    DeisterReaderProvider::~DeisterReaderProvider()
    {
        release();
    }

    void DeisterReaderProvider::release()
    {
    }

    boost::shared_ptr<ReaderUnit> DeisterReaderProvider::createReaderUnit()
    {
        LOG(LogLevel::INFOS) << "Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)";

        boost::shared_ptr<DeisterReaderUnit> ret(new DeisterReaderUnit());
        ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

        return ret;
    }

    bool DeisterReaderProvider::refreshReaderList()
    {
        d_readers.clear();

        std::vector<boost::shared_ptr<SerialPortXml> > ports;
        EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

        for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
        {
            boost::shared_ptr<DeisterReaderUnit> unit(new DeisterReaderUnit());
            boost::shared_ptr<SerialPortDataTransport> dataTransport = boost::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
            dataTransport->setSerialPort(*i);
            unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
            d_readers.push_back(unit);
        }

        return true;
    }
}