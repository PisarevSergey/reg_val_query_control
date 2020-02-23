#pragma once

namespace opened_reg_keys
{
  class keys
  {
  public:
    virtual void get_keys_and_vals(std::list<reg_key::naked_key_and_vals>& keys_info) = 0;
    virtual ~keys() = 0;
  };

  keys* create_keys(LONG& error, unsigned number_ok_keys, unsigned number_of_values_per_key);
}
