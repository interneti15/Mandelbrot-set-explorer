#include <thread>
#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>
#include <mutex>
#include <cmath>

#include <boost/multiprecision/cpp_dec_float.hpp>

using namespace boost::multiprecision;
using namespace std;


const int WIDTH = 900;
const int HEIGHT = 900;

cpp_dec_float_50 minx = -3;
cpp_dec_float_50 maxx = 1;

cpp_dec_float_50 xstep = (maxx - minx) / WIDTH;

cpp_dec_float_50 maxy = xstep * (HEIGHT/2);

cpp_dec_float_50 ystep = xstep;

cpp_dec_float_50 miny = maxy - ( ((maxx - minx) * HEIGHT) / WIDTH);

int max_iterations = 1000;

bool is_1_alive = false;

bool another = false;

bool more_details = false;

int refre = 10;

bool to_clean = false;
bool to_join = false;

bool end_all_threads = false;

vector<cpp_dec_float_50> last_minx = { -3 };
vector< cpp_dec_float_50> last_maxx = { 1 };
vector< cpp_dec_float_50> last_xstep = { (maxx - minx) / WIDTH };

vector< cpp_dec_float_50> last_maxy = { xstep * (HEIGHT / 2) };
vector< cpp_dec_float_50> last_ystep = { xstep };
vector< cpp_dec_float_50> last_miny = { maxy - (((maxx - minx) * HEIGHT) / WIDTH) };


vector<bool> is_alive(13, false);

std::thread calculationThread1;
std::thread calculationThread2;
std::thread calculationThread3;
std::thread calculationThread4;
std::thread calculationThread5;
std::thread calculationThread6;
std::thread calculationThread7;
std::thread calculationThread8;
std::thread calculationThread9;
std::thread calculationThread10;
std::thread calculationThread11;
std::thread calculationThreadE;

std::thread StartingThread;



std::vector<int> screen(WIDTH* HEIGHT, 0);
sf::Uint8* pixels = new sf::Uint8[WIDTH * HEIGHT * 4];

std::mutex screenMutex;



bool from0(cpp_dec_float_50 x, cpp_dec_float_50 y, cpp_dec_float_50 len = 50) {
    if (sqrt((x * x) + (y * y)) >= len) {
        return true;
    }
    else {
        return false;
    }
}

void to_close(int i)
{
    //std::this_thread::sleep_for(chrono::duration<cpp_dec_float_50>(0.01));
    is_alive[i - 1] = false;
}

std::pair<cpp_dec_float_50, cpp_dec_float_50> mandel(cpp_dec_float_50 x, cpp_dec_float_50 y, cpp_dec_float_50 x0, cpp_dec_float_50 y0) {
    cpp_dec_float_50 real = (x * x) + (-y * y) + x0;
    cpp_dec_float_50 imag = (x * y * 2) + y0;
    return std::make_pair(real, imag);
}

int iteration_check(cpp_dec_float_50 lx, cpp_dec_float_50 ly) {
    cpp_dec_float_50 x0 = lx;
    cpp_dec_float_50 y0 = ly;
    int i = 0;

    while (true) {
        if (from0(lx, ly)) {
            return i;
        }
        if (i >= max_iterations) {
            return i;
        }
        std::pair<cpp_dec_float_50, cpp_dec_float_50> to_divide = mandel(lx, ly, x0, y0);
        lx = to_divide.first;
        ly = to_divide.second;
        i++;
    }
}

