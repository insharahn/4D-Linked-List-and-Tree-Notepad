#include "Tree.h"
#include "FileHandling.h"

int main(int argc, char* argv[])
{
	int x = 0, y = 0;

	Notepad notepad;
	NAryTree nTree;
	FileHandling fileHandling;

	std::filesystem::path currentFilePath;
	bool isFileLoaded = false;

	system("cls"); //clears console
	HANDLE  rhnd = GetStdHandle(STD_INPUT_HANDLE);  // handle to read console

	DWORD Events = 0;    
	DWORD EventsRead = 0; 

	bool Running = true;

	bool fileOpened = false;
	int choice;
	filesystem::path filePath;

	do
	{
		std::cout << "+@*~ MAIN MENU ~*@+" << endl;
		std::cout << "Before we begin, remember these keyboard shortcuts:\nalt = display tree structure\nend = display tree word by word\ntab = search or find\n@ = suggest words to complete the prefix of what you are typing\ncrtl = save file and exit\ncaps lock = undo" << endl;
		std::cout << "Pick your poison: " << endl;
		std::cout << "1. Create A New File" << endl;
		std::cout << "2. Load An Existing File" << endl;
		std::cout << "3. Save the Current File" << endl;
		std::cout << "4. Save and Exit" << endl;
		std::cout << "5. Load Into A Blank File (Recommended)" << endl;
		std::cout << "Please enter your choice: ";
		std::cin >> choice;
		switch (choice)
		{
		case 1:
			std::cout << "New file name: ";
			std::cin >> filePath;
			fileHandling.createFile(filePath);
			break;
		case 2:
			std::cout << "Existing file name: ";
			std::cin >> filePath;
			fileHandling.loadFileTwo(filePath, notepad, fileOpened, x, y);
			break;
		case 3: // Save the Current File
			if (fileOpened)
				fileHandling.saveFile(filePath, notepad);
			else
				std::cout << "A file hasn't been loaded yet." << endl;
			break;
		case 4: // Save and Exit
			if (fileOpened)
				fileHandling.saveAndExit(filePath, notepad, Running);
			Running = false; //exit program
			break;
		case 5:
			std::cout << "Existing file name: ";
			std::cin >> filePath;
			fileHandling.loadFile(filePath, fileOpened);
			break;
		default:
			std::cout << "That is not an option. Please try again." << endl;
			break;
		}
		std::cout << endl;
	} while (!fileOpened);

	//file is opened so start writing in it
	ofstream textFile(filePath, std::ios::app); //open the file to append to it

	gotoxy(x, y);

	//programs main loop
	while (Running) {

		// gets the systems current "event" count
		GetNumberOfConsoleInputEvents(rhnd, &Events);

		if (Events != 0) { // if something happened we will handle the events we want

			// create event buffer the size of how many Events
			INPUT_RECORD eventBuffer[200];

			// fills the event buffer with the events and saves count in EventsRead
			ReadConsoleInput(rhnd, eventBuffer, Events, &EventsRead);

			// loop through the event buffer using the saved count
			for (DWORD i = 0; i < EventsRead; ++i) {
				// check if event[i] is a key event && if so is a press not a release
				if (eventBuffer[i].EventType == KEY_EVENT && eventBuffer[i].Event.KeyEvent.bKeyDown) {

					// check if the key press was an arrow key
					switch (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode) {

					case VK_UP: //up
						notepad.moveCursorUD(x, y, 1); //1 for up
						break;
					case VK_DOWN: //down
						notepad.moveCursorUD(x, y, 2); //2 for down
						break;
					case VK_RIGHT: //right
						notepad.moveCursorRight(x, y);
						break;
					case VK_LEFT: //left
						notepad.moveCursorLeft(x, y);
						break;
					case VK_BACK: //backspace
						notepad.backspace(x, y);
						notepad.display(x, y);
						break;
					case VK_RETURN: //enter
						notepad.enter(x, y);
						notepad.display(x, y);
						textFile << "\n"; //write new line to file
						break;
					case VK_TAB: //search
						nTree.search(notepad, x, y);
						gotoxy(++x, y);
						break;
					case VK_CONTROL: //ctrl for saving
						std::cout << "\nSave and Exit" << endl;
						std::cout << "Notepad will save and exit if you choose to continue.\nType Y or y if you want to save and exit: ";
						char ans;
						std::cin >> ans;
						if (ans == 'y' || ans == 'Y')
							fileHandling.saveAndExit(filePath, notepad, Running);
						else
						{
							system("cls"); //clear the message
							gotoxy(x, y);
						}
						break;
					case VK_MENU: ///display the tree using alt
						gotoxy(0, 7);
						nTree.displayTreeStructure();
						gotoxy(++x, y);
						break;
					case VK_END: //end key for word by word display of tree
						nTree.displayTree(x, y);
						break;
					case VK_CAPITAL: //undo
						notepad.undo(x, y);
						break;

					default: //if the key press is not arrows i.e. alphabet input
						char character = eventBuffer->Event.KeyEvent.uChar.AsciiChar;
						if (character == '@')
						{
							cout << character; //output @ (it will be overwritten)

							//find prefix
							char* prefix = notepad.getPrefix();

							//find suggestions using tree
							int suggestionCount = 0;
							char** suggestions = nTree.findSuggestions(prefix, suggestionCount);


							if (suggestions && suggestionCount > 0)
							{
								nTree.displaySuggestions(suggestions, suggestionCount);

								char choice; //make the choice char for easy indexing using the tree
								cin >> choice;

								if (choice >= '1' && choice <= '9')
								{
									int index = choice - '1';
									if (index < suggestionCount)
									{
										nTree.insertSuggestion(suggestions[index], x, y);
										notepad.insertSuggestion(suggestions[index], x, y);
										//nTree.displayTree(); uncomment to see tree as it is made
										notepad.display(x, y);
									}
								}
							}
						}

						//only allow characters or space
						if ((character >= 65 && character <= 90) || //lowercase char
							(character >= 97 && character <= 122) || //uppercase
							(character == 32)) //space
						{
							notepad.insert(character, x, y);
							notepad.display(x, y);
							nTree.insertInTree(character, x, y);
							textFile << character; //write to file
						}
						break;
					}
				}

			} // end EventsRead loop

		}

	} // end program loop
	system("cls"); //clear the screen once the program exists

	textFile.close(); //close the file
	return 0;
}