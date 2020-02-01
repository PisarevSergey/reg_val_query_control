#pragma once

#include <fltKernel.h>

namespace support
{
  const wchar_t* get_reg_value_type_name(ULONG type);
  const wchar_t* get_value_information_class_name(KEY_VALUE_INFORMATION_CLASS info_class);
}
