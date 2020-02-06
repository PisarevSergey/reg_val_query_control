#pragma once

using win_kernel_lib::smart_pointers::auto_pointer;
using win_kernel_lib::deleters::pool_deleter;

namespace rule_facility
{
  class rule final
  {
  public:
    rule(auto_pointer<UNICODE_STRING, pool_deleter>& src_str) noexcept;

    void* __cdecl operator new(size_t, void*) noexcept;
    void __cdecl operator delete(void*) noexcept;

  private:
    auto_pointer<UNICODE_STRING, pool_deleter> reg_key_path;
  };
}
