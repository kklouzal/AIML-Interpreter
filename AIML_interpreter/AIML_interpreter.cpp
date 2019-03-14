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
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

namespace AIML
{
	namespace fs = std::experimental::filesystem;

	class Category
	{
	public:
		std::string Pattern;	// Input pattern
		std::string Topic;		// Topic of this category
		std::string That;		// Context limiter for this category
		bool Srai;				// Rematch using this pattern
		std::vector<std::list<std::string*>> Templates;	// Response templates

		Category(std::string UsePattern, std::string UseTopic) : Pattern(UsePattern), Topic(UseTopic), Srai(false)
		{}

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
			std::cout << "\tPattern: " << Pattern << std::endl;
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
			Stars[0] = "Quick";
			Stars[1] = "Brown";
			Stars[2] = "Fox";
			Stars[3] = "Jumps";
			Stars[4] = "Over";
			Stars[5] = "The";
			Stars[6] = "Lazy";
			Stars[7] = "Dog";
		}
		void DebugStars2() {
			Stars[0] = "Pack";
			Stars[1] = "My";
			Stars[2] = "Box";
			Stars[3] = "With";
			Stars[4] = "Five";
			Stars[5] = "Dozen";
			Stars[6] = "Big";
			Stars[7] = "Jugs";
		}

		/*void walk(const rapidxml::xml_node<>* node, int indent = 0) {
			const auto ind = std::string(indent * 4, ' ');
			printf("%s", ind.c_str());

			const rapidxml::node_type t = node->type();
			switch (t) {
			case rapidxml::node_element:
			{
				printf("<%.*s", node->name_size(), node->name());
				for (const rapidxml::xml_attribute<>* a = node->first_attribute()
					; a
					; a = a->next_attribute()
					) {
					printf(" %.*s", a->name_size(), a->name());
					printf("='%.*s'", a->value_size(), a->value());
				}
				printf(">\n");
				printf("%s", ind.c_str());
				printf("DATA:[%.*s]\n", node->value_size(), node->value());

				for (const rapidxml::xml_node<>* n = node->first_node()
					; n
					; n = n->next_sibling()
					) {
					walk(n, indent + 1);
				}
				printf("%s</%.*s>\n", ind.c_str(), node->name_size(), node->name());
			}
			break;

			case rapidxml::node_data:
				printf("DATA:[%.*s]\n", node->value_size(), node->value());
				break;

			default:
				printf("NODE-TYPE:%d\n", t);
				break;
			}
		}*/

