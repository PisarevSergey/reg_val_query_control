#pragma once

namespace reg_key
{
  struct naked_key_and_vals
  {
    HKEY key;
    std::list<std::wstring> value_names;
  };

  class key
  {
  public:
    virtual void get_key_and_vals(naked_key_and_vals& n_key_and_vals) = 0;
    virtual ~key() = 0;
  };

  key* create_key(LONG& error, unsigned number_of_values_to_create);
}
