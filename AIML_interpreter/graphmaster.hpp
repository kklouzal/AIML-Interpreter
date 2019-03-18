#pragma once

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <list>

namespace AIML
{
	using std::unordered_map;
	using std::unordered_set;
	using std::string;
	using std::queue;
	using std::list;

	//	Node Types in priority order
	enum NodeType {
		PWord = 0,	// $Word
		PZero = 1,	// #0+ Wildcard
		POne = 2,	// _1+ Wildcard
		Word = 3,	// WORD
		Zero = 4,	// ^0+ Wildcard
		One = 5,	// *1+ Wildcard
		//	Not involved in matching
		ROOT = 6,	//	Root Node
	};

	struct Node {
		unordered_map<string, Node> Branches;
		bool EndPoint;
		Node* _Parent;
		NodeType _NodeType;

		Node(Node* Parent = nullptr, NodeType NType = NodeType::ROOT) : _Parent(Parent), EndPoint(false), _NodeType(NType) {}
		
		//	Process a token, removing it from the queue before passing deeper
		void ProcessTokens(queue<string> &Tokens) {
			if (Tokens.empty()) {
				EndPoint = true;
				//	End of tokens
				//	Store <template> in this node
				std::cout << "End Branch" << std::endl;
			}
			else {
				auto Token = Tokens.front();
				NodeType NType = NodeType::Word;
				switch (Token.at(0)) {
				case '$': NType = NodeType::PWord; break;
				case '#': NType = NodeType::PZero; break;
				case '_': NType = NodeType::POne; break;
				case '^': NType = NodeType::Zero; break;
				case '*': NType = NodeType::One; break;
				default: break;
				}
				std::cout << Token << "(" << NType << ")->";
				Tokens.pop();
				Branches.emplace(Token, Node(this, NType)).first->second.ProcessTokens(Tokens);
			}
		}

		//	Traverse the graph searching for a pattern
		const bool FindTokens(list<string>::iterator Tokens, list<string>::iterator End) {
			if (Tokens == End) {
				//	#	0 or more
				auto Branch = Branches.find("#");
				if (Branch != Branches.end()) {
					std::cout << "TRAILING--> #" << std::endl;
					//	Don't increment 'Tokens' on a 0+ wildcard
					if (Branch->second.FindTokens(Tokens, End)) { return true; }
				}
				//	^ 0 or more
				Branch = Branches.find("^");
				if (Branch != Branches.end()) {
					std::cout << "TRAILING--> ^" << std::endl;
					//	Don't increment 'Tokens' on a 0+ wildcard
					if (Branch->second.FindTokens(Tokens, End)) { return true; }
				}
				if (EndPoint) {
					//	Return <template> from this node
					std::cout << "PATTERN MATCH" << std::endl;
					return true;
				}
				else {
					//	Incomplete pattern
					std::cout << "PATTERN INCOMPLETE" << std::endl;
					return false;
				}
			} else {
				//	$	Priority
				auto Branch = Branches.find("$" + *Tokens);
				if (Branch != Branches.end()) {
					std::cout << "[" << *Tokens << "]--> $" << std::endl;
					if (Branch->second.FindTokens(++Tokens, End)) {	return true; }
					else { --Tokens; }
				}
				//	#	0 or more
				Branch = Branches.find("#");
				if (Branch != Branches.end()) {
					std::cout << "[" << *Tokens << "]--> #" << std::endl;
					//	Treat 0+ wildcards as if they match 0 words by not incrementing 'Tokens'
					if (Branch->second.FindTokens(Tokens, End)) { return true; }
				}
				//	_ 	1 or more
				Branch = Branches.find("_");
				if (Branch != Branches.end()) {
					std::cout << "[" << *Tokens << "]--> _" << std::endl;
					if (Branch->second.FindTokens(++Tokens, End)) {	return true; }
					else { --Tokens; }
				}
				//	word
				Branch = Branches.find(*Tokens);
				if (Branch != Branches.end()) {
					std::cout << "[" << *Tokens << "]--> WORD" << std::endl;
					if (Branch->second.FindTokens(++Tokens, End)) {	return true; }
					else { --Tokens; }
				}
				//	^ 0 or more
				Branch = Branches.find("^");
				if (Branch != Branches.end()) {
					std::cout << "[" << *Tokens << "]--> ^" << std::endl;
					//	Treat 0+ wildcards as if they match 0 words by not incrementing 'Tokens'
					if (Branch->second.FindTokens(Tokens, End)) { return true; }
				}
				//	* 1 or more
				Branch = Branches.find("*");
				if (Branch != Branches.end()) {
					std::cout << "[" << *Tokens << "]--> *" << std::endl;
					if (Branch->second.FindTokens(++Tokens, End)) {	return true; }
					else { --Tokens; }
				}
				//	No direct node match; Rematch current node if wildcard
				if (_NodeType == NodeType::PZero) {
					//	Increment 'Tokens' and rematch this node
					std::cout << "(" << *Tokens << ") # +" << std::endl;
					if (FindTokens(++Tokens, End)) { return true; }
					else { --Tokens; }
				}
				else if (_NodeType == NodeType::POne) {
					//	Increment 'Tokens' and rematch this node
					std::cout << "(" << *Tokens << ") _ +" << std::endl;
					if (FindTokens(++Tokens, End)) { return true; }
					else { --Tokens; }
				}
				else if (_NodeType == NodeType::Zero) {
					//	Increment 'Tokens' and rematch this node
					std::cout << "(" << *Tokens << ") ^ +" << std::endl;
					if (FindTokens(++Tokens, End)) { return true; }
					else { --Tokens; }
				}
				else if (_NodeType == NodeType::One) {
					//	Increment 'Tokens' and rematch this node
					std::cout << "(" << *Tokens << ") * +" << std::endl;
					if (FindTokens(++Tokens, End)) { return true; }
					else { --Tokens; }
				}
				if (_Parent) {
					std::cout << "<--[" << *Tokens << "] NODE DEPLEATED" << std::endl;
				}
				else {
					std::cout << "[" << *Tokens << "] NO MATCH AVAILABLE" << std::endl;
				}
				return false;
			}
		}

