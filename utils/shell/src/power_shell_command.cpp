/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "power_shell_command.h"

#include <cerrno>
#include <getopt.h>
#include <string_ex.h>

#include "power_mgr_client.h"
#include "display_power_mgr_client.h"
#include "power_setting_helper.h"
#include "system_ability_definition.h"

extern char *optarg;

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::DisplayPowerMgr;

static const struct option SET_MODE_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
};

static const struct option DISPLAY_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"restore", no_argument, nullptr, 'r'},
    {"update", required_argument, nullptr, 'u'},
    {"override", required_argument, nullptr, 'o'},
    {"boost", required_argument, nullptr, 'b'},
    {"cancel", no_argument, nullptr, 'c'},
};

static const struct option TIME_OUT_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"restore", no_argument, nullptr, 'r'},
    {"override", required_argument, nullptr, 'o'},
};

static const struct option SETTING_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
};

static const std::string HELP_MSG =
    "usage: power-shell\n"
    "command list:\n"
    "  setmode :    Set power mode. \n"
    "  wakeup  :    Wakeup system and turn screen on. \n"
    "  suspend :    Suspend system and turn screen off. \n"
    "  display :    Update or Override display brightness. \n"
    "  timeout :    Override or Restore screen off time. \n"
    "  setting :    Query or Put setting values. \n"
    "  dump    :    Dump power info. \n"
    "  help    :    Show this help menu. \n";

static const std::string SETMODE_HELP_MSG =
    "usage: power-shell setmode [<options>]\n"
    "setmode <power mode: (value is as below)> \n"
    "  600  :  normal mode\n"
    "  601  :  power save mode\n"
    "  602  :  performance mode\n"
    "  603  :  extreme power save mode\n";

static const std::string DISPLAY_HELP_MSG =
    "usage: power-shell display [<options>] 100\n"
    "display <options are as below> \n"
    "  -u  :  update brightness\n"
    "  -o  :  override brightness\n"
    "  -b  :  timing maximum brightness\n"
    "  -c  :  cancel the timing maximum brightness\n";

static const std::string TIME_OUT_HELP_MSG =
    "usage: power-shell timeout [<options>] 1000\n"
    "timeout <options are as below> \n"
    "  -o  :  override screen off time\n"
    "  -r  :  restore screen off time\n";

static const std::string SETTING_HELP_MSG =
    "usage: power-shell setting [query|put] key [value]\n"
    "setting <operations are as below> \n"
    "  query  :  query setting value by key\n"
    "  put    :  put setting value to key\n";

PowerShellCommand::PowerShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, "power-shell")
{}

ErrCode PowerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&PowerShellCommand::RunAsHelpCommand, this)},
        {"setmode", std::bind(&PowerShellCommand::RunAsSetModeCommand, this)},
        {"wakeup", std::bind(&PowerShellCommand::RunAsWakeupCommand, this)},
        {"suspend", std::bind(&PowerShellCommand::RunAsSuspendCommand, this)},
        {"display", std::bind(&PowerShellCommand::RunAsDisplayCommand, this)},
        {"timeout", std::bind(&PowerShellCommand::RunAsTimeOutCommand, this)},
        {"setting", std::bind(&PowerShellCommand::RunAsSettingCommand, this)},
        {"dump", std::bind(&PowerShellCommand::RunAsDumpCommand, this)},
    };

    return ERR_OK;
}

ErrCode PowerShellCommand::CreateMessageMap()
{
    messageMap_ = {};

    return ERR_OK;
}

ErrCode PowerShellCommand::init()
{
    return OHOS::ERR_OK;
}

