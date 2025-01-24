# 4D-Linked-List-and-Tree-Notepad
A console-based notepad/text editor where each character is a node in both a 4D linked list and a Trie Tree.

# Decription
This can be used as a Notepad application as it allows the user to create new files or load into old ones. However, what you see on your screen are not characters in a string, but individual nodes in a linked list, linked forwards, backwards, upwards, and downwards. Typing a new character creates a new node and it is displayed via traversal of the list. The Notepad itself is thus a 4-dimensional linked list. It is made entirely without a single array.

At the same time, a tree similar to a Trie Tree is made based on what characters are entered, allowing for searching and word completion. It grows with the linked list, spanning the length of the entered content.

# Features and their Keyboard Shortcuts
- **alt**: Display the tree's structure node by node
- **end**: Display the tree's structure word by word
- **tab**: Search for a word or phrase (highlights the word/phrase in the notepad and outputs the position)
- **@**: Typing @ after an unfinished word will suggest words to complete based on the prefix (For example, if you type "hello world" and then "h@", you will have the option to complete with "hello")
- **caps lock**: Undo
- **crtl**: Save the file and exit
- Enter and Backspace will work as they do in any text editor


# Happy writing :)
