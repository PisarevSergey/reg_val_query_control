#pragma once

namespace registry_dispatcher
{
  class dispatcher
  {
  public:
    virtual NTSTATUS callback(REG_NOTIFY_CLASS reg_op, void* reg_op_data) = 0;
    virtual NTSTATUS set_rules(unsigned __int32 number_of_rules, um_km_common::key_rule_header* rules, ULONG rules_size) = 0;
    virtual void clear_rules() = 0;

    virtual ~dispatcher() = 0;

    void __cdecl operator delete(void*);
  };

  dispatcher* create_dispatcher(NTSTATUS& stat);
}
