/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2026
 ***********************************************************************************************//**
 * @file PhysicalControllerBackendXInput.cpp
 *   Implementation of the built-in XInput physical controller backend.
 **************************************************************************************************/

#include "PhysicalControllerBackendXInput.h"

#include <cstring>
#include <string_view>

#include "ImportApiXInput.h"
#include "PhysicalControllerTypes.h"

namespace Xidi
{
  using namespace ::Xidi::Controller;

  std::wstring_view PhysicalControllerBackendXInput::PluginName(void)
  {
    return L"XInput (built-in)";
  }

  bool PhysicalControllerBackendXInput::Initialize(void)
  {
    ImportApiXInput::Initialize();
    return true;
  }

  TPhysicalControllerIndex PhysicalControllerBackendXInput::MaxPhysicalControllerCount(void)
  {
    return static_cast<TPhysicalControllerIndex>(ImportApiXInput::kMaxControllerCount);
  }

  bool PhysicalControllerBackendXInput::SupportsControllerByGuidAndPath(const wchar_t* guidAndPath)
  {
    // The documented "best" way of determining if a device supports XInput is to look for
    // "&IG_" in the device path string.
    return (nullptr != wcsstr(guidAndPath, L"&IG_") || nullptr != wcsstr(guidAndPath, L"&ig_"));
  }

  SPhysicalControllerCapabilities PhysicalControllerBackendXInput::GetCapabilities(void)
  {
    return {
        .stick = kPhysicalCapabilitiesAllAnalogSticks,
        .trigger = kPhysicalCapabilitiesAllAnalogTriggers,
        .button = kPhysicalCapabilitiesStandardXInputButtons,
        .forceFeedbackActuator = kPhysicalCapabilitiesStandardXInputForceFeedbackActuators};
  }

  SPhysicalControllerState PhysicalControllerBackendXInput::ReadInputState(
      TPhysicalControllerIndex physicalControllerIndex)
  {
    constexpr uint16_t kUnusedButtonMask = ~(static_cast<uint16_t>(
        (1u << static_cast<unsigned int>(EPhysicalButton::Guide)) |
        (1u << static_cast<unsigned int>(EPhysicalButton::Share))));

    XINPUT_STATE xinputState;
    DWORD xinputGetStateResult =
        ImportApiXInput::XInputGetState(physicalControllerIndex, &xinputState);

    switch (xinputGetStateResult)
    {
      case ERROR_SUCCESS:
        // Directly using wButtons assumes that the bit layout is the same between the internal
        // bitset and the XInput data structure. The static assertions below this function verify
        // this assumption and will cause a compiler error if it is wrong.
        return {
            .deviceStatus = EPhysicalDeviceStatus::Ok,
            .stick =
                {xinputState.Gamepad.sThumbLX,
                 xinputState.Gamepad.sThumbLY,
                 xinputState.Gamepad.sThumbRX,
                 xinputState.Gamepad.sThumbRY},
            .trigger = {xinputState.Gamepad.bLeftTrigger, xinputState.Gamepad.bRightTrigger},
            .button = static_cast<uint16_t>(xinputState.Gamepad.wButtons & kUnusedButtonMask)};

      case ERROR_DEVICE_NOT_CONNECTED:
        return {.deviceStatus = EPhysicalDeviceStatus::NotConnected};

      default:
        return {.deviceStatus = EPhysicalDeviceStatus::Error};
    }
  }

  bool PhysicalControllerBackendXInput::WriteForceFeedbackState(
      TPhysicalControllerIndex physicalControllerIndex, SPhysicalControllerVibration vibrationState)
  {
    XINPUT_VIBRATION xinputVibration = {
        .wLeftMotorSpeed = vibrationState.leftMotor, .wRightMotorSpeed = vibrationState.rightMotor};
    return (
        ERROR_SUCCESS ==
        ImportApiXInput::XInputSetState(
            static_cast<DWORD>(physicalControllerIndex), &xinputVibration));
  }

  static_assert(1 << static_cast<int>(EPhysicalButton::DpadUp) == XINPUT_GAMEPAD_DPAD_UP);
  static_assert(1 << static_cast<int>(EPhysicalButton::DpadDown) == XINPUT_GAMEPAD_DPAD_DOWN);
  static_assert(1 << static_cast<int>(EPhysicalButton::DpadLeft) == XINPUT_GAMEPAD_DPAD_LEFT);
  static_assert(1 << static_cast<int>(EPhysicalButton::DpadRight) == XINPUT_GAMEPAD_DPAD_RIGHT);
  static_assert(1 << static_cast<int>(EPhysicalButton::Start) == XINPUT_GAMEPAD_START);
  static_assert(1 << static_cast<int>(EPhysicalButton::Back) == XINPUT_GAMEPAD_BACK);
  static_assert(1 << static_cast<int>(EPhysicalButton::LS) == XINPUT_GAMEPAD_LEFT_THUMB);
  static_assert(1 << static_cast<int>(EPhysicalButton::RS) == XINPUT_GAMEPAD_RIGHT_THUMB);
  static_assert(1 << static_cast<int>(EPhysicalButton::LB) == XINPUT_GAMEPAD_LEFT_SHOULDER);
  static_assert(1 << static_cast<int>(EPhysicalButton::RB) == XINPUT_GAMEPAD_RIGHT_SHOULDER);
  static_assert(1 << static_cast<int>(EPhysicalButton::A) == XINPUT_GAMEPAD_A);
  static_assert(1 << static_cast<int>(EPhysicalButton::B) == XINPUT_GAMEPAD_B);
  static_assert(1 << static_cast<int>(EPhysicalButton::X) == XINPUT_GAMEPAD_X);
  static_assert(1 << static_cast<int>(EPhysicalButton::Y) == XINPUT_GAMEPAD_Y);
} // namespace Xidi
