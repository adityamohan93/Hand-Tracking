# Hand Tracking and Gesture Recognition

An application based on digital image processing to track hands and locate the fingers and palm based on input taken from a live video feed from webcam. Based on the number of fingers pointed, we can instruct the system to perform functions like initiation of programs or system calls, etc.  
Language used: C++  
Library: OpenCV

##Instructions to run program  
To setup environment on an Ubuntu machine:  
sudo apt-get install libopencv-dev  
pkg-config --modversion opencv

To compile the file:  
g++ -o opencv main.cpp `pkg-config --cflags --libs opencv` -lX11 -lXtst -lXext

Run the program:  
./opencv

If number of fingers detected is 0 , it means the user is showing a first Operations.  
Based on the number of fingers detected, various commands are being execute using Linux system call commands.  
On detecting 1 finger, VLC media player is being opened.  
On detecting 2 or 3 fingers, Mozilla Firefox is being opened.  
On detecting 4 fingers, Gnome Terminal is being opened.  
The ctions can be modified in code.
