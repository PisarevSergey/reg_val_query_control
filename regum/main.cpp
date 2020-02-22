#include "common.h"

int wmain(int argc, wchar_t* argv[])
{
  MessageBoxW(0, L"attach", L"attach", MB_OK);

  if (argc >= 3)
  {
    std::wstring number_of_keys_to_create{ argv[1] }, number_of_values_per_key{ argv[2] };

    wcout << L"need to create " << number_of_keys_to_create << L" keys" << endl;
    wcout << L"each key will have " << number_of_values_per_key << L" values" << endl;

    HANDLE port;
    HRESULT result{ FilterConnectCommunicationPort(um_km_common::communication_port_name,
      0,
      nullptr,
      0,
      nullptr,
      &port) };
    if (S_OK == result)
    {
      wcout << L"port " << um_km_common::communication_port_name << L" successfully opened" << endl;

      LONG error{ ERROR_GEN_FAILURE };
      std::unique_ptr<opened_reg_keys::keys> keys{ opened_reg_keys::create_keys(error, std::stoul(number_of_keys_to_create), std::stoul(number_of_values_per_key)) };
      if (ERROR_SUCCESS == error)
      {
        wcout << L"keys created" << endl;
      }
      else
      {
        wcout << L"failed to create keys" << endl;
      }


      CloseHandle(port);
    }
    else
    {
      wcout << L"FilterConnectCommunicationPort failed to open port " << um_km_common::communication_port_name << L" with result " << result << endl;
    }

  }
  else
  {
    wcout << L"usage:" << endl;
    wcout << argv[0] << L" <number of keys to create> <number of values per key>" << endl;
  }


  return 0;
}
