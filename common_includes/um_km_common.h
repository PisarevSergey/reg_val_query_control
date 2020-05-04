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

    const wchar_t* get_buffer() const
    {
      return reinterpret_cast<const wchar_t*>(this + 1);
    }

    void* first_byte_after_string()
    {
      return (reinterpret_cast<char*>(get_buffer()) + buffer_size_in_bytes);
    }

    const void* first_byte_after_string() const
    {
      return (reinterpret_cast<const char*>(get_buffer()) + buffer_size_in_bytes);
    }
  };
  static_assert(2 == sizeof(counted_string), "wrong size");

#include <pshpack1.h>

  struct key_rule_header
  {
    fixed_size_handle key;
    unsigned __int32 number_of_values;

    counted_string* get_first_value_name()
    {
      return reinterpret_cast<counted_string*>(this + 1);
    }


    const counted_string* get_first_value_name() const
    {
      return reinterpret_cast<const counted_string*>(this + 1);
    }
  };
  static_assert(12 == sizeof(key_rule_header), "wrong size");

  struct rules_header
  {
    unsigned __int32 number_of_rules;
  };
  static_assert(4 == sizeof(rules_header), "wrong size");

  enum class request_type : unsigned __int8
  {
    invalid = 0,
    set_rules,
    clear_rules
  };

  struct request
  {
    request_type rt;
    union
    {
      rules_header rh;
    } request_fixed_part_header;

    void* get_request_specific_data()
    {
      return (this + 1);
    }

    const void* get_request_specific_data() const
    {
      return (this + 1);
    }
  };

#include <poppack.h>

}
