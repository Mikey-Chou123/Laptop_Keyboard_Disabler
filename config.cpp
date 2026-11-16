#include "config.h"

#include <Windows.h>
#include <ShlObj.h>

#include <fstream>
#include <set>
#include <string>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Uuid.lib")

std::wstring GetConfigPath() {
  PWSTR folder = nullptr;
  if (FAILED(
          SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &folder))) {
    return L"";
  }

  std::wstring directory = std::wstring(folder) + L"\\LaptopKeyboardDisabler";
  CoTaskMemFree(folder);

  CreateDirectoryW(directory.c_str(), nullptr);
  return directory + L"\\internal_keyboards.txt";
}

void SaveInternalIds(const std::set<std::wstring>& internal_ids) {
  std::wofstream file("internal_keyboards.txt");

  for (const std::wstring& id : internal_ids) {
    file << id << L'\n';
  }
}

std::set<std::wstring> LoadInternalIds() {
  std::set<std::wstring> internal_ids;

  std::wifstream file(GetConfigPath());

  if (!file) {
    return internal_ids;
  }

  std::wstring line;

  while (std::getline(file, line)) {
    if (line.empty()) {
      continue;
    }

    internal_ids.insert(line);
  }

  return internal_ids;
}