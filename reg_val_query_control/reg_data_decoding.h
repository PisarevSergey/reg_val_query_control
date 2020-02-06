#pragma once

namespace reg_data_decoding
{
  struct decoded_data
  {
    void* key_object;
    UNICODE_STRING value_name;
    const void* data_buffer;
    ULONG data_type;
    ULONG data_length;
  };

  NTSTATUS decode_query_value_key_information(const REG_QUERY_VALUE_KEY_INFORMATION* info,
    bool use_mode_access,
    decoded_data& data);

  NTSTATUS decode_single_value_entry(const REG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION* info,
    const KEY_VALUE_ENTRY* entry,
    const char* values_start,
    const char* values_end,
    bool user_mode_access,
    decoded_data& data);
}
