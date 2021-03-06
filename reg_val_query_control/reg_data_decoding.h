#pragma once

namespace reg_data_decoding
{
  using auto_unicode_string = win_kernel_lib::smart_pointers::auto_pointer<UNICODE_STRING, win_kernel_lib::deleters::pool_deleter>;

  struct decoded_data
  {
    void* key_object;
    auto_unicode_string value_name;
    void* data_buffer;
    ULONG data_type;
    ULONG data_length;
  };

  NTSTATUS decode_query_value_key_information(const REG_QUERY_VALUE_KEY_INFORMATION* info,
    bool use_mode_access,
    decoded_data& data);

  NTSTATUS decode_single_value_entry(const REG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION* info,
    const KEY_VALUE_ENTRY* entry,
    char* values_start,
    const char* values_end,
    bool user_mode_access,
    decoded_data& data);
}
