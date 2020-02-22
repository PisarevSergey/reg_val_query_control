#pragma once

namespace opened_reg_keys
{
  class keys
  {
  public:
    virtual ~keys() = 0;
  };

  keys* create_keys(LONG& error, unsigned number_ok_keys, unsigned number_of_values_per_key);
}
