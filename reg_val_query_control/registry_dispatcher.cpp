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
        verbose_message(REGISTRY_DISPATCHER, "RegNtPostQueryValueKey operation");
        dispatch_post_query_value_key(static_cast<REG_POST_OPERATION_INFORMATION*>(reg_op_data));
        break;
      case RegNtPostQueryMultipleValueKey:
        verbose_message(REGISTRY_DISPATCHER, "RegNtPostQueryMultipleValueKey operation");
        dispatch_post_query_multiple_value_key(static_cast<REG_POST_OPERATION_INFORMATION*>(reg_op_data));
        break;
      }

      return STATUS_SUCCESS;
    }

  private:
    void dispatch_post_query_value_key(REG_POST_OPERATION_INFORMATION* post_op_info)
    {
      if (NT_SUCCESS(post_op_info->Status))
      {
        verbose_message(REGISTRY_DISPATCHER, "operation completed successfully, dispatching");

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
          if (NT_SUCCESS(stat))
          {
            verbose_message(REGISTRY_DISPATCHER, "successfully decoded data");
          }
          else
          {
            error_message(REGISTRY_DISPATCHER, "failed to decode data with status %!STATUS!", stat);
          }
        }
        break;
        }
      }
      else
      {
        verbose_message(REGISTRY_DISPATCHER, "operation failed, skipping");
      }
    }

    void dispatch_post_query_multiple_value_key(REG_POST_OPERATION_INFORMATION* post_op_info)
    {
      do
      {
        if (NT_SUCCESS(post_op_info->Status))
        {
          verbose_message(REGISTRY_DISPATCHER, "operation completed successfully, dispatching");
        }
        else
        {
          verbose_message(REGISTRY_DISPATCHER, "operation failed, skipping");
          break;
        }

        auto pre_info{ static_cast<REG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION*>(post_op_info->PreInformation) };

        const size_t value_entries_size{ pre_info->EntryCount * sizeof(pre_info->ValueEntries[0]) };
        verbose_message(REGISTRY_DISPATCHER, "value entries size is %llu", value_entries_size);

        const bool user_mode_access{ UserMode == ExGetPreviousMode() };
        verbose_message(REGISTRY_DISPATCHER, "%s mode access", (user_mode_access ? "user" : "kernel"));

        if (!user_mode_access ||
            safe_user_mode_data_access::is_valid_user_address(pre_info->ValueEntries, value_entries_size))
        {
          verbose_message(REGISTRY_DISPATCHER, "value entries buffer valid");
        }
        else
        {
          error_message(REGISTRY_DISPATCHER, "value entries buffer invalid");
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
          verbose_message(REGISTRY_DISPATCHER, "max buffer size is %x", max_buffer_size);
        }
        else
        {
          error_message(REGISTRY_DISPATCHER, "safe_user_mode_data_access::copy_data failed with status %!STATUS!", stat);
          break;
        }

        if ((!user_mode_access) ||
          safe_user_mode_data_access::is_valid_user_address(pre_info->ValueBuffer, max_buffer_size, false))
        {
          verbose_message(REGISTRY_DISPATCHER, "value buffer valid");
        }
        else
        {
          error_message(REGISTRY_DISPATCHER, "value buffer invalid");
          break;
        }

        const char* const value_buffer_start{ reinterpret_cast<char*>(pre_info->ValueBuffer) };
        verbose_message(REGISTRY_DISPATCHER, "value buffer starts at %p", value_buffer_start);

        const char* const value_buffer_end{ value_buffer_start + max_buffer_size };
        verbose_message(REGISTRY_DISPATCHER, "value buffer ends before %p", value_buffer_end);

        reg_data_decoding::decoded_data operation_data;
        for (decltype(pre_info->EntryCount) i{ 0 }; i < pre_info->EntryCount; ++i)
        {
          verbose_message(REGISTRY_DISPATCHER, "decoding entry number %x", i);
          stat = reg_data_decoding::decode_single_value_entry(pre_info,
            pre_info->ValueEntries + i,
            value_buffer_start,
            value_buffer_end,
            user_mode_access,
            operation_data);
          if (NT_SUCCESS(stat))
          {
            verbose_message(REGISTRY_DISPATCHER, "decoding success");
          }
          else
          {
            error_message(REGISTRY_DISPATCHER, "decoding failed with status %!STATUS!", stat);
          }
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
