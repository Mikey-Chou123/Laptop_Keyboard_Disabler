#ifndef LAPTOP_KEYBOARD_DISABLER_DEVICE_H_
#define LAPTOP_KEYBOARD_DISABLER_DEVICE_H_

#include <string>

bool SetKeyboardEnabled(const std::wstring& instance_id, bool enable);

#endif