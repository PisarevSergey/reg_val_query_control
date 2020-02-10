#include "common.h"
#include "value_modifier.tmh"

namespace value_modifier_cpp
{
  class modifier_with_rules : public value_modifier::modifier
  {
  public:
    static void* alloc(CLONG size)
    {
      return ExAllocatePoolWithTag(PagedPool, size, 'eluR');
    }

    static void free(void* p)
    {
      if (p)
      {
        ExFreePool(p);
      }
    }

  private:
    win_kernel_lib::avl_list_facility::avl_list<rule_facility::rule, alloc, free> rules;
    win_kernel_lib::locks::eresource rules_guard;
  };

  class modifier_impl : public modifier_with_rules
  {
  public:
    void modify(const reg_data_decoding::decoded_data& data) override
    {
      PCUNICODE_STRING key_path{nullptr};
      modify(key_path, data);
    }

    void modify(PCUNICODE_STRING& key_path, const reg_data_decoding::decoded_data& data)
    {
      NTSTATUS stat{ key_path ? STATUS_SUCCESS : CmCallbackGetKeyObjectID(get_driver()->get_reg_cookie(), data.key_object, nullptr, &key_path) };

      do
      {
        if (NT_SUCCESS(stat))
        {
          verbose_message(VALUE_MODIFIER, "key path is %wZ", key_path);
        }
        else
        {
          error_message(VALUE_MODIFIER, "CmCallbackGetKeyObjectID failed with status %!STATUS!", stat);
          key_path = nullptr;
          break;
        }



      } while (false);

    }
  };

  class top_modifier final : public modifier_impl
  {
  public:
    void* __cdecl operator new(size_t, void* p)
    {
      return p;
    }
  };

  char modifier_memory[sizeof(top_modifier)];
}

value_modifier::modifier::~modifier() {}

void __cdecl value_modifier::modifier::operator delete(void*)
{}


value_modifier::modifier* value_modifier::create_modifier(NTSTATUS& stat)
{
  stat = STATUS_SUCCESS;
  return new (value_modifier_cpp::modifier_memory) value_modifier_cpp::top_modifier;
}