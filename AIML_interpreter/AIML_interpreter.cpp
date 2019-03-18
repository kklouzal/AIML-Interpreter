// AIML_interpreter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <utility>
#include <queue>
#include <vector>
#include <list>
#include <array>
#include <unordered_map>
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

#include "graphmaster.hpp"

int main()
{
	AIML::GraphMaster GM;
	{
		AIML::Bot MyBot = GM.CreateBot();

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
				GM.DebugCategories(&MyBot.Variables, &MyBot.Stars);
			}
			else if (InputText == "dbg2") {
				MyBot.DebugStars2();
				GM.DebugCategories(&MyBot.Variables, &MyBot.Stars);
			}
			else if (InputText == "pstr") {
				MyBot.PrintStars();
			}
			else {
				//std::string Output = MyBot.InputText(InputText);
				GM.MatchPattern(InputText, &MyBot.Variables, &MyBot.Stars);
				//std::cout << "BOT: " << Output.c_str() << std::endl;
			}
		}
		std::system("PAUSE");
	}
}