ErrCode PowerShellCommand::RunAsHelpCommand()
{
    resultReceiver_.clear();
    resultReceiver_.append(HELP_MSG);
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsSetModeCommand()
{
    int ind = 0;
    int option = getopt_long(argc_, argv_, "h", SET_MODE_OPTIONS, &ind);
    resultReceiver_.clear();
    if (option == 'h') {
        resultReceiver_.append(SETMODE_HELP_MSG);
        return ERR_OK;
    }
    if (argList_.empty()) {
        resultReceiver_.append("Error! please input your mode value. \n");
        resultReceiver_.append(SETMODE_HELP_MSG);
        return ERR_OK;
    }

    auto mode = static_cast<uint32_t>(strtol(argList_[0].c_str(), nullptr, 0));
    resultReceiver_.append("Set Mode: ");
    resultReceiver_.append(argList_[0]);
    resultReceiver_.append("\n");
    PowerMgrClient &client = PowerMgrClient::GetInstance();
    client.SetDeviceMode(mode);
    uint32_t result = client.GetDeviceMode();
    if (result == mode) {
        resultReceiver_.append("Set Mode Success!\n");
    } else {
        resultReceiver_.append("Set Mode Failed, current mode is: ");
        resultReceiver_.append(std::to_string(result));
        resultReceiver_.append("\n");
    }

    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsWakeupCommand()
{
    PowerMgrClient &client = PowerMgrClient::GetInstance();
    std::string detail = "shell";
    client.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, detail);
    resultReceiver_.append("WakeupDevice is called\n");
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsSuspendCommand()
{
    PowerMgrClient &client = PowerMgrClient::GetInstance();
    std::string detail = "shell";
    client.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON);
    resultReceiver_.append("SuspendDevice is called\n");
    return ERR_OK;
}

extern "C" void PrintDumpFileError(std::string& receiver, const char* path)
{
    receiver.append("Open Dump file (");
    receiver.append(path);
    receiver.append(") failed: ");
    receiver.append(std::to_string(errno));
    receiver.append("\n");
}

ErrCode PowerShellCommand::RunAsDumpCommand()
{
    resultReceiver_.clear();

    PowerMgrClient &client = PowerMgrClient::GetInstance();
    std::string ret = client.Dump(argList_);
    resultReceiver_.append("Power Dump result: \n");
    resultReceiver_.append(ret);

    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsDisplayCommand()
{
    int ind = 0;
    int option = getopt_long(argc_, argv_, "hru:o:b:c", DISPLAY_OPTIONS, &ind);
    resultReceiver_.clear();
    if (option == 'h') {
        resultReceiver_.append(DISPLAY_HELP_MSG);
        return ERR_OK;
    }
    if (option == 'r') {
        bool ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
        resultReceiver_.append("Restore brightness");
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    if (option == 'c') {
        bool ret = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
        resultReceiver_.append("Cancel boost brightness");
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    if (!optarg) {
        resultReceiver_.append("Error! please input your brightness value.\n");
        resultReceiver_.append(DISPLAY_HELP_MSG);
        return ERR_OK;
    }
    auto value = static_cast<uint32_t>(atoi(optarg));
    if (option == 'u') {
        bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(value);
        resultReceiver_.append("Update brightness to ");
        resultReceiver_.append(std::to_string(value));
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    if (option == 'o') {
        bool ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(value);
        resultReceiver_.append("Override brightness to ");
        resultReceiver_.append(std::to_string(value));
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    if (option == 'b') {
        bool ret = DisplayPowerMgrClient::GetInstance().BoostBrightness(value);
        resultReceiver_.append("Boost brightness timeout ");
        resultReceiver_.append(std::to_string(value)).append("ms");
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsTimeOutCommand()
{
    int ind = 0;
    int option = getopt_long(argc_, argv_, "hro:", TIME_OUT_OPTIONS, &ind);
    resultReceiver_.clear();
    if (option == 'h') {
        resultReceiver_.append(TIME_OUT_HELP_MSG);
        return ERR_OK;
    }
    if (option == 'r') {
        bool ret = PowerMgrClient::GetInstance().RestoreScreenOffTime();
        resultReceiver_.append("Restore screen off time");
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    if (!optarg) {
        resultReceiver_.append("Error! please input your screen off time.\n");
        resultReceiver_.append(TIME_OUT_HELP_MSG);
        return ERR_OK;
    }
    auto timeout = static_cast<int64_t>(atoi(optarg));
    if (option == 'o') {
        bool ret = PowerMgrClient::GetInstance().OverrideScreenOffTime(timeout);
        resultReceiver_.append("Override screen off time to ");
        resultReceiver_.append(std::to_string(timeout));
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsSettingCommand()
{
    int ind = 0;
    int option = getopt_long(argc_, argv_, "h", SETTING_OPTIONS, &ind);
    resultReceiver_.clear();
    if (option == 'h') {
        resultReceiver_.append(SETTING_HELP_MSG);
        return ERR_OK;
    }
    if (argList_.empty()) {
        resultReceiver_.append("Error! please input your operation of setting\n");
        resultReceiver_.append(SETTING_HELP_MSG);
        return ERR_OK;
    }
    const std::string QUERY("query");
    const std::string PUT("put");
    std::string operation = argList_[0];
    if (QUERY != operation && PUT != operation) {
        resultReceiver_.append("Error! please input the correct operation of setting\n");
        resultReceiver_.append(SETTING_HELP_MSG);
        return ERR_OK;
    }
    if (argList_.size() == 1) {
        resultReceiver_.append("Error! please input the key of setting\n");
        resultReceiver_.append(SETTING_HELP_MSG);
        return ERR_OK;
    }
    if (QUERY == operation) {
        std::string key = argList_[1];
        std::string value;
        ErrCode ret = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).GetStringValue(key, value);
        if (ret != ERR_OK) {
            resultReceiver_.append("Error! query setting return failed. ret=" + std::to_string(ret) + "\n");
            return ret;
        }
        resultReceiver_.append("Queried setting key=" + key + ", value=" + value + "\n");
        return ERR_OK;
    }

    if (argList_.size() == 2) {
        resultReceiver_.append("Error! please input the value of setting key\n");
        resultReceiver_.append(SETTING_HELP_MSG);
        return ERR_OK;
    }
    if (PUT == operation) {
        std::string key = argList_[1];
        std::string value = argList_[2];
        ErrCode ret = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).PutStringValue(key, value);
        if (ret != ERR_OK) {
            resultReceiver_.append("Error! put setting return failed. ret=" + std::to_string(ret) + "\n");
            return ret;
        }
        resultReceiver_.append("Put setting key=" + key + ", value=" + value + "\n");
        return ERR_OK;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
