/**
 * \file MifarePlus4KChip.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com
 * \brief MifarePlus 4k chip.
 */

#include "MifarePlus4KChip.h"

namespace LOGICALACCESS
{
	MifarePlus4KChip::MifarePlus4KChip()
		: MifarePlusChip("MifarePlus4K", 40)
	{
		d_nbSectors = 40;
	}

	MifarePlus4KChip::~MifarePlus4KChip()
	{		
	}

	boost::shared_ptr<LocationNode> MifarePlus4KChip::getRootLocationNode()
	{		
		boost::shared_ptr<LocationNode> rootNode = MifarePlusChip::getRootLocationNode();

		for (int i = 32; i < 40; i++)
		{
			addSectorNode(rootNode, i);
		}

		return rootNode;
	}
}