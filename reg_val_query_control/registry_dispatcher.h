#pragma once

namespace registry_dispatcher
{
  class dispatcher
  {
  public:
    virtual NTSTATUS callback(REG_NOTIFY_CLASS reg_op, void* reg_op_data) = 0;

    virtual ~dispatcher() {}

    void __cdecl operator delete(void*);
  };

  dispatcher* create_dispatcher(NTSTATUS& stat);
}
