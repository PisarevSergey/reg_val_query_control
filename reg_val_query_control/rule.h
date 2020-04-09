#pragma once

using win_kernel_lib::smart_pointers::auto_pointer;
using win_kernel_lib::deleters::pool_deleter;
using win_kernel_lib::avl_list_facility::avl_list;
using win_kernel_lib::string_facility::string;

namespace rule_facility
{
  class rule final
  {
  public:
    void set_reg_key(auto_pointer<UNICODE_STRING, pool_deleter>& reg_key_param) noexcept;
    NTSTATUS add_value_name(auto_pointer<UNICODE_STRING, pool_deleter>& value_name) noexcept;

    void* __cdecl operator new(size_t) noexcept;
    void __cdecl operator delete(void*) noexcept;

    friend bool operator<(const rule& a, const rule& b);
    friend bool operator>(const rule& a, const rule& b);

    static void* alloc_paged(CLONG size);
    static void free(void*);

  private:
    auto_pointer<UNICODE_STRING, pool_deleter> reg_key_path;
    avl_list<string, alloc_paged, free> value_names;
  };

  bool operator<(const rule& a, const rule& b);
  bool operator>(const rule& a, const rule& b);
}
