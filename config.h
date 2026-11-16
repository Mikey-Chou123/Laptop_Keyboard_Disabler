#ifndef LAPTOP_KEYBOARD_DISABLER_CONFIG_H_
#define LAPTOP_KEYBOARD_DISABLER_CONFIG_H_

#include <set>
#include <string>

std::wstring GetConfigPath();
void SaveInternalIds(const std::set<std::wstring>& internal_ids);
std::set<std::wstring> LoadInternalIds();

#endif