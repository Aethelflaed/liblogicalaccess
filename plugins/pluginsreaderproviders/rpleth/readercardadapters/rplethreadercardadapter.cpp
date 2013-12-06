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

	std::vector<unsigned char> RplethReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{	
		return command;
	}

	std::vector<unsigned char> RplethReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		return answer;
	}
	
	std::vector<unsigned char> RplethReaderCardAdapter::sendRplethCommand(const std::vector<unsigned char>& data, long timeout)
	{
		COM_("Send Rpleth Command : %s", BufferHelper::getHex(data).c_str());
		std::vector<unsigned char> res;

		boost::shared_ptr<RplethDataTransport> dt = boost::dynamic_pointer_cast<RplethDataTransport>(getDataTransport());
		if (dt)
		{
			if (dt->getBuffer().size() == 0 && dt->getSocket()->available() == 0)
			{
				dt->sendPing();
			}
			if (adaptCommand(data).size() > 0)
			{
				dt->sendll(adaptCommand(data));
			}
			res = adaptAnswer(dt->receive(timeout));
		}
		else
		{
			ERROR_SIMPLE_("Not a Rpleth data transport.");
		}

		return res;
	}
}
