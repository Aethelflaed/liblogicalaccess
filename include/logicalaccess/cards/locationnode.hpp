/**
 * \file locationnode.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location node.
 */

#ifndef LOGICALACCESS_LOCATIONNODE_HPP
#define LOGICALACCESS_LOCATIONNODE_HPP

#include "logicalaccess/cards/location.hpp"

#include <string>
#include <vector>
#include <iostream>

#include <boost/shared_ptr.hpp>

namespace logicalaccess
{
	/**
	 * \brief A location node description. This describe a location on a hierachical view.
	 */
	class LIBLOGICALACCESS_API LocationNode
	{
	public:
		/**
		 * \brief Constructor.
		 */
		LocationNode();

		/**
		 * \brief Destructor.
		 */
		virtual ~LocationNode();

		/**
		 * \brief Set the node name.
		 * \param name The node name.
		 */
		void setName(const std::string& name);

		/**
		 * \brief Get the node name.
		 * \return The node name.
		 */
		std::string getName() const;

		/**
		 * \brief Get the childrens node.
		 * \return The childrens node.
		 */
		std::vector<boost::shared_ptr<LocationNode> >& getChildrens();

		/**
		 * \brief Set the parent node.
		 * \param parent The parent node.
		 */
		void setParent(boost::weak_ptr<LocationNode> parent);

		/**
		 * \brief Get the parent node.
		 * \return The parent node.
		 */
		boost::shared_ptr<LocationNode> getParent();

		/**
		 * \brief Set if authentication is needed.
		 * \param needed Authentication needed.
		 */
		void setNeedAuthentication(bool needed);

		/**
		 * \brief Get if authentication is needed.
		 * \return True if authentication is needed, false otherwise.
		 */
		bool getNeedAuthentication() const;

		/**
		 * \brief Set if the node has properties.
		 * \param hasProperties The node has properties or not.
		 */
		void setHasProperties(bool hasProperties);

		/**
		 * \brief Get if the node has properties.
		 * \return True if the node has properties, false otherwise.
		 */
		bool getHasProperties() const;

		/**
		 * \brief Set the node length.
		 * \param length The node length.
		 */
		void setLength(size_t length);

		/**
		 * \brief Get the node length.
		 * \return The node length.
		 */
		size_t getLength() const;

		/**
		 * \brief Set the associated location.
		 * \param location The associated location.
		 */
		void setLocation(boost::shared_ptr<Location> location);

		/**
		 * \brief Get the associated location.
		 * \return The associated location.
		 */
		boost::shared_ptr<Location> getLocation() const;

		/**
		 * \brief Set the byte unit.
		 * \param unit The byte unit.
		 */
		void setUnit(size_t unit);

		/**
		 * \brief Get the byte unit.
		 * \return The byte unit.
		 */
		size_t getUnit() const;

	protected:

		/**
		 * \brief The childrens node.
		 */
		std::vector<boost::shared_ptr<LocationNode> > d_childrens;

		/**
		 * \brief The parent node.
		 */
		boost::weak_ptr<LocationNode> d_parent;

		/**
		 * \brief Need authentication to access the node data.
		 */
		bool d_needAuthentication;

		/**
		 * \brief The node has properties or not.
		 */
		bool d_hasProperties;

		/**
		 * \brief The data node length.
		 */
		size_t d_length;

		/**
		 * \brief The current location node name.
		 */
		std::string d_name;

		/**
		 * \brief The associated location.
		 */
		boost::shared_ptr<Location> d_location;

		/**
		 * \brief The byte unit.
		 */
		size_t d_unit;
	};	
}

#endif /* LOGICALACCESS_LOCATIONNODE_HPP */