		/*void TryWritePiece(std::string *Value){
			if (Category_List.back().Templates.empty()) {
				Category_List.back().Templates.emplace_back(std::list<std::string*>(1, Value));
			}
			else {
				Category_List.back().Templates.back().emplace_back(Value);
			}
		}*/
		//	Recursively walk through a TEMPLATE constructing response templates for an AIML::Category
		void WalkTemplate(const rapidxml::xml_node<>* node) {

			const rapidxml::node_type t = node->type();
			switch (t) {
			case rapidxml::node_element:
			{
				//	Encountered <random>
				if (strcmp(node->name(), "random") == 0) {
					//	Set flag for use of random responses?
					//	This could eliminate the use of double nested arrays
					//		for single response patterns.
					//std::cout << "<random> " << node->value() << std::endl;
				}
				//	Encountered <li>
				else if (strcmp(node->name(), "li") == 0) {
					//	Create a new Response List at the back of Templates
					Category_List.back().Templates.emplace_back(std::list<std::string*>());
					//TryWritePiece(new std::string(node->value()));
					//std::cout << "<li> " << node->value() << std::endl;
				}
				//	Encountered <srai>
				else if (strcmp(node->name(), "srai") == 0) {
					//	Enable this category to use symbolic reduction (sari)
					Category_List.back().SetSRAI();
					//std::cout << "<srai> " << node->value() << std::endl;
				}
				else if (strcmp(node->name(), "star") == 0) {
					//	Insert reference to appropriate <star/> value
					Category_List.back().Templates.back().push_back(&Stars[0]);
					//TryWritePiece(&Stars[0]);
					//std::cout << "<star> " << node->value() << std::endl;
				}
				else if (strcmp(node->name(), "get") == 0) {
					//	Insert reference to appropriate <get name='...'/> value
					//auto VariableName = node->first_attribute("name");
					//std::cout << "Variable Name: " << VariableName->value() << std::endl;
					//Category_List.back().Templates.back().push_back(&Stars[0]);
					//std::cout << "<get> " << node->value() << std::endl;
				}
				else if (strcmp(node->name(), "set") == 0) {
					//	Update value to appropriate <set name='...'>...</set> value
					//auto VariableName = node->first_attribute("name");
					//std::cout << "Variable Name: " << VariableName->value() << std::endl;
					//std::cout << "Variable Value: " << node->value() << std::endl;
					//Category_List.back().Templates.back().push_back(&Stars[0]);
					//std::cout << "<set> " << node->value() << std::endl;
				}
				else {
					printf("<%s> %s\n", node->name(), node->value());
					for (auto a = node->first_attribute(); a; a = a->next_attribute()) {
						printf("<set/get %s", a->name());
						printf("='%s'", a->value());
						printf(">\n");
					}
				}

				for (auto n = node->first_node(); n; n = n->next_sibling()) {
					WalkTemplate(n);
				}
			}
			break;

			case rapidxml::node_data:
				//	Add this data to the end of the current template
					if (Category_List.back().Templates.empty()) {
						Category_List.back().Templates.emplace_back(std::list<std::string*>(1, new std::string(node->value())));
					}
					else {
						Category_List.back().Templates.back().emplace_back(new std::string(node->value()));
					}
					//printf("DATA:[%s]\n", node->value());
				break;

			default:
				printf("NODE-TYPE:%d\n", t);
				break;
			}
		}

		void ParseCategories(rapidxml::xml_node<>* Root_Node, std::string UseTopic = "") {
			for (rapidxml::xml_node<>* Category_Node = Root_Node->first_node("category"); Category_Node; Category_Node = Category_Node->next_sibling())
			{
				//	Dissect PATTERN
				rapidxml::xml_node<>* Pattern_Node = Category_Node->first_node("pattern");
				if (Pattern_Node == NULL) {
					std::cout << "Skipping Malformed Category: No PATTERN" << std::endl;
					continue;
				}
				std::string Pattern = Pattern_Node->value();
				//std::cout << Pattern << std::endl;
				//	Dissect TEMPLATE
				rapidxml::xml_node<>* Template_Node = Category_Node->first_node("template");
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
				rapidxml::xml_node<>* That_Node = Category_Node->first_node("that");
				if (That_Node != NULL) {
					std::string That = That_Node->value();
					Category_List.back().SetThat(That);
				}
				std::cout << std::endl;
			}
		}

		//	Default debugging values for the Stars array.
		Bot() : Stars({ "*1*","*2*","*3*","*4*","*5*","*6*","*7*","*8*" })
		{
			fs::path AIML_Folder = fs::current_path().append("AIML");
			std::cout << AIML_Folder << std::endl;
			if (fs::exists(AIML_Folder) && fs::is_directory(AIML_Folder)) {
				std::cout << "Found AIML Folder" << std::endl;
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
									rapidxml::xml_attribute<>* Topic_Name = Topic_Node->first_attribute("name");
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
	};
}

int main()
{
	AIML::Bot MyBot;
	std::system("PAUSE");
	MyBot.DebugCategories();
	std::system("PAUSE");
	/*MyBot.DebugStars1();
	MyBot.DebugCategories();
	std::system("PAUSE");
	MyBot.DebugStars2();
	MyBot.DebugCategories();
	std::system("PAUSE");*/
    //std::cout << "Hello World!\n"; 
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
