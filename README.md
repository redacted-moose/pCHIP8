         _           _             _       _     _          _          _          
        /\ \       /\ \           / /\    / /\  /\ \       /\ \      / /\         
       /  \ \     /  \ \         / / /   / / /  \ \ \     /  \ \    / /  \        
      / /\ \ \   / /\ \ \       / /_/   / / /   /\ \_\   / /\ \ \  / / /\ \       
     / / /\ \_\ / / /\ \ \     / /\ \__/ / /   / /\/_/  / / /\ \_\/_/ /\ \ \      
    / / /_/ / // / /  \ \_\   / /\ \___\/ /   / / /    / / /_/ / /\ \ \_\ \ \     
   / / /__\/ // / /    \/_/  / / /\/___/ /   / / /    / / /__\/ /  \ \/__\ \ \    
  / / /_____// / /          / / /   / / /   / / /    / / /_____/    \_____\ \ \   
 / / /      / / /________  / / /   / / /___/ / /__  / / /            \ \ \_\ \ \  
/ / /      / / /_________\/ / /   / / //\__\/_/___\/ / /              \ \___\ \ \ 
\/_/       \/____________/\/_/    \/_/ \/_________/\/_/                \_______\/ 


		     *A CHIP8 interpreter for the Casio PRIZM*
				 By Chris Sadler (AKA redacted-moose)

====================================================================================
====================================================================================
What is this?

	Exactly what it says it is - pCHIP8 is a CHIP8 interpreter for the Casio
	PRIZM (fxCG-10/20) line of graphing calculators.  The CHIP8 was a "virtual"
	machine designed in the 70's to ease the development of games for limited
	computer systems.  It has very simple hardware and a very simple instruction
	set, making it fairly easy to emulate.
	
	Contained in this folder:
	- C Source code (chip8.c, chip8.h, cpu.c, cpu.h, Makefile)
	- This file, of course :P

====================================================================================
====================================================================================
How to use

	To build, a simple 'make' will do (provided you have the corrected toolchain installed)
	
	Then just send pCHIP8.g3a and whichever rom.c8 you choose to the root directory
	of your PRIZM.

	To start, click on or select pCHIP8 from the add-in selection screen.

	Keys are mapped as such:

	Original keypad:		PRIZM keys:
	+---+---+---+---+		+---+---+---+---+
	| 1 | 2 | 3 | C |		| 7 | 8 | 9 |del|
	+---+---+---+---+		+---+---+---+---+
	| 4 | 5 | 6 | D |		| 4 | 5 | 6 | X |
	+---+---+---+---+		+---+---+---+---+
	| 7 | 8 | 9 | E |		| 1 | 2 | 3 | + |
	+---+---+---+---+		+---+---+---+---+
	| A | 0 | B | F |		| 0 | . |exp| - | 
	+---+---+---+---+		+---+---+---+---+

====================================================================================
====================================================================================
Known issues

	Many games are still buggy at this time, if not unplayable, but everything
	"works."  There are a few bugs left that I have yet to work out, such as a
	few display issues and whatnot.
	
====================================================================================
====================================================================================
Future plans

	- Add SCHIP instruction set and beyond
	
	- Fix incompatibilities
	
	- Make it more user-friendly ;)

====================================================================================
====================================================================================
Questions/comments/snide remarks/etc.
	
	I can usually be found on www.omnimaga.org as Mighty Moose.  Feel free to pm
	me, shout at me on irc, or post in the pCHIP8 topic.  I can also be reached
	at mightymoosemn@gmail.com.

====================================================================================
====================================================================================
Special thanks
	
	- PierrotLL for creating the isKeyPressed() routine I use

	- The omnimaga community
	
	- Anyone else I forgot :P

====================================================================================
====================================================================================
Version history

v0.2a.12.5.18 - 5/18/12
*initial release
