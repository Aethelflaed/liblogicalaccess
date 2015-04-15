/**
 * \file omnikeyreaderunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey Reader unit.
 */

#ifndef LOGICALACCESS_OMNIKEYREADERUNIT_HPP
#define LOGICALACCESS_OMNIKEYREADERUNIT_HPP

#include "../pcscreaderunit.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <map>

namespace logicalaccess
{
    /**
     * \brief The Omnikey reader unit base class.
     */
    class LIBLOGICALACCESS_API OmnikeyReaderUnit : public PCSCReaderUnit
    {
    public:

        /**
         * \brief Constructor.
         */
        OmnikeyReaderUnit(const std::string& name);

        /**
         * \brief Destructor.
         */
        virtual ~OmnikeyReaderUnit();

        /**
         * \brief Get a string hexadecimal representation of the reader serial number
         * \return The reader serial number or an empty string on error.
         */
        virtual std::string getReaderSerialNumber();

        /**
         * \brief Get if the current connection is in secure mode.
         * \return True if secure mode is currently use, false otherwise.
         */
        bool getIsSecureConnectionMode();

        /**
         * \brief Set if the current connection is in secure mode.
         * \param isSecure The secure mode value.
         */
        void setIsSecureConnectionMode(bool isSecure);

        /**
         * \brief Wait for a card removal.
         * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
         * \return True if a card was removed, false otherwise. If a card was removed, the name of the reader on which the removal was detected is accessible with getReader().
         */
        virtual bool waitRemoval(unsigned int maxwait);

        virtual bool waitInsertion(unsigned int maxwait) override;

         /**
         * \brief Is secure connection mode ?
         * \remarks We must store it in static memory because the connection mode is global for all connection to the reader
         */
        static std::map<std::string, bool> d_isSecureConnectionMode;

    protected:
    };
}

#endif