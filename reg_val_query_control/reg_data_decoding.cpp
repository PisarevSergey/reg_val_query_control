#include "rvqc_common.h"

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
    const bool is_data_buffer_size_valid{user_mode_access ? safe_user_mode_data_access::is_valid_user_address(info->ResultLength, sizeof(*info->ResultLength)) : true};
    if (is_data_buffer_size_valid)
    {
    }
    else
    {
      stat = STATUS_INVALID_ADDRESS;
      break;
    }

    ULONG max_info_buffer_size{ 0 };
    __try
    {
      max_info_buffer_size = *info->ResultLength;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      stat = GetExceptionCode();
      break;
    }

    const bool is_data_buffer_valid{user_mode_access ? safe_user_mode_data_access::is_valid_user_address(info->KeyValueInformation, max_info_buffer_size) : true};
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
      //ASSERT(FALSE);
      stat = STATUS_NOT_SUPPORTED;
    }

  } while (false);

  return stat;
}