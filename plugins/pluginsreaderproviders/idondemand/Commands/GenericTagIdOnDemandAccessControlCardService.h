/**
 * \file GenericTagIdOnDemandAccessControlCardService.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic Tag IdOnDemand Access Control Card service.
 */

#ifndef LOGICALACCESS_GENERICTAGIDONDEMANDACCESSCONTROLCARDSERVICE_H
#define LOGICALACCESS_GENERICTAGIDONDEMANDACCESSCONTROLCARDSERVICE_H

#include "GenericTagAccessControlCardService.h"


namespace logicalaccess
{
	class GenericTagCardProvider;

	/**
	 * \brief The GenericTag access control card service class for IdOnDemand reader.
	 */
	class LIBLOGICALACCESS_API GenericTagIdOnDemandAccessControlCardService : public GenericTagAccessControlCardService
	{
		public:

			/**
			 * \brief Constructor.
			 * \param cardProvider The associated card provider.
			 */
			GenericTagIdOnDemandAccessControlCardService(boost::shared_ptr<CardProvider> cardProvider);

			/**
			 * \brief Destructor.
			 */
			~GenericTagIdOnDemandAccessControlCardService();

			/**
			 * \brief Read format from the card.
			 * \param format The format to read.
			 * \param location The format location.
			 * \param aiToUse The key's informations to use.
			 * \return The format read on success, null otherwise.
			 */
			virtual boost::shared_ptr<Format> readFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse);

			/**
			 * \brief Write format to the card.
			 * \param format The format to write.
			 * \param location The format location.
			 * \param aiToUse The key's informations to use.
			 * \param aiToWrite The key's informations to write.
			 * \return True on success, false otherwise.
			 */
			virtual bool writeFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite);

		protected:

	};
}

#endif