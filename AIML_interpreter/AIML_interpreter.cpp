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

namespace AIML
{
	namespace fs = std::experimental::filesystem;

	std::list<std::string> split(const std::string& s, char delimiter=' ')
	{
		std::list<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(s);
		while (std::getline(tokenStream, token, delimiter))
		{
			tokens.push_back(token);
		}
		return tokens;
	}

	class Category
	{
	public:
		//std::string Pattern;	// Input pattern
		std::list<std::string> Pattern;	// Input pattern tokenized into individual words
		std::string Topic;		// Topic of this category
		std::string That;		// Context limiter for this category
		bool Srai;				// Rematch using this pattern
		std::vector<std::list<std::string*>> Templates;	// Response templates
		//	unordered_map 'SetList' string/string - Values to be set during template call

		//	Capitalize every letter in the input patter and tokenize it into individual words.
		Category(std::string UsePattern, std::string UseTopic) : Topic(UseTopic), Srai(false) {
			for (auto& x : UsePattern) {
				x = toupper(x);
			}
			std::string token;
			std::istringstream tokenStream(UsePattern);
			while (std::getline(tokenStream, token, ' '))
			{
				Pattern.push_back(token);
			}
		}

		~Category() {
			for (auto Template : Templates) {
				for (auto Piece : Template) {
					//delete Piece;
				}
			}
		}

		//	So we can visually debug the state of our memory
		void PrintData() {
			std::cout << std::endl << "Category Data" << std::endl;
			std::cout << "\tPattern:";
			for (auto Token : Pattern) {
				std::cout << " " << Token;
			}
			std::cout << std::endl;
			std::cout << "\tTopic: " << Topic << std::endl;
			std::cout << "\tThat: " << That << std::endl;
			std::cout << "\tSrai: " << Srai << std::endl;
			std::cout << "\tTemplates: " << std::endl;
			unsigned int TemplateNum = 0;
			for (auto Template : Templates) {
				std::cout << "\t\t[" << TemplateNum << "]:" << std::endl;
				TemplateNum++;
				unsigned int PieceNum = 0;
				for (auto Piece : Template) {
					std::cout << "\t\t\t[" << PieceNum << "]: " << *Piece << std::endl;
					PieceNum++;
				}
			}
		}

		void SetSRAI() { Srai = true; }
		void SetThat(std::string UseThat) { That = UseThat; }
	};

	class Bot
	{
		std::queue<rapidxml::file<>> XML;
		std::queue<rapidxml::xml_document<>> Documents;
	public:
		std::string Topic;	// Current topic
		std::string That;	// Current context
		std::unordered_map<std::string, std::string> Variables;	// Current stored variables
		std::array<std::string, 8> Stars; // Current <star/> inputs
		std::vector<Category> Category_List;

		void DebugCategories() {
			for (auto Category : Category_List) {
				Category.PrintData();
			}
		}
		void DebugStars1() {
			Stars[0] = "Quick";	Stars[1] = "Brown";	Stars[2] = "Fox"; Stars[3] = "Jumps";
			Stars[4] = "Over"; Stars[5] = "The"; Stars[6] = "Lazy";	Stars[7] = "Dog";
		}
		void DebugStars2() {
			Stars[0] = "Pack"; Stars[1] = "My";	Stars[2] = "Box"; Stars[3] = "With";
			Stars[4] = "Five"; Stars[5] = "Dozen"; Stars[6] = "Big"; Stars[7] = "Jugs";
		}

