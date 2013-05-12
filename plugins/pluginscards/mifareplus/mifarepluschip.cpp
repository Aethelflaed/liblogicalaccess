/**
 * \file mifarechip.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Mifare chip.
 */

#include "mifarepluschip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

#define MIFARE_PLUS_2K_SECTOR_NB 32

namespace logicalaccess
{	
	MifarePlusChip::MifarePlusChip()
		: Chip(CHIP_MIFAREPLUS4K), d_nbSectors(MIFARE_PLUS_2K_SECTOR_NB)
	{
		d_profile.reset();
	}

	MifarePlusChip::MifarePlusChip(std::string cardtype, unsigned int nbSectors) : 
		Chip(cardtype), d_nbSectors(nbSectors)
	{
		d_profile.reset();
	}

	MifarePlusChip::~MifarePlusChip()
	{
		
	}	

	unsigned int MifarePlusChip::getNbSectors() const
	{
		return d_nbSectors;
	}

	void MifarePlusChip::addSectorNode(boost::shared_ptr<LocationNode> rootNode, int sector)
	{
		char tmpName[255];
		boost::shared_ptr<LocationNode> sectorNode;
		sectorNode.reset(new LocationNode());

		sprintf(tmpName, "Sector %u", sector);
		sectorNode->setName(tmpName);
		sectorNode->setLength((sector >= MIFARE_PLUS_2K_SECTOR_NB) ? 256 : 64);
		sectorNode->setNeedAuthentication(true);

		boost::shared_ptr<MifarePlusLocation> location;
		location.reset(new MifarePlusLocation());
		location->sector = sector;
		location->byte = 0;
		location->block = sector * ((sector >= MIFARE_PLUS_2K_SECTOR_NB) ? ((4 * MIFARE_PLUS_2K_SECTOR_NB) + (16 * (sector - MIFARE_PLUS_2K_SECTOR_NB))) : 4 );

		sectorNode->setLocation(location);
		sectorNode->setParent(rootNode);
		rootNode->getChildrens().push_back(sectorNode);
	}

	boost::shared_ptr<LocationNode> MifarePlusChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;		
		rootNode.reset(new LocationNode());

		rootNode->setName("Mifare Plus");
		boost::shared_ptr<MifarePlusLocation> rootLocation;
		rootLocation.reset(new MifarePlusLocation());
		rootLocation->sector = (unsigned int)-1;
		rootNode->setLocation(rootLocation);

		if (d_cardprovider)
		{
			for (int i = 0; i < MIFARE_PLUS_2K_SECTOR_NB; i++)
			{
				addSectorNode(rootNode, i);
			}
		}

		return rootNode;
	}
}