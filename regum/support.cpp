#include "common.h"

RPC_STATUS support::create_random_string(std::wstring& str)
{
  UUID guid;
  RPC_STATUS stat{ UuidCreate(&guid) };
  if (RPC_S_OK == stat)
  {
    unsigned short* str_buffer{ nullptr };
    stat = UuidToStringW(&guid, &str_buffer);
    if (RPC_S_OK == stat)
    {
      str.assign(reinterpret_cast<wchar_t*>(str_buffer));
      RpcStringFreeW(&str_buffer);

      wcout << L"generated string " << str << endl;
    }
    else
    {
      wcout << L"UuidToStringW failed with status " << stat << endl;
    }
  }
  else
  {
    wcout << L"UuidCreate failed with status " << stat << endl;
  }

  return stat;
}
