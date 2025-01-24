#pragma once
#include "Notepad.h"

struct TreeNode
{
	char data;
	TreeNode* children[26] = { nullptr };  //  26 lowercase characters as the nodes
	bool finished = false;
	int frequency = 0;
	int posX, posY;
	int coordinates[100][2]; //store all the coordinates
	int numCoordinates;

	TreeNode() : finished(false), frequency(0), numCoordinates(0) {}

	TreeNode(char d)
	{
		finished = false;
		for (int i = 0; i < 26; i++)
			children[i] = nullptr;
		frequency = 0;
		numCoordinates = 0;
		posX = 0;
		posY = 0;
		data = d;
	}

	TreeNode(char d, int x, int y)
	{
		finished = false;
		for (int i = 0; i < 26; i++)
			children[i] = nullptr;
		frequency = 0;
		numCoordinates = 0;
		posX = x;
		posY = y;
		data = d;
	}

	void addCoordinates(int x, int y) //helper function to add coordinates to the array
	{
		if (numCoordinates < 100)
		{
			coordinates[numCoordinates][0] = x;
			coordinates[numCoordinates++][1] = y;
		}
	}

	void displayCoordinatesArray(int cursorX, int& cursorY)
	{
		for (int i = 0; i < numCoordinates; i++)
		{
			gotoxy(cursorX, cursorY);
			cout << "(" << coordinates[i][0] << ", " << coordinates[i][1] << ")" << endl;
			++cursorY;
		}
	}

	void displayCoordinates()
	{
		cout << "(" << posX << "," << posY << ")" << endl;
	}

	void displayYCoordinates(int currentX, int& currentY)
	{
		for (int i = 0; i < numCoordinates; i++)
		{
			gotoxy(currentX, currentY);
			cout << coordinates[i][1] << endl;
			++currentY;
		}
	}
};

class NAryTree
{
public:
	TreeNode* root;
	TreeNode* lastInserted;
	NAryTree()
	{
		root = new TreeNode; //create empty head
		lastInserted = root;
	}

	char lowercase(char& c) //convert character to lowercase
	{
		if (c >= 'A' && c <= 'Z')
			c += 32;
		return c;
	}

	void insertInTree(char character, int& x, int& y)
	{
		if (character == ' ')
		{
			if (lastInserted != root) //end of the word
			{
				lastInserted->finished = true;
			}
			lastInserted = root; //reset for next word
			return;
		}

		//convert character to lowercase
		lowercase(character);

		if (y > 18) //reached height limit
			return;

		// get the current cursor position
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		int cursorX = csbi.dwCursorPosition.X;
		int cursorY = csbi.dwCursorPosition.Y;

		//	TreeNode* node = new TreeNode(character, cursorX, cursorY);

		int index = character - 'a';

		if (lastInserted == root) //start of a word
		{
			if (!root->children[index]) //current character first of its kind
			{
				TreeNode* node = new TreeNode(character);
				node->addCoordinates(cursorX, cursorY);
				root->children[index] = node;
				lastInserted = node;
			}
			else
			{
				root->children[index]->frequency++;
				root->children[index]->addCoordinates(cursorX, cursorY);
				lastInserted = root->children[index];
			}
		}
		else
		{
			if (!lastInserted->children[index]) //current character first of its kind
			{
				TreeNode* node = new TreeNode(character);
				node->addCoordinates(cursorX, cursorY);
				lastInserted->children[index] = node;
				lastInserted = node;
			}
			else
			{
				lastInserted->children[index]->frequency++;
				lastInserted->children[index]->addCoordinates(cursorX, cursorY);
				lastInserted = lastInserted->children[index];
			}
		}

	}

	void displayTreeHierarchy(TreeNode* node, int level)
	{
		if (!node)
			return;

		for (int i = 0; i < level; i++)  //add space based on the level of the tree
			cout << " ";

		cout << node->data << " (Freq: " << node->frequency << ")";
		if (node->finished)
			cout << " [complete]" << endl; //mark a finished word
		else
			cout << "" << endl;

		for (int i = 0; i < 26; ++i)
		{
			if (node->children[i]) //traverse children
			{
				displayTreeHierarchy(node->children[i], level + 1);
			}
		}
	}

	void displayTreeStructure()
	{
		cout << "\nTree:" << std::endl;
		displayTreeHierarchy(root, 0);  // start from level 0
	}

	void displayTreeMeow(TreeNode* node, char* word, int level) //word by word
	{
		if (!node)
			return;

		//add current node to the array
		word[level] = node->data;


		if (node->finished) //if word finished, output
		{
			word[level + 1] = '\0';
			cout << word << " (Frequency: " << node->frequency << ")" << endl;
		}

		for (int i = 0; i < 26; i++) //traverse children
		{
			if (node->children[i])
				displayTreeMeow(node->children[i], word, level + 1); //increment level num
		}
	}

	void displayTree(int& x, int& y)
	{
		gotoxy(0, 10);
		cout << "\nTree:" << endl;
		char word[100];
		displayTreeMeow(root, word, 0);  //display level by level
		gotoxy(x, y);
	}

