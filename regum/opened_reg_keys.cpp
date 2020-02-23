#include "common.h"

namespace opened_reg_keys_cpp
{
  class keys_collection : public opened_reg_keys::keys
  {
  public:
    keys_collection(LONG& error, unsigned number_of_keys, unsigned number_of_values_per_key)
    {
      error = ERROR_SUCCESS;

      for (unsigned i{ 0 }; i < number_of_keys; ++i)
      {
        std::unique_ptr<reg_key::key> current_key{reg_key::create_key(error, number_of_values_per_key)};
        if (ERROR_SUCCESS == error)
        {
          collection.push_back(std::move(current_key));
        }
        else
        {
          break;
        }
      }
    }

    void get_keys_and_vals(std::list<reg_key::naked_key_and_vals>& keys_info) override
    {
      for (const auto& c : collection)
      {
        reg_key::naked_key_and_vals current_key_and_vals;
        c->get_key_and_vals(current_key_and_vals);
        keys_info.push_back(std::move(current_key_and_vals));
      }
    }
  private:
    std::list<std::unique_ptr<reg_key::key>> collection;
  };
}

opened_reg_keys::keys::~keys() {}

opened_reg_keys::keys* opened_reg_keys::create_keys(LONG& error, unsigned number_of_keys, unsigned number_of_values_per_key)
{
  auto ks{ new opened_reg_keys_cpp::keys_collection{error, number_of_keys, number_of_values_per_key} };
  if (ERROR_SUCCESS != error)
  {
    delete ks;
    ks = nullptr;
  }

  return ks;
}
