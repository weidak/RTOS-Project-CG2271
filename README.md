# CG2271

## Project Specifications:

### Set up main.c:
- [x] InitPWMMotors
- [x] InitPWMBuzzer
- [x] InitUART
- [x] InitLED (on/off states)
- [x] Set up Remote/Self-Driving state

### Motor Controls:
1. The robot must be able to move in all FOUR directions, Forward, Left, Right and Back.
    - [x] Move Forward 
    - [x] Turn Left (Stationary Turns)
    - [x] Turn Right (Stationary Turns)
    - [x] Reverse 
2. The robot must be able to perform curved turns while moving.
    - [x] Turn left while moving
    - [x] Turn right while moving
3. The robot must stop all movement if no command is being sent.
    - [x] No auto move forwards

### LED Control:
1. The front 8-10 Green LED’s must be in a Running Mode (1 LED at a time) whenever the robot is moving (in any direction).
    - [x] Moving state (front)
2. The front 8-10 Green LED’s must all be lighted up continuously whenever the robot is stationery.
    - [x] Stop state (front)
3. The rear 8-10 Red LED’s must be flashing continuously at a rate of 500ms ON, 500ms OFF, while the robot is moving (in any direction).
    - [x] Moving state (rear)
4. The rear 8-10 Red LED’s must be flashing continuously at a rate of 250ms ON, 250ms OFF, while the robot is stationery.
    - [x] Stop state (rear)

### Audio Control:
1. The robot must continuously play a Song tune from the start of the challenge run till the end.* There should not be any break in the song even if the robot is not moving.
    - [x] Any movement starts the song
2. When the robot completes the challenge run, the robot must play a unique tone to end the timing.
    - [x] Button to start ending song

### Self-Driving Ability:
1. A “Start” button on the App must activate the robot to perform self-driving
    - [x] Bot enters state to 
2. The robot must be able to go straight for at least 60cm and STOP once an obstacle is detected. Once the robot has stopped, it can remain in that STOP position or continue with any other action.
    - [x] Configure ultrasonic sensor to detect distance
    - [x] Configure: 45 degree left turn, forwards, 90 degree right turn (x3), forwards, 45 degree left turn to complete a rhombus shape
3. The LED’s and Audio are required to fulfil the requirements specified in Part C and D.
    - [ ] Connect up the codes between the 2 states
