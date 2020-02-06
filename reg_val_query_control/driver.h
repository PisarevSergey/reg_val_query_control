#pragma once

class driver
{
public:
  virtual ~driver() = 0;
  virtual PLARGE_INTEGER get_reg_cookie() = 0;
};

NTSTATUS create_driver(PDRIVER_OBJECT);

driver* get_driver();