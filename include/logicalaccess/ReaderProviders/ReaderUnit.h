/**
 * \file ReaderUnit.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader unit.
 */

#ifndef LOGICALACCESS_ReaderUnit_H
#define LOGICALACCESS_ReaderUnit_H

#include "logicalaccess/ReaderProviders/ReaderUnitConfiguration.h"
#include "logicalaccess/ReaderProviders/LCDDisplay.h"
#include "logicalaccess/ReaderProviders/LEDBuzzerDisplay.h"
#include "logicalaccess/MyException.h"
#include <map>

#ifdef UNIX
#include <PCSC/wintypes.h>
#endif

#define	PLUGINOBJECT_MAXLEN		64

namespace LOGICALACCESS
{
	/**
	 * \brief A liblogicalaccess exception class.
	 */
	class LIBLOGICALACCESS_API LibLOGICALACCESSException : public Exception::exception
	{
	public:
		LibLOGICALACCESSException(const std::string& message)
		  : exception(message)
		{};
		};

	/**
	 * \brief A card exception class.
	 */
	class LIBLOGICALACCESS_API CardException : public LibLOGICALACCESSException
	{
	public:
		CardException(const std::string& message)
			: LibLOGICALACCESSException(message)
		{};
	};

	#define EXCEPTION_MSG_CONNECTED			"Already connected to a card. Please disconnect before." /**< \brief Not connected exception message */
	#define EXCEPTION_MSG_NOREADER			"No reader found." /**< \brief No reader found exception format */
	#define EXCEPTION_MSG_LICENSENOACCESS	"The current license doesn't allow this functionality." /**< \brief License no access exception message  */

	class Chip;
	class Profile;
	class ReaderCardAdapter;
	class CardsFormatComposite;
	class ReaderFormatComposite;
	class ReaderProvider;

	/**
	 * \brief The card types.
	 */

	/**
	 * \brief The reader unit base class. This is an object for a specific reader instance to detect card and interact with the reader.
	 */
	class LIBLOGICALACCESS_API ReaderUnit : public XmlSerializable, public boost::enable_shared_from_this<ReaderUnit>
	{
		public:

			/**
			 * \brief Constructor.
			 */
			ReaderUnit();

			/**
			 * \brief Destructor.
			 */
			virtual ~ReaderUnit();

			/**
			 * \brief Wait for a card insertion.
			 * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
			 * \return True if a card was inserted, false otherwise. If a card was inserted, the name of the reader on which the insertion was detected is accessible with getReader().
			 * \warning If the card is already connected, then the method always fail.
			 */
			virtual bool waitInsertion(unsigned int maxwait) = 0;

			/**
			 * \brief Wait for a specific card insertion.
			 * \param identifier The chip identifier to wait.
			 * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
			 * \return True if a card was inserted, false otherwise. If a card was inserted, the name of the reader on which the insertion was detected is accessible with getReader().
			 * \warning If the card is already connected, then the method always fail. Generic and heavy behavior, should be implemented correctly for each reader type.
			 */
			virtual bool waitInsertion(const std::vector<unsigned char>& identifier, unsigned int maxwait);

			/**
			 * \brief Wait for a card removal.
			 * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
			 * \return True if a card was removed, false otherwise. If a card was removed, the name of the reader on which the removal was detected is accessible with getReader().
			 */
			virtual bool waitRemoval(unsigned int maxwait) = 0;

			/**
			 * \brief Check if the card is connected.
			 * \return True if the card is connected, false otherwise.
			 */
			virtual bool isConnected() = 0;

			/**
			 * \brief Get the forced card type.
			 * \return The forced card type.
			 */
			inline std::string getCardType() const { return d_card_type; };

			/**
			 * \brief Set the forced card type.
			 * \param cardType The forced card type.
			 */
			virtual void setCardType(std::string cardType) = 0;

			/**
			 * \brief Create the chip object from card type.
			 * \param type The card type.
			 * \return The chip.
			 */
			virtual boost::shared_ptr<Chip> createChip(std::string type);
			
			/**
			 * \brief Create the chip object from card type with identifier.
			 * \param type The card type.
			 * \param identifier The chip identifier.
			 * \return The chip.
			 */
			virtual boost::shared_ptr<Chip> createChip(std::string type, const std::vector<unsigned char>& identifier);

			/**
			 * \brief Get the number from the embedded licensing configuration.
			 * \param chip The chip object.
			 * \return The identifier number.
			 */
			virtual std::vector<unsigned char> getNumber(boost::shared_ptr<Chip> chip);

			/**
			 * \brief Get the first and/or most accurate chip found.
			 * \return The single chip.
			 */
			virtual boost::shared_ptr<Chip> getSingleChip() = 0;

