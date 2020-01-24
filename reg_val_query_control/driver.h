#pragma once

class driver
{
public:
  virtual ~driver() {}
  virtual PLARGE_INTEGER get_reg_cookie() = 0;

  void __cdecl operator delete(void*) {}
};

NTSTATUS create_driver(PDRIVER_OBJECT);

driver* get_driver();