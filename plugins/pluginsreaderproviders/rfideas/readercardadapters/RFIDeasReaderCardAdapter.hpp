/**
 * \file rfideasreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default RFIDeas reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTRFIDEASREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTRFIDEASREADERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"
#include "../rfideasreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{	
	/**
	 * \brief A default RFIDeas reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API RFIDeasReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			RFIDeasReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~RFIDeasReaderCardAdapter();					

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 2000);			
			
			/**
			 * \brief Get the RFIDeas reader unit.
			 * \return The RFIDeas reader unit.
			 */
			boost::shared_ptr<RFIDeasReaderUnit> getRFIDeasReaderUnit() const { return boost::dynamic_pointer_cast<RFIDeasReaderUnit>(getReaderUnit()); };			
			

		protected:						
	};

}

#endif /* LOGICALACCESS_DEFAULTRFIDEASREADERCARDADAPTER_HPP */

 
