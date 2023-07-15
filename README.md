If you want to build an application, you have to download boost library from https://www.boost.org/ (Project used 1.82.0 version) and locate it in ```boost_1_82_0``` folder in root directory. 
Another libary that you have to download is SFML (Project used 2.6.0) and locate it in ```sfml``` folder in root directory.

Project has been set up for Visual Studio (Used VS 17 2022 with C++ 17)

Application is using (your CPU number of threads + 1) threads.


Keybinds:
Click and hold left mouse button to select the area of interest.
Click right mouse button cancel selection.

Click middle mouse button stop generation.

Click XButton1 (backwards on mouse) to select the previous area of interest.

(Buttons down below you can only use when the image is done generating)

Click XButton2 (forward on mouse) to toggle between More/Less details.  (I wrote the generating algorithm to in way to only calculate half the pixels and fill the space inbetween them if they have the same color with respective color, then regenerate the missing pixels if the color is different)

Click keyboard numbers(1-6) to switch between different max iterations:
    1 - 100
    2 - 250
    3 - 500
    4 - 1000
    5 - 2000
    6 - 10000