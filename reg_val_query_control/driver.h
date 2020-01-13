#pragma once

class driver
{
public:
  virtual ~driver() {}
  void __cdecl operator delete(void*) {}
};

NTSTATUS create_driver(PDRIVER_OBJECT);

driver* get_driver();