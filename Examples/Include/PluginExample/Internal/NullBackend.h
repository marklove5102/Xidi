/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2026
 ***********************************************************************************************//**
 * @file NullBackend.h
 *   Interface declaration for a complete, working Xidi physical controller backend that supports
 *   only a single controller but has no capabilities and does nothing.
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
  class NullBackend : public IPhysicalControllerBackend
  {
  public:

    // IPlugin
    std::wstring_view PluginName(void) override;
    void Initialize(void) override;

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
