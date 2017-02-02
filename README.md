# PacMan-on-Arduino-Mega-
Replica of Pacman Game made on Arduino Mega with the use of TFT screen and joystick

Project created by Riya and Alex Tyrell in full collaboration.

Wiring Instructions:

+5v <---> BB positive bus,
GND <---> BB negative bus,


Wiring for screen:
BB positive bus <---> LITE,
50 <---> MISO (Master In Slave Out),
52 <---> SCK (Clock),
51 <---> MOSI (Master Out Slave In),
6 <---> TFT_CS (TFT/screen Chip Select),
5 <---> CARD_CS (Card Chip Select),
7 <---> D/C (Data/Command),
8 <---> RESET,
BB positive bus <---> VCC,
BB negative bus <---> GND,


Wiring for joystick:
BB positive bus <---> VCC,
Pin A0 <---> VERT,
Pin A1 <---> HORIZ,
Pin 9 <---> SEL,
BB GND bus <---> GND,


Thought Process:


The maze was implemented using a cell array, with sections of 8x8 pixels. We filled that cell (if it had a value of 1) with a white rectangle, which created maze walls on the LED screen. The cursor (yellow Pacman) is controlled by the joystick. Every time the joystick was moved (scanned) the 4 corners of the cursors are checked to make that the cell values of the corner are 0.  


The food was implemented using another cell array (that had a value of 1). We used 4 nested for loops to check to make sure that no pixel of the cursor was overtop the food pixel. If the food pixel was covered, the score of the game was increased by 10, and the value of that cell array was reset to 0. 


The ghosts were created to move automatically, drawing the black background behind them. This was done by having a time delay, and increasing/decreasing the x or y coordinate by one. The direction was recorded by creating a variable named direction for each ghost (going up or left was one, and going down or right was 0). The pac man was created using the following logic. It drew the black background behind him when he moved. The game ended when a ghost collided with the pacman. This was done by creating a function called collision check for each of the ghosts. In the function, it is checked if each pixel of the ghost overlapped with each pixel of the pacman. If it did, the game was automatically over and the screen became black, printing the score and two options (PLAY AGAIN, or NO SEE HIGH SCORE TABLE).


The concept of “states” was used. State 0 was the state where the game is being played. State 1 is the state when game is over and “play again and See high score” options are available. Scrolling through the options was enabled in the same method at assignment 2.  As soon as the state is changed to the game over screen, the score value (if it wasn’t 0 and doesn’t already exist in the highscore array), the array is updated. If there are less than 19 scores present in the highscore array, the new score value is assigned to the highscore array at the score_index(number of the new entry). The array is sorted in descending order using selection sort. If there are 19 scores already present in the High score array, the lowest score of the previously sorted array at the last index  is replaced with the new score if that new score is greater than the lowest entry. The array is then sorted again in descending order using selection sort.Then  It is checked what the selection is at the game over screen. 


If the play again function was chosen, the function resets the variable values to as they were when the code first started (before the game was first played) The maze was redrawn and the cells of the food array were reset to their original values. The food pixels were redrawn and the ghosts began their paths again.  The pac man reappeared in its original starting spot.


If the “No, See high score table was chosen, The high score table was printed on the screen only showing the top 19 scores (and only if they were different and not 0).


The user can then press and release the joystick to go back to the “Game over” screen and have the “play again” and “see highscore” options again.

Notes and Improvements:

Our pacman game was a high-level replica of the actual pacman game. We can improve the functionality of our version of the pacman game to replicate the popular version by adding ghosts in more “random” directions and have them “follow” pacman. This can be done by storing the direction (left or right) of the pacman in x and y directions in variables and setting the ghost directions in the opposite way. We can also make the ghost directions more “random” by checking if the ghost has collided with the maze wall and then changing its direction. The maze can be made more complex as well. 
