/**
 * \file serialport.hpp
 * \author Julien KAUFFMANN <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A serial port class.
 */

#ifndef SERIALPORT_HPP
#define SERIALPORT_HPP

#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "logicalaccess/readerproviders/readerunit.hpp"

namespace logicalaccess
{
	/**
	 * \brief A class that represents a serial (COM) port.
	 *
	 * SerialPort is a simple class that help dealing with serial port in a platform independant manner.
	 *
	 * The serial port must open after construction. After it has been opened, a SerialPort should be configured using setConfiguration().
	 *
	 * Then you can either read() or write() data, or simply wait for it using select().
	 */
	class LIBLOGICALACCESS_API SerialPort :  public boost::noncopyable
	{
		public:

			/**
			 * \brief Constructor.
			 *
			 * Tries to open "COM1" on Windows, "/dev/tty0" on Linux.
			 */
			SerialPort();

			/**
			 * \brief Constructor.
			 * \param dev The device name. (Example: "/dev/tty0")
			 */
			SerialPort(const std::string& dev);

			/**
			 * \brief Destructor.
			 * \see close()
			 * Automatically closes the serial port. See close().
			 */
			virtual inline ~SerialPort()
			{
				close();
			}

			/**
			 * \brief Open the serial port.
			 * \see close()
			 * \see reopen()
			 * \see getConfiguration()
			 * \see setConfiguration()
			 *
			 * Once the serial port is open, you must configure the serial port before any I/O operation.
			 *
			 * If the open fails, a std::exception is thrown to give more information about the failure.
			 *
			 * Attempting to open() a SerialPort for which isOpen() returns true has no effect.
			 */
			void open();

			/**
			 * \brief Reopen the serial port.
			 * \see open()
			 * \see close()
			 *
			 * Convenience method. reopen() has the exact same behavior than a call to close() followed by a call to open().
			 */
			void reopen();

			/**
			 * \brief Close the serial port.
			 * \see open()
			 * \see reopen()
			 *
			 * Calling close() on an already closed serial port has no effect.
			 */
			void close();

			bool isOpen();

			/**
			 * \brief Get the device name.
			 * \return The device name.
			 */
			inline const std::string& deviceName() const
			{
				return m_dev;
			}

			/**
			 * \brief Read some data from the serial port.
			 * \param buf The buffer into which the new data must be stored.
			 * \param cnt The count of bytes to read. If cnt is 0, the call with return immediatly with a return value of 0.
			 * \return The count of read bytes. buf.size() is also set, accordingly. Return value is 0 if no data is available.
			 * \warning If buf contains data, it will be discarded, even if the call fails.
			 *
			 * The serial port must be open before the call or an LibLogicalAccessException will be thrown.
			 * The call may also throw a std::exception in case of failure.
			 */
			size_t read(std::vector<unsigned char>& buf, size_t cnt);

			/**
			 * \brief Write some data to the serial port.
			 * \param buf The data to write to the serial port.
			 * \return The count of bytes written.
			 *
			 * The serial port must be open before the call or an LibLogicalAccessException will be thrown.
			 * The call may also throw a std::exception in case of failure.
			 */
			size_t write(const std::vector<unsigned char>& buf);

			void setBaudrate(unsigned int rate);
			unsigned int getBaudrate();

			void setFlowControl(const boost::asio::serial_port_base::flow_control::type& type);
			boost::asio::serial_port_base::flow_control::type getFlowControl();

			void setParity(const boost::asio::serial_port_base::parity::type& parity);
			boost::asio::serial_port_base::parity::type getParity();

			void setStopBits(const boost::asio::serial_port_base::stop_bits::type& stop_bits);
			boost::asio::serial_port_base::stop_bits::type getStopBits();

			void setCharacterSize(unsigned int character_size);
			unsigned int getCharacterSize();

			void setTimeout(int timeout) { m_timeout = timeout; }
			int getTimeout() { return m_timeout; }

		private:

			/**
			 * \brief The internal device name.
			 */
			std::string m_dev;

			boost::asio::io_service m_io;

			boost::asio::serial_port m_serial_port;


			/**
			 * \brief Read Timeout.
			 */
			int m_timeout;
	};
}

void read_callback(std::size_t& data_available, boost::asio::deadline_timer& timeout, const boost::system::error_code& error, std::size_t bytes_transferred);
void wait_callback(boost::asio::serial_port& ser_port, const boost::system::error_code& error);

#endif /* SERIALPORT_HPP */