void calculateMandelbrot(int startRow, int endRow, int i, bool dry_run) {

    if (dry_run)
    {
        is_alive[i - 1] = false;
        return;
    }

    is_alive[i - 1] = true;
    to_clean = true;

    int last_color = 0, color = 0;
    bool first = true, change = false;
    int x = 0 ,temp_color = 0;
    cpp_dec_float_50 y_loc = 0, x_loc = 0;

    cout << "Started: " << i << endl;
    
    if (more_details)
    {
        for (int y = startRow; y < endRow and !end_all_threads; y++) {
            y_loc = maxy - (ystep * y) - (ystep/2);

            int x = 0;

            for (int x = 0; x < WIDTH; x++) {
                x_loc = minx + (xstep * x) + (xstep / 2);

                color = iteration_check(x_loc, y_loc);

                screenMutex.lock();
                screen[y * WIDTH + x] = color;
                screenMutex.unlock();
            }
        }
    }
    else
    {
        for (int y = startRow; y < endRow and !end_all_threads; y++) {
            y_loc = maxy - (ystep * y);

            x = 0;
            x_loc = minx + (xstep * x);

            color = iteration_check(x_loc, y_loc);

            screenMutex.lock();
            screen[y * WIDTH + x] = color;
            screenMutex.unlock();

            last_color = color;

            x += 2;
            while (x < WIDTH) {
                x_loc = minx + (xstep * x);

                color = iteration_check(x_loc, y_loc);

                screenMutex.lock();
                screen[y * WIDTH + x] = color;
                screenMutex.unlock();

                if (change)
                {
                    change = false;
                    x += 1;
                    color = temp_color;
                }
                else if (last_color != color)
                {
                    temp_color = color;
                    x -= 1;
                    change = true;
                    continue;
                }
                else if (last_color == color)
                {
                    screenMutex.lock();
                    screen[y * WIDTH + (x - 1)] = color;
                    screenMutex.unlock();
                }

                if (x + 2 < HEIGHT - 1)
                {
                    x += 2;
                }
                else
                {
                    x += 1;
                }
                last_color = color;
            }
            //cout << y << endl;
        }
    }
    cout << "Ended: " << i << endl;
    //thread c1(to_close, 1);
    is_alive[i - 1] = false;
}

std::tuple<int, int, int> numberToRGB3(int number) {
    // Scale the number to the range 0-255
    int scaledNumber = static_cast<int>((number / max_iterations) * 255);

    // Calculate the RGB components
    int red = 255 - scaledNumber;
    int green = 0;
    int blue = scaledNumber;

    return std::make_tuple(red, green, blue);
}

std::tuple<int, int, int> numberToRGB(int number) {
    if (number == 0) {
        return std::make_tuple(0, 0, 0); // Black color
    }
    else if (number == max_iterations) {
        return std::make_tuple(0, 0, 0); // Black color
    }
    else {
        int scaledNumber = (number * 255) / max_iterations;
        int red = int((255 - scaledNumber) / 1.3) % 256;
        int green = (scaledNumber * 7) % 256;
        int blue = (scaledNumber * 13) % 256;
        return std::make_tuple(red, green, blue);
    }
}


void updatePixels() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int color = screen[y * WIDTH + x];
            auto rgb = numberToRGB(color);

            int pixelIndex = (x + y * WIDTH) * 4;
            pixels[pixelIndex] = std::get<0>(rgb);
            pixels[pixelIndex + 1] = std::get<1>(rgb);
            pixels[pixelIndex + 2] = std::get<2>(rgb);
            pixels[pixelIndex + 3] = 255;
        }
    }
}

void updatePallete(int new_max, int old) 
{
    double maths = 0;
    for (int y = 0; y < HEIGHT; y++) 
    {
        for (int x = 0; x < WIDTH; x++) 
        {
            //screenMutex.lock();
            maths = ((double)screen[y * WIDTH + x] / (double)old);
            //cout << maths << endl;
            screen[y * WIDTH + x] = (int)(maths * new_max);
            //cout << (screen[y * WIDTH + x] / old) << endl;
            //screenMutex.unlock();
            
        }
    }
}

void starter_old()
{
    calculationThread1 = std::thread(calculateMandelbrot, 0, floor((HEIGHT / 11) * 1), 1, false);
    calculationThread2 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 1), floor((HEIGHT / 11) * 2), 2, false);
    calculationThread3 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 2), floor((HEIGHT / 11) * 3), 3, false);
    calculationThread4 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 3), floor((HEIGHT / 11) * 4), 4, false);
    calculationThread5 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 4), floor((HEIGHT / 11) * 5), 5, false);
    calculationThread6 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 5), floor((HEIGHT / 11) * 6), 6, false);
    calculationThread7 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 6), floor((HEIGHT / 11) * 7), 7, false);
    calculationThread8 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 7), floor((HEIGHT / 11) * 8), 8, false);
    calculationThread9 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 8), floor((HEIGHT / 11) * 9), 9, false);
    calculationThread10 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 9), floor((HEIGHT / 11) * 10), 10, false);
    calculationThread11 = std::thread(calculateMandelbrot, floor((HEIGHT / 11) * 10), HEIGHT, 11, false);
}

