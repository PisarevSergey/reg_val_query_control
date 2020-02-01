#include "support.h"

const wchar_t* support::get_reg_value_type_name(ULONG type)
{
  const wchar_t* name;

  switch (type)
  {
    case REG_NONE:
      name = L"REG_NONE";
      break;
    case REG_SZ:
      name = L"REG_SZ";
      break;
    case REG_EXPAND_SZ:
      name = L"REG_EXPAND_SZ";
      break;
    case REG_BINARY:
      name = L"REG_BINARY";
      break;
    case REG_DWORD:
      name = L"REG_DWORD";
      break;
    case REG_DWORD_BIG_ENDIAN:
      name = L"REG_DWORD_BIG_ENDIAN";
      break;
    case REG_LINK:
      name = L"REG_LINK";
      break;
    case REG_MULTI_SZ:
      name = L"REG_MULTI_SZ";
      break;
    case REG_RESOURCE_LIST:
      name = L"REG_RESOURCE_LIST";
      break;
    case REG_FULL_RESOURCE_DESCRIPTOR:
      name = L"REG_FULL_RESOURCE_DESCRIPTOR";
      break;
    case REG_RESOURCE_REQUIREMENTS_LIST:
      name = L"REG_RESOURCE_REQUIREMENTS_LIST";
      break;
    case REG_QWORD:
      name = L"REG_QWORD";
      break;
    default:
      name = L"invalid type";
      break;
  }

  return name;
}


/*
KeyValueBasicInformation,
KeyValueFullInformation,
KeyValuePartialInformation,
KeyValueFullInformationAlign64,
KeyValuePartialInformationAlign64,
KeyValueLayerInformation,
*/


const wchar_t* support::get_value_information_class_name(KEY_VALUE_INFORMATION_CLASS info_class)
{
  const wchar_t* name;

  switch (info_class)
  {
    case KeyValueBasicInformation:
      name = L"KeyValueBasicInformation";
      break;
    case KeyValueFullInformation:
      name = L"KeyValueFullInformation";
      break;
    case KeyValuePartialInformation:
      name = L"KeyValuePartialInformation";
      break;
    case KeyValueFullInformationAlign64:
      name = L"KeyValueFullInformationAlign64";
      break;
    case KeyValuePartialInformationAlign64:
      name = L"KeyValuePartialInformationAlign64";
      break;
    case KeyValueLayerInformation:
      name = L"KeyValueLayerInformation";
      break;
    default:
      name = L"invalid info class";
      break;
  }

  return name;
}
