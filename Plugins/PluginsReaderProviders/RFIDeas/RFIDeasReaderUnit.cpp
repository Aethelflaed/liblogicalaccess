/**
 * \file RFIDeasReaderUnit.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief RFIDeas reader unit.
 */

#include "RFIDeasReaderUnit.h"


#include <iostream>
#include <iomanip>
#include <sstream>

#include "RFIDeasReaderProvider.h"
#include "logicalaccess/Services/AccessControl/CardsFormatComposite.h"
#include "logicalaccess/Cards/Chip.h"
#include "ReaderCardAdapters/RFIDeasReaderCardAdapter.h"
//#include "GenericTagChip.h"
#include <boost/filesystem.hpp>
#include "logicalaccess/DynLibrary/LibraryManager.h"
#include "logicalaccess/DynLibrary/IDynLibrary.h"

namespace LOGICALACCESS
{
	RFIDeasReaderUnit::RFIDeasReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new RFIDeasReaderUnitConfiguration());
		d_defaultReaderCardAdapter.reset(new RFIDeasReaderCardAdapter());
		d_card_type = "GenericTag";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/RFIDeasReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "GenericTag");
		}
		catch (...) { }

		isCOMConnection = false;

		initExternFct();		
	}

	RFIDeasReaderUnit::~RFIDeasReaderUnit()
	{
		uninitExternFct();
		uninitReaderCnx();
	}

	void RFIDeasReaderUnit::initExternFct()
	{
#ifdef _WINDOWS
		hWejAPIDLL = (HMODULE)LoadLibrary("pcProxAPI.DLL");
		EXCEPTION_ASSERT_WITH_LOG(hWejAPIDLL != NULL, LibLOGICALACCESSException, "Can't load the RFIDeas library");

		fnGetLibVersion = (GetLibVersion)GetProcAddress( hWejAPIDLL, "GetLibVersion" );
		fnCOMConnect = (ComConnect)GetProcAddress( hWejAPIDLL, "ComConnect" );
		fnCOMDisconnect = (ComDisconnect)GetProcAddress( hWejAPIDLL, "ComDisconnect" );
		fnSetComSrchRange = (SetComSrchRange)GetProcAddress( hWejAPIDLL, "SetComSrchRange" );
		fnUSBConnect = (USBConnect)GetProcAddress( hWejAPIDLL, "USBConnect" );
		fnUSBDisconnect = (USBDisconnect)GetProcAddress( hWejAPIDLL, "USBDisconnect" );
		fnGetLastLibErr = (GetLastLibErr)GetProcAddress( hWejAPIDLL, "GetLastLibErr" );
		fnReadCfg = (ReadCfg)GetProcAddress( hWejAPIDLL, "ReadCfg" );
		fnWriteCfg = (WriteCfg)GetProcAddress( hWejAPIDLL, "WriteCfg" );
		fnGetFlags = (GetFlags)GetProcAddress( hWejAPIDLL, "GetFlags" );
		fnSetFlags = (SetFlags)GetProcAddress( hWejAPIDLL, "SetFlags" );
		fnGetActiveID = (GetActiveID)GetProcAddress( hWejAPIDLL, "GetActiveID" );
		fnGetActiveID32 = (GetActiveID32)GetProcAddress( hWejAPIDLL, "GetActiveID32" );
		fnGetDevCnt = (GetDevCnt)GetProcAddress( hWejAPIDLL, "GetDevCnt" );
		fnSetActDev = (SetActDev)GetProcAddress( hWejAPIDLL, "SetActDev" );
		fnSetLUID = (SetLUID)GetProcAddress( hWejAPIDLL, "SetLUID" );
		fnGetQueuedID = (GetQueuedID)GetProcAddress( hWejAPIDLL, "GetQueuedID" );
		fnGetQueuedID_index = (GetQueuedID_index)GetProcAddress( hWejAPIDLL, "GetQueuedID_index" );
		fnSetConnectProduct = (SetConnectProduct)( hWejAPIDLL, "SetConnectProduct" );
		fnSetDevTypeSrch = (SetDevTypeSrch)( hWejAPIDLL, "SetDevTypeSrch" );

		if ((fnGetLibVersion == NULL) || (fnSetComSrchRange == NULL) || (fnCOMConnect == NULL) || (fnCOMDisconnect == NULL) ||
			(fnUSBConnect == NULL) || (fnUSBDisconnect == NULL) || (fnGetLastLibErr == NULL) || (fnReadCfg == NULL) ||
			(fnWriteCfg == NULL) || (fnGetFlags == NULL) || (fnSetFlags == NULL) || (fnGetActiveID == NULL) || (fnGetActiveID32 == NULL) ||
			(fnGetDevCnt == NULL) || (fnSetActDev == NULL) || (fnSetLUID == NULL) || (fnGetQueuedID == NULL) || (fnGetQueuedID_index == NULL)
			|| (fnSetConnectProduct == NULL) || (fnSetDevTypeSrch == NULL))
		{
			uninitExternFct();
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't initialize RFIDeas library functions pointer");
		}
#endif
	}

	void RFIDeasReaderUnit::uninitExternFct()
	{
#ifdef _WINDOWS
		FreeLibrary( hWejAPIDLL );
		hWejAPIDLL = NULL;
#endif
	}

	void RFIDeasReaderUnit::initReaderCnx()
	{
#ifdef _WINDOWS
		SetConnectProduct(PRODUCT_PCPROX);
        SetDevTypeSrch(PRXDEVTYP_USB);

		if (fnUSBConnect(&d_deviceId) == 0)
		{
			fnSetComSrchRange(1, 15);
			EXCEPTION_ASSERT_WITH_LOG(fnCOMConnect(&d_deviceId) != 0, LibLOGICALACCESSException, "Can't connect to the RFIDeas device. Please be sure a reader is plugged");
			isCOMConnection = false;
		}
		else
		{
			isCOMConnection = true;
		}
#endif
	}

	void RFIDeasReaderUnit::uninitReaderCnx()
	{
#ifdef _WINDOWS
		if (d_deviceId != 0)
		{
			if (isCOMConnection)
			{
				fnCOMDisconnect();
			}
			else
			{
				fnUSBDisconnect();
			}
		}
#endif
		d_deviceId = 0;
	}

	std::string RFIDeasReaderUnit::getName() const
	{
		string ret = "";
		return ret;
	}

	std::string RFIDeasReaderUnit::getConnectedName()
	{
		char conv[64];
		sprintf(conv, "RFIDeas Device ID: %ld", d_deviceId);

		return std::string(conv);
	}

	void RFIDeasReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool RFIDeasReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		unsigned int currentWait = 0;

		do
		{
			std::vector<unsigned char> tagid = getTagId();
			if (tagid.size() > 0)
			{
				d_insertedChip = ReaderUnit::createChip(
					(d_card_type == "UNKNOWN") ? "GenericTag" : d_card_type,
					tagid
				);
				inserted = true;
			}

#ifdef _WINDOWS
			Sleep(250);
#elif defined(LINUX)
			usleep(250000);
#endif
			currentWait += 250;
		} while (!inserted && (maxwait == 0 || currentWait < maxwait));

		return inserted;
	}

	bool RFIDeasReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;

		if (d_insertedChip)
		{
			do
			{
				std::vector<unsigned char> tagid = getTagId();
				if (tagid.size() > 0)
				{
					if (tagid != d_insertedChip->getChipIdentifier())
					{
						d_insertedChip.reset();
						removed = true;
					}
				}
				else
				{
					d_insertedChip.reset();
					removed = true;
				}

				if (!removed)
				{
	#ifdef _WINDOWS
					Sleep(250);
	#elif defined(LINUX)
					usleep(250000);
	#endif
					currentWait += 250;
				}
			} while (!removed && (maxwait == 0 || currentWait < maxwait));
		}

		return removed;
	}

	bool RFIDeasReaderUnit::connect()
	{
		return true;
	}

	void RFIDeasReaderUnit::disconnect()
	{
	}

	bool RFIDeasReaderUnit::connectToReader()
	{
		initReaderCnx();
		return true;
	}

	void RFIDeasReaderUnit::disconnectFromReader()
	{
		uninitReaderCnx();
	}

	std::vector<unsigned char> RFIDeasReaderUnit::getTagId()
	{
		std::vector<unsigned char> tagid;

#ifdef _WINDOWS
		unsigned char idbuf[32];
		memset(idbuf, 0x00, sizeof(idbuf));
		d_lastTagIdBitsLength = fnGetActiveID(idbuf, sizeof(idbuf));
		if (d_lastTagIdBitsLength > 16)
		{
			int bytes = (d_lastTagIdBitsLength + 7) / 8;

			tagid.insert(tagid.end(), idbuf, idbuf + bytes);
			// Swap LSByte => MSByte
			std::reverse(tagid.begin(), tagid.end());
		}
		else
		{
			long err = fnGetLastLibErr();
			if (err > 0)
			{
				WARNING_("An error occured when getting the active ID (%d)", err);
				INFO_("Disconnecting from the reader...");
				disconnectFromReader();
				INFO_("Reconnecting to the reader...");
				connectToReader();
			}
		}
#endif

		return tagid;
	}

	boost::shared_ptr<Chip> RFIDeasReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);
		setTagIdBitsLengthFct setagfct;

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<CardProvider> cp;

			if (type == "GenericTag")
			{
				rca = getDefaultReaderCardAdapter();
				cp = LibraryManager::getInstance()->getCardProvider(type);

				*(void**)(&setagfct) = LibraryManager::getInstance()->getFctFromName("setTagIdBitsLengthOfGenericTagChip", LibraryManager::CARDS_TYPE);
				setagfct(&chip, d_lastTagIdBitsLength);
			}
			else
				return chip;


			rca->setReaderUnit(shared_from_this());
			if(cp)
			{
				chip->setCardProvider(cp);
			}
		}
		return chip;
	}

	boost::shared_ptr<Chip> RFIDeasReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > RFIDeasReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<RFIDeasReaderCardAdapter> RFIDeasReaderUnit::getDefaultRFIDeasReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<RFIDeasReaderCardAdapter>(adapter);
	}

	string RFIDeasReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool RFIDeasReaderUnit::isConnected()
	{
		return (d_insertedChip);
	}

	void RFIDeasReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void RFIDeasReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<RFIDeasReaderProvider> RFIDeasReaderUnit::getRFIDeasReaderProvider() const
	{
		return boost::dynamic_pointer_cast<RFIDeasReaderProvider>(getReaderProvider());
	}

	boost::shared_ptr<RFIDeasReaderUnit> RFIDeasReaderUnit::getSingletonInstance()
	{
		static boost::shared_ptr<RFIDeasReaderUnit> instance;
		if (!instance)
		{
			instance.reset(new RFIDeasReaderUnit());
		}
		return instance;
	}
}
