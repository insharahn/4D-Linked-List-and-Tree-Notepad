# 4D-Linked-List-and-Tree-Notepad
A console-based notepad/text editor where each character is a node in both a 4D linked list and a tree.

# Decription
This can be used as a Notepad application as it allows the user to create new files or load into old ones. However, what you see on your screen are not characters in a string, but individual nodes in a linked list, linked forwards (right), backwards (left), upwards, and downwards. The notepad is the 4d linked list itself.

At the same time, a tree similar to a Trie Tree is made based on what characters are entered, allowing for searching and word completion. It grows with the linked list, spanning the length of the entered content.

# Explanation of Data Structures
## Linked List
The linked list is created as more characters are entered. Typing a new character creates a new node and it is displayed via traversal of the list. The nodes that represent the first character in a line are not linked leftwards to the last character in the previous line, but point to null. If the sentence in the previous line is longer, the characters that exceed the length of the line below are not linked downwards to anything to ensure consistency. Similarly, the last node in a line is not linked 
to anything rightwards. While typing, if a word exceeds the set length of the Notepad's screen, it shifts to the next line, forming upward links with the nodes on the line above it, updating the now previous-previous line's downward links. As the user continues to update the Notepad, the links in the linked list continue to change.

Every line can be considered a miniature linked list itself with its own head, but the Notepad only has one head to control the traversal. It is made entirely without a single array.

## Tree
The tree in this Notepad is one similar to a Trie Tree, called an NAry Tree because I was unfamiliar with the concept of Trie Trees when I made this. The root of the tree is empty and every first letter in a word is a child of said root--a node can have a maximum of 26 children, each child coressponding to an alphabet (case insenstive.) The second character entered in a word is then added as a child of the first character and so on and so forth. The frequency of each character is dependent on how many times it appears in a specific position and the height of the tree depends on how many unique characters are entered in unique positions within a word.

The tree is used for searching in O(log n) and word completion. It is independent of the linked list, only interacting with it to color found words after a search. It grows with the linked list, spanning the length of the entered content.

# Features and their Keyboard Shortcuts
- **alt**: Display the tree's structure node by node
- **end**: Display the tree's structure word by word
- **tab**: Search for a word or phrase (highlights the word/phrase in the notepad and outputs the position)
- **@**: Typing @ after an unfinished word will suggest words to complete based on the prefix (For example, if you type "hello world" and then "h@", you will have the option to complete with "hello")
- **caps lock**: Undo
- **crtl**: Save the file and exit
- Enter and Backspace will work as they do in any text editor


# Happy writing :)
