#include "common.h"

namespace
{
  class unsafe_ruler_with_rules : public rule_manager::ruler
  {
  public:
    unsafe_ruler_with_rules() : rules{ &rule_entry_allocator }
    {}

    class list_entry_allocator
    {
    public:
      void* allocate(CLONG size)
      {
        return ExAllocatePoolWithTag(PagedPool, size, 'eluR');
      }

      void deallocate(void* p)
      {
        if (p)
        {
          ExFreePool(p);
        }
      }
    };

    rule_facility::rule* add_rule_to_list_unsafe(rule_facility::rule* r)
    {
      rule_facility::rule* rule_in_list{nullptr};

      bool inserted{false};
      auto in_list_entry = rules.insert(r, inserted);
      if (inserted)
      {
        r->reference();
      }

      if (in_list_entry)
      {
        rule_in_list = *in_list_entry;
        ASSERT(rule_in_list);
        rule_in_list->reference();
      }

      return rule_in_list;
    }

    bool is_rule_in_list_unsafe(const UNICODE_STRING* key_path)
    {
      rule_facility::rule* search_rule{ nullptr };
      char search_rule_mem[sizeof(*search_rule)];
      search_rule = new(search_rule_mem) rule_facility::rule;
      search_rule->set_reg_key(key_path);

      return (rules.get_element_by_key(search_rule) ? true : false);
    }

    void clear_rules_unsafe()
    {
      rules.clear();
    }

  private:
    list_entry_allocator rule_entry_allocator;
    win_kernel_lib::avl_list_facility::avl_list<rule_facility::rule,
      list_entry_allocator,
    win_kernel_lib::deleters::referenced_object_deleter<rule_facility::rule>> rules;
  };

  class guarded_ruler_with_rules : public unsafe_ruler_with_rules
  {
  public:
    rule_facility::rule* add_rule_to_list(rule_facility::rule* r)
    {
      rules_guard.lock_exclusive();
      auto rule_in_list = add_rule_to_list_unsafe(r);
      rules_guard.release();

      return rule_in_list;
    }

    bool is_rule_in_list(const UNICODE_STRING* key_path)
    {
      bool in_list;
      rules_guard.lock_shared();
      in_list = is_rule_in_list_unsafe(key_path);
      rules_guard.release();
      return in_list;
    }

    void clear_rules()
    {
      rules_guard.lock_exclusive();
      clear_rules_unsafe();
      rules_guard.release();
    }
  private:
    win_kernel_lib::locks::eresource rules_guard;
  };

  class top_ruler : public guarded_ruler_with_rules
  {
  public:
    void* __cdecl operator new(size_t sz)
    {
      return ExAllocatePoolWithTag(NonPagedPool, sz, 'rluR');
    }
  };
}

rule_manager::ruler::~ruler()
{}

void __cdecl rule_manager::ruler::operator delete(void* p)
{
  if (p)
  {
    ExFreePool(p);
  }
}

rule_manager::ruler* rule_manager::create_ruler(NTSTATUS& stat)
{
  auto rlr{ new top_ruler };

  stat = (rlr ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES);

  return rlr;
}
