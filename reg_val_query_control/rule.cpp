#include "common.h"

rule_facility::rule::rule(auto_pointer<UNICODE_STRING, pool_deleter>& src_str) noexcept : reg_key_path{src_str.release()}
{}

void* __cdecl rule_facility::rule::operator new(size_t sz, void* p) noexcept
{
  UNREFERENCED_PARAMETER(sz);
  ASSERT(sizeof(rule) == sz);

  return p;
}

void __cdecl rule_facility::rule::operator delete(void*) noexcept
{}
