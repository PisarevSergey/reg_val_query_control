#pragma once

namespace reg_data_decoding
{
  struct decoded_data
  {
    void* data_buffer;
    ULONG data_type;
    ULONG data_length;
  };

  NTSTATUS decode_query_value_key_information(const REG_QUERY_VALUE_KEY_INFORMATION* info,
    bool use_mode_access,
    decoded_data& data);
}
