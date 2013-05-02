/**
 * \file SAMKeyStorage.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief SAM key storage.
 */

#include "logicalaccess/Cards/SAMKeyStorage.h"

namespace LOGICALACCESS
{
	SAMKeyStorage::SAMKeyStorage()
	{
		d_key_slot = 0x00;
	}

	KeyStorageType SAMKeyStorage::getType() const
	{
		return KST_SAM;
	}

	void SAMKeyStorage::setKeySlot(unsigned char key_slot)
	{
		d_key_slot = key_slot;
	}

	unsigned char SAMKeyStorage::getKeySlot() const
	{
		return d_key_slot;
	}

	void SAMKeyStorage::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("KeySlot", d_key_slot);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SAMKeyStorage::unSerialize(boost::property_tree::ptree& node)
	{
		d_key_slot = node.get_child("KeySlot").get_value<unsigned char>();
	}

	std::string SAMKeyStorage::getDefaultXmlNodeName() const
	{
		return "SAMKeyStorage";
	}
}

