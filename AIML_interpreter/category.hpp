#pragma once

#include <random>

namespace AIML
{
	enum TemplateWordType : unsigned char {
		TWT_Word = 0,
		TWT_Wildcard = 1,
		TWT_SetClear = 3,
		TWT_SetWord = 4,
		TWT_SetWildcard = 5,
		TWT_Get = 6
	};

	class TemplateWord {
		const TemplateWordType _Type;								//	Defines how this word acts
		std::array<std::string*, 8>** _Stars;						//	<star/> list
		std::unordered_map<std::string, std::string>** _Variables;	//	<set/get> list
		const bool _Thinking;										//	Should this word output
		const unsigned int _StarIndex;								//	star list index
		const char* _Word;											//	word to print
		std::string _VariableIndex;							//	variable list index
	public:

		//	Normal Word
		TemplateWord(std::unordered_map<std::string, std::string>** Variables, std::array<std::string*, 8>** Stars, const bool Thinking, const char* Word)
			: _Variables(Variables), _Stars(Stars), _Thinking(Thinking), _Type(TWT_Word), _StarIndex(0), _Word(Word), _VariableIndex() {}
		//	Normal Wildcard
		TemplateWord(std::unordered_map<std::string, std::string>** Variables, std::array<std::string*, 8>** Stars, const bool Thinking, const unsigned int StarIndex)
			: _Variables(Variables), _Stars(Stars), _Thinking(Thinking), _Type(TWT_Wildcard), _StarIndex(StarIndex), _Word(nullptr), _VariableIndex() {}
		//	<set> Clear
		TemplateWord(std::unordered_map<std::string, std::string>** Variables, std::string VariableIndex)
			: _Variables(Variables), _Stars(nullptr), _Thinking(false), _Type(TWT_SetClear), _StarIndex(0), _Word(nullptr), _VariableIndex(VariableIndex) {}
		//	<set> Word
		TemplateWord(std::unordered_map<std::string, std::string>** Variables, std::array<std::string*, 8>** Stars, const bool Thinking, const char* Word, std::string VariableIndex)
			: _Variables(Variables), _Stars(Stars), _Thinking(Thinking), _Type(TWT_SetWord), _StarIndex(0), _Word(Word), _VariableIndex(VariableIndex) {}
		//	<set> Wildcard
		TemplateWord(std::unordered_map<std::string, std::string>** Variables, std::array<std::string*, 8>** Stars, const bool Thinking, const unsigned int StarIndex, std::string VariableIndex)
			: _Variables(Variables), _Stars(Stars), _Thinking(Thinking), _Type(TWT_SetWildcard), _StarIndex(StarIndex), _Word(nullptr), _VariableIndex(VariableIndex) {}
		//	<get>
		TemplateWord(std::unordered_map<std::string, std::string>** Variables, std::array<std::string*, 8>** Stars, const bool Thinking, std::string VariableIndex)
			: _Variables(Variables), _Stars(Stars), _Thinking(Thinking), _Type(TWT_Get), _StarIndex(0), _Word(nullptr), _VariableIndex(VariableIndex) {}

		auto GetWord()
		{
			switch (_Type)
			{
			case TWT_Word: {
				return _Word;
			}
			break;

			case TWT_Wildcard: {
				return (**_Stars)[_StarIndex]->c_str();
			}
			break;

			case TWT_SetClear: {
				(**_Variables)[_VariableIndex] = "";
				return "";
			}
			break;

			case TWT_SetWord: {
				(**_Variables)[_VariableIndex] += _Word;
				if (_Thinking) {
					return "";
				}
				else {
					return (**_Variables)[_VariableIndex].c_str();
				}
			}
			break;

			case TWT_SetWildcard: {
				(**_Variables)[_VariableIndex] += *(**_Stars)[_StarIndex];
				if (_Thinking) {
					return "";
				}
				else {
					return (**_Variables)[_VariableIndex].c_str();
				}
			}
			break;

			case TWT_Get: {
				return (**_Variables)[_VariableIndex].c_str();
			}
			break;

			default: return "No Value";
			}
		}

		operator const char*() {
			return GetWord();
		}

		friend std::ostream& operator<< (std::ostream& out, TemplateWord& TWord) {
			return out << TWord.GetWord();
		}
	};

	class Category
	{
		//std::default_random_engine RNG;							// Random numbers
		//std::uniform_int_distribution<int> TNum;					// Returns a random template index
		std::unordered_map<std::string, std::string>** _Variables;	// Current stored variables
		std::array<std::string*, 8>** _Stars;						// Current <star/> inputs
		bool Srai;													// Rematch using this pattern
		std::vector<std::list<TemplateWord>> Templates;				// Response templates
		//	unordered_map 'SetList' string/string - Values to be set during template call

