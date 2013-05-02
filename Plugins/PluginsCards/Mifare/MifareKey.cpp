/**
 * \file MifareKey.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare Key.
 */

#include "MifareKey.h"

namespace LOGICALACCESS
{
	MifareKey::MifareKey()
		: Key()
	{
		clear();
	}	

	MifareKey::MifareKey(const std::string& str)
		: Key()
	{
		fromString(str);
	}

	MifareKey::MifareKey(const void* buf, size_t buflen)
		: Key()
	{
		clear();

		if (buf != NULL)
		{
			if (buflen >= MIFARE_KEY_SIZE)
			{
				memcpy(d_key, buf, MIFARE_KEY_SIZE);
				d_isEmpty = false;
			}
		}
	}

	void MifareKey::clear()
	{		
		memset(getData(), 0xFF, getLength());
	}

	void MifareKey::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		Key::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void MifareKey::unSerialize(boost::property_tree::ptree& node)
	{
		Key::unSerialize(node);
	}

	std::string MifareKey::getDefaultXmlNodeName() const
	{
		return "MifareKey";
	}
}