void dry()
{
    calculationThread1 = std::thread(calculateMandelbrot, 0, 0, 1, true);
    calculationThread2 = std::thread(calculateMandelbrot, 0, 0, 2, true);
    calculationThread3 = std::thread(calculateMandelbrot, 0, 0, 3, true);
    calculationThread4 = std::thread(calculateMandelbrot, 0, 0, 4, true);
    calculationThread5 = std::thread(calculateMandelbrot, 0, 0, 5, true);
    calculationThread6 = std::thread(calculateMandelbrot, 0, 0, 6, true);
    calculationThread7 = std::thread(calculateMandelbrot, 0, 0, 7, true);
    calculationThread8 = std::thread(calculateMandelbrot, 0, 0, 8, true);
    calculationThread9 = std::thread(calculateMandelbrot, 0, 0, 9, true);
    calculationThread10 = std::thread(calculateMandelbrot, 0, 0, 10, true);
    calculationThread11 = std::thread(calculateMandelbrot, 0, 0, 11, true);
    calculationThreadE = std::thread(calculateMandelbrot, 0, 0, 12, true);
}

void starter()
{
    int delta = 25;


    to_join = false;
    cout << "Starter..." << endl;
    int i = 0;

    while (!end_all_threads)
    {
        if (i + delta >= HEIGHT)
        {
            calculationThreadE.join();
            calculationThreadE = std::thread(calculateMandelbrot, i, HEIGHT, 12, false);
            break;
        }
        
        else if (!is_alive[0])
        {
            is_alive[0] = true;
            calculationThread1.join();
            calculationThread1 = std::thread(calculateMandelbrot, i, i + delta, 1, false);
            i += delta;
        }
        else if (!is_alive[1])
        {
            is_alive[1] = true;
            calculationThread2.join();
            calculationThread2 = std::thread(calculateMandelbrot, i, i + delta, 2, false);
            i += delta;
        }
        else if (!is_alive[2])
        {
            is_alive[2] = true;
            calculationThread3.join();
            calculationThread3 = std::thread(calculateMandelbrot, i, i + delta, 3, false);
            i += delta;
        }
        else if (!is_alive[3])
        {
            is_alive[3] = true;
            calculationThread4.join();
            calculationThread4 = std::thread(calculateMandelbrot, i, i + delta, 4, false);
            i += delta;
        }
        else if (!is_alive[4])
        {
            is_alive[4] = true;
            calculationThread5.join();
            calculationThread5 = std::thread(calculateMandelbrot, i, i + delta, 5, false);
            i += delta;
        }
        else if (!is_alive[5])
        {
            is_alive[5] = true;
            calculationThread6.join();
            calculationThread6 = std::thread(calculateMandelbrot, i, i + delta, 6, false);
            i += delta;
        }
        else if (!is_alive[6])
        {
            is_alive[6] = true;
            calculationThread7.join();
            calculationThread7 = std::thread(calculateMandelbrot, i, i + delta, 7, false);
            i += delta;
        }
        else if (!is_alive[7])
        {
            is_alive[7] = true;
            calculationThread8.join();
            calculationThread8 = std::thread(calculateMandelbrot, i, i + delta, 8, false);
            i += delta;
        }
        else if (!is_alive[8])
        {
            is_alive[8] = true;
            calculationThread9.join();
            calculationThread9 = std::thread(calculateMandelbrot, i, i + delta, 9, false);
            i += delta;
        }
        else if (!is_alive[9])
        {
            is_alive[9] = true;
            calculationThread10.join();
            calculationThread10 = std::thread(calculateMandelbrot, i, i + delta, 10, false);
            i += delta;
        }
        else if (!is_alive[10])
        {
            is_alive[10] = true;
            calculationThread11.join();
            calculationThread11 = std::thread(calculateMandelbrot, i, i + delta, 11, false);
            i += delta;
        }
    }

    cout << "Starter finished..." << endl;
    to_join = true;
}

