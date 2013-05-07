/**
 * \file ReaderCommunication.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader communication. 
 */

#ifndef LOGICALACCESS_READERCOMMUNICATION_H
#define LOGICALACCESS_READERCOMMUNICATION_H

#include "logicalaccess/ReaderProviders/ReaderUnit.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace LOGICALACCESS
{
	/**
	 * \brief A reader communication base class.
	 */
	class LIBLOGICALACCESS_API ReaderCommunication
	{
		public:

			/**
			 * \brief Send a request command from the PCD to the PICC.
			 * \return The PICC answer to the request command.
			 */
			virtual std::vector<unsigned char> request() = 0;

			/**
			 * \brief Manage collision.
			 * \return PICC serial number.
			 */
			virtual std::vector<unsigned char> anticollision() = 0;

			/**
			 * \brief Select a PICC.
			 * \param uid The PICC serial number.
			 */

			virtual void selectIso(const std::vector<unsigned char>& uid) = 0;
	};

}

#endif /* LOGICALACCESS_READERCOMMUNICATION_H */

