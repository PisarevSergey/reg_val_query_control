#include "common.h"

namespace reg_key_cpp
{
  class created_key : public reg_key::key
  {
  public:
    created_key(LONG& error) : key{ 0 }, need_to_delete_key{ false }, need_to_close_key{ false }
    {
      std::wstring key_name;
      auto status = support::create_random_string(key_name);
      if (RPC_S_OK == status)
      {
        DWORD disposition;
        error = RegCreateKeyExW(HKEY_CURRENT_USER, key_name.c_str(), 0, nullptr, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr, &key, &disposition);
        if (ERROR_SUCCESS == error)
        {
          wcout << L"RegCreateKeyExW success" << endl;
          need_to_close_key = true;
          if (REG_CREATED_NEW_KEY == disposition)
          {
            wcout << L"new key was created" << endl;
            need_to_delete_key = true;
          }
          else if (REG_OPENED_EXISTING_KEY == disposition)
          {
            wcout << L"opened existing key" << endl;
          }
          else
          {
            assert(false);
          }
        }
        else
        {
          wcout << L"RegCreateKeyExW failed with error " << error << endl;
        }
      }
      else
      {
        error = ERROR_GEN_FAILURE;
        wcout << L"failed to create random key" << endl;
      }
    }

    ~created_key()
    {
      if (need_to_delete_key)
      {
        ntdll::delete_key(key);
      }

      if (need_to_close_key)
      {
        CloseHandle(key);
      }
    }
  protected:
    HKEY key;
  private:
    bool need_to_close_key;
    bool need_to_delete_key;
  };

  class key_with_values : public created_key
  {
  public:
    key_with_values(LONG& error, unsigned number_of_values_to_create) : created_key{ error }
    {
      if (ERROR_SUCCESS == error)
      {
        for (unsigned i{ 0 }; i < number_of_values_to_create; ++i)
        {
          std::wstring current_value_name;
          auto status = support::create_random_string(current_value_name);
          if (RPC_S_OK == status)
          {
            DWORD data{i % 2};
            error = RegSetValueExW(key, current_value_name.c_str(), 0, REG_DWORD, reinterpret_cast<PBYTE>(&data), sizeof(data));
            if (ERROR_SUCCESS == error)
            {
              value_names.push_back(std::move(current_value_name));
              wcout << L"RegSetValueExW success" << endl;
            }
            else
            {
              wcout << L"RegSetValueExW failed with error " << error << endl;
              break;
            }
          }
          else
          {
            error = ERROR_GEN_FAILURE;
            break;
          }
        }

      }
    }

    ~key_with_values()
    {
      for (const auto& cur_value_name : value_names)
      {
        RegDeleteValueW(key, cur_value_name.c_str());
      }
    }
  private:
    std::list<std::wstring> value_names;
  };
}

reg_key::key::~key() {}

reg_key::key* reg_key::create_key(LONG& error, unsigned number_of_values_to_create)
{
  auto k{ new reg_key_cpp::key_with_values{error, number_of_values_to_create} };
  if (ERROR_SUCCESS != error)
  {
    delete k;
    k = nullptr;
  }

  return k;
}
