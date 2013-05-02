/**
 * \file aes.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Initialization vector base class.
 */

#include "logicalaccess/crypto/initialization_vector.h"

#include <cstring>
#include <openssl/evp.h>

namespace LOGICALACCESS
{
	namespace openssl
	{
		InitializationVector::InitializationVector(size_t size, bool random) :
			d_data(size)
		{
			if (random)
			{
				randomize();
			}
			else
			{
				zero();
			}
		}

		InitializationVector::InitializationVector(const std::vector<unsigned char>& _data) :
			d_data(_data)
		{
		}

		void InitializationVector::zero()
		{
			size_t size = d_data.size();
			d_data.clear();
			d_data.resize(size, 0x00);
		}

		void InitializationVector::randomize()
		{
			for (size_t i = 0; i < d_data.size(); ++i)
			{
				d_data[i] = static_cast<char>(rand() & 0xFF);
			}
		}
	}
}
