// AIML_interpreter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <utility>
#include <queue>
#include <list>
#include <array>
#include <unordered_map>
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

#include "bot.hpp"

int main()
{
	AIML::GraphMaster GM;
	AIML::Bot MyBot(&GM);

	bool ExitProgram = false;
	std::string InputText;
	char c;

	while (!ExitProgram) {
		InputText = "";
		while (std::cin.get(c) && c != '\n') {
			InputText += c;
		}
		if (InputText == "quit" || InputText == "exit") {
			ExitProgram = true;
		}
		else if (InputText == "dbg1") {
			MyBot.DebugStars1();
			GM.DebugCategories(MyBot.GetVars(), MyBot.GetStars());
		}
		else if (InputText == "dbg2") {
			MyBot.DebugStars2();
			GM.DebugCategories(MyBot.GetVars(), MyBot.GetStars());
		}
		else if (InputText == "pstr") {
			MyBot.PrintStars();
		}
		else {
			std::string Output = MyBot.InputText(InputText);
			std::cout << "BOT: " << Output.c_str() << std::endl;
		}
	}
	std::system("PAUSE");
}