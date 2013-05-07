/**
 * \file Chip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Chip descriptor.
 */

#ifndef LOGICALACCESS_CHIP_H
#define LOGICALACCESS_CHIP_H

#include "logicalaccess/ReaderProviders/ReaderUnit.h"
#include "logicalaccess/Cards/Profile.h"
#include "logicalaccess/Cards/CardProvider.h"
#include "logicalaccess/Cards/LocationNode.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "logicalaccess/logs.h"

#ifdef LINUX
#include <wintypes.h>
#endif

namespace LOGICALACCESS
{
	/**
	 * \brief The power status.
	 */
	typedef enum {
		CPS_NO_POWER = 0x00, /**< No power */
		CPS_UNKNOWN = 0x01, /**< Unknown power status */
		CPS_POWER_LOW = 0x02, /**< Power low */
		CPS_POWER_HIGH = 0x03 /**< Power high */
	} ChipPowerStatus;

	/**
	 * \brief The base chip class for all chip. Each chip have is own object and providers according to himself and the reader used to access the chip.
	 */
	class LIBLOGICALACCESS_API Chip : public boost::enable_shared_from_this<Chip>
	{
		public:

			/**
			 * \brief Constructor.
			 */
			Chip();

			/**
			 * \brief Constructor.
			 * \param cardtype The Card type of the chip.
			 */
			Chip(std::string cardtype);

			/**
			 * \brief Destructor.
			 */
			virtual ~Chip() {};

			/**
			 * \brief Get the card type of the chip.
			 * \return The chip's card type.
			 */
			virtual const std::string& getCardType() const;

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const;

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			/**
			 * \brief Get the card provider for I/O access.
			 * \return The card provider.
			 */
			boost::shared_ptr<CardProvider> getCardProvider() const { return d_cardprovider; };

			/**
			 * \brief Set the card provider for I/O access.
			 * \param provider The card provider.
			 */
			void setCardProvider(boost::shared_ptr<CardProvider> provider);

			/**
			 * \brief Get the profile.
			 * \return The profile.
			 */
			boost::shared_ptr<Profile> getProfile() const { return d_profile; };

			/**
			 * \brief Set the profile.
			 * \param rofile The profile.
			 */
			void setProfile(boost::shared_ptr<Profile> profile) { d_profile = profile; };

			/**
			 * \brief Get the chip identifier.
			 * \return The chip identifier.
			 */
			const std::vector<unsigned char> getChipIdentifier() const { return d_chipIdentifier; };

			/**
			 * \brief Set the chip identifier.
			 * \param identifier The chip identifier.
			 */
			virtual void setChipIdentifier(std::vector<unsigned char> identifier) { d_chipIdentifier = identifier; };

			/**
			 * \brief Get the chip power status.
			 * \return The power status.
			 */
			ChipPowerStatus getPowerStatus() const { return d_powerStatus; };

			/**
			 * \brief Set the chip power status.
			 * \param powerStatus The power status.
			 */
			void setPowerStatus(ChipPowerStatus powerStatus) { d_powerStatus = powerStatus; };	

			/**
			 * \brief Get the chip reception level.
			 * \return The reception level.
			 */
			unsigned char getReceptionLevel() const { return d_receptionLevel; };

			/**
			 * \brief Set the chip reception level.
			 * \param powerStatus The reception level.
			 */
			void setReceptionLevel(unsigned char receptionLevel) { d_receptionLevel = receptionLevel; };

			bool operator < (const Chip& chip)
			{
				return d_receptionLevel < chip.getReceptionLevel();
			}

		protected:

			/**
			 * \brief The card type of the profile.
			 */
			std::string d_cardtype;

			/**
			 * \brief The chip identifier.
			 */
			std::vector<unsigned char> d_chipIdentifier;

			/**
			 * \brief The chip power status. Most chip doesn't have his own power supply.
			 */
			ChipPowerStatus d_powerStatus;

			/**
			 * \brief The card provider.
			 */
			boost::shared_ptr<CardProvider> d_cardprovider;

			/**
			 * \brief The profile.
			 */
			boost::shared_ptr<Profile> d_profile;

			/**
			 * \brief The chip reception level.
			 */
			unsigned char d_receptionLevel;
	};
}

#endif /* LOGICALACCESS_CHIP_H */