			/**
			 * \brief Get chip available in the RFID rang.
			 * \return The chip list.
			 */
			virtual std::vector<boost::shared_ptr<Chip> > getChipList() = 0;

			/**
			 * \brief Get the number from the reader format composite.
			 * \param chip The chip object.
			 * \param composite The card format composite.
			 * \return The identifier number.
			 */
			virtual std::vector<unsigned char> getNumber(boost::shared_ptr<Chip> chip, boost::shared_ptr<CardsFormatComposite> composite);

			/**
			 * \brief Get the number as decimal a number.
			 * \param number The buffer with the number.
			 * \param padding Add or soustract a value to the number (Generaly used for a cards pack).
			 * \return The number as a decimal representation.
			 */
			uint64_t getFormatedNumber(const std::vector<unsigned char>& number, int padding);

			/**
			 * \brief Get a string hexadecimal representation of the number.
			 * \param number The buffer with the number.
			 * \return The number or an empty string on error.
			 */
			std::string getFormatedNumber(const std::vector<unsigned char>& number);

			/**
			 * \brief Connect to the card.
			 * \return True if the card was connected without error, false otherwise.
			 *
			 * If the card handle was already connected, connect() first call disconnect(). If you intend to do a reconnection, call reconnect() instead.
			 */
			virtual bool connect() = 0;

			/**
			 * \brief Disconnect from the card.
			 */
			virtual void disconnect() = 0;

			/**
			 * \brief Connect to the reader. Implicit connection on first command sent.
			 * \return True if the connection successed.
			 */
			virtual bool connectToReader() = 0;

			/**
			 * \brief Disconnect from reader.
			 */
			virtual void disconnectFromReader() = 0;

			/**
			 * \brief Get the reader unit name.
			 * \return The reader unit name.
			 */
			virtual std::string getName() const = 0;

			/**
			 * \brief Get the connected reader unit name.
			 * \return The connected reader unit name.
			 */
			virtual std::string getConnectedName() { return getName(); };

			/**
			 * \brief Get the reader unit configuration.
			 * \return The reader unit configuration.
			 */
			virtual boost::shared_ptr<ReaderUnitConfiguration> getConfiguration();

			/**
			 * \brief Set the reader unit configuration.
			 * \param config The reader unit configuration.
			 */
			virtual void setConfiguration(boost::shared_ptr<ReaderUnitConfiguration> config);

			/**
			 * \brief Get the default reader/card adapter.
			 * \return The default reader/card adapter.
			 */
			virtual boost::shared_ptr<ReaderCardAdapter> getDefaultReaderCardAdapter();

			/**
			 * \brief Get a string hexadecimal representation of the reader serial number
			 * \return The reader serial number or an empty string on error.
			 */
			virtual std::string getReaderSerialNumber() = 0;

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

			/**
			 * \brief UnSerialize object from a Xml node.
			 * \param node The Xml node.
			 * \param rootNode The root node.
			 * \return True on success, false otherwise.
			 */
			virtual bool unSerialize(boost::property_tree::ptree& node, const std::string& rootNode);

			/**
			 * \brief Get the associated reader provider.
			 * \return The associated reader provider.
			 */
			boost::shared_ptr<ReaderProvider> getReaderProvider() const { return d_readerProvider.lock(); };

			/**
			 * \brief Set the associated reader provider.
			 * \param provider The associated reader provider.
			 */
			void setReaderProvider(boost::weak_ptr<ReaderProvider> provider) { d_readerProvider = provider; };

			/**
			 * \brief Get the LCD Display for this reader unit.
			 * \return The LCD Display.
			 */
			virtual boost::shared_ptr<LCDDisplay> getLCDDisplay();

			/**
			 * \brief Get the LED/Buzzer Display for this reader unit.
			 * \return The LED/Buzzer Display.
			 */
			virtual boost::shared_ptr<LEDBuzzerDisplay> getLEDBuzzerDisplay();

		protected:

			/**
			 * \brief The forced card type.
			 */
			std::string d_card_type;

			/**
			 * \brief The associated reader provider for the reader unit.
			 */
			boost::weak_ptr<ReaderProvider> d_readerProvider;

			/**
			 * \brief The reader unit configuration.
			 */
			boost::shared_ptr<ReaderUnitConfiguration> d_readerUnitConfig;

			/**
			 * \brief The default reader/card adapter.
			 */
			boost::shared_ptr<ReaderCardAdapter> d_defaultReaderCardAdapter;

			/**
			 * \brief The inserted chip.
			 */
			boost::shared_ptr<Chip> d_insertedChip;

			/**
			 * \brief The inserted chip.
			 */
			boost::shared_ptr<LCDDisplay> d_lcdDisplay;

			/**
			 * \brief The inserted chip.
			 */
			boost::shared_ptr<LEDBuzzerDisplay> d_ledBuzzerDisplay;
	};
}

#endif
