#include "common.h"

void rule_facility::rule::set_reg_key(auto_pointer<UNICODE_STRING, pool_deleter>& reg_key_param) noexcept
{
  reg_key_path.reset(reg_key_param.release());
}

void* __cdecl rule_facility::rule::operator new(size_t sz, void* p) noexcept
{
  UNREFERENCED_PARAMETER(sz);
  ASSERT(sizeof(rule) == sz);

  return p;
}

void __cdecl rule_facility::rule::operator delete(void*) noexcept
{}

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
