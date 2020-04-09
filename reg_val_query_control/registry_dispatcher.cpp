#include "common.h"
#include "registry_dispatcher.tmh"

using win_kernel_lib::safe_user_mode_data_access::copy_data;
using win_kernel_lib::safe_user_mode_data_access::is_valid_user_address;
using win_kernel_lib::smart_pointers::auto_pointer;

namespace registry_dispatcher_cpp
{
  class registry_dispatcher_with_modifier : public registry_dispatcher::dispatcher
  {
  public:
    registry_dispatcher_with_modifier(NTSTATUS& stat) : modif{ value_modifier::create_modifier(stat) }
    {
      if (NT_SUCCESS(stat))
      {
        info_message(REGISTRY_DISPATCHER, "value_modifier::create_modifier success");
      }
      else
      {
        error_message(REGISTRY_DISPATCHER, "value_modifier::create_modifier failed with status %!STATUS!", stat);
      }
    }

    NTSTATUS set_rules(unsigned __int32 number_of_rules, um_km_common::key_rule_header* rules, ULONG rules_size)
    {
      return modif->set_rules(number_of_rules, rules, rules_size);
    }
  protected:
    auto_pointer<value_modifier::modifier> modif;
  };

  class registry_dispatcher_impl : public registry_dispatcher_with_modifier
  {
  public:
    registry_dispatcher_impl(NTSTATUS& stat) : registry_dispatcher_with_modifier(stat)
    {}

    NTSTATUS callback(REG_NOTIFY_CLASS reg_op, void* reg_op_data)
    {
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
            modif->modify(operation_data);
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
            is_valid_user_address(pre_info->ValueEntries, value_entries_size))
        {
          verbose_message(REGISTRY_DISPATCHER, "value entries buffer valid");
        }
        else
        {
          error_message(REGISTRY_DISPATCHER, "value entries buffer invalid");
          break;
        }

        ULONG max_buffer_size{ 0 };
        NTSTATUS stat{ copy_data(&max_buffer_size,
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
          is_valid_user_address(pre_info->ValueBuffer, max_buffer_size, false))
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
            PCUNICODE_STRING key_path{ nullptr };
            modif->modify(key_path, operation_data);
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
    top_dispatcher(NTSTATUS& stat) : registry_dispatcher_impl(stat)
    {}

    void* __cdecl operator new(size_t, void* p)
    {
      return p;
    }
  };

  char registry_dispatcher_memory[sizeof(top_dispatcher)];
}

registry_dispatcher::dispatcher::~dispatcher() {}

void __cdecl registry_dispatcher::dispatcher::operator delete(void*)
{}

registry_dispatcher::dispatcher* registry_dispatcher::create_dispatcher(NTSTATUS& stat)
{
  auto p{ new (registry_dispatcher_cpp::registry_dispatcher_memory) registry_dispatcher_cpp::top_dispatcher(stat) };

  if (NT_SUCCESS(stat))
  {
    info_message(REGISTRY_DISPATCHER, "registry dispatcher create success");
  }
  else
  {
    error_message(REGISTRY_DISPATCHER, "registry dispatcher create failed with status %!STATUS!", stat);
    delete p;
    p = nullptr;
  }

  return p;
}
