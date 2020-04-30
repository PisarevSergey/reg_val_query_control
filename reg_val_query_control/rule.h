#pragma once

using win_kernel_lib::smart_pointers::auto_pointer;
using win_kernel_lib::deleters::pool_deleter;
using win_kernel_lib::avl_list_facility::avl_list;
using win_kernel_lib::string_facility::string;

namespace rule_facility
{
  class value_names_allocator
  {
  public:
    void* allocate(CLONG size);
    void deallocate(void* p);
  };

  class rule final : public win_kernel_lib::refcounted_object::base<rule>
  {
    friend class win_kernel_lib::deleters::default_deleter<rule>;

  public:
    static rule* create_rule() { return new rule; }

    rule();

    void set_reg_key(auto_pointer<const UNICODE_STRING, pool_deleter>& reg_key_param) noexcept;
    void set_reg_key(const UNICODE_STRING* reg_key_param) noexcept;
    NTSTATUS add_value_name(auto_pointer<UNICODE_STRING, pool_deleter>& value_name) noexcept;

    bool is_value_in_rule(const UNICODE_STRING& value_name_to_search);

    void* __cdecl operator new(size_t, void* p) noexcept;

    friend bool operator<(const rule& a, const rule& b);
    friend bool operator>(const rule& a, const rule& b);

  private:
    void* __cdecl operator new(size_t) noexcept;
    void __cdecl operator delete(void*) noexcept;
    ~rule() {}

  private:
    value_names_allocator val_names_allocator;
    auto_pointer<const UNICODE_STRING, pool_deleter> reg_key_path;
    avl_list<string, value_names_allocator, win_kernel_lib::deleters::default_deleter<string>> value_names;
  };

  bool operator<(const rule& a, const rule& b);
  bool operator>(const rule& a, const rule& b);
}
