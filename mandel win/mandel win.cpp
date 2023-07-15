#include <thread>
#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>
#include <mutex>
#include <cmath>

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;
using namespace std;

const int WIDTH = 900;
const int HEIGHT = 900;

string maths = "1";

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

//zoom 110

int delta = 15;
int numCores = std::thread::hardware_concurrency();
//int numCores = 30;

vector<cpp_dec_float_50> last_minx = { -3 };
vector< cpp_dec_float_50> last_maxx = { 1 };
vector< cpp_dec_float_50> last_xstep = { (maxx - minx) / WIDTH };

vector< cpp_dec_float_50> last_maxy = { xstep * (HEIGHT / 2) };
vector< cpp_dec_float_50> last_ystep = { xstep };
vector< cpp_dec_float_50> last_miny = { maxy - (((maxx - minx) * HEIGHT) / WIDTH) };


vector<bool> is_alive(numCores + 1, false);


std::vector<std::thread> calculationThreads(numCores);


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


std::pair<cpp_dec_float_50, cpp_dec_float_50> mandel(cpp_dec_float_50 x, cpp_dec_float_50 y, cpp_dec_float_50 x0, cpp_dec_float_50 y0) {
    cpp_dec_float_50 real = (x * x) + (-y * y) + x0;
    cpp_dec_float_50 imag = (x * y * 2) + y0;
    return std::make_pair(real, imag);
}

