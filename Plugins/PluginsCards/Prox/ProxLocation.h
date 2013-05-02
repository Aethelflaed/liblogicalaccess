/**
 * \file ProxLocation.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief ProxLocation.
 */

#ifndef LOGICALACCESS_PROXLOCATION_H
#define LOGICALACCESS_PROXLOCATION_H

#include "logicalaccess/Cards/Location.h"

namespace LOGICALACCESS
{
	/**
	 * \brief A  Prox location informations.
	 */
	class LIBLOGICALACCESS_API ProxLocation : public Location
	{
		public:
			/**
			 * \brief Constructor.
			 */
			ProxLocation();

			/**
			 * \brief Destructor.
			 */
			virtual ~ProxLocation();
			
			/**
			 * \brief Export DESFire location informations to a buffer.
			 * \return The data.
			 */
			virtual std::vector<unsigned char> getLinearData() const;

			/**
			 * \brief Import DESFire location informations from a buffer.
			 * \param data The buffer.
			 * \param offset An offset.
			 */
			virtual void setLinearData(const std::vector<unsigned char>&, size_t offset = 0);

			/**
			 * \brief Prox location informations data size.
			 */
			virtual size_t getDataSize();

			/**
			 * \brief Serialize the current object to XML.
			 * \param parentNode The parent node.
			 */
			virtual void serialize(boost::property_tree::ptree& parentNode);

			/**
			 * \brief UnSerialize a XML node to the current object.
			 * \param node The XML node.
			 */
			virtual void unSerialize(boost::property_tree::ptree& node);

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

			/**
			 * \brief Get the card type for this location.
			 * \return The card type.
			 */
			virtual std::string getCardType() { return "Prox"; }

			/**
			 * \brief Equality operator
			 * \param location Location to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const Location& location) const;

		public:
			/**
			 * \brief The bit.
			 */
			int bit;
	};	
}

#endif /* LOGICALACCESS_PROXLOCATION_H */
