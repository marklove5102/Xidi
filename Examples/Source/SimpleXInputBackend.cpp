/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2026
 ***********************************************************************************************//**
 * @file SimpleXInputBackend.cpp
 *   Implementation of a simplified version of the built-in XInput physical controller backend,
 *   shown as a complete, working example of a Xidi plugin.
 **************************************************************************************************/

#include "SimpleXInputBackend.h"

#include <windows.h>
#include <xinput.h>

#include <cstring>

#include "Xidi/PhysicalControllerBackend.h"

namespace XidiPluginExample
{
  using namespace ::Xidi;

  std::wstring_view SimpleXInputBackend::PluginName(void)
  {
    return L"SimpleXInput";
  }

  bool SimpleXInputBackend::Initialize(void)
  {
    return true;
  }

  TPhysicalControllerIndex SimpleXInputBackend::MaxPhysicalControllerCount(void)
  {
    return XUSER_MAX_COUNT;
  }

  bool SimpleXInputBackend::SupportsControllerByGuidAndPath(const wchar_t* guidAndPath)
  {
    // The documented "best" way of determining if a device supports XInput is to look for
    // "&IG_" in the device path string.
    return (nullptr != wcsstr(guidAndPath, L"&IG_") || nullptr != wcsstr(guidAndPath, L"&ig_"));
  }

  SPhysicalControllerCapabilities SimpleXInputBackend::GetCapabilities(void)
  {
    return {
        .stick = Controller::kPhysicalCapabilitiesAllAnalogSticks,
        .trigger = Controller::kPhysicalCapabilitiesAllAnalogTriggers,
        .button = Controller::kPhysicalCapabilitiesStandardXInputButtons,
        .forceFeedbackActuator =
            Controller::kPhysicalCapabilitiesStandardXInputForceFeedbackActuators};
  }

  SPhysicalControllerState SimpleXInputBackend::ReadInputState(
      TPhysicalControllerIndex physicalControllerIndex)
  {
    XINPUT_STATE xinputState;
    DWORD xinputGetStateResult = XInputGetState(physicalControllerIndex, &xinputState);

    switch (xinputGetStateResult)
    {
      case ERROR_SUCCESS:
        // Directly using wButtons assumes that the bit layout is the same between the internal
        // bitset and the XInput data structure.
        return {
            .deviceStatus = Controller::EPhysicalDeviceStatus::Ok,
            .stick =
                {xinputState.Gamepad.sThumbLX,
                 xinputState.Gamepad.sThumbLY,
                 xinputState.Gamepad.sThumbRX,
                 xinputState.Gamepad.sThumbRY},
            .trigger = {xinputState.Gamepad.bLeftTrigger, xinputState.Gamepad.bRightTrigger},
            .button = static_cast<uint16_t>(xinputState.Gamepad.wButtons)};

      case ERROR_DEVICE_NOT_CONNECTED:
        return {.deviceStatus = Controller::EPhysicalDeviceStatus::NotConnected};

      default:
        return {.deviceStatus = Controller::EPhysicalDeviceStatus::Error};
    }
  }

  bool SimpleXInputBackend::WriteForceFeedbackState(
      TPhysicalControllerIndex physicalControllerIndex, SPhysicalControllerVibration vibrationState)
  {
    XINPUT_VIBRATION xinputVibration = {
        .wLeftMotorSpeed = vibrationState.leftMotor, .wRightMotorSpeed = vibrationState.rightMotor};
    return (
        ERROR_SUCCESS ==
        XInputSetState(static_cast<DWORD>(physicalControllerIndex), &xinputVibration));
  }
} // namespace XidiPluginExample