		//	Recursively walk through a TEMPLATE constructing response templates for an AIML::Category
		void WalkTemplate(const rapidxml::xml_node<>* node, bool IsThinking=false, bool IsSetting=false) {
			switch (node->type()) {
			case rapidxml::node_element:
			{
				//	Encountered <random>
				if (strcmp(node->name(), "random") == 0) {}
				//	Encountered <li>
				else if (strcmp(node->name(), "li") == 0) {
					//	Create a new Response List at the back of Templates
					Category_List.back().Templates.emplace_back(std::list<std::string*>());
				}
				//	Encountered <think>
				else if (strcmp(node->name(), "think") == 0) {
					//	Set children of this node to 'think' status
					IsThinking = true;
				}
				//	Encountered <srai>
				else if (strcmp(node->name(), "srai") == 0) {
					//	Enable this category to use symbolic reduction (sari)
					Category_List.back().SetSRAI();
				}
				else if (strcmp(node->name(), "star") == 0) {
					//	Insert reference to appropriate <star/> value
					Category_List.back().Templates.back().push_back(&Stars[0]);
				}
				else if (strcmp(node->name(), "get") == 0) {
					//	Insert reference to appropriate <get name='...'/> value
					auto VariableName = node->first_attribute("name");
					std::cout << "Variable Name: " << VariableName->value() << std::endl;
					Category_List.back().Templates.back().push_back(&Variables[VariableName->value()]);
					//std::cout << "<get> " << node->value() << std::endl;
				}
				else if (strcmp(node->name(), "set") == 0) {
					//	Update reference to appropriate <set name='...'>...</set> value
					auto VariableName = node->first_attribute("name");
					std::cout << "Variable Name: " << VariableName->value() << std::endl;
					std::cout << "Variable Value: " << node->value() << std::endl;
					//Category_List.back().Templates.back().push_back(&Stars[0]);
					//std::cout << "<set> " << node->value() << std::endl;
				}
				else {
					printf("Unknown Tag <%s> %s\n", node->name(), node->value());
					for (auto a = node->first_attribute(); a; a = a->next_attribute()) {
						printf("<set/get %s", a->name());
						printf("='%s'", a->value());
						printf(">\n");
					}
				}
				//	Walk through all children of this node
				//	If this node is <think> set 'IsThinking' to true
				for (auto n = node->first_node(); n; n = n->next_sibling()) {
					WalkTemplate(n, IsThinking);
				}
			}
			break;

			case rapidxml::node_data:
			{
				//	Add this data to the end of the current template
				if (!IsThinking) {
					if (Category_List.back().Templates.empty()) {
						Category_List.back().Templates.emplace_back(std::list<std::string*>(1, new std::string(node->value())));
					}
					else {
						Category_List.back().Templates.back().emplace_back(new std::string(node->value()));
					}
				}
			}
			break;

			default: printf("Unknown Node Type\n"); break;
			}
		}

		void ParseCategories(rapidxml::xml_node<>* Root_Node, std::string UseTopic = "") {
			for (auto Category_Node = Root_Node->first_node("category"); Category_Node; Category_Node = Category_Node->next_sibling())
			{
				//	Dissect PATTERN
				auto Pattern_Node = Category_Node->first_node("pattern");
				if (Pattern_Node == NULL) {
					std::cout << "Skipping Malformed Category: No PATTERN" << std::endl;
					continue;
				}
				std::string Pattern = Pattern_Node->value();
				//	Dissect TEMPLATE
				auto Template_Node = Category_Node->first_node("template");
				if (Template_Node == NULL) {
					std::cout << "Skipping Malformed Category: No TEMPLATE" << std::endl;
					continue;
				}
				//	Start constructing a new CATEGORY
				Category_List.emplace_back(Category(Pattern, UseTopic));
				for (auto Node = Template_Node->first_node(); Node; Node = Node->next_sibling()) {
					WalkTemplate(Node);
				}
				//	Dissect THAT
				auto That_Node = Category_Node->first_node("that");
				if (That_Node != NULL) {
					std::string That = That_Node->value();
					Category_List.back().SetThat(That);
				}
			}
		}

		//	Default debugging values for the Stars array.
		Bot() : Stars({ "*1*","*2*","*3*","*4*","*5*","*6*","*7*","*8*" })
		{
			fs::path AIML_Folder = fs::current_path().append("AIML");
			std::cout << AIML_Folder << std::endl;
			if (fs::exists(AIML_Folder) && fs::is_directory(AIML_Folder)) {
				for (const auto& entry : fs::directory_iterator(AIML_Folder))
				{
					auto filename = entry.path().filename();
					if (fs::is_directory(entry.status()))
					{
						std::cout << "Unable to load nested folders (yet)" << std::endl;
					}
					else if (fs::is_regular_file(entry.status())) {
						std::cout << "Loading (" << filename << ").." << std::endl;
						XML.emplace(entry.path().string().c_str());
						Documents.emplace();
						try {
							//	Parse the raw XML data into an XML Document object
							Documents.back().parse<rapidxml::parse_default | rapidxml::parse_normalize_whitespace>(XML.back().data());
							//	Iterate through the XML extracting C++ objects from the AIML
							rapidxml::xml_node<>* Root_Node = Documents.back().first_node("aiml");
							for (rapidxml::xml_node<>* Topic_Node = Root_Node->first_node("topic"); Topic_Node; Topic_Node = Topic_Node->next_sibling())
							{
								if (strcmp(Topic_Node->name(), "topic") == 0) {
									auto Topic_Name = Topic_Node->first_attribute("name");
									if (Topic_Name == NULL) {
										std::cout << "Skipping Malformed Topic: No NAME" << std::endl;
										continue;
									}
									ParseCategories(Topic_Node, Topic_Name->value());
								}
							}
							ParseCategories(Root_Node);
						}
						catch (const rapidxml::parse_error & e) {
							std::cout << "\tXML Error: " << e.what() << " - " << e.where<char>() << std::endl;
						}
					}
				}
			}
		}

		~Bot() {
			while (!Documents.empty()) {
				Documents.pop();
			}
			while (!XML.empty()) {
				XML.pop();
			}
		}

