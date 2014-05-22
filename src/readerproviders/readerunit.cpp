/**
 * \file readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader unit.
 */

#include "logicalaccess/readerproviders/readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstring>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/readerproviders/datatransport.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <map>


namespace logicalaccess
{
	ReaderUnit::ReaderUnit()
		: XmlSerializable(), d_card_type("UNKNOWN")
	{

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/ReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	ReaderUnit::~ReaderUnit()
	{
	}

	std::vector<unsigned char> ReaderUnit::getPingCommand() const
	{
		return std::vector<unsigned char>();
	}

	boost::shared_ptr<LCDDisplay> ReaderUnit::getLCDDisplay()
	{
		if (d_lcdDisplay)
		{
			d_lcdDisplay->setReaderCardAdapter(getDefaultReaderCardAdapter());
		}
		return d_lcdDisplay;
	}

	boost::shared_ptr<LEDBuzzerDisplay> ReaderUnit::getLEDBuzzerDisplay()
	{
		if (d_ledBuzzerDisplay)
		{
			d_ledBuzzerDisplay->setReaderCardAdapter(getDefaultReaderCardAdapter());
		}
		return d_ledBuzzerDisplay;
	}

	bool ReaderUnit::waitInsertion(const std::vector<unsigned char>& identifier, unsigned int maxwait)
	{
		INFO_("Started for identifier %s - maxwait {%u}", BufferHelper::getHex(identifier).c_str(), maxwait);

		bool inserted = false;

		boost::posix_time::ptime currentDate = boost::posix_time::second_clock::local_time();
		boost::posix_time::ptime maxDate = currentDate + boost::posix_time::milliseconds(maxwait);

		while (!inserted && currentDate < maxDate)
		{
			if (waitInsertion(maxwait))
			{
				INFO_("Chip(s) detected ! Looking in the list to find the chip...");
				bool found = false;
				std::vector<boost::shared_ptr<Chip> > chipList = getChipList();
				for (std::vector<boost::shared_ptr<Chip> >::iterator i = chipList.begin(); i != chipList.end() && !found; ++i)
				{
					std::vector<unsigned char> tmp = (*i)->getChipIdentifier();
					INFO_("Processing chip %s...", BufferHelper::getHex(tmp).c_str());

					if (tmp == identifier)
					{
						INFO_("Chip found !");
						// re-assign the chip
						d_insertedChip = *i;
						found = true;
					}
				}

				if (found)
				{
					inserted = true;
				}
				else
				{
					d_insertedChip.reset();
				}
			}
			currentDate = boost::posix_time::second_clock::local_time();
		}

		INFO_("Returns inserted {%d} - expired {%d}", inserted, (currentDate >= maxDate));

		return inserted;
	}

	std::vector<unsigned char> ReaderUnit::getNumber(boost::shared_ptr<Chip> chip, boost::shared_ptr<CardsFormatComposite> composite)
	{
		INFO_("Started for chip type {0x%s(%s)}", chip->getCardType().c_str(), chip->getGenericCardType().c_str());
		std::vector<unsigned char> ret;

		if (composite)
		{
			INFO_("Composite used to find the chip identifier {%s}", boost::dynamic_pointer_cast<XmlSerializable>(composite)->serialize().c_str());
			composite->setReaderUnit(shared_from_this());

			CardTypeList ctList = composite->getConfiguredCardTypes();
			std::string useCardType = chip->getCardType();
			CardTypeList::iterator itct = std::find(ctList.begin(), ctList.end(), useCardType);
			// Try to use the generic card type
			if (itct == ctList.end())
			{
				useCardType = chip->getGenericCardType();
				INFO_("No configuration found for the chip type ! Looking for the generic type (%s) configuration...", useCardType.c_str());
				itct = std::find(ctList.begin(), ctList.end(), useCardType);
			}
			// Try to use the configuration for all card (= generic tag), because the card type isn't configured
			if (itct == ctList.end())
			{
				INFO_("No configuration found for the chip type ! Looking for \"GenericTag\" configuration...");
				useCardType = "GenericTag";
				itct = std::find(ctList.begin(), ctList.end(), useCardType);
			}

			// Try to read the number only if a configuration exists (for this card type or default)
			if (itct != ctList.end())
			{
				INFO_("Configuration found in the composite ! Retrieving format for card...");
				boost::shared_ptr<AccessInfo> ai;
				boost::shared_ptr<Location> loc;
				boost::shared_ptr<Format> format;
				composite->retrieveFormatForCard(useCardType, &format, &loc, &ai);

				if (format)
				{
					INFO_("Format retrieved successfully ! Reading the format...");
					format = composite->readFormat(chip);	// Read format on chip

					if (format)
					{
						INFO_("Format read successfully ! Getting identifier...");
						ret = format->getIdentifier();
					}
					else
					{
						ERROR_("Unable to read the format !");
					}
				}
				else
				{
					WARNING_("Cannot retrieve the format for card ! Trying using getNumber directly...");
					ret = getNumber(chip);
				}
			}
			else
			{
				ERROR_("No configuration found !");
			}
		}
		else
		{
			INFO_("Composite card format is NULL ! Reader chip identifier directly...");
			ret = chip->getChipIdentifier();
		}

		INFO_("Returns number %s", BufferHelper::getHex(ret).c_str());

		return ret;
	}

	std::vector<unsigned char> ReaderUnit::getNumber(boost::shared_ptr<Chip> chip)
	{
		// Read encoded format if specified in the license.
		return getNumber(chip, boost::shared_ptr<CardsFormatComposite>());
	}

	uint64_t ReaderUnit::getFormatedNumber(const std::vector<unsigned char>& number, int padding)
	{
		INFO_("Getting formated number... number %s-{%s} padding {%d}", BufferHelper::getHex(number).c_str(), padding);
		uint64_t longnumber = 0x00;

		for (size_t i = 0, j = number.size()-1; i < number.size(); ++i, --j)
		{
			longnumber |= ((static_cast<uint64_t>(number[i])) << (j*8));
		}

		longnumber += padding;

		INFO_("Returns long number {0x%lx(%ld)}", longnumber, longnumber);
		return longnumber;
	}

	string ReaderUnit::getFormatedNumber(const std::vector<unsigned char>& number)
	{
		INFO_("Getting formated number... number %s", BufferHelper::getHex(number).c_str());
		std::ostringstream oss;
		oss << std::setfill('0');

		for (size_t i = 0; i < number.size(); ++i)
		{
			oss << std::hex << std::setw(2) << static_cast<unsigned int>(number[i]);
		}

		INFO_("Returns number {%s}", oss.str().c_str()); 
		return oss.str();
	}

	boost::shared_ptr<Chip> ReaderUnit::createChip(std::string type, const std::vector<unsigned char>& identifier)
	{
		INFO_("Creating chip for card type {%s} and identifier %s...", type.c_str(), BufferHelper::getHex(identifier).c_str());
		boost::shared_ptr<Chip> chip = createChip(type);
		chip->setChipIdentifier(identifier);
		return chip;
	}

	boost::shared_ptr<Chip> ReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> ret = LibraryManager::getInstance()->getCard(type);

		if (!ret)
		{
			ret.reset(new Chip(type));
		}

		return ret;
	}

