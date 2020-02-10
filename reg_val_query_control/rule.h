#pragma once

using win_kernel_lib::smart_pointers::auto_pointer;
using win_kernel_lib::deleters::pool_deleter;

namespace rule_facility
{
  class rule final
  {
  public:
    void set_reg_key(auto_pointer<UNICODE_STRING, pool_deleter>& reg_key_param) noexcept;

    void* __cdecl operator new(size_t, void*) noexcept;
    void __cdecl operator delete(void*) noexcept;

    friend bool operator<(const rule& a, const rule& b);
    friend bool operator>(const rule& a, const rule& b);

  private:
    auto_pointer<UNICODE_STRING, pool_deleter> reg_key_path;
  };

  bool operator<(const rule& a, const rule& b);
  bool operator>(const rule& a, const rule& b);
}
