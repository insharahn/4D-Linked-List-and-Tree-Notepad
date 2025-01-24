#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>
using namespace std;

void gotoxy(int x, int y)
{
	COORD c = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

//set text color
void setColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

//custom stack
template <typename T>
class StackNode
{
public:
	T data;
	StackNode* next;

	StackNode(T value) : data(value), next(nullptr) {}
};

template <typename T>
class Stack
{
private:
	StackNode<T>* topNode;
	int currentSize;

public:
	Stack() : topNode(nullptr), currentSize(0) {}

	void push(const T& value)
	{
		StackNode<T>* newNode = new StackNode<T>(value);
		newNode->next = topNode;
		topNode = newNode;
		currentSize++;
	}

	void pop()
	{
		if (!empty())
		{
			StackNode<T>* temp = topNode;
			topNode = topNode->next;
			delete temp;
			currentSize--;
		}
	}

	T& top()
	{
		if (!empty())
			return topNode->data;
	}

	bool empty() const
	{
		return topNode == nullptr;
	}

	int size() const
	{
		return currentSize;
	}

	~Stack()
	{
		while (!empty())
			pop();
	}
};


struct Node
{
	char character;
	Node* left, * right, * up, * down;
	int posX, posY; //position relative to the screen
	int color = 7; //white (default color) is 7
	Node()
	{
		character = '\0';
		left = nullptr;
		right = nullptr;
		up = nullptr;
		down = nullptr;
	}
	Node(char c)
	{
		character = c;
		left = nullptr;
		right = nullptr;
		up = nullptr;
		down = nullptr;
	}
	Node(char c, int x, int y)
	{
		character = c;
		left = nullptr;
		right = nullptr;
		up = nullptr;
		down = nullptr;
		posX = x;
		posY = y;
	}
	void displayCoordinates()
	{
		std::cout << "(" << posX << "," << posY << ")" << endl;
	}
};

struct Action
{
	Node* firstChar;  //store first character of node
	int x, y;         //node coordinates
	bool isInsertion; //insertion or deletion
};


class Notepad
{
private:
	Node* head; //start of list
	Node* latestChar; //the latest inputted character (not the last character in the entire list)
	Node* currentWordFirstChar; //for undo and redo implementation
	int numLines;
	int consoleWidth;
	int prefixLength; //for insertion
	bool color;
	Stack<Action> undoStack;
	Stack<Action> redoStack;
public:
	Notepad()
	{
		head = nullptr;
		latestChar = head;
		numLines = 0;
		prefixLength = 0;
		consoleWidth = 96; //leftover width after drawing search area
		color = false; //dont color a word until needed
	}

	//tree related functions
	char* getPrefix()
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		int cursorX = csbi.dwCursorPosition.X;
		int cursorY = csbi.dwCursorPosition.Y;

		//find the node behind the cursor position
		Node* current = head;
		while (current->right && (current->right->posX != cursorX || current->right->posY != cursorY))
		{
			current = current->right;
		}

		//find the start of the word
		Node* start = current;
		int prefixLength = 0;
		while (start && start->left && start->left->character != ' ')
		{
			start = start->left;
			prefixLength++;
		}

		char* prefix = new char[prefixLength + 2]; // +1 for null terminator, +1 for last char
		int i = 0;

		//add into the array from the start position to current
		Node* temp = start;
		while (temp != current)
		{
			prefix[i++] = temp->character;
			temp = temp->right;
		}
		prefix[i++] = temp->character;
		prefix[i] = '\0';

		this->prefixLength = i; //set length for insertion purposes

		return prefix;
	}

	void insertSuggestion(const char* suggestion, int& x, int& y)
	{
		cout << suggestion[prefixLength];
		for (int i = prefixLength; suggestion[i] != '\0'; i++) //start insertion after the index of the prefix to avoid duplicate characters
		{
			insertInList(suggestion[i], x, y);
		}
	}

	void highlightSearch(char* search, int& x, int& y)
	{
		int length = 0;
		while (search[length] != '\0')
			++length;

		Node* current = head;
		while (current)
		{
			Node* lineStart = current;
			while (current)
			{
				current->color = 7;  // reset to default
				current = current->right;
			}
			current = lineStart->down;
		}

		//seearch
		current = head;
		while (current)
		{
			Node* lineStart = current;
			while (current)
			{
				Node* tempNode = current;
				Node* start = current; // start of match
				int index = 0;

				//check for substring match starting at current node
				while (tempNode && search[index] != '\0')
				{
					if (tempNode->character == search[index] || //normal case
						(tempNode->character >= 'A' && tempNode->character <= 'Z') && tempNode->character + 32 == search[index]) //uuppercase cjaracter
					{
						tempNode = tempNode->right;
						index++;
					}
					else
						break;
				}

				if (index == length) //all characters matched = substring found
				{
					// change color of all nodes in substring
					tempNode = start;
					for (int i = 0; i < length; i++)
					{
						tempNode->color = 5; //purple
						tempNode = tempNode->right;
					}
					current = start->right;  //move current forward to check entire list
				}
				else //not the same
					current = current->right;
			}

			current = lineStart->down;
		}

		//display text
		displayNoClear(x, y);

		//reset color
		current = head;
		while (current)
		{
			Node* lineStart = current;
			while (current)
			{
				current->color = 7;  // reset to default
				current = current->right;
			}
			current = lineStart->down;
		}
	}

	void insertInList(char character, int& x, int& y) //special case for suggestions
	{
		Node* newChar = new Node(character, ++x, y);

		latestChar->right = newChar;
		newChar->left = latestChar;
		latestChar = newChar;
		gotoxy(x, y);
	}


	//notepad
	void drawScreen(int x, int y)
	{
		gotoxy(x, y);
		int totalWidth = 120; //total width
		int totalHeight = 30;  //total height
		int textAreaHeight = totalHeight * 0.6; //60% for the main text area, 18 lines
		int suggestionsAreaHeight = totalHeight * 0.2; //20% for suggestions area
		int searchAreaHeight = totalHeight * 0.2; //20% for the search area
		int searchAreaWidth = 0.2 * 120; //20%

		//search area 
		gotoxy(totalWidth - searchAreaWidth, 0);
		std::cout << "| Search" << endl;
		for (int i = 0; i < textAreaHeight; i++)
		{
			for (int j = 0; j < totalWidth - searchAreaWidth; j++)
				std::cout << " ";
			std::cout << "|" << endl;
		}

		//horizonral line
		gotoxy(0, textAreaHeight + 1);
		for (int i = 0; i <= totalWidth - searchAreaWidth; i++)
			std::cout << "-";

		//suggestions area
		gotoxy(0, textAreaHeight + 2);
		std::cout << "Word Suggestions " << endl;

	}


	void display(int x, int y)
	{
		//clear screen
		system("cls");

		//draw background
		drawScreen(0, 0);

		Node* current = head;
		while (current)
		{
			Node* lineStart = current;
			while (current)
			{
				setColor(current->color); //set the color of the node

				gotoxy(current->posX, current->posY); //move the cursor to the node's position
				std::cout << current->character;
				current = current->right;
			}

			//move to next line
			current = lineStart->down;
		}

		setColor(7); //reset to default color for each call

		gotoxy(++x, y);

	}

	void displayNoClear(int x, int y)
	{
		//DONT clear screen :)

		//draw background
		drawScreen(0, 0);

		Node* current = head;
		while (current)
		{
			Node* lineStart = current;
			while (current)
			{
				setColor(current->color); //set the color of the node

				gotoxy(current->posX, current->posY); //move the cursor to the node's position
				std::cout << current->character;
				current = current->right;
			}

			//move to next line
			current = lineStart->down;
		}

		setColor(7); //reset to default color for each call

		gotoxy(++x, y);

	}


	Node* findAbove(int x, int y) //find the character above (y - 1 is passed)
	{
		Node* current = head;
		while (current)
		{
			if (current->posY == y) //correct row
			{
				Node* temp = current;
				while (temp)
				{
					if (temp->posX == x)
					{
						return temp; //found above
					}
					temp = temp->right;
				}
			}
			current = current->down; //next row
		}

		return nullptr; //return nullptr if no character is found above
	}

	Node* findBelow(int x, int y) //find the node below
	{
		Node* current = head;
		while (current)
		{
			if (current->posY == y) //correct row
			{
				Node* temp = current;
				while (temp)
				{
					if (temp->posX == x)
					{
						return temp; //found below
					}
					temp = temp->right;
				}
			}
			current = current->down; //next row
		}

		return nullptr; //return nullptr if no character is found above

	}

	void checkWordAndShift(Node* newChar, int& x, int& y)
	{
		//end of the current line
		if (newChar->posX == consoleWidth - 1)
		{
			numLines++;

			//new line
			y++;
			x = 0;

			Node* temp = newChar;

			//part of a word or just one character
			if (temp->left && temp->left->character != ' ')
			{
				//check if whole word
				while (temp->left && temp->left->character != ' ')
				{
					temp = temp->left;
				}
			}

			//move the entire word to the new line
			while (temp)
			{
				temp->posX = x;
				temp->posY = y;

				////link up down pointers for the new line
				Node* above = findAbove(x, y - 1);
				if (above)
				{
					above->down = temp;
					temp->up = above;
				}

				temp = temp->right;
				x++;
			}
			x--; //overshot the cursor for the very last character so bring it back
		}
	}

	void insert(char character, int& x, int& y)
	{

		numLines = y;
		if (numLines > 18) //reached height limit
			return;

		// get the current cursor position
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		int cursorX = csbi.dwCursorPosition.X;
		int cursorY = csbi.dwCursorPosition.Y;

		Node* newChar = new Node(character, cursorX, cursorY);


		//list is empty = first character being inserted
		if (!head)
		{
			head = newChar;
			latestChar = newChar;
			gotoxy(x, numLines);
			return;
		}

		//find the node at the current cursor position
		Node* temp = head;
		while (temp && (temp->posX != cursorX || temp->posY != cursorY))
			temp = temp->right;

		//if we found a node at the cursor's position, insert the new character before this node
		if (temp)
		{
			if (numLines == 0) //first line
			{
				//link the character to character at the cursor and the character behind the character at the cursor
				newChar->right = temp;
				newChar->left = temp->left;

				//character before the current character, link it to the new character
				if (temp->left)
				{
					temp->left->right = newChar;
				}
				else
				{
					//no character to the left, newChar becomes the head
					head = newChar;
				}

				// Now link the current character back to the new character
				temp->left = newChar;

				//move all of the characters forward starting from the character right after the newest one
				while (temp)
				{
					temp->posX += 1;
					if (temp->posX == consoleWidth - 1)
						checkWordAndShift(temp, x, y);
					temp = temp->right;
				}
			}
			else //not first line
			{
				//link the character to character at the cursor and the character behind the character at the cursor
				newChar->right = temp;
				newChar->left = temp->left;

				//character before the current character, link it to the new character
				if (temp->left)
				{
					temp->left->right = newChar;
				}
				else
				{
					//no character to the left, newChar becomes the head
					head = newChar;
				}
				//link the current character back to the new character
				temp->left = newChar;

				//link up and down for new character
				Node* aboveNew = findAbove(newChar->posX, newChar->posY - 1);
				if (aboveNew)
				{
					aboveNew->down = newChar;
					newChar->up = aboveNew;
				}

				//move all of the characters forward starting from the character right after the newest one and update their up and down pointers
				while (temp)
				{
					temp->posX += 1;
					if (temp->posX == consoleWidth - 1)
						checkWordAndShift(temp, x, y);

					//link up and down pointers for the rest
					Node* above = findAbove(temp->posX, temp->posY - 1);
					if (above)
					{
						above->down = temp;
						temp->up = above;
					}
					temp = temp->right;
				}
			}

		}
		else //at end of list
		{

			if (!latestChar) //after backspace
			{
				latestChar = head;
			}

			if (numLines == 0) //first line
			{
				if (latestChar->posX == cursorX) //when insertion happens after inserting in between characters
				{
					latestChar->right = newChar;
					newChar->left = latestChar;
					latestChar = newChar;
				}
				else //find last node
				{
					//go to last node
					if (latestChar)
					{
						if (latestChar->posX != cursorX && latestChar->posY != cursorY)
						{
							Node* find = head;
							//find the node
							bool found = false;
							while (find && !found)
							{
								Node* lineStart = find;
								while (find)
								{
									if (find->posX == cursorX - 1 && find->posY == cursorY)
									{
										found = true;
										break;
									}
									find = find->right;
								}
								if (!found)
									find = lineStart->down;
							}
							latestChar = find;
						}

						while (latestChar && latestChar->right)
						{
							latestChar = latestChar->right;
						}
						//insert after
						latestChar->right = newChar;
						newChar->left = latestChar;
						latestChar = newChar;
					}
				}
			}
			else //not first line
			{
				if (latestChar->posX == cursorX) //when insertion happens after inserting in between characters
				{
					//link left right
					latestChar->right = newChar;
					newChar->left = latestChar;
					latestChar = newChar;

					//link up down
					//find the character in the place above the latest character
					Node* characterAbove = findAbove(latestChar->posX, latestChar->posY - 1);
					if (characterAbove)
					{
						newChar->up = characterAbove;
						characterAbove->down = newChar;
					}
				}
				else //find last node
				{
					//go to last node
					if (latestChar)
					{
						if (x > 0 && (latestChar->posX != x && latestChar->posY != y)) //find the actual last character
						{
							Node* find = head;
							//find the node
							bool found = false;
							while (find && !found)
							{
								Node* lineStart = find;
								while (find)
								{
									if (find->posX == cursorX - 1 && find->posY == cursorY)
									{
										found = true;
										break;
									}
									find = find->right;
								}
								if (!found)
									find = lineStart->down;
							}
							latestChar = find;
						}

						while (latestChar->right)
						{
							latestChar = latestChar->right;
						}
						//insert after
						latestChar->right = newChar;
						newChar->left = latestChar;
						latestChar = newChar;

						//link up down
						//find the character in the place above the latest character
						Node* characterAbove = findAbove(latestChar->posX, latestChar->posY - 1);
						if (characterAbove)
						{
							newChar->up = characterAbove;
							characterAbove->down = newChar;
						}
					}
				}
			}

		}

		//null terminate from opposite end
		if (newChar->posX == 0) //first character in new line
			newChar->left = nullptr;

		//character at end of screen
		if (newChar->posX == consoleWidth - 1)
			checkWordAndShift(newChar, x, y); //shifts within the function
		else
			gotoxy(++x, numLines);

		if (newChar->posX == 0) //start of a new word
		{
			currentWordFirstChar = newChar;

			if (undoStack.size() >= 5) //max 5
			{
				Stack<Action> tempStack;
				while (undoStack.size() > 1) //move everything but last
				{
					tempStack.push(undoStack.top());
					undoStack.pop();
				}

				undoStack.pop(); //pop last

				while (!tempStack.empty()) //move back
				{
					undoStack.push(tempStack.top());
					tempStack.pop();
				}
			}

			Action newAction = { currentWordFirstChar, currentWordFirstChar->posX, currentWordFirstChar->posY, true };
			undoStack.push(newAction);

		}
		else if (newChar->left)
		{
			if (newChar->left->character == ' ')
			{
				currentWordFirstChar = newChar;

				//only 5 actions
				if (undoStack.size() >= 5)
				{
					Stack<Action> tempStack;

					// move all but last into temp 
					while (undoStack.size() > 1)
					{
						tempStack.push(undoStack.top());
						undoStack.pop();
					}

					//pop last action
					undoStack.pop();

					//move into original
					while (!tempStack.empty())
					{
						undoStack.push(tempStack.top());
						tempStack.pop();
					}
				}

				Action newAction = { currentWordFirstChar, currentWordFirstChar->posX, currentWordFirstChar->posY, true };
				undoStack.push(newAction);
			}
		}

	}

	void undo(int& x, int& y)
	{
		// get the current cursor position
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		int cursorX = csbi.dwCursorPosition.X;
		int cursorY = csbi.dwCursorPosition.Y;

		if (!undoStack.empty())
		{
			Action lastAction = undoStack.top();
			undoStack.pop();

			if (lastAction.isInsertion)
			{
				Node* current = lastAction.firstChar;
				Node* prev = current->left; // node before the word
				Node* next = nullptr;

				int newX, newY;

				// save position of character before 
				if (prev)
				{
					newX = prev->posX;
					newY = prev->posY;
				}
				else //enter/first
				{
					newX = -1;
					newY = current->posY;
				}

				Node* toDelete = current; //traverse to end of word so we can delete backwards
				while (toDelete->right && toDelete->character != ' ')
				{
					toDelete = toDelete->right;
				}

				if (current->posX > 0) //not first word in line
				{
					//save the point where we will reconnect it
					Node* reconnect = toDelete->right;

					//delete characters backwards until encountering the character behind the first character of the word we r deleting
					while (toDelete->left != prev)
					{
						Node* here = toDelete;
						toDelete = toDelete->left;
						toDelete->right = nullptr;
						delete here;
					}
					prev->right = toDelete->right;
					latestChar = prev;

					x = newX;
					y = newY;
					gotoxy(x, y);
					display(x, y);
					return;
				}
				else
				{
					//delete characters backwards until encountering the character behind the first character of the word we r deleting
					while (toDelete->left != prev)
					{
						Node* here = toDelete;
						toDelete = toDelete->left;
						toDelete->right = nullptr;
						delete here;
					}

					//weve deleted the last node so we need to insert a new one
					Node* emptyNode = new Node('\n', 0, ++y);
					toDelete->left = emptyNode;

					//link above
					Node* above = findAbove(toDelete->posX, toDelete->posY - 1);
					if (above)
					{
						above->down = emptyNode;
						emptyNode->up = above;
					}
					//null terminate the previous line
					while (above && above->right != toDelete)
						above = above->right;
					if (above)
						above->right = nullptr;
					x = newX;
					y = newY;
					gotoxy(x, y);
					display(x, y);
					return;
				}
			}

			else
			{
				//reinsert deleted word
			}

			//push the undone action to the redo stack
			if (redoStack.size() >= 5)
				redoStack.pop();
			redoStack.push(lastAction);
		}
	}

	void backspace(int& x, int& y)
	{
		numLines = y;
		// get the current cursor position
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		int cursorX = csbi.dwCursorPosition.X;
		int cursorY = csbi.dwCursorPosition.Y;

		//empty list
		if (!head)
			return;

		Node* temp = head;
		//find the node
		bool found = false;
		while (temp && !found)
		{
			Node* lineStart = temp;
			while (temp)
			{
				if (temp->posX == cursorX - 1 && temp->posY == cursorY)
				{
					found = true;
					break;
				}
				temp = temp->right;
			}
			if (!found)
				temp = lineStart->down;
		}

		//cursor is at the start of the document and head is null
		if (!temp && x <= 1)
			return;

		// if there's no character at the current cursor position move the cursor back to the previous character
		if (!temp)
		{
			x--;
			gotoxy(x, y);
			return;
		}
		//if we delete the first character
		if (temp == head)
		{
			head = head->right;
			if (head)
				head->left = nullptr;
			delete temp;
			temp = nullptr;
			return;
		}
		else if (temp->posX == 0 && (!temp->right || temp->right->posX != 1)) //first and last character in a line
		{
			Node* above = temp->up;
			if (above->character == '\n') //empty line above
			{
				x = above->up->posX;
				y = above->up->posY;
				gotoxy(x, y);
				return;
			}

			//find the last character of the line above
			if (above)
			{
				while (above->right != temp)
				{
					above = above->right;
				}

				//relink
				if (temp->right)
				{
					above->right = temp->right;
					temp->right->left = above;
				}
				else
				{
					above->right = nullptr; //current node only one in its line
					if (!temp->down) //no subsequent lines
					{
						x = above->posX;
						y = above->posY;
						temp->left = nullptr;
						if (temp->left)
							temp->left->right = nullptr;
						if (temp->up)
							temp->up->down = nullptr;
						delete temp;
						gotoxy(x, y);
						return;
					}
				}

				//shift all the subsequent lines up
				Node* shift = temp->right;
				while (shift)
				{
					shift->posY -= 1;
					Node* relinkAbove = findAbove(shift->posX, shift->posY - 1);
					if (relinkAbove)
					{
						relinkAbove->down = shift;
						shift->up = relinkAbove;
					}
					shift = shift->right;
				}

				//relink down linking
				Node* current = above->right;
				while (current)
				{
					Node* below = findBelow(current->posX, current->posY + 1);
					if (below)
					{
						current->down = below;
						below->up = current;
					}
					current = current->right;
				}

				//delete the temp node
				delete temp;
				temp = nullptr;
				x = above->posX + 1;
				y = above->posY;
				gotoxy(x, y);
			}

			return;
		}
		//node found
		else if (temp)
		{

			//check if we are deleting between lines before anything else
			Node* betweenLineCheck = temp;
			bool betweenLines = false;
			while (betweenLineCheck)
			{
				if (betweenLineCheck->down)
				{
					betweenLines = true;
					break;
				}
				betweenLineCheck = betweenLineCheck->left; //traverse backwards to check if theres a single character that is sandwiched between two lines
			}

			if (betweenLines) //delete the nodes within that line only and add an empty node
			{

				Node* toDelete = temp;
				Node* leftOfDeleted = temp->left;

				if (!temp->right || temp->right->posX == 0) //last character in a line
				{
					if (leftOfDeleted)
						leftOfDeleted->right = temp->right;
					delete toDelete;

					//relink nodes
					Node* above = findAbove(leftOfDeleted->posX, leftOfDeleted->posY - 1);
					if (above)
					{
						above->down = leftOfDeleted;
						leftOfDeleted->up = above;
					}
					Node* below = findBelow(leftOfDeleted->posX, leftOfDeleted->posY + 1);
					if (below)
					{
						below->up = leftOfDeleted;
						leftOfDeleted->down = below;
					}
					x = leftOfDeleted->posX;
					y = leftOfDeleted->posY;
					gotoxy(x, y);
					return;
				}
				else
				{
					Node* current = temp;
					Node* lastCharacterInLine = current;
					while (lastCharacterInLine->right && lastCharacterInLine->right->posX > 0)
						lastCharacterInLine = lastCharacterInLine->right;
					//shift until last
					Node* toShift = current;
					while (toShift != lastCharacterInLine)
					{
						toShift->character = toShift->right->character;
						toShift = toShift->right;
					}
					//unlink dupliocate at end
					if (toShift)
					{
						if (toShift->left)
							toShift->left->right = toShift->right;
						toShift->left = nullptr;
						toShift->right = nullptr;
						delete toShift;
					}
					--x;
					gotoxy(x, y);
					return;
				}
			}

			if (temp->posX != 0) //not first character in the line
			{
				//shift backwards the node backwards line by line
				Node* current = temp;
				while (current->right) //not last character in line
				{
					current->character = current->right->character;
					current = current->right;
				}
				//current = last node, must be deleted
				if (current->left)
				{
					current->left->right = nullptr;
					if (current->left)
					{
						latestChar = current->left; //update latestChar to the left character
					}
					else
						latestChar = nullptr;

					delete current;
					current = nullptr;
				}
				x--;
				gotoxy(x, y);

				return;
			}
			else if (temp->posX == 0) //first character in a line
			{
				Node* above = temp->up;
				//find the last character in the previous line
				int num = 0;
				if (above)
				{
					if (above->right)
					{
						while (above->right->right)
							above = above->right;
						//delete character and move cursor up
						above->right = nullptr;
						temp->up->down = nullptr;
						delete temp;
						temp = nullptr;
					}
				}
				x = above->posX;
				y = above->posY;
				return;

			}
		}
	}

	void enter(int& x, int& y)
	{
		if (y > 18 || numLines > 18)
			return;

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		int cursorX = csbi.dwCursorPosition.X;
		int cursorY = csbi.dwCursorPosition.Y;


		Node* splitPoint = nullptr;

		Node* temp = head;
		//find the node
		bool found = false;
		while (temp && !found)
		{
			Node* lineStart = temp;
			while (temp)
			{
				if (temp->posX == cursorX && temp->posY == cursorY)
				{
					found = true;
					break;
				}
				temp = temp->right;
			}
			if (!found)
				temp = lineStart->down;
		}

		//enter starts from behind the cursor
		if (temp)
			temp = temp->left;

		//no text after the cursor, move to the next line (create an empty line)
		if (!temp || !temp->right)
		{

			Node* lastInLine = head;
			bool foundLast = false;
			//find node
			while (lastInLine && !foundLast)
			{
				Node* lineStart = lastInLine;
				while (lastInLine)
				{
					if (lastInLine->posX == cursorX - 1 && lastInLine->posY == cursorY)
					{
						foundLast = true;
						break;
					}
					lastInLine = lastInLine->right;
				}
				if (!foundLast)
					lastInLine = lineStart->down;
			}

			//null terminate the previous line
			if (lastInLine)
			{
				lastInLine->right = nullptr;
			}

			//represent the empty line
			Node* emptyNode = new Node('\n', 0, ++y);

			// link
			if (temp)
			{
				temp->down = emptyNode;
				emptyNode->up = temp;
			}

			Node* above = findAbove(emptyNode->posX, emptyNode->posY - 1);
			if (above)
			{
				emptyNode->up = above;
				above->down = emptyNode;
			}

			//if there's a line below
			Node* below = findBelow(0, emptyNode->posY + 1);
			if (below)
			{
				emptyNode->down = below;
				below->up = emptyNode;
			}

			//empty new line, set it as the current line for future insertions
			if (!head)
			{
				head = emptyNode;
			}

			//null terminate the line
			emptyNode->left = nullptr;

			//move the cursor to the start of the new line
			x = 0;
			gotoxy(--x, y);
			return;
		}

		splitPoint = temp->right; // first node to move to the new line

		//splitting behind the last character, move to a new empty line
		if (!splitPoint)
		{
			//null terminate before moving
			if (temp)
			{
				temp->right = nullptr;
			}

			y++;
			x = 0;
			gotoxy(0, y);
			return;
		}


		//null terminate previous line
		if (splitPoint->left)
			splitPoint->left->right = nullptr;

		//update the positions of the remaining nodes
		Node* current = splitPoint;
		x = 0;
		y++;

		while (current)
		{
			current->posX = x;
			current->posY = y;

			//up and down links
			if (current->up)
			{
				current->up->down = nullptr; //break the old up-down link
				current->up = nullptr;
			}

			Node* above = findAbove(current->posX, current->posY - 1);
			if (above)
			{
				current->up = above;
				above->down = current;
			}

			Node* below = findBelow(current->posX, current->posY + 1);
			if (below)
			{
				current->down = below;
				below->up = current;
			}

			current = current->right;
			x++;
		}

		numLines = y;

		//make sure first character's left is null
		if (splitPoint)
		{
			splitPoint->left = nullptr;
		}

		//move the cursor to the start of the newly created line
		x = -1;
		y = numLines;
	}

	~Notepad()
	{
		Node* current = head;
		Node* next = nullptr;
		if (current)
			next = current->right;
		while (current)
		{
			if (current)
				next = current->right;
			delete current;
			current = next;
		}
	}

	void moveCursorUD(int& x, int& y, int upOrDown)
	{
		Node* current = head;
		bool found = false, goingRight = false, goingLeft = false;

		//find the node
		while (current && !found)
		{
			Node* lineStart = current;
			while (current)
			{
				if (current->posX == x && current->posY == y)
				{
					found = true;
					break;
				}
				current = current->right;
			}
			if (!found)
				current = lineStart->down;
		}

		if (found)
		{
			switch (upOrDown)
			{
			case 1: //1 for up
				if (current->up)
				{
					current = current->up;
					y--;
				}
				break;

			case 2: //2 for down
				if (current->down)
				{
					current = current->down;
					y++;
				}
				break;
			default:
				if (current)
					gotoxy(current->posX, current->posY);
				return;
			}

			// move cursor to the new node's position
			gotoxy(current->posX, current->posY);
		}
	}

	void moveCursorRight(int& x, int& y) //move all the way to the end, it isnt, skipping the very lasr chacreter
	{
		Node* current = head;
		bool found = false;

		if (!head)
			return;

		//find the node
		while (current && !found)
		{
			Node* lineStart = current;
			while (current)
			{
				if (current->posX == x && current->posY == y)
				{
					found = true;
					break;
				}
				current = current->right;
			}
			if (!found)
				current = lineStart->down;
		}

		if (current)
		{
			if (current->right)
			{
				current = current->right; //move right first
				x++;
				gotoxy(current->posX + 1, current->posY);
				return;
			}
			else //current's right is null i.e. at the end of the line before it is shifted down
			{
				//first character in this line
				Node* original = current;
				while (current->left)
					current = current->left;
				//move down
				if (current->down)
				{
					current = current->down;
					x = 0;
					y++;
				}
				else //there is no other line
				{
					x = original->posX;
					y = original->posY;
					gotoxy(x, y);
					return;
				}
				x++;
				gotoxy(current->posX + 1, current->posY);
				return;
			}
		}
		else //current is null: called for enter, not shifted
		{
			if (x != -1) //not middle enter
			{
				y++;
				x = 0;
				//check for the character behind the cursor instead of in front of it
				current = head;
				found = false;
				while (current && !found)
				{
					Node* lineStart = current;
					while (current->right)
					{
						if (current->posX == x && current->posY == y)
						{
							found = true;
							break;
						}
						if (!found)
							current = current->right;
					}
					if (!found)
						current = lineStart->down;
				}
				x++;
				if (current)
				{
					gotoxy(current->posX + 1, current->posY);
				}
				else
					gotoxy(x, y);
			}
			else //enter before word
			{
				gotoxy(++x, y);
			}
		}

	}

	void moveCursorLeft(int& x, int& y)
	{
		Node* current = head;
		bool found = false;

		if (x == 0 && y == 0) //at beginning
		{
			gotoxy(0, 0);
			return;
		}
		else if (x == 0 && y > 0) //first charcter in line, logic is different
		{
			while (current && !found)
			{
				Node* lineStart = current;
				while (current)
				{
					if (current->posX == x && current->posY == y)
					{
						found = true;
						break;
					}
					if (!found)
						current = current->right;
				}
				if (!found)
					current = lineStart->down;
			}
			if (current->up)
			{
				Node* above = current->up;
				while (above->right)
					above = above->right;
				x = above->posX;
				--y;
				gotoxy(x, y);
				return;
			}
			return;
		}

		//find the node
		while (current && !found)
		{
			Node* lineStart = current;
			while (current)
			{
				if (current->posX == x - 1 && current->posY == y)
				{
					found = true;
					break;
				}
				if (!found)
					current = current->right;
			}
			if (!found)
				current = lineStart->down;
		}
		if (current && current->left) //move left if there is a character to the left
		{
			current = current->left;
			gotoxy(--x, y);
			return;
		}
		else if (current && !current->left && current->up) //line exists above after pressing enter
		{
			Node* temp = current->up;
			//go to the end of the previous line
			while (temp && temp->right != current)
			{
				temp = temp->right;
			}
			if (temp)
			{
				x = temp->posX;
				y = temp->posY;
				--x; //to be able to traverse empty lines
				gotoxy(temp->posX + 1, temp->posY);
				return;
			}
			else
			{
				gotoxy(0, --y);
				return;
			}
		}
	}

};



