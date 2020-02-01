#include "reg_data_decoding.h"
#include <safe_user_mode_data_access.h>

#include "tracing.h"
#include "reg_data_decoding.tmh"

namespace reg_data_decoding_cpp
{
  NTSTATUS decode_value_full_information(KEY_VALUE_FULL_INFORMATION* info,
    const ULONG max_info_buffer_size,
    reg_data_decoding::decoded_data& data)
  {
    NTSTATUS stat{STATUS_UNSUCCESSFUL};

    __try
    {
      const ULONG data_offset{info->DataOffset}, data_length{info->DataLength};
      if ((data_offset + data_length) <= max_info_buffer_size)
      {
        data.data_buffer = (reinterpret_cast<char*>(info) + data_offset);
        data.data_type = info->Type;
        data.data_length = data_length;

        stat = STATUS_SUCCESS;
      }
      else
      {
        stat = STATUS_INVALID_BUFFER_SIZE;
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      stat = GetExceptionCode();
    }

    return stat;
  }

  NTSTATUS decode_value_partial_information_align_64(KEY_VALUE_PARTIAL_INFORMATION_ALIGN64* info,
    const ULONG max_info_buffer_size,
    reg_data_decoding::decoded_data& data)
  {
    NTSTATUS stat{ STATUS_UNSUCCESSFUL };

    __try
    {
      const ULONG data_length{ info->DataLength };
      data.data_buffer = info->Data;
      const ULONG_PTR end_of_buffer{ reinterpret_cast<ULONG_PTR>(data.data_buffer) + data_length };
      const ULONG_PTR start_of_buffer{reinterpret_cast<ULONG_PTR>(info)};

      if ((end_of_buffer - start_of_buffer) <= max_info_buffer_size)
      {
        data.data_type = info->Type;
        data.data_length = data_length;

        stat = STATUS_SUCCESS;
      }
      else
      {
        stat = STATUS_INVALID_BUFFER_SIZE;
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      stat = GetExceptionCode();
    }

    return stat;
  }

  NTSTATUS decode_value_partial_information(KEY_VALUE_PARTIAL_INFORMATION* info,
    const ULONG max_info_buffer_size,
    reg_data_decoding::decoded_data& data)
  {
    NTSTATUS stat{ STATUS_UNSUCCESSFUL };

    __try
    {
      const ULONG data_length{ info->DataLength };
      data.data_buffer = info->Data;
      const ULONG_PTR end_of_buffer{ reinterpret_cast<ULONG_PTR>(data.data_buffer) + data_length };
      const ULONG_PTR start_of_buffer{ reinterpret_cast<ULONG_PTR>(info) };

      if ((end_of_buffer - start_of_buffer) <= max_info_buffer_size)
      {
        data.data_type = info->Type;
        data.data_length = data_length;

        stat = STATUS_SUCCESS;
      }
      else
      {
        stat = STATUS_INVALID_BUFFER_SIZE;
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      stat = GetExceptionCode();
    }

    return stat;
  }

}

NTSTATUS reg_data_decoding::decode_query_value_key_information(const REG_QUERY_VALUE_KEY_INFORMATION* info,
  bool user_mode_access,
  decoded_data& data)
{
  NTSTATUS stat{STATUS_UNSUCCESSFUL};

  do
  {
    ULONG max_info_buffer_size{ 0 };
    stat = safe_user_mode_data_access::copy_data(&max_info_buffer_size,
      sizeof(max_info_buffer_size),
      info->ResultLength,
      sizeof(*info->ResultLength),
      user_mode_access);
    if (NT_SUCCESS(stat))
    {
    }
    else
    {
      break;
    }

    const bool is_data_buffer_valid{user_mode_access ? safe_user_mode_data_access::is_valid_user_address(info->KeyValueInformation, max_info_buffer_size, false) : true};
    if (is_data_buffer_valid)
    {
    }
    else
    {
      stat = STATUS_INVALID_ADDRESS;
      break;
    }

    switch (info->KeyValueInformationClass)
    {
    case KeyValueFullInformation:
    case KeyValueFullInformationAlign64:
      stat = reg_data_decoding_cpp::decode_value_full_information(static_cast<KEY_VALUE_FULL_INFORMATION*>(info->KeyValueInformation),
        max_info_buffer_size,
        data);
      break;
    case KeyValuePartialInformation:
      stat = reg_data_decoding_cpp::decode_value_partial_information(static_cast<KEY_VALUE_PARTIAL_INFORMATION*>(info->KeyValueInformation),
        max_info_buffer_size,
        data);
      break;
    case KeyValuePartialInformationAlign64:
      stat = reg_data_decoding_cpp::decode_value_partial_information_align_64(static_cast<KEY_VALUE_PARTIAL_INFORMATION_ALIGN64*>(info->KeyValueInformation),
        max_info_buffer_size,
        data);
      break;
    default:
      ASSERT(FALSE);
      stat = STATUS_NOT_SUPPORTED;
    }

  } while (false);

  return stat;
}

NTSTATUS reg_data_decoding::decode_single_value_entry(const REG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION* info,
  const KEY_VALUE_ENTRY* entry,
  const char* values_start,
  const char* values_end,
  bool user_mode_access,
  decoded_data& data)
{
  NTSTATUS stat{STATUS_SUCCESS};

  __try
  {
    do
    {
      stat = safe_user_mode_data_access::copy_data(&data.value_name, sizeof(data.value_name), entry->ValueName, sizeof(*entry->ValueName), user_mode_access);
      if (NT_SUCCESS(stat))
      {
      }
      else
      {
        break;
      }

      if (!user_mode_access ||
        safe_user_mode_data_access::is_valid_user_address(data.value_name.Buffer, data.value_name.Length))
      {
      }
      else
      {
        break;
      }

      data.data_buffer = values_start + entry->DataOffset;
      data.data_length = entry->DataLength;
      if ((static_cast<const char*>(data.data_buffer) + data.data_length) <= values_end)
      {
      }
      else
      {
        stat = STATUS_INVALID_PARAMETER;
      }

      data.key_object = info->Object;
      data.data_type = entry->Type;

    } while (false);

  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    stat = GetExceptionCode();
  }

  return stat;
}
