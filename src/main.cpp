#include <Windows.h>
#include <ShellAPI.h>

#include <cwchar>
#include <set>
#include <string>

#include "config.h"
#include "device.h"
#include "keyboards.h"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")

namespace {

const UINT kWmTray = WM_APP + 1;
const int kIdExit = 1001;
const int kIdList = 1002;
const int kIdCalibrate = 1003;
const int kIdDisable = 1004;
const int kIdEnable = 1005;
const wchar_t kClassName[] = L"LaptopKeyboardDisablerWindow";

NOTIFYICONDATAW tray_icon = {};
std::set<std::wstring> internal_ids;

void ShowKeyboards(HWND hwnd) {
  std::wstring text;

  for (const Keyboard& keyboard : ListKeyboards()) {
    text += keyboard.name + L"\n";
    text += keyboard.instance_id + L"\n";
    text += IsExternal(keyboard.instance_id, internal_ids)
                ? L"[external]\n\n"
                : L"[internal or ignored]\n\n";
  }

  if (text.empty()) {
    text = L"No keyboards found.";
  }

  MessageBoxW(hwnd, text.c_str(), L"Keyboards", MB_OK);
}

void Calibrate(HWND hwnd) {
  int answer = MessageBoxW(
      hwnd,
      L"Unplug every external keyboard, including USB receivers, and turn "
      L"off any Bluetooth keyboards. Then click OK.",
      L"Calibrate", MB_OKCANCEL | MB_ICONINFORMATION);
  if (answer != IDOK) {
    return;
  }

  internal_ids.clear();
  std::wstring text = L"Saved as internal:\n\n";
  for (const Keyboard& keyboard : ListKeyboards()) {
    internal_ids.insert(keyboard.instance_id);
    text += keyboard.name + L"\n";
  }

  SaveInternalIds(internal_ids);
  MessageBoxW(hwnd, text.c_str(), L"Calibrate", MB_OK);
}

void SetInternalKeyboards(HWND hwnd, bool enable) {
  if (internal_ids.empty()) {
    MessageBoxW(hwnd, L"Please calibrate first.", L"Error",
                MB_OK | MB_ICONWARNING);
    return;
  }

  if (!enable && !HasExternalKeyboard(internal_ids)) {
    MessageBoxW(hwnd, L"No external keyboard detected.", L"Error",
                MB_OK | MB_ICONWARNING);
    return;
  }

  int count_false = 0;
  DWORD last_error = 0;
  for (const auto& id : internal_ids) {
    if (!SetKeyboardEnabled(id, enable)) {
      ++count_false;
      last_error = GetLastError();
    }
  }

  if (count_false > 0) {
    std::wstring text = std::to_wstring(count_false) +
                        L" keyboard(s) could not be changed.\nError code: " +
                        std::to_wstring(last_error);
    MessageBoxW(hwnd, text.c_str(), L"Error", MB_OK | MB_ICONWARNING);
  }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM w_param,
                         LPARAM l_param) {
  switch (message) {
    case kWmTray:
      if (l_param == WM_RBUTTONUP) {
        POINT cursor;
        GetCursorPos(&cursor);

        HMENU menu = CreatePopupMenu();
        AppendMenuW(menu, MF_STRING, kIdList, L"Show keyboards");
        AppendMenuW(menu, MF_STRING, kIdCalibrate, L"Calibrate");
        AppendMenuW(menu, MF_STRING, kIdDisable, L"Disable internal keyboard");
        AppendMenuW(menu, MF_STRING, kIdEnable, L"Enable internal keyboard");
        AppendMenuW(menu, MF_STRING, kIdExit, L"Exit");

        SetForegroundWindow(hwnd);

        int choice = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                    cursor.x, cursor.y, 0, hwnd, nullptr);

        DestroyMenu(menu);

        if (choice == kIdList) {
          ShowKeyboards(hwnd);
        } else if (choice == kIdCalibrate) {
          Calibrate(hwnd);
        } else if (choice == kIdDisable) {
          SetInternalKeyboards(hwnd, false);
        } else if (choice == kIdEnable) {
          SetInternalKeyboards(hwnd, true);
        } else if (choice == kIdExit) {
          DestroyWindow(hwnd);
        }
      }

      return 0;

    case WM_DESTROY:
      if (!internal_ids.empty()) {
        SetInternalKeyboards(hwnd, true);
      }
      Shell_NotifyIconW(NIM_DELETE, &tray_icon);
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProcW(hwnd, message, w_param, l_param);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
  WNDCLASSW window_class = {};
  window_class.lpfnWndProc = WndProc;
  window_class.hInstance = instance;
  window_class.lpszClassName = kClassName;

  RegisterClassW(&window_class);

  HWND hwnd = CreateWindowExW(0, kClassName, L"Laptop Keyboard Disabler", 0, 0,
                              0, 0, 0, nullptr, nullptr, instance, nullptr);

  if (!hwnd) {
    return 1;
  }

  internal_ids = LoadInternalIds();

  tray_icon.cbSize = sizeof(tray_icon);
  tray_icon.hWnd = hwnd;
  tray_icon.uID = 1;
  tray_icon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  tray_icon.uCallbackMessage = kWmTray;
  tray_icon.hIcon = LoadIconW(nullptr, IDI_APPLICATION);

  wcscpy_s(tray_icon.szTip, L"Laptop Keyboard Disabler");

  Shell_NotifyIconW(NIM_ADD, &tray_icon);

  MSG message;

  while (GetMessageW(&message, nullptr, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

  return 0;
}