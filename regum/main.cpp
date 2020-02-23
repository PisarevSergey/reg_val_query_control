#include "common.h"

namespace main_cpp
{
  class handle_closer
  {
  public:
    void operator()(HANDLE* h)
    {
      CloseHandle(*h);
    }
  };
}

int wmain(int argc, wchar_t* argv[])
{
  MessageBoxW(0, L"attach", L"attach", MB_OK);

  do
  {
    if (argc < 3)
    {
      wcout << L"usage:" << endl;
      wcout << argv[0] << L" <number of keys to create> <number of values per key>" << endl;
      break;
    }

    std::wstring number_of_keys_to_create{ argv[1] }, number_of_values_per_key{ argv[2] };

    wcout << L"need to create " << number_of_keys_to_create << L" keys" << endl;
    wcout << L"each key will have " << number_of_values_per_key << L" values" << endl;

    HANDLE port_raw;
    HRESULT result{ FilterConnectCommunicationPort(um_km_common::communication_port_name,
      0,
      nullptr,
      0,
      nullptr,
      &port_raw) };
    if (result != S_OK)
    {
      wcout << L"FilterConnectCommunicationPort failed to open port " << um_km_common::communication_port_name << L" with result " << result << endl;
      break;
    }

    wcout << L"port " << um_km_common::communication_port_name << L" successfully opened" << endl;
    std::unique_ptr<HANDLE, main_cpp::handle_closer> port{ &port_raw };

    LONG error{ ERROR_GEN_FAILURE };
    std::unique_ptr<opened_reg_keys::keys> keys{ opened_reg_keys::create_keys(error, std::stoul(number_of_keys_to_create), std::stoul(number_of_values_per_key)) };
    if (ERROR_SUCCESS != error)
    {
      wcout << L"failed to create keys" << endl;
      break;
    }
    wcout << L"keys created" << endl;

    std::list<reg_key::naked_key_and_vals> keys_info;
    keys->get_keys_and_vals(keys_info);

    um_km_common::request* rules{ nullptr };
    size_t rule_buffer_size{ sizeof(*rules) };
    for (const auto& c : keys_info)
    {
      rule_buffer_size += sizeof(um_km_common::key_rule_header);
      for (const auto& cur_val_name : c.value_names)
      {
        rule_buffer_size += (sizeof(um_km_common::counted_string) + cur_val_name.size() * sizeof(wchar_t));
      }
    }

    std::unique_ptr<char[]> request_buffer{new char[rule_buffer_size]};

    rules = reinterpret_cast<um_km_common::request*>(request_buffer.get());
    memset(rules, 0x00, rule_buffer_size);

    rules->rt = um_km_common::request_type::set_rules;

    auto current_position{static_cast<char*>(rules->get_request_specific_data())};
    for (const auto& c : keys_info)
    {
      um_km_common::key_rule_header* k_r_h{reinterpret_cast<um_km_common::key_rule_header*>(current_position)};
      k_r_h->key.handle_val = c.key;
      k_r_h->number_of_values = 0;

      auto cur_val_name_in_rule{ k_r_h->get_first_value_name() };
      for (const auto& cur_val_name : c.value_names)
      {
        cur_val_name_in_rule->buffer_size_in_bytes = static_cast<unsigned short>(cur_val_name.size() * sizeof(wchar_t));
        wmemcpy(cur_val_name_in_rule->get_buffer(), cur_val_name.c_str(), cur_val_name.size());
        ++k_r_h->number_of_values;
        cur_val_name_in_rule = static_cast<um_km_common::counted_string*>(cur_val_name_in_rule->first_byte_after_string());
      }
      current_position = reinterpret_cast<char*>(cur_val_name_in_rule);
    }
    assert((request_buffer.get() + rule_buffer_size) == current_position);

    DWORD returned;
    result = FilterSendMessage(*port.get(), rules, rule_buffer_size, nullptr, 0, &returned);
    if (S_OK != result)
    {
      wcout << L"FilterSendMessage failed with result " << result << endl;
      break;
    }
    wcout << L"rules has been successfully set" << endl;

  } while (false);

  return 0;
}
