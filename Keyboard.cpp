#include "keyboards.h"

#include <ctype.h>
#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>

#pragma comment(lib, "setupapi.lib")

namespace {

std::wstring ReadProperty(HDEVINFO set, SP_DEVINFO_DATA* data, DWORD property) {
  wchar_t buffer[512];
  if (SetupDiGetDeviceRegistryPropertyW(set, data, property, nullptr,
                                        reinterpret_cast<PBYTE>(buffer),
                                        sizeof(buffer), nullptr)) {
    return buffer;
  }
  return L"";
}

}  // namespace

std::vector<Keyboard> ListKeyboards() {
  std::vector<Keyboard> result;

  HDEVINFO set = SetupDiGetClassDevsW(&GUID_DEVCLASS_KEYBOARD, nullptr, nullptr,
                                      DIGCF_PRESENT);
  if (set == INVALID_HANDLE_VALUE) {
    return result;
  }

  SP_DEVINFO_DATA data = {};
  data.cbSize = sizeof(data);

  for (DWORD i = 0; SetupDiEnumDeviceInfo(set, i, &data); ++i) {
    wchar_t id[512];
    if (!SetupDiGetDeviceInstanceIdW(set, &data, id, 512, nullptr)) {
      continue;
    }

    Keyboard keyboard;
    keyboard.instance_id = id;
    keyboard.name = ReadProperty(set, &data, SPDRP_FRIENDLYNAME);
    if (keyboard.name.empty()) {
      keyboard.name = ReadProperty(set, &data, SPDRP_DEVICEDESC);
    }

    result.push_back(keyboard);
  }

  SetupDiDestroyDeviceInfoList(set);
  return result;
}

bool IsExternalBus(const std::wstring& instance_id) {
  std::wstring copy = instance_id;
  for (wchar_t& ch : copy) {
    ch = toupper(ch);
  }

  if (copy.find(L"USB\\") != std::wstring::npos ||
      copy.find(L"HID\\") != std::wstring::npos ||
      copy.find(L"BTH") != std::wstring::npos) {
    return true;
  }
  return false;
}

bool IsExternal(const std::wstring& instance_id,
                const std::set<std::wstring>& internal_ids) {
                    if (internal_ids.count(instance_id) != 0){
                        return false;
                    } else {
                        return IsExternalBus(instance_id);
                    }
}