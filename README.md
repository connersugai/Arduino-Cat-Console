# Arduino-Cat-Console
This toy project was started to both automate the training and entertainment of my cat as well as to more generally test cats' intelligence in visual pattern discrimination and memorization. As a long term goal, it is my hope that this project represents the beginnings of a rigorous experimental investigation that determines once and for all whether the domestic cat can learn to play tic-tac-toe, also known as juego del gato...

Disclaimer: This is my first forray into using Git (not to mention C++) and my code is far from clearly organized and optomized so all feedback, critiques, and contributions are more than welcome. 

## Game Stages

 ### Stage 1 \[Complete]
Purpose: Get the cat used to operating the machine.

Description: A simple 'X' is shown, near constantly, and each successful press while the 'X' is illuminated is rewarded with a treat. (I say ‘near constantly’ because the LED matrix is cleared temporarily during motor operation/when extraneous inputs are blocked after a successful button press).

Rationale: For this to go anywhere the cat must first learn that a pushing of the button, specifically when a symbol is displayed, is rewarded. I chose to start with the 'X' because I thought perhaps it was the slightly more distinguishable starting point.

### Stage 2 \[Complete]
Purpose: Help the cat learn to wait for the LED display.

Description: The same simple ‘X’ is shown for a set time (e.g. three seconds max without a response) before dimming for a set period (e.g. five seconds). Only successful clicks made while the ‘X’ is displayed are rewarded.

Rationale: The cat must also learn that the button isn’t just a wand for summoning snacks and is in actuality associated with the LED matrix display, specifically this repeated symbol ‘X.’

### Stage 3.A [Benched]
Purpose: Introduce random timing earlier to the learning process.

