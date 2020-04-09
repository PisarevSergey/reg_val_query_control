#include "common.h"
#include "rule.tmh"

using win_kernel_lib::string_facility::string;

void rule_facility::rule::set_reg_key(auto_pointer<UNICODE_STRING, pool_deleter>& reg_key_param) noexcept
{
  reg_key_path.reset(reg_key_param.release());
}


NTSTATUS rule_facility::rule::add_value_name(auto_pointer<UNICODE_STRING, pool_deleter>& value_name) noexcept
{
  NTSTATUS stat{ STATUS_INSUFFICIENT_RESOURCES };

  string* val_string{ new (PagedPool, 'nlaV') string};
  if (val_string)
  {
    val_string->reset(value_name.release());

    auto inserted{ value_names.insert(val_string) };
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

void* rule_facility::rule::alloc_paged(CLONG size)
{
  return ExAllocatePoolWithTag(PagedPool, size, 'elaV');
}

void rule_facility::rule::free(void* p)
{
  if (p)
  {
    ExFreePool(p);
  }
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
