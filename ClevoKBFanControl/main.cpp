//
//  main.cpp
//  ClevoKBFanControl
//
//  Created by datasone on 8/8/2017.
//  Copyright © 2017 datasone. All rights reserved.
//

#include <iostream>
#include <sys/kern_control.h>
#include <sys/kern_event.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "args.hxx"
#include "ECCtrl.h"

#define SET_KB_LED 0x67
#define SET_FAN 0x79

using std::string;

void sendctl(struct ECCtrl ctrl)
{
	struct ctl_info info;
	struct sockaddr_ctl addr;
	int fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
	if (fd != -1)
	{
		bzero(&addr, sizeof(addr));
		addr.sc_len = sizeof(addr);
		addr.sc_family = AF_SYSTEM;
		addr.ss_sysaddr = AF_SYS_CONTROL;
		memset(&info, 0, sizeof(info));
		strcpy(info.ctl_name, "moe.datasone.clevocontrol.ctl");
		if (ioctl(fd, CTLIOCGINFO, &info))
		{
			exit(-1);
		}
		addr.sc_id = info.ctl_id;
		addr.sc_unit = 0;
		if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_ctl)))
			exit(-1);
		send(fd, &ctrl, sizeof(ECCtrl), 0);
	}
}

int main(int argc, const char * argv[]) {
	args::ArgumentParser parser("Control keyboard backlight & fan for Clevo laptop. Only for full color keyboard backlight(without extra)", "Note that:\n You need to toggle backlight on manually.\n Please set three fan parameters simultaneously(I was just too lazy to implement read operations)\n The color options will be ignored if toggle option is set and colors option will be ignored when color option is set.");
	args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
	args::ValueFlag<string> bltoggle(parser, "ON|OFF", "Toggle keyboard backlight", {'l', "light"});
	args::ValueFlag<string> mode(parser, "BREATHE|CYCLE|DANCE|FLASH|RANDOM_COLOR|TEMPO|WAVE", "Set keyboard backlight mode", {'m', "mode"});
	args::ValueFlag<int> brightness(parser, "BRIGHTNESS(0|1|2|3)", "Set keyboard backlight brightness", {'b', "brightness"});
	args::ValueFlag<string> color(parser, "COLOR", "Keyboard backlight color in RGB hex", {'c', "color"});
	args::ValueFlag<string> colorList(parser, "COLORLIST", "Keyboard backlight colors, from left to right, split by comma", {"cl", "colors"});
	args::Flag autoFan(parser, "auto", "Set fan speed to auto", {'a', "auto"});
	args::ValueFlag<int> startTemp(parser, "TEMPERATURE", "Fan start temperature", {"stt", "startTemp"});
	args::ValueFlag<int> stopTemp(parser, "TEMPERATURE", "Fan stop temperature", {"spt", "stopTemp"});
	args::ValueFlag<int> fanMaxSpeed(parser, "PERCENTAGE", "Max fan speed", {'s', "speed"});
	try
	{
		parser.ParseCLI(argc, argv);
	}
	catch (args::Help)
	{
		std::cout << parser;
		return 0;
	}
	catch (args::ValidationError e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}
	struct ECCtrl ctrl;
	if (bltoggle)
	{
		string state = args::get(bltoggle);
		ctrl.arg0 = 0;
		ctrl.arg1 = SET_KB_LED;
		ctrl.arg2 = 0xE0000000;
		if (state == "OFF" || state == "off")
			ctrl.arg2 |= 0x003001;
		else if (state == "ON" || state == "on")
			ctrl.arg2 |= 0x07F001;
		else
		{
			std::cerr << "Wrong state!";
			return 1;
		}
		sendctl(ctrl);
	}
	else if (mode)
	{
		string state = args::get(mode);
		struct ECCtrl ctrl2;
		ctrl2.arg0 = 0;
		ctrl2.arg1 = SET_KB_LED;
		ctrl2.arg2 = 0x10000000;
		ctrl.arg0 = 0;
		ctrl.arg1 = SET_KB_LED;
		if (state == "BREATHE" || state == "breathe")
			ctrl.arg2 = 0x1002A000;
		else if (state == "CYCLE" || state == "cycle")
			ctrl.arg2 = 0x33010000;
		else if (state == "DANCE" || state == "dance")
			ctrl.arg2 = 0x80000000;
		else if (state == "FLASH" || state == "flash")
			ctrl.arg2 = 0xA0000000;
		else if (state == "RANDOM_COLOR" || state == "random_color")
			ctrl.arg2 = 0x70000000;
		else if (state == "TEMPO" || state == "tempo")
			ctrl.arg2 = 0x90000000;
		else if (state == "WAVE" || state == "wave")
			ctrl.arg2 = 0xB0000000;
		else
		{
			std::cerr << "Wrong mode!";
			return 1;
		}
		sendctl(ctrl2);
		sendctl(ctrl);
	}
	else if (color)
	{
		string state = args::get(color);
		if (state.size() != 6)
		{
			std::cerr << "Wrong color!";
			return 1;
		}
		string state_brg = state.substr(4, 2) + state.substr(0, 2) + state.substr(2, 2);
		uint32_t i = std::stoul(state_brg, nullptr, 16);
		struct ECCtrl ctrl2;
		ctrl2.arg0 = 0;
		ctrl2.arg1 = SET_KB_LED;
		ctrl2.arg2 = 0x10000000;
		sendctl(ctrl2);
		ctrl.arg0 = 0;
		ctrl.arg1 = SET_KB_LED;
		ctrl.arg2 = 0xF0000000;
		ctrl.arg2 |= i;
		sendctl(ctrl);
		ctrl.arg2 = 0xF1000000;
		ctrl.arg2 |= i;
		sendctl(ctrl);
		ctrl.arg2 = 0xF2000000;
		ctrl.arg2 |= i;
		sendctl(ctrl);
	}
	else if (colorList)
	{
		string stateList = args::get(colorList);
		if (stateList.size() != 20)
		{
			std::cerr << "Wrong color!";
			return 1;
		}
		struct ECCtrl ctrl2;
		ctrl2.arg0 = 0;
		ctrl2.arg1 = SET_KB_LED;
		ctrl2.arg2 = 0x10000000;
		sendctl(ctrl2);
		ctrl.arg0 = 0;
		ctrl.arg1 = SET_KB_LED;
		string colors[3];
		for (int i = 0; i < 3; ++i) {
			colors[i] = stateList.substr(6 * i + i, 6);
			colors[i] = colors[i].substr(4, 2) + colors[i].substr(0, 2) + colors[i].substr(2, 2);
			uint32_t j = std::stoul(colors[i], nullptr, 16);
			ctrl.arg2 = 0xF0000000 + i * 0x1000000;
			ctrl.arg2 |= j;
			sendctl(ctrl);
		}
	}
	if (brightness) {
		uint8_t lvl_to_raw[] = { 63, 126, 189, 252 };
		int b = args::get(brightness);
		if (b < 0 || b > 3)
		{
			std::cerr << "Wrong brightness value!";
			return 1;
		}
		ctrl.arg0 = 0;
		ctrl.arg1 = SET_KB_LED;
		ctrl.arg2 = 0xF4000000 | lvl_to_raw[b];
		sendctl(ctrl);
	}
	if (autoFan)
	{
		ctrl.arg0 = 0;
		ctrl.arg1 = SET_FAN;
		ctrl.arg2 = 0x1000000;
		sendctl(ctrl);
		ctrl.arg2 = 0;
		ctrl.arg2 |= 0x7000000;
		sendctl(ctrl);
	}
	else if (startTemp && stopTemp && fanMaxSpeed)
	{
		int ctrlData = (args::get(fanMaxSpeed) << 16) + (args::get(stopTemp) << 8) + args::get(startTemp);
		ctrlData |= 0x7000000;
		ctrl.arg0 = 0;
		ctrl.arg1 = SET_FAN;
		ctrl.arg2 = ctrlData;
		sendctl(ctrl);
	}
	return 0;
}
