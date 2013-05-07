/**
 * \file ProxChip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox chip.
 */

#ifndef LOGICALACCESS_PROXCHIP_H
#define LOGICALACCESS_PROXCHIP_H

#include "logicalaccess/Cards/Chip.h"
#include "ProxCardProvider.h"
#include "ProxProfile.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	#define CHIP_PROX		"Prox"

	/**
	 * \brief The HID Prox base chip class.
	 */
	class LIBLOGICALACCESS_API ProxChip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ProxChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~ProxChip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return CHIP_PROX; };

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			/**
			 * \brief Get the Prox card provider for I/O access.
			 * \return The Prox card provider.
			 */
			boost::shared_ptr<ProxCardProvider> getProxCardProvider() { return boost::dynamic_pointer_cast<ProxCardProvider>(getCardProvider()); };

			/**
			 * \brief Get the Prox profile.
			 * \return The Prox profile.
			 */
			boost::shared_ptr<ProxProfile> getProxProfile() { return boost::dynamic_pointer_cast<ProxProfile>(getProfile()); };

		protected:

	};
}

#endif
