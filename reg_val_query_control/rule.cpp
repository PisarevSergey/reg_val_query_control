#include "common.h"
#include "rule.tmh"

using win_kernel_lib::string_facility::string;

rule_facility::rule::rule() : value_names{ &val_names_allocator }
{}

void rule_facility::rule::set_reg_key(auto_pointer<const UNICODE_STRING, pool_deleter>& reg_key_param) noexcept
{
  reg_key_path.reset(reg_key_param.release());
}

void rule_facility::rule::set_reg_key(const UNICODE_STRING* reg_key_param) noexcept
{
  reg_key_path.reset(reg_key_param);
}

NTSTATUS rule_facility::rule::add_value_name(auto_pointer<UNICODE_STRING, pool_deleter>& value_name) noexcept
{
  NTSTATUS stat{ STATUS_INSUFFICIENT_RESOURCES };

  string* val_string{ new (PagedPool, 'nlaV') string};
  if (val_string)
  {
    val_string->reset(value_name.release());

    bool inserted{false};
    value_names.insert(val_string, inserted);
    if (inserted)
    {
      stat = STATUS_SUCCESS;
      verbose_message(RULE, "value name was successfully inserted");
    }
    else
    {
      stat = STATUS_INSUFFICIENT_RESOURCES;
      error_message(RULE, "failed to insert value name");

      delete val_string;
    }
  }

  return stat;
}

void* rule_facility::value_names_allocator::allocate(CLONG size)
{
  return ExAllocatePoolWithTag(PagedPool, size, 'elaV');
}

void rule_facility::value_names_allocator::deallocate(void* p)
{
  if (p)
  {
    ExFreePool(p);
  }
}

void* __cdecl rule_facility::rule::operator new(size_t, void* p) noexcept
{
  return p;
}

void* __cdecl rule_facility::rule::operator new(size_t sz) noexcept
{
  ASSERT(sizeof(rule) == sz);

  return ExAllocatePoolWithTag(PagedPool, sz, 'lrqR');
}

void __cdecl rule_facility::rule::operator delete(void* p) noexcept
{
  if (p)
  {
    ExFreePool(p);
  }
}

bool rule_facility::rule::is_value_in_rule(const UNICODE_STRING& value_to_search)
{
  bool in_list{ false };

  win_kernel_lib::string_facility::string* search_string{ nullptr };
  char search_string_memory[sizeof(*search_string)];

  search_string = new(search_string_memory)string;
  search_string->reset(const_cast<UNICODE_STRING*>(&value_to_search));

  if (value_names.get_element_by_key(search_string))
  {
    in_list = true;
  }

  return in_list;
}

bool rule_facility::operator<(const rule& a, const rule& b)
{
  return (RtlCompareUnicodeString(a.reg_key_path.get(), \
    b.reg_key_path.get(), \
    TRUE) < 0);
}

bool rule_facility::operator>(const rule& a, const rule& b)
{
  return (RtlCompareUnicodeString(a.reg_key_path.get(), \
    b.reg_key_path.get(), \
    TRUE) > 0);
}
