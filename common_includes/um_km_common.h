#pragma once

namespace um_km_common
{
  const wchar_t communication_port_name[] = L"\\reg_val_query_control_port";

  union fixed_size_handle
  {
    unsigned __int64 int_val;
    HANDLE handle_val;
  };
  static_assert(8 == sizeof(fixed_size_handle), "wrong size");

  struct counted_string
  {
    unsigned __int16 buffer_size_in_bytes;

    wchar_t* get_buffer()
    {
      return reinterpret_cast<wchar_t*>(this + 1);
    }

    void* first_byte_after_string()
    {
      return win_kernel_lib::pointer_manipulation::add_to_ptr<void, wchar_t>(get_buffer(), buffer_size_in_bytes);
    }
  };
  static_assert(2 == sizeof(counted_string), "wrong size");

#include <pshpack1.h>

  struct key_rule_header
  {
    fixed_size_handle root_key_object;
    unsigned __int32 number_of_values;
    counted_string relative_key_name;

    counted_string* get_first_value_name()
    {
      return static_cast<counted_string*>(relative_key_name.first_byte_after_string());
    }
  };
  static_assert(14 == sizeof(key_rule_header), "wrong size");

  enum class request_type : unsigned __int8
  {
    invalid = 0,
    set_rules
  };

  struct request
  {
    request_type rt;

    void* get_request_specific_data()
    {
      return (this + 1);
    }
  };
  static_assert(sizeof(request::rt) == sizeof(request), "wrong size");

#include <poppack.h>

}
