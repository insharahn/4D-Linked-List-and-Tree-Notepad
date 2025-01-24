#pragma once
#include "Notepad.h"

//class to make the notepad files
class FileHandling
{
public:
	void createFile(const std::filesystem::path& filePath)
	{
		std::ofstream file(filePath);
		if (file)
		{
			std::cout << "File created successfully: " << filePath << endl;
			std::cout << "Continuing..." << endl;
		}
		else
			std::cout << "Error creating file: " << filePath << endl;
		file.close();
	}

	void loadFileTwo(const std::filesystem::path& filePath, Notepad& notepad, bool& fileOpened, int& x, int& y) //load existing file
	{
		std::ifstream file(filePath);
		fileOpened = true;

		if (!file)
		{
			std::cout << "This file doesn't exist, so try again." << std::endl;
			fileOpened = false;
			return;
		}

		std::cout << "Your file " << filePath << " has been loaded. You will now be taken to the notepad." << std::endl;

		x = 0;
		y = 0;
		gotoxy(x, y);

		char character;
		while (file.get(character)) // character by character
		{
			notepad.insert(character, x, y);
		}
		notepad.insert('\n', x, y);

		file.close();
		system("pause"); //press key to continue
		system("cls"); //clear screen to show notepad
	}


	void loadFile(const std::filesystem::path& filePath, bool& fileOpened) //overwrites files
	{
		std::ifstream file(filePath);
		fileOpened = true;
		if (!file)
		{
			std::cout << "This file doesn't exist, so try again." << endl;
			fileOpened = false;
			return;
		}
		std::cout << "Your file " << filePath << ".txt has been loaded. You will now be taken to the notepad." << endl;
		file.close();
		system("pause"); //press key to continue
		system("cls"); //clear screen to show notepad
	}

	void saveFile(const std::filesystem::path& filePath, Notepad& notepad)
	{
		std::ofstream file(filePath, std::ios::trunc); //overwrite whatever was in the file
		if (file)
			std::cout << "File saved successfully: " << filePath << endl;
		else
			std::cout << "Error saving file: " << filePath << endl;
		file.close();
	}

	void saveAndExit(const std::filesystem::path& filePath, Notepad& notepad, bool& Running)
	{
		saveFile(filePath, notepad); //save the current file
		Running = false; //exit the main loop
		std::cout << "Exiting the program..." << endl;
	}

};