	boost::shared_ptr<ReaderCardAdapter> ReaderUnit::getDefaultReaderCardAdapter()
	{
		if (d_defaultReaderCardAdapter)
		{
			if (getDataTransport())
			{
				d_defaultReaderCardAdapter->setDataTransport(getDataTransport());
			}

			if (d_defaultReaderCardAdapter->getDataTransport())
			{
				d_defaultReaderCardAdapter->getDataTransport()->setReaderUnit(shared_from_this());
			}
		}
		return d_defaultReaderCardAdapter;
	}

	void ReaderUnit::setDefaultReaderCardAdapter(boost::shared_ptr<ReaderCardAdapter> defaultRca)
	{
		d_defaultReaderCardAdapter = defaultRca;
	}

	boost::shared_ptr<DataTransport> ReaderUnit::getDataTransport() const
	{
		return d_dataTransport;
	}

	void ReaderUnit::setDataTransport(boost::shared_ptr<DataTransport> dataTransport)
	{
		d_dataTransport = dataTransport;
	}

	boost::shared_ptr<ReaderUnitConfiguration> ReaderUnit::getConfiguration()
	{
		//INFO_("Getting reader unit configuration...");
		if (d_readerUnitConfig)
		{
			//INFO_("Reader unit configuration {%s}", d_readerUnitConfig->serialize().c_str());
		}

		return d_readerUnitConfig;
	}

	void ReaderUnit::setConfiguration(boost::shared_ptr<ReaderUnitConfiguration> config)
	{
		d_readerUnitConfig = config;
	}

	std::string ReaderUnit::getDefaultXmlNodeName() const
	{
		return "ReaderUnit";
	}

	void ReaderUnit::serialize(boost::property_tree::ptree& node)
	{
		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_readerUnitConfig->serialize(node);
		if (getDataTransport())
		{
			node.put("TransportType", getDataTransport()->getTransportType());
			getDataTransport()->serialize(node);
		}
		else
		{
			node.put("TransportType", "");
		}
	}

	void ReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		disconnectFromReader();
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
		std::string transportType = node.get_child("TransportType").get_value<std::string>();
		boost::shared_ptr<DataTransport> dataTransport = LibraryManager::getInstance()->getDataTransport(transportType);
		// Cannot create data transport instance from xml, use default one
		if (!dataTransport)
		{
			dataTransport = getDataTransport();
		}
		if (dataTransport && transportType == dataTransport->getTransportType())
		{
			dataTransport->unSerialize(node.get_child(dataTransport->getDefaultXmlNodeName()));
			setDataTransport(dataTransport);
		}
	}

	void ReaderUnit::unSerialize(boost::property_tree::ptree& node, const std::string& rootNode)
	{
		boost::property_tree::ptree parentNode = node.get_child(rootNode);
		BOOST_FOREACH(boost::property_tree::ptree::value_type const& v, parentNode)
		{
			if (v.first == getDefaultXmlNodeName())
			{
				if (static_cast<std::string>(v.second.get_child("<xmlattr>.type").get_value<std::string>()) == getReaderProvider()->getRPType())
				{
					boost::property_tree::ptree r = v.second;
					unSerialize(r);		
				}
			}
		}
	}
}