Description: The same basic conditions as Stage 2, however the wait time in between displays is now a random variable that is normally distributed (Two parameters, maybe the mean is kept at the previous wait time (e.g. 5 seconds) and the standard deviation is played with to generate desired variability. 

Rationale: This should be an easy transition from the previous stage and strengthens the association between successful button presses and the LED display independent of timing. 

### Stage 3.B \[Complete]
Purpose: Introduce symbol discrimination earlier to the learning process.

Description: The same display/wait times as Stage 2  (e.g. 3 seconds/5 seconds),  but there is a random (50-50) chance that either an ‘X’ or an ‘O’ is displayed. Only button presses when the ‘X’ is shown will be rewarded.

Rationale: This demonstrates that not all LED displays are equal and this first challenge in symbol discrimination is perhaps aided by the previously learned timing regularity (assuming that the cat picked up much on the regularity of wait times in the first place, likely a negligible influence). 

### Stage 4
Purpose: Train discrimination between two symbols shown at random, with random wait times between display events. 

Description: At random either an ‘X’ or ‘O’ is shown for the previous set times with random wait times in between. Only button presses when the ‘X’ is shown will be rewarded. 

Rationale: This the natural progression from either option A or B for Stage 3, where both display unpredictability and discrimination are now at play. Whether Stage 3.A or 3.B is a better intermediate isn’t immediately clear and depends on whether we think introducing random wait times or symbol discrimination earlier (and in isolation) would be a more helpful learning progression.

### Ideas for Additional Stages
*  All at once, or one at a time, transition the 'X' and 'O' to the slightly more complex AND more co-similar 'smiley' and 'fontX' already in the code.
*  Test adaptive learning by switching out the more complex ‘smiley’ or ‘fontX’ with a novel symbol that differs from both (e.g. the 'beer' symbol in the code that is square, both filled and hollow, and is off center). 
*  Test fine grained discrimination: normal  ‘X’ vs. fontX, or normal ‘O’ vs. smiley.
*  Test response to reverting one symbol to a simpler/past symbol that was also compared against the current antagonist symbol.
*  Test switching/cue blocking: flip what symbol is rewarded completely (previously ‘X’, now ‘O’). It would be interesting to see how quickly/with how much effort this would work. Perhaps the multiple, simpler stages leading up to the reward ‘flip’ will have to be similarly flipped around to build up to it. This seems like it could be an insightful measure of intelligent identification/discrimination, but there is a tension between rewarding an 'x-like' or 'o-like' symbol for a long time such that an ingrained association can be used as a solid foundation for more complex tasks, and switching the reward early during simpler tasks when it may be easier to transition completely, if possible at all. Likely this can't be determined until one sees how fast a cat clears levels and their aptitude for distinguishing symbols. Several months of rewarding 'X' for example would seem to make the transition to rewarding 'O' extremely difficult, but the cats behavior may have been reliably shaped in such a way that it will consistently select 'X' in much more complex arrangements (e.g. the holy grail: tic tac toe).
*  Test discrimination between more than two symbols.
*  Reward selection of a specific sequences of numbers (so you can pretend your cat can do sums for guests, or something of the like).
*  Test reaction time: (slowly shorten the display time and therefore the window of time in which a successful click can happen, maybe increase treat pay out if personal records are smashed). This would be particularly interesting if paired with symbol discrimination. 
*  AND PROBABLY A LOT MORE (e.g. you get an RGB LED matrix and can train color recognition, keeping in mind cats have much fewer cones and see in blue-violet/yellow-green ranges).

## The Basic Physical Build
All components used were available in Elegoo’s “The Most Complete Starter Kit for MEGA2560” with the exception of the I2C/IIC serial interface adapter/backpack for the 1602 LCD screen. It should be noted that an Arduino UNO has enough digital output pins for this build as well, I was just using what I had at the moment. 
*	**Solderless Bread Board** - Used mainly for the secondary power supply as well as for organizing other connections and adding stuff.
*	**Push Button** - Real classic. Put in the top of a small card board box with a lever/pressure pad to make it sensitive to the slightest paw pressure. Pro tip: cut a hole in the top of the small cardboard box (I used a small box from a pair of earrings) just below the button. The box itself will help to project the noise of the button being pressed which creates two clear clicks with different tones. This mimics the role of a clicker used in hand training by serving as a clear and consistent cue for the target behavior. Timing is important but it could be no more precise in this arrangement as this ensures it sounds EXACTLY as the cat correctly taps the pad. 
*	**28BYJ-48 Stepper Motor** - Used to dispense treats by rotating a wheel (cardboard) with 16 spokes. 15 treats sit in the empty spaces between the ‘spokes’ with the final open segment serving as the starting point that is initially positioned over the hole that the treats will fall through after each sixteenth of a revolution/correct button press. I mounted it on top of the metal bottom of a Pringles can to both keep it away from prying paws as well as to give the dispenser a chute to output the treats in front of the box that housed everything. The reasoning for the small load count is that in general 10-20 treats (at least of the kind I used) a day is recommended for a single adult cat. Additionally, I also wanted to monitor each run and in my experience many small training sessions spread out across multiple days seems to be a more effective approach when working with cats as opposed to, say, a dog who would be perfectly happy to drill the same target behaviors over and over without getting distracted or bored. 
*	**ULN2003A Stepper Motor Driver** - Used to drive the stepper motor; hooked up to a secondary power source.
*	**MB 102 Bread Board Power Supply Module** - Accepts an input voltage less than 12 volts (I used a 9V battery) and outputs 3.3V or 5.0V. 
*	**MAX7219 8x8 Matrix Display Module** - An LED matrix with a common-cathode display driver for displaying the symbols that act as a visual cue for the cat to click the button. 
*	**LCD1602A LCD Display** - Purely for aesthetics/to display data so I don’t have to hook up my computer to see the hits/misses/treats remaining/average response times etc. Also used as a stage selection screen to toggle through the different challenges/difficulties.
*	 **2004 1602 LCD Display IIC/I2C Adapter IIC Serial Interface Adapter** - Can be attached to the LCD so that the only pins you need on the Arduino for display operation is the SDA and SDL pins. It has a built in potentiometer for screen contrast  and a jumper that you can remove to turn off the backlight or swap out for different resistors to change the backlight intensity. 
*	**15ish F-M Dupont Wires** - To connect all the header pins of the various modules to the board/Arduino.
*	**M-M Dupont Wires, Resistors, LED’s, etc.** - Just other useful bits and bobs for all other connections and organization and whatnot. 

