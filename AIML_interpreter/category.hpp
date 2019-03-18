#pragma once

namespace AIML
{
	class Category
	{
		std::unordered_map<std::string, std::string*> *Variables;	// Current stored variables
		std::array<std::string*, 8> *Stars;							// Current <star/> inputs
		bool Srai;													// Rematch using this pattern
		std::vector<std::list<std::string*>> Templates;				// Response templates
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
					Templates.emplace_back(std::list<std::string*>());
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
					Templates.back().push_back(Stars->operator[](0));
				}
				else if (strcmp(node->name(), "get") == 0) {
					//	Insert reference to appropriate <get name='...'/> value
					auto VariableName = node->first_attribute("name");
					std::cout << "Variable Name: " << VariableName->value() << std::endl;
					Templates.back().push_back(Variables->operator[](VariableName->value()));
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
						Templates.emplace_back(std::list<std::string*>(1, new std::string(node->value())));
					}
					else {
						Templates.back().emplace_back(new std::string(node->value()));
					}
				}
			}
			break;

			default: printf("Unknown Node Type\n"); break;
			}
		}

	public:

		//	Capitalize every letter in the input patter and tokenize it into individual words.
		Category(const rapidxml::xml_node<>* node, std::unordered_map<std::string, std::string*>* _Variables, std::array<std::string*, 8>* _Stars) : Variables(_Variables), Stars(_Stars), Srai(false) {
			for (auto Node = node->first_node(); Node; Node = Node->next_sibling()) {
				WalkTemplate(Node);
			}
		}

		//	So we can visually debug the state of our memory
		void PrintData(std::unordered_map<std::string, std::string*>* _Variables, std::array<std::string*, 8>* _Stars) {
			Variables = _Variables;
			Stars = _Stars;
			std::cout << std::endl << "Category Data" << std::endl;
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
	};
}