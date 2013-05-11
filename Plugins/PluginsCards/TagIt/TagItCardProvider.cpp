/**
 * \file TagItCardProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It card profiles.
 */

#include "TagItCardProvider.h"


namespace logicalaccess
{
	TagItCardProvider::TagItCardProvider()
	{

	}

	TagItCardProvider::~TagItCardProvider()
	{

	}

	bool TagItCardProvider::isLocked(size_t block)
	{
		unsigned char status = getISO15693Commands()->getSecurityStatus(block);
		
		return ((status & 0x01) != 0);
	}
}