	void search(Notepad& notepad, int& x, int& y)
	{
		//boundaries
		int startingX = 97, startingY = 1, maxWidth = 23;
		int maxY = 18;
		gotoxy(startingX, startingY);

		char msg[] = "You have pressed TAB.\n\nEnter what you want to search:";
		//output according to boundaries
		int currentX = 0, currentY = startingY;
		for (int i = 0; msg[i] != '\0'; i++)
		{
			if (currentX < maxWidth)
			{
				cout << msg[i];
				++currentX;
			}
			else
			{
				currentX = 0;
				if (currentY < maxY)
					gotoxy(startingX, ++currentY);
				else
					break; //more than 18 lines
				cout << msg[i];
				++currentX;
			}
		}
		cout << endl;


		//	char* toSearch = new char[1000];
		char toSearch[100]{ '\0' }; //changed to static because cin is giving an error?????
		if (currentY < maxY)
			gotoxy(startingX, ++currentY);

		cin >> toSearch;

		searchInTree(toSearch, currentY, maxY);

		notepad.highlightSearch(toSearch, x, y);
	}

	void searchInTree(char* word, int& currentY, int maxY)
	{
		int length = 0;
		while (word[length] != '\0')
			++length;

		char currentWord[1000];

		// search from each child node in the tree
		for (int i = 0; i < 26; i++)
		{
			if (root->children[i])
				searchOccurrences(root->children[i], word, length, currentY, maxY, currentWord, 0);
		}
	}

	void searchOccurrences(TreeNode* node, char* word, int length, int& currentY, int maxY, char* currentWord, int currentWordLength)
	{

		currentWord[currentWordLength] = node->data; //current node added to current word array
		currentWordLength++;

		//check if current word contains the searched word
		if (currentWordLength >= length)
		{
			bool isMatch = true;
			for (int j = 0; j < length; ++j)
			{
				if (currentWord[currentWordLength - length + j] != word[j]) //mismatch at any point = not present
				{
					isMatch = false;
					break;
				}
			}

			char foundMsg[] = { "found at position(s): " };
			if (isMatch && currentY < maxY)
			{
				gotoxy(97, currentY);
				cout << word << endl;
				gotoxy(97, ++currentY);
				int i = 0;
				int currentX = 0;
				while (foundMsg[i] != '\0')
				{
					if (currentX < 120) //within width range
						cout << foundMsg[i];
					else
					{
						gotoxy(95, ++currentY);
						cout << foundMsg[i];
					}
					currentX++;
					i++;
				}
				gotoxy(97, ++currentY);
				node->displayCoordinatesArray(97, currentY);
				if (currentY < maxY)
				{
					gotoxy(97, currentY);
					cout << "Line number(s): ";
					node->displayYCoordinates(97, ++currentY);
					++currentY;
				}
			}

		}

		//check currents children for more instgances
		for (int i = 0; i < 26; i++)
		{
			if (node->children[i])
			{
				searchOccurrences(node->children[i], word, length, currentY, maxY, currentWord, currentWordLength);
			}
		}

		//remove last character
		currentWordLength--;
	}

	void getSuggestions(TreeNode* node, char* prefix, int prefixLength, char** suggestions, int& suggestionCount)
	{
		if (!node || suggestionCount >= 50) //limit to 50 suggestions for funsies
			return;

		if (node->finished) //completed word
		{
			suggestions[suggestionCount] = new char[prefixLength + 1];
			for (int i = 0; i < prefixLength; ++i)
			{
				suggestions[suggestionCount][i] = prefix[i]; //copy the prefix in
			}
			suggestions[suggestionCount][prefixLength] = '\0';
			suggestionCount++;
		}

		//do the same for all children
		for (int i = 0; i < 26; ++i)
		{
			if (node->children[i])
			{
				//add current character to the prefix
				prefix[prefixLength] = node->children[i]->data;

				getSuggestions(node->children[i], prefix, prefixLength + 1, suggestions, suggestionCount);

				prefix[prefixLength] = '\0';
			}
		}
	}

	char** findSuggestions(const char* prefix, int& suggestionCount)
	{
		TreeNode* node = root;
		int prefixLength = 0;

		//go to the end of the prefix in the tree
		for (int i = 0; prefix[i] != '\0'; i++)
		{
			int index = prefix[i] - 'a';
			if (!node->children[index])
				return nullptr;
			node = node->children[index];
			prefixLength++;
		}

		char** suggestions = new char* [50];
		suggestionCount = 0;

		char prefixArray[50];
		for (int i = 0; i < prefixLength; ++i) //copy prefixes for tracking
		{
			prefixArray[i] = prefix[i];
		}
		prefixArray[prefixLength] = '\0';

		//call recursively
		getSuggestions(node, prefixArray, prefixLength, suggestions, suggestionCount);
		return suggestions;
	}

	void displaySuggestions(char** suggestions, int suggestionCount)
	{
		gotoxy(0, 21);
		for (int i = 0; i < suggestionCount; ++i)
			cout << (i + 1) << ": " << suggestions[i] << "   ";
		cout << "\nChoose one to complete your word: ";
	}

	void insertSuggestion(const char* suggestion, int& x, int& y)
	{
		for (int i = 1; suggestion[i] != '\0'; i++)
		{
			insertInTree(suggestion[i], x, y);
		}
	}

};