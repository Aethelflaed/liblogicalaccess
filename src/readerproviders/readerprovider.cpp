/**
 * \file readerprovider.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Reader provider.
 */

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include <boost/filesystem.hpp>
#include <map>
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/logs.hpp"

namespace logicalaccess
{	
	ReaderProvider::ReaderProvider()
	{
	}

	ReaderProvider::~ReaderProvider()
	{
	}	

	boost::shared_ptr<ReaderProvider> ReaderProvider::getReaderProviderFromRPType(std::string rpt)
	{
		return LibraryManager::getInstance()->getReaderProvider(rpt);
	}

	const std::vector<boost::shared_ptr<ReaderUnit> > ReaderProvider::WaitForReaders(std::vector<std::string> readers, bool all)
	{
		std::vector<boost::shared_ptr<ReaderUnit> > ret;
		bool notfound = true;

		while (notfound)
		{
			ret.clear();
			refreshReaderList();
			ReaderList rl = getReaderList();
			for (ReaderList::iterator it = rl.begin(); it != rl.end(); ++it)
			{
				if (std::find(readers.begin(), readers.end(), (*it)->getName()) != readers.end())
					ret.push_back(*it);
			}

			if ((all == false && ret.size() != 0) || (all == true && ret.size() == readers.size()))
				notfound = false;
			else
#ifdef _WINDOWS
				Sleep(1000);
#elif defined(__unix__)
				usleep(1000);
#endif
		}
		return ret;
	}
}
