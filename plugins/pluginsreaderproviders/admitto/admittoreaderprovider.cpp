/**
 * \file admittoreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Admitto reader provider.
 */

#include "admittoreaderprovider.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#ifdef __unix__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>
#include <iomanip>

#include "admittoreaderunit.hpp"


namespace logicalaccess
{
	AdmittoReaderProvider::AdmittoReaderProvider() :
		ReaderProvider()
	{
	}

	AdmittoReaderProvider::~AdmittoReaderProvider()
	{
		release();
	}

	void AdmittoReaderProvider::release()
	{

	}

	boost::shared_ptr<AdmittoReaderProvider> AdmittoReaderProvider::getSingletonInstance()
	{
		INFO_("Getting singleton instance...");
		static boost::shared_ptr<AdmittoReaderProvider> instance;
		if (!instance)
		{
			instance.reset(new AdmittoReaderProvider());
			instance->refreshReaderList();
		}

		return instance;
	}

	boost::shared_ptr<ReaderUnit> AdmittoReaderProvider::createReaderUnit()
	{
		INFO_("Creating new reader unit with empty port... (Serial port auto-detect will be used when connecting to reader)");

		boost::shared_ptr<AdmittoReaderUnit> ret(new AdmittoReaderUnit());
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		return ret;
	}	

	bool AdmittoReaderProvider::refreshReaderList()
	{
		d_readers.clear();

		std::vector<boost::shared_ptr<SerialPortXml> > ports;
		EXCEPTION_ASSERT_WITH_LOG(SerialPortXml::EnumerateUsingCreateFile(ports), LibLogicalAccessException, "Can't enumerate the serial port list.");

		for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			boost::shared_ptr<AdmittoReaderUnit> unit(new AdmittoReaderUnit());
			boost::shared_ptr<SerialPortDataTransport> dataTransport = boost::dynamic_pointer_cast<SerialPortDataTransport>(unit->getDataTransport());
			dataTransport->setSerialPort(*i);
			unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
			d_readers.push_back(unit);
		}

		return true;
	}	
}

