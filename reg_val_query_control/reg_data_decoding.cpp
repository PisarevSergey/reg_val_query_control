#include "common.h"
#include "reg_data_decoding.tmh"

using win_kernel_lib::safe_user_mode_data_access::copy_data;
using win_kernel_lib::safe_user_mode_data_access::is_valid_user_address;

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
        verbose_message(REG_DATA_DECODING, "data starts at %p", data.data_buffer);

        data.data_length = data_length;
        verbose_message(REG_DATA_DECODING, "data length is %x", data.data_length);

        data.data_type = info->Type;
        verbose_message(REG_DATA_DECODING, "value type is %S", support::get_reg_value_type_name(data.data_type));

        stat = STATUS_SUCCESS;
      }
      else
      {
        error_message(REG_DATA_DECODING, "invalid sizes, data offset is %x, data length is %x, buffer size is %x", data_offset, data_length, max_info_buffer_size);
        stat = STATUS_INVALID_BUFFER_SIZE;
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      stat = GetExceptionCode();
      error_message(REG_DATA_DECODING, "failed with status %!STATUS!", stat);
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
      verbose_message(REG_DATA_DECODING, "data starts at %p", data.data_buffer);

      const ULONG_PTR end_of_data{ reinterpret_cast<ULONG_PTR>(data.data_buffer) + data_length };
      verbose_message(REG_DATA_DECODING, "data ends before %llu", end_of_data);

      const ULONG_PTR start_of_buffer{reinterpret_cast<ULONG_PTR>(info)};
      verbose_message(REG_DATA_DECODING, "buffer starts at %llu", start_of_buffer);

      if ((end_of_data - start_of_buffer) <= max_info_buffer_size)
      {
        data.data_type = info->Type;
        verbose_message(REG_DATA_DECODING, "data type is %S", support::get_reg_value_type_name(data.data_type));

        data.data_length = data_length;
        verbose_message(REG_DATA_DECODING, "data length is %x", data.data_length);

        stat = STATUS_SUCCESS;
      }
      else
      {
        error_message(REG_DATA_DECODING, "wrong buffer size");
        stat = STATUS_INVALID_BUFFER_SIZE;
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      stat = GetExceptionCode();
      error_message(REG_DATA_DECODING, "failed with status %!STATUS!", stat);
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
      data.data_buffer = info->Data;
      verbose_message(REG_DATA_DECODING, "data starts at %p", data.data_buffer);

      const ULONG_PTR start_of_buffer{ reinterpret_cast<ULONG_PTR>(info) };
      verbose_message(REG_DATA_DECODING, "buffer starts at %llu", start_of_buffer);

      const ULONG data_length{ info->DataLength };
      verbose_message(REG_DATA_DECODING, "data length is %x", data_length);

      const ULONG_PTR end_of_data{ reinterpret_cast<ULONG_PTR>(data.data_buffer) + data_length };
      verbose_message(REG_DATA_DECODING, "data ends before %llu", end_of_data);

      if ((end_of_data - start_of_buffer) <= max_info_buffer_size)
      {
        data.data_type = info->Type;
        verbose_message(REG_DATA_DECODING, "value type is %S", support::get_reg_value_type_name(data.data_type));

        data.data_length = data_length;

        stat = STATUS_SUCCESS;
      }
      else
      {
        error_message(REG_DATA_DECODING, "wrong buffer sizes");
        stat = STATUS_INVALID_BUFFER_SIZE;
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      stat = GetExceptionCode();
      error_message(REG_DATA_DECODING, "failed with stastus %!STATUS!", stat);
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
    stat = copy_data(&max_info_buffer_size,
      sizeof(max_info_buffer_size),
      info->ResultLength,
      sizeof(*info->ResultLength),
      user_mode_access);
    if (NT_SUCCESS(stat))
    {
      verbose_message(REG_DATA_DECODING, "max info buffer size is %x", max_info_buffer_size);
    }
    else
    {
      error_message(REG_DATA_DECODING, "safe_user_mode_data_access::copy_data failed with status %!STATUS!", stat);
      break;
    }

    const bool is_data_buffer_valid{user_mode_access ? is_valid_user_address(info->KeyValueInformation, max_info_buffer_size, false) : true};
    if (is_data_buffer_valid)
    {
      verbose_message(REG_DATA_DECODING, "data buffer is valid");
    }
    else
    {
      error_message(REG_DATA_DECODING, "ivalid data buffer");
      stat = STATUS_INVALID_ADDRESS;
      break;
    }

    verbose_message(REG_DATA_DECODING, "value information class is %S", support::get_value_information_class_name(info->KeyValueInformationClass));

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

    if (NT_SUCCESS(stat))
    {
      verbose_message(REG_DATA_DECODING, "data decoding success");
    }
    else
    {
      error_message(REG_DATA_DECODING, "data decoding failed with status %!STATUS!", stat);
      break;
    }

    auto_unicode_string val_name_copy{static_cast<UNICODE_STRING*>(ExAllocatePoolWithTag(PagedPool, sizeof(UNICODE_STRING) + info->ValueName->Length, 'sqvR'))};
    if (val_name_copy.get())
    {
      verbose_message(REG_DATA_DECODING, "memory allocation for value name success");
    }
    else
    {
      stat = STATUS_INSUFFICIENT_RESOURCES;
      error_message(REG_DATA_DECODING, "failed to allocate memory for value name");
      break;
    }

    val_name_copy->Length = 0;
    val_name_copy->MaximumLength = info->ValueName->Length;
    val_name_copy->Buffer = static_cast<wchar_t*>(Add2Ptr(val_name_copy.get(), sizeof(UNICODE_STRING)));

    __try
    {
      RtlCopyUnicodeString(val_name_copy.get(), info->ValueName);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      stat = GetExceptionCode();
    }

    if (NT_SUCCESS(stat))
    {
      verbose_message(REG_DATA_DECODING, "copy value name success");
    }
    else
    {
      error_message(REG_DATA_DECODING, "failed to copy value name with status %!STATUS!", stat);
      break;
    }
    data.value_name.reset(val_name_copy.release());

    data.key_object = info->Object;

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

  auto_unicode_string local_unicode_string;

  __try
  {
    do
    {
      UNICODE_STRING value_name_local_copy;

      stat = copy_data(&value_name_local_copy,
        sizeof(value_name_local_copy),
        entry->ValueName,
        sizeof(*entry->ValueName),
        user_mode_access);
      if (NT_SUCCESS(stat))
      {
        verbose_message(REG_DATA_DECODING, "value name copy success");
      }
      else
      {
        error_message(REG_DATA_DECODING, "failed to copy value name with status %!STATUS!", stat);
        break;
      }

      if (!user_mode_access ||
        is_valid_user_address(value_name_local_copy.Buffer,
          value_name_local_copy.Length))
      {
        verbose_message(REG_DATA_DECODING, "value name buffer valid");
      }
      else
      {
        error_message(REG_DATA_DECODING, "value name buffer invalid");
        stat = STATUS_INVALID_PARAMETER;
        break;
      }

      local_unicode_string.reset(static_cast<UNICODE_STRING*>(ExAllocatePoolWithTag(PagedPool, sizeof(UNICODE_STRING) + value_name_local_copy.Length, 'sqvR')));
      if (local_unicode_string.get())
      {
        verbose_message(REG_DATA_DECODING, "value name memory allocated");
      }
      else
      {
        stat = STATUS_INSUFFICIENT_RESOURCES;
        error_message(REG_DATA_DECODING, "value name memory allocation failed");
        break;
      }

      local_unicode_string->Length = 0;
      local_unicode_string->MaximumLength = value_name_local_copy.Length;
      local_unicode_string->Buffer = static_cast<wchar_t*>(Add2Ptr(local_unicode_string.get(), sizeof(UNICODE_STRING)));
      RtlCopyUnicodeString(local_unicode_string.get(), &value_name_local_copy);

      data.value_name.reset(local_unicode_string.release());

      data.data_buffer = values_start + entry->DataOffset;
      verbose_message(REG_DATA_DECODING, "data starts at %p", data.data_buffer);

      data.data_length = entry->DataLength;
      verbose_message(REG_DATA_DECODING, "data length is %x", data.data_length);

      if ((static_cast<const char*>(data.data_buffer) + data.data_length) <= values_end)
      {
        verbose_message(REG_DATA_DECODING, "data buffer size valid");
      }
      else
      {
        error_message(REG_DATA_DECODING, "data buffer size invalid");
        stat = STATUS_INVALID_PARAMETER;
        break;
      }

      data.key_object = info->Object;
      data.data_type = entry->Type;
      verbose_message(REG_DATA_DECODING, "value data type is %S", support::get_reg_value_type_name(data.data_type));

    } while (false);

  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    stat = GetExceptionCode();
    error_message(REG_DATA_DECODING, "failed with status %!STATUS!", stat);
  }

  return stat;
}
