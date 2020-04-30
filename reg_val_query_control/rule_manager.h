#pragma once

namespace rule_manager
{
  class ruler
  {
  public:
    virtual rule_facility::rule* add_rule_to_list(rule_facility::rule* r) = 0;
    virtual rule_facility::rule* get_referenced_rule_from_list(const UNICODE_STRING* key_path) = 0;
    virtual void clear_rules() = 0;

    virtual ~ruler() = 0;

    void __cdecl operator delete(void* p);
  };

  ruler* create_ruler(NTSTATUS& stat);
}
