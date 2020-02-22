#pragma once

namespace reg_key
{
  class key
  {
  public:
    virtual ~key() = 0;
  };

  key* create_key(LONG& error, unsigned number_of_values_to_create);
}
