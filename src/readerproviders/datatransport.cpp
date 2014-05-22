/**
 * \file datatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Data transport base class.
 */

#include "logicalaccess/readerproviders/datatransport.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{
	std::vector<unsigned char> DataTransport::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		LOG(LogLevel::COMS) << , "Sending command %s command size {%d} timeout {%d}...", BufferHelper::getHex(command).c_str(), command.size(), timeout);

		std::vector<unsigned char> res;
		d_lastCommand = command;
		d_lastResult.clear();

		if (command.size() > 0)
		{
			connect();

			send(command);
		}

		res = receive(timeout);

		if (res.size() > 0 && getResultChecker())
		{
			getResultChecker()->CheckResult(&res[0], res.size());
		}

		d_lastResult = res;
		LOG(LogLevel::COMS) << , "Response received successfully ! Reponse: %s size {%d}", BufferHelper::getHex(res).c_str(), res.size());

		return res;
	}
}