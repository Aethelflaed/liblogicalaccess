/**	
 * \file rplethreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader/card adapter.
 */

#include "rplethreadercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{		
	RplethReaderCardAdapter::RplethReaderCardAdapter()
		: ReaderCardAdapter()
	{
	}

	RplethReaderCardAdapter::~RplethReaderCardAdapter()
	{
		
	}

	std::vector<unsigned char> RplethReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{	
		if (command[0] == 0x01 && command[1] == 0x05)
			COM_("Sending command: %s", BufferHelper::getStdString(command).c_str());
		else
			COM_("Sending command: %s", BufferHelper::getHex(command).c_str());

		std::vector<unsigned char> res;
		std::vector<unsigned char> cmd;
		cmd.insert (cmd.begin(), command.begin(), command.end());
		cmd.push_back (calcChecksum(cmd));
		if (cmd.size() > 0) 
		{
			boost::shared_ptr<boost::asio::ip::tcp::socket> socket = getRplethReaderUnit()->getSocket();
			socket->send(boost::asio::buffer(cmd));

			res = receiveAnwser(cmd, timeout);
		}
		return res;
	}

	std::vector<unsigned char> RplethReaderCardAdapter::sendAsciiCommand(const std::string& command, long int timeout)
	{
		std::vector<unsigned char> cmd;
		std::vector<unsigned char> res;
		cmd.push_back (static_cast<unsigned char>(Device::HID));
		cmd.push_back (static_cast<unsigned char>(HidCommand::COM));
		cmd.push_back (static_cast<unsigned char>(command.size()));
		cmd.insert(cmd.end(), command.begin(), command.end());
		res = sendCommand (cmd, timeout);
		return res;
	}

	std::vector<unsigned char> RplethReaderCardAdapter::receiveAnwser(const std::vector<unsigned char>& command, long int timeout)
	{
		std::vector<unsigned char> res(4);
		// wait to receive the full frame
	#ifdef _WINDOWS
			Sleep(100);
	#elif defined(LINUX)
			usleep(100000);
	#endif
		boost::shared_ptr<boost::asio::ip::tcp::socket> socket = getRplethReaderUnit()->getSocket();
		long int currentWait = 0;
		while (socket->available() == 0 && (timeout == 0 || currentWait < timeout))
		{  
	#ifdef _WINDOWS
			Sleep(250);
	#elif defined(LINUX)
			usleep(250000);
	#endif
			currentWait += 250;
		}

		try
		{
			size_t frameLength = socket->receive (boost::asio::buffer(res));
			if (frameLength == 4)
			{
				// +1 for checksum
				std::vector<unsigned char> data (static_cast<int> (res[3]+1));
				size_t dataLength = socket->receive (boost::asio::buffer(data));
				if (dataLength == data.size())
				{
					res.insert(res.end(), data.begin(), data.begin() + dataLength);
					res = handleAnswerBuffer (command, res);
					COM_("Reiceve answer: %s", BufferHelper::getHex(res).c_str());
				}
				else
				{
					ERROR_("Unexcepted buffer size (%d != %d)", dataLength, data.size());
				}
			}
		}
		catch (std::exception&)
		{
			if (res.size() > 0)
				ERROR_("Error when handling the answer: %d", (int)res[0]);
			else
				ERROR_SIMPLE_("Error when handling the answer: no answer");

			res.clear();
		}
		return res;
	}

	unsigned char RplethReaderCardAdapter::calcChecksum(const std::vector<unsigned char>& data)
	{
		unsigned char bcc = 0x00;

		for (unsigned int i = 0; i < data.size(); ++i)
		{
			bcc ^= data[i];
		}

		return bcc;
	}

	std::vector<unsigned char> RplethReaderCardAdapter::handleAnswerBuffer(const std::vector<unsigned char>& cmdbuf, const std::vector<unsigned char>& ansbuf)
	{
		EXCEPTION_ASSERT_WITH_LOG(ansbuf.size() > 4, std::invalid_argument, "A valid answer buffer size must be at least 4 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x01, std::invalid_argument, "The supplied answer buffer get the stat : Command failure");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x02, std::invalid_argument, "The supplied answer buffer get the stat : Bad checksum in command");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x03, std::invalid_argument, "The supplied answer buffer get the stat : Timeout");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x04, std::invalid_argument, "The supplied answer buffer get the stat : Bad size of command");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x05, std::invalid_argument, "The supplied answer buffer get the stat : Bad device in command");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[1] == cmdbuf [0] && ansbuf[2] == cmdbuf [1], std::invalid_argument, "The supplied answer buffer is not corresponding with command send");

		std::vector<unsigned char> res;
		std::vector<unsigned char> bufnoc = std::vector<unsigned char>(ansbuf.begin(), ansbuf.end() - 1);
		unsigned char checksum_receive = ansbuf[ansbuf.size()-1];
		EXCEPTION_ASSERT_WITH_LOG(calcChecksum(bufnoc) == checksum_receive, std::invalid_argument, "The supplied answer buffer get the stat : Bad checksum in answer");
		if (bufnoc.size() > 4)
		{
			res = std::vector<unsigned char> (bufnoc.begin()+4, bufnoc.end());
			if (res.size() == 3)
			{
				if (res[1] == '\r' && res[2] == '\n')
				{
					EXCEPTION_ASSERT_WITH_LOG(res[0] != 'N', LibLogicalAccessException, "No tag present in rfid field.");
					EXCEPTION_ASSERT_WITH_LOG(res[0] != 'F', LibLogicalAccessException, "Fail to execute the command.");
				}
			}
		}
		return res;
	}
}
