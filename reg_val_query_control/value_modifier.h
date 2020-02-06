#pragma once

namespace value_modifier
{
  class modifier
  {
  public:
    virtual void modify(const reg_data_decoding::decoded_data& data) = 0;
    virtual void modify(PCUNICODE_STRING& key_path, const reg_data_decoding::decoded_data& data) = 0;
    virtual ~modifier() = 0;
    void __cdecl operator delete(void*);
  };

  modifier* create_modifier(NTSTATUS& stat);
}
