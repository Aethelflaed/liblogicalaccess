#ifndef UNIXDYNLIBRARY_HPP_
#define UNIXDYNLIBRARY_HPP_

#include <string>

#include "idynlibrary.hpp"

namespace LOGICALACCESS
{
  class	PosixDynLibrary : public IDynLibrary
  {
  private:
    std::string	_name;
    void* _handle;

  public:
    PosixDynLibrary(const std::string& dlName);
    ~PosixDynLibrary()
    { ::dlclose(_handle); }

    const std::string& getName(void) const
    { return _name; }
    void* getSymbol(const char* symName);
  };
}

#endif
