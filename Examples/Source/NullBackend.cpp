/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2026
 ***********************************************************************************************//**
 * @file NullBackend.cpp
 *   Implementation of a complete, working Xidi physical controller backend that supports only a
 *   single controller but has no capabilities and does nothing.
 **************************************************************************************************/

#include "NullBackend.h"

#include "Xidi/PhysicalControllerBackend.h"

namespace XidiPluginExample
{
  using namespace ::Xidi;

  std::wstring_view NullBackend::PluginName(void)
  {
    return L"Null";
  }

  void NullBackend::Initialize(void) {}

  TPhysicalControllerIndex NullBackend::MaxPhysicalControllerCount(void)
  {
    return 1;
  }

  bool NullBackend::SupportsControllerByGuidAndPath(const wchar_t* guidAndPath)
  {
    // This backend reports itself as supporting all possible controllers.
    return true;
  }

  SPhysicalControllerCapabilities NullBackend::GetCapabilities(void)
  {
    // Empty capabilities means no physical controller elements are read and no force feedback
    // actuators are written.
    return {};
  }

  SPhysicalControllerState NullBackend::ReadInputState(
      TPhysicalControllerIndex physicalControllerIndex)
  {
    // Every read is successful but contains no data.
    return {.deviceStatus = Controller::EPhysicalDeviceStatus::Ok};
  }

  bool NullBackend::WriteForceFeedbackState(
      TPhysicalControllerIndex physicalControllerIndex, SPhysicalControllerVibration vibrationState)
  {
    // Every write is successful but does not do anything.
    return true;
  }
} // namespace XidiPluginExample
