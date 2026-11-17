#ifndef LAPTOP_KEYBOARD_DISABLER_KEYBOARDS_H_
#define LAPTOP_KEYBOARD_DISABLER_KEYBOARDS_H_

#include <set>
#include <string>
#include <vector>

#include "config.h"

struct Keyboard {
  std::wstring name;
  std::wstring instance_id;
};

std::vector<Keyboard> ListKeyboards();
bool IsExternalBus(const std::wstring& instance_id);
bool IsExternal(const std::wstring& instance_id,
                const std::set<std::wstring>& internal_ids);

bool HasExternalKeyboard(const std::set<std::wstring>& internal_ids);
#endif