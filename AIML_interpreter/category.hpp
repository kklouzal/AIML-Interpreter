#pragma once

namespace AIML
{
	class TemplateWord {
		const unsigned int _StarIndex;
		const char* _Word;
	public:
		std::array<std::string*, 8> ** _Stars;

		TemplateWord(std::array<std::string*, 8>** Stars, const unsigned int StarIndex) : _Stars(Stars), _StarIndex(StarIndex), _Word(nullptr) {}
		TemplateWord(std::array<std::string*, 8>** Stars, const char* Word) : _Stars(Stars), _StarIndex(0), _Word(Word) {}

		friend std::ostream& operator<< (std::ostream& out, const TemplateWord& TWord)
		{
			if (!TWord._Word) {
				out << *(**TWord._Stars)[TWord._StarIndex];
			}
			else {
				out << TWord._Word;
			}
			return out;
		}
	};

	class Category
	{
		std::unordered_map<std::string, std::string*>** _Variables;	// Current stored variables
		std::array<std::string*, 8>** _Stars;						// Current <star/> inputs
		bool Srai;													// Rematch using this pattern
		std::vector<std::list<TemplateWord>> Templates;				// Response templates
		//	unordered_map 'SetList' string/string - Values to be set during template call

		//	Recursively walk through a TEMPLATE constructing response templates for an AIML::Category
		void WalkTemplate(const rapidxml::xml_node<>* node, bool IsThinking = false, bool IsSetting = false) {
			switch (node->type()) {
			case rapidxml::node_element:
			{
				//	Encountered <random>
				if (strcmp(node->name(), "random") == 0) {}
				//	Encountered <li>
				else if (strcmp(node->name(), "li") == 0) {
					//	Create a new Response List at the back of Templates
					Templates.emplace_back(std::list<TemplateWord>());
				}
				//	Encountered <think>
				else if (strcmp(node->name(), "think") == 0) {
					//	Set children of this node to 'think' status
					IsThinking = true;
				}
				//	Encountered <srai>
				else if (strcmp(node->name(), "srai") == 0) {
					//	Enable this category to use symbolic reduction (sari)
					SetSRAI();
				}
				else if (strcmp(node->name(), "star") == 0) {
					//	Insert reference to appropriate <star/> value
					const unsigned int Index = 0;
					Templates.back().push_back(TemplateWord(_Stars, Index));
					std::cout << std::endl;
				}
				else if (strcmp(node->name(), "get") == 0) {
					//	Insert reference to appropriate <get name='...'/> value
					auto VariableName = node->first_attribute("name");
					std::cout << "Variable Name: " << VariableName->value() << std::endl;
					//Templates.back().push_back(Variables->operator[](VariableName->value()));
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
					if (Templates.empty()) {
						Templates.emplace_back(std::list<TemplateWord>());
					}
					Templates.back().push_back(TemplateWord(_Stars, node->value()));
				}
			}
			break;

			default: printf("Unknown Node Type\n"); break;
			}
		}

		void SetSRAI() { Srai = true; }

	public:

		//	Capitalize every letter in the input patter and tokenize it into individual words.
		Category(const rapidxml::xml_node<>* node, std::unordered_map<std::string, std::string*>** DefaultVariables, std::array<std::string*, 8>** DefaultStars)
			: _Variables(DefaultVariables), _Stars(DefaultStars), Srai(false) {
			for (auto Node = node->first_node(); Node; Node = Node->next_sibling()) {
				WalkTemplate(Node);
			}
		}

		//	So we can visually debug the state of our memory
		void PrintData(std::unordered_map<std::string, std::string*> * Variables, std::array<std::string*, 8> * Stars) {
			*_Variables = Variables;
			*_Stars = Stars;
			std::cout << std::endl << "Category Data" << std::endl;
			std::cout << "\tSrai: " << Srai << std::endl;
			std::cout << "\tTemplates: " << std::endl;
			unsigned int TemplateNum = 0;
			for (auto Template : Templates) {
				std::cout << "\t\t[" << TemplateNum << "]:" << std::endl;
				TemplateNum++;
				unsigned int PieceNum = 0;
				for (auto Piece : Template) {
					std::cout << "\t\t\t[" << PieceNum << "]: " << Piece << std::endl;
					PieceNum++;
				}
			}
		}
	};
}