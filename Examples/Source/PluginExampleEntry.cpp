/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2026
 ***********************************************************************************************//**
 * @file PluginExampleEntry.cpp
 *   Entry point for the example Xidi plugin.
 **************************************************************************************************/

#include "NullBackend.h"
#include "SimpleXInputBackend.h"

#include "Xidi/Plugin.h"

static ::Xidi::IPlugin* plugins[] = {
    new ::XidiPluginExample::NullBackend(), new ::XidiPluginExample::SimpleXInputBackend()};

extern "C" int __fastcall XidiPluginGetCount(void)
{
  return _countof(plugins);
}

extern "C" ::Xidi::IPlugin* __fastcall XidiPluginGetInterface(int index)
{
  if (index < _countof(plugins)) return plugins[index];
  return nullptr;
}