std::pair<cpp_dec_float_50, cpp_dec_float_50> julia(cpp_dec_float_50 x, cpp_dec_float_50 y, cpp_dec_float_50 cx, cpp_dec_float_50 cy) {
    cx = -0.775573102;
    cy = -0.338761181;
    cpp_dec_float_50 real = (x * x) - (y * y) + cx;
    cpp_dec_float_50 imag = (2 * x * y) + cy;
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

void delayMilliseconds(int milliseconds) {
    std::chrono::milliseconds duration(milliseconds);
    std::this_thread::sleep_for(duration);
}

void calculateMandelbrot(int startRow, int endRow, int i, bool dry_run) {

    if (dry_run)
    {
        is_alive[i] = false;
        return;
    }

    is_alive[i] = true;
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

            for (int x = 0; x < WIDTH and !end_all_threads; x++) {
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
            while (x < WIDTH and !end_all_threads) {
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
    is_alive[i] = false;
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
    double maths1 = 0;
    for (int y = 0; y < HEIGHT; y++) 
    {
        for (int x = 0; x < WIDTH; x++) 
        {
            //screenMutex.lock();
            maths1 = ((double)screen[y * WIDTH + x] / (double)old);
            //cout << maths1 << endl;
            screen[y * WIDTH + x] = (int)(maths1 * new_max);
            //cout << (screen[y * WIDTH + x] / old) << endl;
            //screenMutex.unlock();
            
        }
    }
}

void dry()
{
    for (size_t i = 0; i < calculationThreads.size(); i++)
    {
        calculationThreads[i] = std::thread(calculateMandelbrot, 0, 0, i, true);
    }
}

void starter()
{
    int starter_delta = delta;


    to_join = false;
    cout << "Starter..." << endl;
    int c = 0;

    bool last_start = false;

    while (!end_all_threads and !last_start)
    {

        for (size_t i = 0; i < calculationThreads.size() and !end_all_threads; i++)
        {

            if (calculationThreads.size() * starter_delta + c > HEIGHT and starter_delta > 4)
            {
                starter_delta -= 1;
                //cout << "                                 now: " << starter_delta << endl;
            }

            if (!is_alive[i])
            {
                is_alive[i] = true;

                calculationThreads[i].join();

                if (c + starter_delta <= HEIGHT)
                {
                    calculationThreads[i] = std::thread(calculateMandelbrot, c, c + starter_delta, i, false);
                }
                else
                {
                    calculationThreads[i] = std::thread(calculateMandelbrot, c, HEIGHT, i, false);
                    last_start = true;
                    break;
                }
                
                c += starter_delta;
            }
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

    maths = "";
    maths = to_string((cpp_int)(((last_maxx[0] - last_minx[0]) * (last_maxy[0] - last_miny[0])) / ((last_maxx[last_maxx.size() - 1] - last_minx[last_maxx.size() - 1]) * (last_maxy[last_maxx.size() - 1] - last_miny[last_maxx.size() - 1]))));
    string t = maths.substr(0, 1);

    if (maths.size() > 20)
    {
        t.append(".");
        t.append(maths.substr(1, 2));
        t.append(" * 10^");
        t.append(to_string(maths.size() - 1));
        maths = t;
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set", sf::Style::Titlebar | sf::Style::Close);

    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    bool focus = false;

    sf::Texture texture;
    texture.create(WIDTH, HEIGHT);

    sf::Sprite sprite(texture);

    sf::Font font;
    font.loadFromFile("arial.ttf");

   //sf::Text text(std::string("Zoom: " + maths + "X"), font, 25);
    sf::Text text("", font, 25);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(1);

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
        focus = window.hasFocus();

        if (ended or refre > 0 or 1)
        {
            updatePixels();
            texture.update(pixels);

            window.clear();
            window.draw(sprite);

        }

        //cout << refre << endl;

        if (!ended or 1)
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
            mouseX = mousePosition.x;
            mouseY = mousePosition.y;

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) and !mousevar and mouseX >= 0 and mouseY >= 0 and mouseX <= WIDTH and mouseY <= HEIGHT and !sf::Mouse::isButtonPressed(sf::Mouse::Right) and mousevar2 and focus)
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

        if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) and mouseX >= 0 and mouseY >= 0 and mouseX <= WIDTH and mouseY <= HEIGHT and mousevar and mousevar2 and !sf::Mouse::isButtonPressed(sf::Mouse::Right) and focus)
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
            //ended = all_ended();

            end_all_threads = true;

            ended = all_ended();
            while (ended)
            {
                //std::cout << "m" << std::endl;
                delayMilliseconds(10);
                ended = all_ended();

            }
            end_all_threads = false;

            if (StartingThread.joinable())
            {
                StartingThread.join();
                to_join = false;
            }



            to_join = false;
            StartingThread = std::thread(starter);
            refre = 15;
        }        

        else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) and !sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            mousevar = false;
            mousevar2 = true;
        }

        focus = window.hasFocus();

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) and focus and mousevar2 and mouseX >= 0 and mouseY >= 0 and mouseX <= WIDTH and mouseY <= HEIGHT and clicked and !sf::Mouse::isButtonPressed(sf::Mouse::Right) and mousevar2)
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
            start = std::chrono::steady_clock::now();
            var1 = false;
        }
        if (ended)
        {
            var1 = true;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Middle) and focus)
        {
            
            end_all_threads = true;

            ended = all_ended();
            while (ended)
            {
                //std::cout << "m" << std::endl;
                delayMilliseconds(10);
                ended = all_ended();
                
            }
            end_all_threads = false;

        }
        else
        {
            end_all_threads = false;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::XButton1) and mousevar3 and focus)
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

            //ended = all_ended();
            end_all_threads = true;

            ended = all_ended();
            while (ended)
            {
                //std::cout << "m" << std::endl;
                delayMilliseconds(10);
                ended = all_ended();

            }
            end_all_threads = false;
            if (StartingThread.joinable())
            {
                StartingThread.join();
                to_join = false;
            }
            

            maths = "";
            maths = to_string((cpp_int)(((last_maxx[0] - last_minx[0]) * (last_maxy[0] - last_miny[0])) / ((last_maxx[last_maxx.size() - 1] - last_minx[last_maxx.size() - 1]) * (last_maxy[last_maxx.size() - 1] - last_miny[last_maxx.size() - 1]))));
            string t = maths.substr(0, 1);

            if (maths.size() > 20)
            {
                t.append(".");
                t.append(maths.substr(1, 2));
                t.append(" * 10^");
                t.append(to_string(maths.size() - 1));
                maths = t;
            }

            to_join = false;
            StartingThread = std::thread(starter);
            refre = 15;

        }

        if (!sf::Mouse::isButtonPressed(sf::Mouse::XButton1))
        {
            mousevar3 = true;
        }

        focus = window.hasFocus();

        if (to_join)
        {
            StartingThread.join();
            to_join = false;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::XButton2) and mousevar4 and focus)
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
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) and keyvar1 and focus)
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

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) and keyvar2 and focus)
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

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3) and keyvar3 and focus)
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

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4) and keyvar4 and focus)
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

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5) and keyvar5 and focus)
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

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6) and keyvar6 and focus)
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

        text.setString((std::string("Zoom: " + maths + "X")));

        window.draw(text);

        window.display();

    }

    delete[] pixels;

    return 0;
}