bool all_ended()
{
    for (size_t i = 0; i < is_alive.size(); i++)
    {
        if (is_alive[i])
        {   
            return true;
        }
    }
    return false;
}

void cords_setup(cpp_dec_float_50 new_minX, cpp_dec_float_50 new_maxX, cpp_dec_float_50 new_minY, cpp_dec_float_50 new_maxY)
{
    last_minx.push_back(minx);
    last_maxx.push_back(maxx);
    last_xstep.push_back(xstep);

    last_maxy.push_back(maxy);
    last_ystep.push_back(ystep);
    last_miny.push_back(miny);

    if (new_minX > new_maxX)
    {
        cpp_dec_float_50 temp;

        temp = new_maxX;
        new_maxX = new_minX;
        new_minX = temp;
    }

    if (new_minY > new_maxY)
    {
        cpp_dec_float_50 temp;

        temp = new_maxY;
        new_maxY = new_minY;
        new_minY = temp;
    }
    
    cpp_dec_float_50 old_xstep = xstep;

    maxx = minx + new_maxX * xstep;
    minx = minx + new_minX * xstep;
   
    xstep = (maxx - minx) / WIDTH;
    ystep = xstep;
    
    maxy = maxy - old_xstep * new_minY;
    miny = maxy - xstep * HEIGHT;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set", sf::Style::Titlebar | sf::Style::Close);

    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    string maths;

    sf::Texture texture;
    texture.create(WIDTH, HEIGHT);

    sf::Sprite sprite(texture);

    dry();

    start = std::chrono::steady_clock::now();
    to_join = false;
    StartingThread = std::thread(starter);

    refre = 10;

    bool ended = false;
    bool mousevar = false;
    bool mousevar2 = true;
    bool mousevar3 = true;
    bool mousevar4 = true;
    bool var1 = true;

    bool keyvar1 = true, keyvar2 = true, keyvar3 = true, keyvar4 = true, keyvar5 = true, keyvar6 = true;

    int mouseX = 0;
    int mouseY = 0;

    int startX = 0, endX = 0;
    int startY = 0, endY = 0;

    int new_maxX = 0, new_minX = 0, new_minY = 0, new_maxY = 0;

    bool clicked = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        //cout << calculationThread.joinable() << ":" << is_alive[0] << endl;

        ended = all_ended();

        if (ended or refre > 0 or 1)
        {
            updatePixels();
            texture.update(pixels);

            window.clear();
            window.draw(sprite);

        }

        //cout << refre << endl;

        if (!ended)
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
            mouseX = mousePosition.x;
            mouseY = mousePosition.y;

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) and !mousevar and mouseX >= 0 and mouseY >= 0 and mouseX <= WIDTH and mouseY <= HEIGHT and !sf::Mouse::isButtonPressed(sf::Mouse::Right) and mousevar2 and window.hasFocus())
            {
                startX = mouseX;
                startY = mouseY;

                clicked = true;
                mousevar = true;
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            mousevar2 = false;
        }

        //cout << startX << ':' << startY << " :-- : " << endX << ':' << endY << endl;

        if (!ended and !sf::Mouse::isButtonPressed(sf::Mouse::Left) and mouseX >= 0 and mouseY >= 0 and mouseX <= WIDTH and mouseY <= HEIGHT and mousevar and mousevar2 and !sf::Mouse::isButtonPressed(sf::Mouse::Right) and window.hasFocus())
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
            mouseX = mousePosition.x;
            mouseY = mousePosition.y;

            endX = mouseX;
            //endY = mouseY;

            if (startY < mouseY)
            {
                if (startX < mouseX)
                {
                    endY = (float)(((mouseX - startX) * HEIGHT / WIDTH) + startY);
                }
                else
                {
                    endY = (float)(-((mouseX - startX) * HEIGHT / WIDTH) + startY);
                }
            }
            else
            {
                if (startX > mouseX)
                {
                    endY = (float)(((mouseX - startX) * HEIGHT / WIDTH) + startY);;
                }
                else
                {
                    endY = (float)(-((mouseX - startX) * HEIGHT / WIDTH) + startY);
                }
            }

            cout << startX << ':' << startY << " :--: " << endX << ':' << endY << endl;

            cords_setup(startX, endX, startY, endY);

            mousevar = false;

            start = std::chrono::steady_clock::now();
            to_join = false;
            StartingThread = std::thread(starter);
            refre = 15;
        }        

        else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) and !sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            mousevar = false;
            mousevar2 = true;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) and !ended and window.hasFocus() and mousevar2 and mouseX >= 0 and mouseY >= 0 and mouseX <= WIDTH and mouseY <= HEIGHT and clicked and !sf::Mouse::isButtonPressed(sf::Mouse::Right) and mousevar2)
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
            mouseX = mousePosition.x;
            mouseY = mousePosition.y;

            sf::VertexArray line(sf::Lines);

            sf::Vector2f startPoint((float)startX, (float)startY);
            sf::Vector2f endPoint((float)mouseX, (float)startY);

            line.append(sf::Vertex(startPoint, sf::Color::White));
            line.append(sf::Vertex(endPoint, sf::Color::White));

            window.draw(line);

            sf::VertexArray line2(sf::Lines);

            startPoint = sf::Vector2f((float)startX, (float)startY);
            if (startY < mouseY)
            {
                if (startX < mouseX)
                {
                    endPoint = sf::Vector2f((float)startX, (float)(((mouseX - startX) * HEIGHT / WIDTH) + startY));
                }
                else
                {
                    endPoint = sf::Vector2f((float)startX, (float)(-((mouseX - startX) * HEIGHT / WIDTH) + startY));
                }
            }
            else
            {
                if (startX > mouseX)
                {
                    endPoint = sf::Vector2f((float)startX, (float)(((mouseX - startX) * HEIGHT / WIDTH) + startY));
                }
                else
                {
                    endPoint = sf::Vector2f((float)startX, (float)(-((mouseX - startX) * HEIGHT / WIDTH) + startY));
                }
            }

            int hei = endPoint.y - startY;

            line2.append(sf::Vertex(startPoint, sf::Color::White));
            line2.append(sf::Vertex(endPoint, sf::Color::White));

            //cout << startX << " : " << startY << " --- " << mouseX << " : " << endPoint.y << endl;

            window.draw(line2);

            sf::VertexArray line3(sf::Lines);

            startPoint = sf::Vector2f((float)startX, (float)startY + hei);
            endPoint = sf::Vector2f((float)mouseX, (float)startY + hei);

            line3.append(sf::Vertex(startPoint, sf::Color::White));
            line3.append(sf::Vertex(endPoint, sf::Color::White));

            window.draw(line3);

            sf::VertexArray line4(sf::Lines);

            startPoint = sf::Vector2f((float)mouseX, (float)startY);
            endPoint = sf::Vector2f((float)mouseX, (float)startY + hei);

            line4.append(sf::Vertex(startPoint, sf::Color::White));
            line4.append(sf::Vertex(endPoint, sf::Color::White));

            window.draw(line4);
        }

        ended = all_ended();

        if (!ended and var1)
        {
            cout << endl <<"Image ready!" << endl;
            end = std::chrono::steady_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            cout << "Calculations took: " << (elapsed.count() / 1000) << " seconds." << endl;
            var1 = false;
        }
        if (ended)
        {
            var1 = true;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Middle))
        {
            end_all_threads = true;

            while (ended)
            {
                ended = all_ended();
            }
        }
        else
        {
            end_all_threads = false;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::XButton1) and !ended and mousevar3)
        {
            mousevar3 = false;

            if (last_maxx.size() == 1)
            {
                minx = last_minx[0];
                maxx = last_maxx[0];
                xstep = last_xstep[0];

                miny = last_miny[0];
                maxy = last_maxy[0];
                ystep = last_ystep[0];

            }
            else
            {
                int lnum = last_maxx.size() - 1;

                minx = last_minx[lnum];
                maxx = last_maxx[lnum];
                xstep = last_xstep[lnum];

                miny = last_miny[lnum];
                maxy = last_maxy[lnum];
                ystep = last_ystep[lnum];

                last_minx.pop_back();
                last_maxx.pop_back();
                last_xstep.pop_back();

                last_miny.pop_back();
                last_maxy.pop_back();
                last_ystep.pop_back();
            }

            start = std::chrono::steady_clock::now();
            to_join = false;
            StartingThread = std::thread(starter);
            refre = 15;

        }

        if (!sf::Mouse::isButtonPressed(sf::Mouse::XButton1))
        {
            mousevar3 = true;
        }

        if (to_join)
        {
            StartingThread.join();
            to_join = false;
        }

        if (to_clean and !ended and false)
        {
            cout << "Image generated." << endl;
            cout << "Cleaning started!" << endl;
            calculationThread1.join();
            calculationThread2.join();
            calculationThread3.join();
            calculationThread4.join();
            calculationThread5.join();
            calculationThread6.join();
            calculationThread7.join();
            calculationThread8.join();
            calculationThread9.join();
            calculationThread10.join();
            calculationThread11.join();
            calculationThreadE.join();

            to_clean = false;
            cout << "Threads cleaned!" << endl;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::XButton2) and mousevar4)
        {
            mousevar4 = false;

            if (more_details)
            {
                cout << "More details: OFF" << endl;
                more_details = false;
            }
            else
            {
                cout << "More details: ON" << endl;
                more_details = true;
            }
        }
        if (!sf::Mouse::isButtonPressed(sf::Mouse::XButton2))
        {
            mousevar4 = true;
        }

        ended = all_ended();

        if (!ended)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) and keyvar1)
            {
                updatePallete( 100, max_iterations);
                //updatePallete(100, max_iterations);
                keyvar1 = false;
                max_iterations = 100;
                cout << "Max iterations set to: 100" << endl;
                
            }
            else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
            {
                keyvar1 = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) and keyvar2)
            {
                
                updatePallete(250, max_iterations);
                keyvar2 = false;
                max_iterations = 250;
                cout << "Max iterations set to: 250" << endl;
            }
            else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
            {
                keyvar2 = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3) and keyvar3)
            {
                updatePallete(500, max_iterations);
                keyvar3 = false;
                max_iterations = 500;
                cout << "Max iterations set to: 500" << endl;
            }
            else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
            {
                keyvar3 = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4) and keyvar4)
            {
                updatePallete(1000, max_iterations);
                keyvar4 = false;
                max_iterations = 1000;
                cout << "Max iterations set to: 1000" << endl;
            }
            else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Num4))
            {
                keyvar4 = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5) and keyvar5)
            {
                updatePallete(2000, max_iterations);
                keyvar5 = false;
                max_iterations = 2000;
                cout << "Max iterations set to: 2000" << endl;
            }
            else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Num5))
            {
                keyvar5 = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6) and keyvar6)
            {
                updatePallete(10000, max_iterations);
                keyvar6 = false;
                max_iterations = 10000;
                cout << "Max iterations set to: 10000" << endl;
            }
            else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Num6))
            {
                keyvar6 = true;
            }
        }

        maths = to_string((uint64_t)(((last_maxx[0] - last_minx[0]) * (last_maxy[0] - last_miny[0])) / ((last_maxx[last_maxx.size() - 1] - last_minx[last_maxx.size() - 1]) * (last_maxy[last_maxx.size() - 1] - last_miny[last_maxx.size() - 1]))));

        sf::Font font;
        font.loadFromFile("arial.ttf");
        sf::Text text(std::string("Zoom: " + maths + "X"), font, 25);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(1);

        window.draw(text);

        window.display();

    }

    delete[] pixels;

    return 0;
}
