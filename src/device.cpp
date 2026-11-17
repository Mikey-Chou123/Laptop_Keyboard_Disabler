#include "device.h"

#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <devguid.h>
#include <wchar.h>

#include <string>

#pragma comment(lib, "setupapi.lib")

bool SetKeyboardEnabled(const std::wstring& instance_id, bool enable) {
  HDEVINFO set = SetupDiGetClassDevsW(&GUID_DEVCLASS_KEYBOARD, nullptr,
                                      nullptr, DIGCF_PRESENT);
  if (set == INVALID_HANDLE_VALUE) {
    return false;
  }

  bool succeeded = false;
  DWORD error = ERROR_NOT_FOUND;
  SP_DEVINFO_DATA data = {};
  data.cbSize = sizeof(data);

  for (DWORD i = 0; SetupDiEnumDeviceInfo(set, i, &data); ++i) {
    wchar_t id[512];
    if (!SetupDiGetDeviceInstanceIdW(set, &data, id, 512, nullptr)) {
      continue;
    }
    if (_wcsicmp(instance_id.c_str(), id) != 0) {
      continue;
    }

    SP_PROPCHANGE_PARAMS params = {};
    params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    params.StateChange = enable ? DICS_ENABLE : DICS_DISABLE;
    params.Scope = DICS_FLAG_GLOBAL;
    params.HwProfile = 0;

    if (SetupDiSetClassInstallParamsW(set, &data, &params.ClassInstallHeader,
                                      sizeof(params)) &&
        SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, set, &data)) {
      succeeded = true;
      error = ERROR_SUCCESS;
    } else {
      error = GetLastError();
    }
    break;
  }

  SetupDiDestroyDeviceInfoList(set);
  SetLastError(error);
  return succeeded;
}