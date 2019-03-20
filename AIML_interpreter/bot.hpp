#pragma once

#include "graphmaster.hpp"

namespace AIML
{
	using std::string;
	class Bot
	{
		GraphMaster* _GM;
		std::string Topic;	// Current topic
		std::string That;	// Current context
		std::unordered_map<std::string, std::string> Variables;	// Current stored variables
		std::array<std::string*, 8> Stars; // Current <star/> inputs
	public:

		void DebugStars1() {
			*Stars[0] = "Quick"; *Stars[1] = "Brown"; *Stars[2] = "Fox"; *Stars[3] = "Jumps";
			*Stars[4] = "Over"; *Stars[5] = "The"; *Stars[6] = "Lazy";	*Stars[7] = "Dog";
		}
		void DebugStars2() {
			*Stars[0] = "Pack"; *Stars[1] = "My";	*Stars[2] = "Box"; *Stars[3] = "With";
			*Stars[4] = "Five"; *Stars[5] = "Dozen"; *Stars[6] = "Big"; *Stars[7] = "Jugs";
		}
		void PrintStars() {
			for (auto Star : Stars) {
				std::cout << *Star << std::endl;
			}
		}
		
		//	Default debugging values for the Stars array.
		Bot(GraphMaster* GM) : _GM(GM), Stars({new string("*1*"),new string("*2*"),new string("*3*"),new string("*4*"),new string("*5*"),new string("*6*"),new string("*7*"),new string("*8*")}) {}

		~Bot() {
			for (auto Obj : Stars) {
				delete Obj;
			}
		}

		std::string InputText(std::string StrInput) {
			auto Result = _GM->MatchPattern(StrInput, GetVars(), GetStars());
			if (Result != nullptr) {
				Result->SetPointers(GetVars(), GetStars());
				return *Result;
			}
			return "No Response Found..";
		}

		std::unordered_map<std::string, std::string> * GetVars() { return &Variables; }
		std::array<std::string*, 8> * GetStars() { return &Stars; }

	};
}