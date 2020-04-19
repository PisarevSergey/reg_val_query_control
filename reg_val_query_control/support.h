#pragma once

namespace support
{
  using win_kernel_lib::smart_pointers::auto_pointer;
  using win_kernel_lib::deleters::pool_deleter;

  const wchar_t* get_reg_value_type_name(ULONG type);
  const wchar_t* get_value_information_class_name(KEY_VALUE_INFORMATION_CLASS info_class);
  NTSTATUS query_name_for_user_mode_key_handle(HANDLE user_mode_key_handle, auto_pointer<const UNICODE_STRING, pool_deleter>& key_path);

  bool is_entity_inside_buffer(const void* base, const void* first_byte_after_buffer, const void* entity_base, size_t entity_size);
}