		//	0 = No Match			[ WORD != WORD ]
		//	1 = Direct Word Match	[ WORD == WORD ]
		//	2 = Direct * Match		[ WORD -> * ]
		//	3 = Previous * Match	[ WORD -> (*<-WORD) ]
		//
		//	PatternWord must be supplied in all caps.
		//	Do not check Previous * Match when bFirst is true.
		const unsigned short CheckWords(const std::string& InputWord, std::list<std::string>::const_iterator& PatternWord, const bool bFirst) const
		{
			//	Debug: print *PatternWord to ensure we've not jacked the sequence up and are going in order.
			//std::cout << "\t\t CheckWords-> " << *PatternWord << std::endl;
			if (*PatternWord == "*") {
				return 2;
			}
			else if (	equal(InputWord.begin(), InputWord.end(),
						(*PatternWord).begin(), (*PatternWord).end(),
						[](char Input, char Pattern) {
							return toupper(Input) == Pattern;
				})) {
				return 1;
			}
			else if (!bFirst && *--PatternWord == "*") {
				// Don't worry about incrementing the iterator back to where it was?
				// Chances are the next 'InputWord' being checked will be of type 3.
				// If the pattern match fails at this point we'll scrap this category anyways.
				//	--I think all that's correct ?:D
				return 3;
			}
			return 0;
		}

		std::string InputText(std::string InText) {
			std::vector<Category*> Matches;

			auto InWords = split(InText);

			for (auto Cat : Category_List) {
				auto PatWords = Cat.Pattern;

				auto PatIter = PatWords.cbegin();
				auto InIter = InWords.cbegin();

				bool bTryCategory = true;
				while (bTryCategory) {
					switch (CheckWords(*InIter, PatIter, (PatIter == PatWords.cbegin())))
					{
					case 0: bTryCategory = false; break;	//	Halt; No Match

					case 1:	//	Direct Word Match
					{
						std::cout << *InIter << "\tWORD Match\n";
						auto PatternEnd = (++PatIter == PatWords.cend());
						auto InputEnd = (++InIter == InWords.cend());
						if (PatternEnd) {
							if (InputEnd) {
								//	Pattern END
								//	Input	END
								//	-- Complete match
								bTryCategory = false;
								//std::cout << "\tEnd Pattern & Input\n";
							}
							else {
								//	Pattern END
								//	Input	Remaining
								//	-- Partial Input Match
								bTryCategory = false;
								//std::cout << "\tEnd Pattern\n";
							}
						}
						else if (InputEnd) {
							//	Pattern Remaining
							//	Input	END
							//	-- Partial pattern match (no match)
							bTryCategory = false;
							//std::cout << "\tEnd Input\n";
						}
					}
					break;

					case 2:	//	Direct * Match
					{
						std::cout << *InIter << "\t* Match\n";
						auto PatternEnd = (++PatIter == PatWords.cend());
						auto InputEnd = (++InIter == InWords.cend());

						if (PatternEnd) {
							if (InputEnd) {
								//	Pattern END
								//	Input	END
								//	-- Complete match
								bTryCategory = false;
								//std::cout << "\tEnd Pattern & Input\n";
							}
							else {
								//	Pattern END *
								//	Continue processing input
								PatIter--;
							}
						}
						else if (InputEnd) {
							//	Pattern Remaining
							//	Input	END
							//	-- Partial pattern match (no match)
							bTryCategory = false;
							//std::cout << "\tEnd Input\n";
						}
					}
					break;

					case 3:	//	Previous * Match
					{
						std::cout << *InIter << "\tPrevious * Match\n";
						auto PatternEnd = (++PatIter == PatWords.cend());
						auto InputEnd = (++InIter == InWords.cend());

						if (PatternEnd) {
							if (InputEnd) {
								//	Pattern END
								//	Input	END
								//	-- Complete match
								bTryCategory = false;
								//std::cout << "\tEnd Pattern & Input\n";
							}
							else {
								//	Pattern END *
								//	Continue processing input
								//	Already decremented in CheckWords()
								//PatIter--;
							}
						}
						else if (InputEnd) {
							//	Pattern Remaining
							//	Input	END
							//	-- Partial pattern match (no match)
							bTryCategory = false;
							//std::cout << "\tEnd Input\n";
						}
					}
					break;

					default: std::cout << "Pattern Matching Error" << std::endl; break;
					}
				}
				//do stuff
			}
			return std::string("placeholder");
		}
	};
}

int main()
{
	AIML::Bot MyBot;
	//std::system("PAUSE");
	//MyBot.DebugCategories();
	//std::system("PAUSE");
	/*MyBot.DebugStars1();
	MyBot.DebugCategories();
	std::system("PAUSE");
	MyBot.DebugStars2();
	MyBot.DebugCategories();
	std::system("PAUSE");*/

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
		else {
			std::string Output = MyBot.InputText(InputText);
			std::cout << "BOT: " << Output.c_str() << std::endl;
		}
	}
	std::system("PAUSE");
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
