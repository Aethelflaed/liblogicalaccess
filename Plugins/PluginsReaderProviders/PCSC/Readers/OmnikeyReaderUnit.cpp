/**
 * \file OmnikeyReaderUnit.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Omnikey reader unit.
 */

#include "../Readers/OmnikeyReaderUnit.h"
#include "../ReaderCardAdapters/PCSCReaderCardAdapter.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/BufferHelper.h"

namespace LOGICALACCESS
{
	std::map<std::string, bool> OmnikeyReaderUnit::d_isSecureConnectionMode;

	OmnikeyReaderUnit::OmnikeyReaderUnit(const std::string& name)
		: PCSCReaderUnit(name)
	{
		
	}

	OmnikeyReaderUnit::~OmnikeyReaderUnit()
	{
		
	}

	string OmnikeyReaderUnit::getReaderSerialNumber()
	{
		string ret;
		
		unsigned char ucReceivedData[66];
		size_t ulNoOfDataReceived = sizeof(ucReceivedData);
		memset(ucReceivedData, 0x00, sizeof(ucReceivedData));

		getDefaultPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x9A, 0x01, 0x05, 0x00, ucReceivedData, &ulNoOfDataReceived);
		size_t le = ulNoOfDataReceived - 2;

		if (le > 0)
		{
			std::vector<unsigned char> buf(ucReceivedData, ucReceivedData + le);
			ret = BufferHelper::getStdString(buf);
		}

		try
		{
			ulNoOfDataReceived = sizeof(ucReceivedData);		
			getDefaultPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x9A, 0x04, 0x01, ucReceivedData, &ulNoOfDataReceived);
		}
		catch(std::exception&)
		{
			
		}

		return ret;
	}

	bool OmnikeyReaderUnit::getIsSecureConnectionMode()
	{
		if (d_isSecureConnectionMode.find(getConnectedName()) == d_isSecureConnectionMode.end())
		{
			return false;
		}

		return d_isSecureConnectionMode[getConnectedName()];
	}

	void OmnikeyReaderUnit::setIsSecureConnectionMode(bool isSecure)
	{
		d_isSecureConnectionMode[getConnectedName()] = isSecure;
	}

	void OmnikeyReaderUnit::disconnect()
	{
		if (getIsSecureConnectionMode())
		{
			//SecureModeCloseSession(getHandle());
			setIsSecureConnectionMode(false);
		}

		PCSCReaderUnit::disconnect();		
	}
}
