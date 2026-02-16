/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2026
 ***********************************************************************************************//**
 * @file PluginRegistry.cpp
 *   Implementation of functionality for loading and keeping track of Xidi plugins.
 **************************************************************************************************/

#include "PluginRegistry.h"

#include <map>
#include <mutex>
#include <string>
#include <string_view>

#include <Infra/Core/Configuration.h>
#include <Infra/Core/Message.h>
#include <Infra/Core/TemporaryBuffer.h>

#include "ApiWindows.h"
#include "Globals.h"
#include "PhysicalControllerBackend.h"
#include "PluginTypes.h"
#include "Strings.h"

#define XIDI_PLUGIN_GET_COUNT_PROC_NAME     "XidiPluginGetCount"
#define XIDI_PLUGIN_GET_INTERFACE_PROC_NAME "XidiPluginGetInterface"

namespace Xidi
{
  /// Type alias for mapping from a plugin's name to its interface pointer.
  using TPluginNameToInterfaceMap =
      std::map<std::wstring, IPlugin*, Infra::Strings::CaseInsensitiveLessThanComparator<wchar_t>>;

  /// Two-level map of all plugins from their names to their interface pointers, indexed by plugin
  /// type.
  static TPluginNameToInterfaceMap
      loadedPluginsByTypeAndName[static_cast<unsigned int>(EPluginType::Count)];

  /// Retrieves and return a string representation for a plugin type enumerator.
  /// @param [in] pluginType Plugin type enumerator for which a string is desired.
  /// @return String representation of the plugin type enumerator.
  static std::wstring_view PluginTypeToString(EPluginType pluginType)
  {
    switch (pluginType)
    {
      case EPluginType::PhysicalControllerBackend:
        return L"PhysicalControllerBackend";
      default:
        return L"(unknown)";
    }
  }

  /// Attempts to load and register a single plugin identified by its filename.
  /// @param [in] pluginFilename Filename of the plugin to load and register.
  static void LoadSinglePlugin(const wchar_t* pluginFilename)
  {
    using TXidiPluginGetCountProc = int(__fastcall*)(void);
    using TXidiPluginGetInterfaceProc = IPlugin*(__fastcall*)(int);

#ifdef _WIN64
    constexpr const char* kXidiPluginGetCountProcName = XIDI_PLUGIN_GET_COUNT_PROC_NAME;
    constexpr const char* kXidiPluginGetInterfaceProcName = XIDI_PLUGIN_GET_INTERFACE_PROC_NAME;
#else
    constexpr const char* kXidiPluginGetCountProcName = "@" XIDI_PLUGIN_GET_COUNT_PROC_NAME "@0";
    constexpr const char* kXidiPluginGetInterfaceProcName =
        "@" XIDI_PLUGIN_GET_INTERFACE_PROC_NAME "@4";
#endif

    HMODULE pluginHandle = LoadLibraryW(pluginFilename);
    if (NULL == pluginHandle)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Failed to load plugin \"%s\": %s",
          pluginFilename,
          Infra::Strings::FromSystemErrorCode(GetLastError()).AsCString());
      return;
    }

    TXidiPluginGetCountProc xidiPluginGetCountProc = reinterpret_cast<TXidiPluginGetCountProc>(
        GetProcAddress(pluginHandle, kXidiPluginGetCountProcName));
    if (NULL == xidiPluginGetCountProc)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Failed to load plugin \"%s\": Unable to locate entry point \"%s\": %s",
          pluginFilename,
          _CRT_WIDE(XIDI_PLUGIN_GET_COUNT_PROC_NAME),
          Infra::Strings::FromSystemErrorCode(GetLastError()).AsCString());
      FreeLibrary(pluginHandle);
      return;
    }

    TXidiPluginGetInterfaceProc xidiPluginGetInterfaceProc =
        reinterpret_cast<TXidiPluginGetInterfaceProc>(
            GetProcAddress(pluginHandle, kXidiPluginGetInterfaceProcName));
    if (NULL == xidiPluginGetInterfaceProc)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Failed to load plugin \"%s\": Unable to locate entry point \"%s\": %s",
          pluginFilename,
          _CRT_WIDE(XIDI_PLUGIN_GET_INTERFACE_PROC_NAME),
          Infra::Strings::FromSystemErrorCode(GetLastError()).AsCString());
      FreeLibrary(pluginHandle);
      return;
    }

    Infra::Message::OutputFormatted(
        Infra::Message::ESeverity::Info, L"Successfully loaded plugin: \"%s\"", pluginFilename);

    for (int i = 0; i < xidiPluginGetCountProc(); ++i)
    {
      IPlugin* plugin = xidiPluginGetInterfaceProc(i);
      if (nullptr == plugin)
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Info, L"  [%u]: %s", i, L"(null)");
        continue;
      }

      const EPluginType pluginType = plugin->PluginType();
      if (static_cast<unsigned int>(pluginType) >= static_cast<unsigned int>(EPluginType::Count))
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Info, L"  [%u]: %s", i, L"(unrecognized plugin type)");
        continue;
      }

      const std::wstring_view pluginName = plugin->PluginName();
      const std::wstring_view pluginTypeStr = PluginTypeToString(pluginType);

      const bool pluginRegistrationSucceeded =
          loadedPluginsByTypeAndName[static_cast<unsigned int>(pluginType)]
              .emplace(pluginName, plugin)
              .second;
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Info,
          L"  [%u]: type = %.*s, name = \"%.*s\" (%s)",
          i,
          static_cast<int>(pluginTypeStr.length()),
          pluginTypeStr.data(),
          static_cast<int>(pluginName.length()),
          pluginName.data(),
          ((true == pluginRegistrationSucceeded) ? L"successfully registered"
                                                 : L"failed to register due to a name collision"));
    }
  }

  void LoadConfiguredPlugins(void)
  {
    // There is overhead to using call_once, even after the operation is completed.
    static bool isInitialized = false;
    if (true == isInitialized) return;

    static std::once_flag loadedPluginsFlag;
    std::call_once(
        loadedPluginsFlag,
        []() -> void
        {
          const auto& configData = Globals::GetConfigurationData();
          for (const auto& pluginToLoad : configData[Infra::Configuration::kSectionNameGlobal]
                                                    [Strings::kStrConfigurationSettingPlugin]
                                                        .Values())
            LoadSinglePlugin(Strings::PluginFilename(pluginToLoad.GetString()).c_str());
        });
  }

  IPhysicalControllerBackend* GetPhysicalControllerBackendInterface(std::wstring_view name)
  {
    auto& loadedPlugins = loadedPluginsByTypeAndName[static_cast<unsigned int>(
        EPluginType::PhysicalControllerBackend)];
    auto requestedPlugin = loadedPlugins.find(name);
    return (
        (loadedPlugins.cend() == requestedPlugin)
            ? nullptr
            : static_cast<IPhysicalControllerBackend*>(requestedPlugin->second));
  }
} // namespace Xidi
