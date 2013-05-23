/**
 * \file rplethreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth Reader unit configuration.
 */

#ifndef LOGICALACCESS_RPLETHREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_RPLETHREADERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"


namespace logicalaccess
{	
	/**
	 * \brief The Rpleth reader unit configuration base class.
	 */
	class LIBLOGICALACCESS_API RplethReaderUnitConfiguration : public ReaderUnitConfiguration
	{
		public:

			/**
			 * \brief Constructor.
			 */
			RplethReaderUnitConfiguration();

			/**
			 * \brief Destructor.
			 */
			virtual ~RplethReaderUnitConfiguration();

			/**
			 * \brief Reset the configuration to default values
			 */
			virtual void resetConfiguration();

			/**
			 * \brief Set the configuration to param values
			 * \param readerAddress The reader address
			 * \param port The reader port
			 */
			virtual void setConfiguration(const std::string& readerAddress, int port);

			void setWiegandConfiguration (byte offset, byte lenght);

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

			std::string getReaderAddress() const;

			void setReaderAddress(const std::string& readerAddress);

			int getPort() const;

			void setPort(int port);

			byte getOffset() const;

			void setOffset(byte offset);
			
			byte getLenght() const;

			void setLenght(byte lenght);

		protected:

			std::string d_readerAddress;

			int d_port;

			byte d_offset;

			byte d_lenght;
	};
}

#endif
