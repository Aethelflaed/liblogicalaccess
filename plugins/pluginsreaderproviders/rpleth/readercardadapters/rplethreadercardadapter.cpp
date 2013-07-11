/**	
 * \file rplethreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader/card adapter.
 */

#include "rplethreadercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "../rplethdatatransport.hpp"

namespace logicalaccess
{		
	RplethReaderCardAdapter::RplethReaderCardAdapter()
		: ReaderCardAdapter()
	{
	}

	RplethReaderCardAdapter::~RplethReaderCardAdapter()
	{
		
	}

	std::vector<unsigned char> RplethReaderCardAdapter::sendRplethCommand(const std::vector<unsigned char>& data, long timeout)
	{
		COM_("Send Rpleth Command : %s", BufferHelper::getHex(data).c_str());
		std::vector<unsigned char> res;
		boost::shared_ptr<RplethDataTransport> dt = boost::dynamic_pointer_cast<RplethDataTransport>(getDataTransport());
		if (dt)
		{
			if (adaptCommand(data).size() > 0)
				dt->sendll(adaptCommand(data));
			res = adaptAnswer(dt->receive(timeout));
		}

		return res;
	}

	std::vector<unsigned char> RplethReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{	
		return command;
	}

	std::vector<unsigned char> RplethReaderCardAdapter::sendAsciiCommand(const std::string& command, long int timeout)
	{
		std::vector<unsigned char> cmd;
		cmd.insert(cmd.end(), command.begin(), command.end());
		return sendCommand (cmd, timeout);
	}

	std::vector<unsigned char> RplethReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		if (answer.size() == 3)
		{
			if (answer[1] == '\r' && answer[2] == '\n')
			{
				EXCEPTION_ASSERT_WITH_LOG(answer[0] != 'N', LibLogicalAccessException, "No tag present in rfid field.");
				EXCEPTION_ASSERT_WITH_LOG(answer[0] != 'F', LibLogicalAccessException, "Fail to execute the command.");
			}
		}

		return answer;
	}	
}
