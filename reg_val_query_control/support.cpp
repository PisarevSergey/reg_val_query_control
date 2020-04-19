#include "common.h"
#include "support.tmh"

using win_kernel_lib::pointer_manipulation::add_to_ptr;

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

bool support::is_entity_inside_buffer(const void* base, const void* first_byte_after_buffer, const void* entity_base, size_t entity_size)
{
  auto inside{ false };

  if ((entity_base >= base) &&
      (add_to_ptr<const void, const void>(entity_base, entity_size) <= first_byte_after_buffer))
  {
    inside = true;
  }

  return inside;
}

NTSTATUS support::query_name_for_user_mode_key_handle(HANDLE user_mode_key_handle, auto_pointer<const UNICODE_STRING, pool_deleter>& key_path)
{
  void* key_object{ nullptr };
  NTSTATUS stat { ObReferenceObjectByHandle(user_mode_key_handle, 0, *CmKeyObjectType, UserMode, &key_object, nullptr) };
  if (NT_SUCCESS(stat))
  {
    verbose_message(SUPPORT, "ObReferenceObjectByHandle success");

    PCUNICODE_STRING key_name{ nullptr };
    stat = CmCallbackGetKeyObjectID(get_driver()->get_reg_cookie(), key_object, nullptr, &key_name);
    if (NT_SUCCESS(stat))
    {
      verbose_message(SUPPORT, "key name is %wZ", key_name);

      key_path.reset(win_kernel_lib::unicode_strings::createStringCopy(*key_name, 'mnkR'));
      if (key_path.get())
      {
        ASSERT(NT_SUCCESS(stat));
        verbose_message(SUPPORT, "successfully allocated unicode string for key name");
      }
      else
      {
        stat = STATUS_INSUFFICIENT_RESOURCES;
        error_message(SUPPORT, "failed to allocate unicode string for key name");
      }
    }
    else
    {
      error_message(SUPPORT, "CmCallbackGetKeyObjectID failed with status %!STATUS!", stat);
    }

    ObDereferenceObject(key_object);
  }
  else
  {
    error_message(SUPPORT, "ObReferenceObjectByHandle failed with status %!STATUS!", stat);
  }

  return stat;
}
