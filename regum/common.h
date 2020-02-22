#pragma once

#include <Windows.h>
#include <fltUser.h>
#include <rpc.h>
#include <winternl.h>

#include <iostream>
#include <list>
#include <cassert>
#include <string>

#include <um_km_common.h>

#include "support.h"
#include "ntdll.h"
#include "opened_reg_keys.h"
#include "reg_key.h"

using std::wcout;
using std::endl;