		unsigned int BranchCount() {
			unsigned int iBranches = (!_Parent ? 0:1);
			for (auto Branch : Branches) {
				iBranches += Branch.second.BranchCount();
			}
			return iBranches;
		}

		unsigned int PatternCount() {
			unsigned int iPatterns = EndPoint;
			for (auto Branch : Branches) {
				iPatterns += Branch.second.PatternCount();
			}
			return iPatterns;
		}
	};

	class GraphMaster {
		Node RootNode;					//	Root node for all pattern branches
		unordered_set<string> Words;	//	Unique set of words across all branches

		//	Tokenize an input string
		//	Tokens are converted to UPPER CASE
		//	ToDo: Punctuation and contractions are stripped
		queue<string> TokenQueue(string Input)
		{
			queue<string> Tokens;
			for (auto& x : Input) {
				x = toupper(x);
			}
			string token;
			std::istringstream tokenStream(Input);
			while (getline(tokenStream, token, ' '))
			{
				auto tmp = (*Words.insert(token).first);
				Tokens.push(tmp);
			}
			return Tokens;
		}
		list<string> TokenList(string Input)
		{
			list<string> Tokens;
			for (auto& x : Input) {
				x = toupper(x);
			}
			string token;
			std::istringstream tokenStream(Input);
			while (getline(tokenStream, token, ' '))
			{
				auto tmp = (*Words.insert(token).first);
				Tokens.push_back(tmp);
			}
			return Tokens;
		}

	public:

		GraphMaster() : RootNode() {}

		//	Add a pattern to the graph
		void AddPattern(string Pattern)
		{
			//	Tokenize our input pattern
			auto Tokens = TokenQueue(Pattern);
			//	Pass our tokens into the RootNode
			RootNode.ProcessTokens(Tokens);
		}

		//	Try to match a pattern from the graph
		void MatchPattern(string Pattern)
		{
			//	Tokenize our input pattern
			auto Tokens = TokenList(Pattern);
			//	Pass our tokens into the RootNode
			RootNode.FindTokens(Tokens.begin(), Tokens.end());
		}

		void PrintDebug() {
			std::cout << "Unique Words:\t" << Words.size() << std::endl;
			std::cout << "Total Branches:\t" << RootNode.BranchCount() << std::endl;
			std::cout << "Total Patterns:\t" << RootNode.PatternCount() << std::endl;
		}
	};
}