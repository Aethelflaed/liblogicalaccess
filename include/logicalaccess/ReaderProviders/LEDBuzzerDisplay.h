/**
 * \file LEDBuzzerDisplay.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief LED/Buzzer Display. 
 */

#ifndef LOGICALACCESS_LEDBUZZERDISPLAY_H
#define LOGICALACCESS_LEDBUZZERDISPLAY_H

#include "logicalaccess/XmlSerializable.h"

#include "logicalaccess/logs.h"

namespace logicalaccess
{
	class ReaderCardAdapter;

	/**
	 * \brief A LED/Buzzer Display base class.
	 */
	class LIBLOGICALACCESS_API LEDBuzzerDisplay
	{
		public:

			/**
			 * \brief Set the green led to a status.
			 * \param status True to show the green led, false otherwise.
			 */
			virtual void setGreenLed(bool status) = 0;

			/**
			 * \brief Set the red led to a status.
			 * \param status True to show the red led, false otherwise.
			 */
			virtual void setRedLed(bool status) = 0;

			/**
			 * \brief Set the buzzer to a status.
			 * \param status True to play the buzzer, false otherwise.
			 */
			virtual void setBuzzer(bool status) = 0;

			/**
			 * \brief Get the reader card adapter.
			 * \return The reader card adapter.
			 */
			boost::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const { return d_readerCardAdapter; };

			/**
			 * \brief Set the reader card adapter.
			 * \param adapter The reader card adapter.
			 */
			void setReaderCardAdapter(boost::shared_ptr<ReaderCardAdapter> adapter) { d_readerCardAdapter = adapter; };


		protected:

			/**
			 * \brief The reader card adapter.
			 */
			boost::shared_ptr<ReaderCardAdapter> d_readerCardAdapter;
	};
}

#endif /* LOGICALACCESS_LEDBUZZERDISPLAY_H */