		//	Recursively walk through a TEMPLATE constructing response templates for an AIML::Category
		void WalkTemplate(const rapidxml::xml_node<>* node, bool Thinking = false, char* Setting = nullptr) {
			switch (node->type()) {
			case rapidxml::node_element:
			{
				//	<random>
				if (strcmp(node->name(), "random") == 0) {}
				//	<li>
				else if (strcmp(node->name(), "li") == 0) {
					//	Create a new Response List at the back of Templates
					Templates.emplace_back(std::list<TemplateWord>());
				}
				//	<think>
				else if (strcmp(node->name(), "think") == 0) {
					//	Set children of this node to 'think' status
					Thinking = true;
				}
				//	<srai>
				else if (strcmp(node->name(), "srai") == 0) {
					//	Enable this category to use symbolic reduction (srai)
					SetSRAI();
				}
				//	<star/>
				else if (strcmp(node->name(), "star") == 0) {
					//	Insert reference to appropriate <star/> value
					auto StarIndex = node->first_attribute("index");
					const unsigned int Index = (!StarIndex ? 0 : atoi(StarIndex->value())-1);
					if (Setting == nullptr) {
						Templates.back().push_back(TemplateWord(_Variables, _Stars, Thinking, Index));
					}
					else {
						Templates.back().push_back(TemplateWord(_Variables, _Stars, Thinking, Index, Setting));
					}
				}
				//	<get/>
				else if (strcmp(node->name(), "get") == 0) {
					//	Insert reference to appropriate <get name='...'/> value
					auto VariableName = node->first_attribute("name");
					Templates.back().push_back(TemplateWord(_Variables, _Stars, Thinking, std::string(VariableName->value())));
				}
				//	<set>
				else if (strcmp(node->name(), "set") == 0) {
					//	Set children of this node to append their value onto a variable
					auto VariableName = node->first_attribute("name");
					Setting = VariableName->value();
					Templates.back().push_back(TemplateWord(_Variables, Setting));
				}
				//	Unknown tag
				else {
					printf("Unknown Tag <%s> %s\n", node->name(), node->value());
					for (auto a = node->first_attribute(); a; a = a->next_attribute()) {
						printf("\t%s\n\t%s\n", a->name(), a->value());
					}
				}
				//	Walk through all children of this node
				for (auto n = node->first_node(); n; n = n->next_sibling()) {
					WalkTemplate(n, Thinking, Setting);
				}
			}
			break;

			case rapidxml::node_data:
			{
				if (Setting == nullptr) {
					if (!Thinking) {
						if (Templates.empty()) {
							Templates.emplace_back(std::list<TemplateWord>());
						}
						Templates.back().push_back(TemplateWord(_Variables, _Stars, Thinking, node->value()));
					}
				}
				else {
					Templates.back().push_back(TemplateWord(_Variables, _Stars, Thinking, node->value(), Setting));
				}
			}
			break;

			default: printf("Unknown Node Type\n"); break;
			}
		}

		void SetSRAI() { Srai = true; }

	public:

		//	Capitalize every letter in the input patter and tokenize it into individual words.
		Category(const rapidxml::xml_node<>* node, std::unordered_map<std::string, std::string>** DefaultVariables, std::array<std::string*, 8>** DefaultStars)
			: _Variables(DefaultVariables), _Stars(DefaultStars), Srai(false)/*, RNG()*/ {
			for (auto Node = node->first_node(); Node; Node = Node->next_sibling()) {
				WalkTemplate(Node);
			}
			//TNum = std::uniform_int_distribution<int>(1, (int)Templates.size());
		}

		//	So we can visually debug the state of our memory
		void PrintData(std::unordered_map<std::string, std::string> * Variables, std::array<std::string*, 8> * Stars) {
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

		void SetPointers(std::unordered_map<std::string, std::string>* Variables, std::array<std::string*, 8>* Stars) {
			*_Variables = Variables;
			*_Stars = Stars;
		}

		operator std::string() const
		{
			const unsigned int TemplateCount = Templates.size();
			const unsigned int TemplateNum = 1 + rand() % TemplateCount;

			std::string Output;
			for (auto Str : Templates[TemplateNum - 1]) {
				Output += Str;
			}
			return Output;
		}

		friend std::ostream& operator<< (std::ostream& out, const Category& TCat)
		{
			const unsigned int TemplateCount = TCat.Templates.size();
			const unsigned int TemplateNum = 1 + rand() % TemplateCount;

			for (auto Str : TCat.Templates[TemplateNum - 1]) {
				out << Str << " ";
			}
			return out;
		}
	};
}