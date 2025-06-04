// AIML_interpreter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <utility>
#include <queue>
#include <vector>
#include <list>
#include <array>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <sstream>
#include <ctime>
#include <filesystem>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

namespace AIML
{
	namespace fs = std::filesystem;

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
		std::list<std::string> Pattern;	// Input pattern tokenized into individual words
		std::string Topic;		// Topic of this category
		std::string That;		// Context limiter for this category
		bool Srai;				// Rematch using this pattern
		std::vector<std::list<std::string*>> Templates;	// Response templates

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
					delete Piece;
				}
			}
		}

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
				if (strcmp(node->name(), "random") == 0) {
					// Make sure we have a template to append to
					if (Category_List.back().Templates.empty()) {
						Category_List.back().Templates.emplace_back(std::list<std::string*>());
					}
					
					std::vector<std::list<std::string*>> randomResponses;
					
					// Process each <li> element and build separate responses
					for (auto li_node = node->first_node("li"); li_node; li_node = li_node->next_sibling("li")) {
						// Create a new response for this option
						randomResponses.emplace_back(std::list<std::string*>());
						
						// Process text content directly in the <li> tag
						if (li_node->value() && strlen(li_node->value()) > 0) {
							randomResponses.back().push_back(new std::string(li_node->value()));
							}
						
						// Process child nodes of this <li>
						for (auto child = li_node->first_node(); child; child = child->next_sibling()) {
							// Create a temporary Category to capture this li's template
							Category tempCat("TEMP", "");
							tempCat.Templates.emplace_back(std::list<std::string*>());
							
							// Save current category
							Category* originalCat = &Category_List.back();
							
							// Temporarily replace with our temp category
							Category_List.back() = tempCat;
							
							// Process the li's content
							WalkTemplate(child, IsThinking);
							
							// Get the processed content
							if (!Category_List.back().Templates.empty() && !Category_List.back().Templates.back().empty()) {
								// Copy pointers to the random response
								for (auto* piece : Category_List.back().Templates.back()) {
									randomResponses.back().push_back(piece);
								}
								
								// Clear pointers from temp category so they're not deleted
								Category_List.back().Templates.back().clear();
							}
							
							// Restore the original category
							Category_List.back() = *originalCat;
						}
					}
					
					// Select a random response
					if (!randomResponses.empty()) {
						int randomIndex = std::rand() % randomResponses.size();
						
						// Add the chosen response to the current template
						for (auto* piece : randomResponses[randomIndex]) {
							Category_List.back().Templates.back().push_back(piece);
						}
						
						// Delete unused responses to prevent memory leaks
						for (size_t i = 0; i < randomResponses.size(); i++) {
							if (i != randomIndex) {
								for (auto* piece : randomResponses[i]) {
									delete piece;
								}
							}
						}
					}
					
					// Don't need to recurse as we've manually processed children
					return;
				}
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
					 // Insert reference to appropriate <star/> value
					auto indexAttr = node->first_attribute("index");
					int index = 0; // Default to first star
					
					if (indexAttr) {
						index = std::stoi(indexAttr->value()) - 1; // AIML uses 1-based indexing
						// Bounds checking
						if (index < 0 || index >= Stars.size()) {
							index = 0;
						}
					}
					
					Category_List.back().Templates.back().push_back(&Stars[index]);
				}
				else if (strcmp(node->name(), "get") == 0) {
					//	Insert reference to appropriate <get name='...'/> value
					auto VariableName = node->first_attribute("name");
					if (VariableName) {
						// Make sure the variable exists
						if (Variables.find(VariableName->value()) == Variables.end()) {
							Variables[VariableName->value()] = "";
						}
						Category_List.back().Templates.back().push_back(&Variables[VariableName->value()]);
					}
				}
				else if (strcmp(node->name(), "set") == 0) {
					// Update reference to appropriate <set name='...'>...</set> value
					auto VariableName = node->first_attribute("name");
					if (VariableName) {
						std::string varName = VariableName->value();
						
						// Create a temporary list to collect content pieces
						std::list<std::string*> contentPieces;
						std::string collectedContent;
						
						// Process child nodes to build content
						for (auto child = node->first_node(); child; child = child->next_sibling()) {
							// Save current template to restore after processing
							auto currentTemplate = Category_List.back().Templates.back();
							
							// Set a temporary empty template for collecting content
							Category_List.back().Templates.back() = contentPieces;
							
							// Process the child node
							WalkTemplate(child, false, true);
							
							// Get back processed content
							contentPieces = Category_List.back().Templates.back();
							
							// Restore original template
							Category_List.back().Templates.back() = currentTemplate;
						}
						
						// Collect text content
						for (auto* piece : contentPieces) {
							collectedContent += *piece;
							delete piece;  // Clean up temporary pieces
						}
						
						// Also handle direct text in the set tag
						if (node->value() && strlen(node->value()) > 0) {
							collectedContent += node->value();
						}
						
						// Set the variable value
						Variables[varName] = collectedContent;
						
						// Add variable reference to template
						Category_List.back().Templates.back().push_back(&Variables[varName]);
					}
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
			// Seed random number generator
			std::srand(static_cast<unsigned int>(std::time(nullptr)));
			
			fs::path AIML_Folder = fs::current_path() / "AIML";
			std::cout << AIML_Folder << std::endl;
			if (fs::exists(AIML_Folder) && fs::is_directory(AIML_Folder)) {
				for (const auto& entry : fs::directory_iterator(AIML_Folder))
				{
					auto filename = entry.path().filename();
					if (fs::is_directory(entry.path())) {
						std::cout << "Unable to load nested folders (yet)" << std::endl;
					}
					else if (fs::is_regular_file(entry.path())) {
						std::cout << "Loading (" << filename << ").." << std::endl;
						XML.emplace(entry.path().string().c_str());
						Documents.emplace();
						try {
							//	Parse the raw XML data into an XML Document object
							Documents.back().parse<rapidxml::parse_default | rapidxml::parse_normalize_whitespace>(XML.back().data());
							//	Iterate through the XML extracting C++ objects from the AIML
							rapidxml::xml_node<>* Root_Node = Documents.back().first_node("aiml");
							if (!Root_Node) {
								std::cout << "\tError: Not a valid AIML file (missing <aiml> root node)" << std::endl;
								continue;
							}

							for (rapidxml::xml_node<>* Topic_Node = Root_Node->first_node("topic"); Topic_Node; Topic_Node = Topic_Node->next_sibling("topic"))
							{
								auto Topic_Name = Topic_Node->first_attribute("name");
								if (Topic_Name == NULL) {
									std::cout << "Skipping Malformed Topic: No NAME" << std::endl;
									continue;
								}
								ParseCategories(Topic_Node, Topic_Name->value());
							}
							ParseCategories(Root_Node);
						}
						catch (const rapidxml::parse_error & e) {
							std::cout << "\tXML Error: " << e.what() << std::endl;
						}
					}
				}
			} else {
				std::cout << "AIML folder not found or not a directory. Creating folder..." << std::endl;
				try {
					fs::create_directory(AIML_Folder);
					std::cout << "AIML folder created. Please add AIML files to continue." << std::endl;
				} catch (const std::exception& e) {
					std::cout << "Error creating AIML folder: " << e.what() << std::endl;
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

		std::string InputText(std::string InText) {
			// Reset Stars array before processing
			for (int i = 0; i < Stars.size(); i++) {
				Stars[i] = "*" + std::to_string(i+1) + "*";
			}
            
			std::vector<Category*> Matches;
			
			// Capitalize input for matching
			std::string uppercaseInput = InText;
			for (auto& c : uppercaseInput) {
				c = toupper(c);
			}
			
			auto InWords = split(uppercaseInput);
			const auto Input_End = InWords.cend();
			
			// Find matching categories
			for (auto& Cat : Category_List) {
				auto Input = InWords.cbegin();
				auto Pattern = Cat.Pattern.cbegin();
				
				const auto Pattern_Begin = Cat.Pattern.cbegin();
				const auto Pattern_End = Cat.Pattern.cend();
				
				bool IsMatch = true;
				bool InputEnd = false;
				bool PatternEnd = false;
				int starIndex = 0;
				
				while (IsMatch && !(InputEnd && PatternEnd)) {
					// Direct Wildcard Match
					if (Pattern != Pattern_End && *Pattern == "*") {
						 // Start of wildcard capture
						auto wildcardStart = Input;
						std::string wildcardValue;
						
						// Handle wildcard by consuming input until next pattern match or end
						Pattern++;
						PatternEnd = (Pattern == Pattern_End);
						
						if (PatternEnd) {
							// If wildcard is last element, capture all remaining input
							std::stringstream wildcardStream;
							while (Input != InWords.cend()) {
								wildcardStream << *Input << " ";
								Input++;
							}
							
							// Store captured value in next Star slot
							if (starIndex < Stars.size()) {
								Stars[starIndex++] = wildcardStream.str();
							}
                            
							Input = InWords.cend();
							InputEnd = true;
							break;
						} else {
							// Find next matching word after wildcard
							bool foundNext = false;
							auto searchInput = Input;
							
							while (searchInput != InWords.cend()) {
								if (equal((*searchInput).begin(), (*searchInput).end(), (*Pattern).begin(), (*Pattern).end(),
									[](char I, char P) { return toupper(I) == P; })) {
									foundNext = true;
									break;
								}
								searchInput++;
							}
							
							if (!foundNext) {
								IsMatch = false;
								break;
							}
                            
							// Capture wildcard text
							std::stringstream wildcardStream;
							while (Input != searchInput) {
								wildcardStream << *Input << " ";
								Input++;
							}
							
							// Store captured value in next Star slot
							if (starIndex < Stars.size()) {
								Stars[starIndex++] = wildcardStream.str();
							}
						}
					}
					// Direct Word Match
					else if (Input != Input_End && Pattern != Pattern_End && 
							equal((*Input).begin(), (*Input).end(), (*Pattern).begin(), (*Pattern).end(),
							[](char I, char P) { return toupper(I) == P; })) {
						Input++;
						Pattern++;
						InputEnd = (Input == Input_End);
						PatternEnd = (Pattern == Pattern_End);
					}
					else {
						IsMatch = false;
						break;
					}
				}
				
				// Complete match found
				if (IsMatch && (PatternEnd || (Pattern != Pattern_End && *Pattern == "*"))) {
					// Check Topic and That constraints
					bool topicMatches = Cat.Topic.empty() || Cat.Topic == Topic;
					bool thatMatches = Cat.That.empty() || Cat.That == That;
					
					if (topicMatches && thatMatches) {
						Matches.push_back(&Cat);
					}
				}
			}
			
			// If there are matches, select one and generate a response
			if (!Matches.empty()) {
				// For now, just use the first match
				Category* bestMatch = Matches[0];
				
				// Select a random template if there are multiple
				int templateIdx = 0;
				if (bestMatch->Templates.size() > 1) {
					templateIdx = std::rand() % bestMatch->Templates.size();
				}
				
				// Check for SRAI (symbolic reduction)
				if (bestMatch->Srai) {
					// Get the SRAI content
					std::stringstream sraiContent;
					for (auto& piece : bestMatch->Templates[templateIdx]) {
						sraiContent << *piece;
					}
					
					// Save current Stars
					std::array<std::string, 8> savedStars = Stars;
					
					// Process SRAI recursively
					std::string result = InputText(sraiContent.str());
					
					// Restore original Stars
					Stars = savedStars;
					
					return result;
				}
				
				// Store the current response as the new "that" context
				std::stringstream responseStream;
				for (auto& piece : bestMatch->Templates[templateIdx]) {
					responseStream << *piece;
				}
				
				That = responseStream.str();
				return That;
			}
			
			// Default response if no match found
			That = "I'm not sure how to respond to that.";
			return That;
		}
	};
}

int main()
{
	AIML::Bot MyBot;

	std::cout << "AIML Bot initialized. Type 'quit' or 'exit' to end." << std::endl;
	bool ExitProgram = false;
	std::string InputText;

	while (!ExitProgram) {
		std::cout << "USER: ";
		std::getline(std::cin, InputText);

		if (InputText == "quit" || InputText == "exit") {
			ExitProgram = true;
		}
		else {
			std::string Output = MyBot.InputText(InputText);
			std::cout << "BOT: " << Output << std::endl;
		}
	}
	std::system("PAUSE");
}