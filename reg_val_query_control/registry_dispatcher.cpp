#include "registry_dispatcher.h"
#include "reg_data_decoding.h"

#include <safe_user_mode_data_access.h>

#include "tracing.h"
#include "registry_dispatcher.tmh"

namespace registry_dispatcher_cpp
{
  class registry_dispatcher_impl : public registry_dispatcher::dispatcher
  {
  public:
    NTSTATUS callback(REG_NOTIFY_CLASS reg_op, void* reg_op_data)
    {
      //{
      //  PCUNICODE_STRING key_name{nullptr};
      //  NTSTATUS stat{ CmCallbackGetKeyObjectID(get_driver()->get_reg_cookie(), static_cast<REG_POST_OPERATION_INFORMATION*>(reg_op_data)->Object, nullptr, &key_name) };
      //  if (NT_SUCCESS(stat))
      //  {
      //    info_message(REGISTRY_DISPATCHER, "CmCallbackGetKeyObjectID success, key name is %wZ", key_name);
      //  }
      //  else
      //  {
      //    error_message(REGISTRY_DISPATCHER, "CmCallbackGetKeyObjectID failed with status %!STATUS!", stat);
      //  }
      //}

      switch (reg_op)
      {
      case RegNtPostQueryValueKey:
        dispatch_post_query_value_key(static_cast<REG_POST_OPERATION_INFORMATION*>(reg_op_data));
        break;
      case RegNtPostQueryMultipleValueKey:
        dispatch_post_query_multiple_value_key(static_cast<REG_POST_OPERATION_INFORMATION*>(reg_op_data));
        break;
      }
      //RegNtPreQueryValueKey	REG_QUERY_VALUE_KEY_INFORMATION
      //RegNtPostQueryValueKey	REG_POST_OPERATION_INFORMATION
      //RegNtPreQueryMultipleValueKey	REG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION
      //RegNtPostQueryMultipleValueKey	REG_POST_OPERATION_INFORMATION

      return STATUS_SUCCESS;
    }

  private:
    void dispatch_post_query_value_key(REG_POST_OPERATION_INFORMATION* post_op_info)
    {
      if (NT_SUCCESS(post_op_info->Status))
      {
        auto pre_info{ static_cast<const REG_QUERY_VALUE_KEY_INFORMATION*>(post_op_info->PreInformation) };

        switch (pre_info->KeyValueInformationClass)
        {
        case KeyValueFullInformation:
        case KeyValueFullInformationAlign64:
        case KeyValuePartialInformation:
        case KeyValuePartialInformationAlign64:
        {
          reg_data_decoding::decoded_data operation_data;
          NTSTATUS stat = reg_data_decoding::decode_query_value_key_information(pre_info,
            (UserMode == ExGetPreviousMode()),
            operation_data);
          stat = stat;
        }
        break;
        }
      }
    }

    void dispatch_post_query_multiple_value_key(REG_POST_OPERATION_INFORMATION* post_op_info)
    {
      do
      {
        if (NT_SUCCESS(post_op_info->Status))
        {
        }
        else
        {
          break;
        }

        auto pre_info{ static_cast<REG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION*>(post_op_info->PreInformation) };
        const size_t value_entries_size{ pre_info->EntryCount * sizeof(pre_info->ValueEntries[0]) };
        const bool user_mode_access{ UserMode == ExGetPreviousMode() };

        if (!user_mode_access ||
            safe_user_mode_data_access::is_valid_user_address(pre_info->ValueEntries, value_entries_size))
        {
        }
        else
        {
          break;
        }

        ULONG max_buffer_size{ 0 };
        NTSTATUS stat{ safe_user_mode_data_access::copy_data(&max_buffer_size,
            sizeof(max_buffer_size),
            pre_info->BufferLength,
            sizeof(*pre_info->BufferLength),
            user_mode_access) };
        if (NT_SUCCESS(stat))
        {
        }
        else
        {
          break;
        }

        if ((!user_mode_access) ||
          safe_user_mode_data_access::is_valid_user_address(pre_info->ValueBuffer, max_buffer_size, false))
        {
        }
        else
        {
          break;
        }

        const char* const value_buffer_start{ reinterpret_cast<char*>(pre_info->ValueBuffer) };
        const char* const value_buffer_end{ value_buffer_start + max_buffer_size };

        for (decltype(pre_info->EntryCount) i{ 0 }; i < pre_info->EntryCount; ++i)
        {
          reg_data_decoding::decoded_data operation_data;
          stat = reg_data_decoding::decode_single_value_entry(pre_info,
            pre_info->ValueEntries + i,
            value_buffer_start,
            value_buffer_end,
            user_mode_access,
            operation_data);
        }

      } while (false);

    }
  };

  class top_dispatcher final : public registry_dispatcher_impl
  {
  public:
    void* __cdecl operator new(size_t sz)
    {
      return ExAllocatePoolWithTag(NonPagedPool, sz, 'sidR');
    }
  };
}

registry_dispatcher::dispatcher::~dispatcher() {}

void __cdecl registry_dispatcher::dispatcher::operator delete(void* p)
{
  if (p)
  {
    ExFreePool(p);
  }
}

registry_dispatcher::dispatcher* registry_dispatcher::create_dispatcher(NTSTATUS& stat)
{
  auto p{new registry_dispatcher_cpp::top_dispatcher};

  stat = (p ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES);

  return p;
}
