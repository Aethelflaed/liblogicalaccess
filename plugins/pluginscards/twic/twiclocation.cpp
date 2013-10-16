/**
 * \file twiclocation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for Twic card.
 */

#include "twiclocation.hpp"

namespace logicalaccess
{
	TwicLocation::TwicLocation()
	{
		dataObject = 0;
	}

	TwicLocation::~TwicLocation()
	{		
	}

	void TwicLocation::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("DataObject", dataObject);
		node.put("Tag", tag);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void TwicLocation::unSerialize(boost::property_tree::ptree& node)
	{
		dataObject = node.get_child("DataObject").get_value<uint64_t>();
	}

	std::string TwicLocation::getDefaultXmlNodeName() const
	{
		return "TwicLocation";
	}

	bool TwicLocation::operator==(const Location& location) const
	{
		if (!Location::operator==(location))
			return false;

		const TwicLocation* pxLocation = dynamic_cast<const TwicLocation*>(&location);

		return (dataObject == pxLocation->dataObject
			);
	}
}

