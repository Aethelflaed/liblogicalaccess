/**
 * \file idondemandreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  IdOnDemand reader unit configuration.
 */

#include "idondemandreaderunitconfiguration.hpp"
#include "idondemandreaderprovider.hpp"


namespace logicalaccess
{
	IdOnDemandReaderUnitConfiguration::IdOnDemandReaderUnitConfiguration()
		: ReaderUnitConfiguration(READER_IDONDEMAND)
	{
		resetConfiguration();
	}

	IdOnDemandReaderUnitConfiguration::~IdOnDemandReaderUnitConfiguration()
	{
	}

	void IdOnDemandReaderUnitConfiguration::resetConfiguration()
	{
		d_authCode = "8112";
	}

	void IdOnDemandReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("AuthenticateCode", d_authCode);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void IdOnDemandReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		INFO_("Unserializing reader unit configuration...");

		d_authCode = node.get_child("AuthenticateCode").get_value<std::string>();

		INFO_("Authenticate Code unSerialized {%s}", d_authCode.c_str());
	}

	std::string IdOnDemandReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "IdOnDemandReaderUnitConfiguration";
	}

	std::string IdOnDemandReaderUnitConfiguration::getAuthCode() const
	{
		INFO_("Get Authenticate Code {%s}", d_authCode.c_str());
		return d_authCode;
	}

	void IdOnDemandReaderUnitConfiguration::setAuthCode(std::string authCode)
	{
		INFO_("Set Authenticate Code {%s}", authCode.c_str());
		d_authCode = authCode;
	}
}
