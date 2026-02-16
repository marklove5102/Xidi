/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2026
 ***********************************************************************************************//**
 * @file SimpleXInputBackend.h
 *   Interface declaration for a simplified version of the built-in XInput physical controller
 *   backend, shown as a complete, working example of a Xidi plugin.
 **************************************************************************************************/

#pragma once

#include "Xidi/PhysicalControllerBackend.h"

namespace XidiPluginExample
{
  using namespace ::Xidi;

  /// Simplified version of the built-in XInput physical controller backend, but as a Xidi plugin
  /// that can be configured and loaded externally. This plugin's implementation directly links with
  /// the XInput import library and hence does not rely on dynamic loading via `LoadLibrary` and
  /// `GetProcAddress`.
  class SimpleXInputBackend : public IPhysicalControllerBackend
  {
  public:

    // IPlugin
    std::wstring_view PluginName(void) override;
    bool Initialize(void) override;

    // IPhysicalControllerBackend
    TPhysicalControllerIndex MaxPhysicalControllerCount(void) override;
    bool SupportsControllerByGuidAndPath(const wchar_t* guidAndPath) override;
    SPhysicalControllerCapabilities GetCapabilities(void) override;
    SPhysicalControllerState ReadInputState(
        TPhysicalControllerIndex physicalControllerIndex) override;
    bool WriteForceFeedbackState(
        TPhysicalControllerIndex physicalControllerIndex,
        SPhysicalControllerVibration vibrationState) override;
  };
} // namespace XidiPluginExample
