#include <Windows.h>
#include <string>

#include "MainWindow.h"
#include "SDL_syswm.h"

#define BLACK 0, 0, 0
#define WHEAT 245, 222, 179
#define RGB_WHEAT RGB(245, 222, 179)

#define ISKEYDOWN(key) ((GetAsyncKeyState(key) & 0x8000) == 0x8000)

constexpr int STEP = 5;
constexpr int FONT_W = 100;

/*
 * Helper to transform the window as layered and top most
 */
void setWindowColorKey(SDL_Window *window)
{
    // Retrieves the hWnd of the window
    SDL_SysWMinfo wmInfo{};
    SDL_VERSION(&wmInfo.version);
    (void)SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    // Make Window Layered
    SetWindowLong(hwnd, GWL_EXSTYLE,
                  GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED |
                      WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST);

    // Sets Color key to Wheat
    (void)SetLayeredWindowAttributes(hwnd, RGB_WHEAT, 0, LWA_COLORKEY);

    // Sets Window Top Most
    (void)SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

int MainWindow::Init()
{
    (void)SDL_Init(SDL_INIT_VIDEO);
    (void)TTF_Init();

    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    try
    {
        // Tries to load the config, or creates a default one if non-existant
        this->config = std::make_unique<CrosshairConfig>(screenX, screenY);
    }
    catch (std::exception &e)
    {
        // Show a message box if there is an error
        std::string s("Config error: ");
        s.append(e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Config error",
                                 s.c_str(), nullptr);

        // Close program
        return 1;
    }

    // Initializes the window as an hidden window.
    // This is done because the color key is set later, and this would show
    // a black screen until it is set.
    this->window =
        SDL_CreateWindow("Crosshair", 0, 0, screenX, screenY,
                         SDL_WINDOW_HIDDEN | SDL_WINDOW_ALWAYS_ON_TOP);
    this->renderer = SDL_CreateRenderer(window, -1, 0);

    // Opens the arial font that should be available in all windows systems
    this->Arial40 = TTF_OpenFont("C:\\Windows\\Fonts\\arialbd.ttf", 40);

    // If for some reason the font is unavailable
    if (this->Arial40 == nullptr)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Initialization error",
                                 "The arial font cannot be found. Please make "
                                 "sure it is installed on your system.",
                                 nullptr);

        // Close program
        return 1;
    }

    this->size.w = screenX;
    this->size.h = screenY;

    // Makes the window transparent and top most
    setWindowColorKey(window);

    // Displays the window
    SDL_ShowWindow(this->window);

    while (!this->shouldQuit)
    {
        this->Loop();
    }

    return 0;
}

void MainWindow::Loop()
{
    SDL_Event event;

    SDL_Delay(2);
    (void)SDL_PollEvent(&event);

    switch (event.type)
    {
    case SDL_QUIT:
        this->shouldQuit = true;
        break;
    default:
        break;
    }

    this->Logic();
    this->Draw();
}

void MainWindow::Draw()
{
    // Sets the background color as the color key
    (void)SDL_SetRenderDrawColor(renderer, WHEAT, SDL_ALPHA_OPAQUE);
    (void)SDL_RenderClear(renderer);

    // If we should draw the crosshair
    if (this->shouldDraw)
    {
        CrosshairColor color = this->config->getCrosshairColor();

        // Get the positions based on the loaded config, plus their offsets
        int wx = this->config->getX() + this->offsetX;
        int wy = this->config->getY() + this->offsetY;

        // Sets color to the one specified in the config
        (void)SDL_SetRenderDrawColor(renderer, color.rgb[2], color.rgb[1],
                                     color.rgb[0], SDL_ALPHA_OPAQUE);

        // Draws the horizontal line
        (void)SDL_RenderDrawLine(renderer, wx - 5, wy, wx + 5, wy);
        // Draws the vertical line
        (void)SDL_RenderDrawLine(renderer, wx, wy - 5, wx, wy + 5);

        // When switching between a light background and a dark one,
        // the crosshair would dissapear. This creates a second crosshair
        // but in black and offseted to the down-right by 1 pixel.
        wx++;
        wy++;

        // Sets the color to black
        (void)SDL_SetRenderDrawColor(renderer, BLACK, SDL_ALPHA_OPAQUE);

        // Draws the horizontal line
        (void)SDL_RenderDrawLine(renderer, wx - 5, wy, wx + 5, wy);
        // Draws the vertical line
        (void)SDL_RenderDrawLine(renderer, wx, wy - 5, wx, wy + 5);
    }

    // if we should display the position text
    if (this->shouldDrawText)
    {
        // 8 characters because "X: " is 3 characters, the highest current
        // resolution is 4 digits long and the NULL terminator.
        char buf[8];
        SDL_snprintf(buf, 8, "X: %d", this->config->getX() + this->offsetX);

        CrosshairColor color = this->config->getTextColor();

        // Creates a texture to render the text
        SDL_Color c = {color.rgb[2], color.rgb[1], color.rgb[0], 0x99};
        SDL_Surface *surface = TTF_RenderText_Solid(Arial40, buf, c);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

        // Position of the text for X. To be on the right, its is placed to
        // the screen size - the width of the font - somme padding.
        SDL_Rect x_rect = {size.w - FONT_W - 25, 50, FONT_W, 50};
        SDL_RenderCopy(renderer, texture, nullptr, &x_rect);

        // Cleanup for texture and surface
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);

        // Exact same thing but for y
        SDL_snprintf(buf, 8, "Y: %d", this->config->getY() + this->offsetY);

        surface = TTF_RenderText_Solid(Arial40, buf, c);
        texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_Rect y_rect = {size.w - FONT_W - 25, 100, FONT_W, 50};
        SDL_RenderCopy(renderer, texture, nullptr, &y_rect);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    // Renders to the screen
    SDL_RenderPresent(renderer);
}

void MainWindow::Logic()
{
    // If the super is down, we show the position's text
    this->shouldDrawText = this->config->isSuperDown();

    this->shouldQuit = this->config->isQuitDown();

    if (this->config->isToggleDown())
    {
        this->shouldDraw = !this->shouldDraw;
        // Debounce for toogleing
        Sleep(250);
    }

    if (this->config->isResetDown())
    {
        this->offsetX = 0;
        this->offsetY = 0;
    }

    // Change crosshair position with super key and arrow keys
    if (this->config->isSuperDown() && ISKEYDOWN(VK_LEFT))
    {
        this->offsetX -= STEP;
    }

    if (this->config->isSuperDown() && ISKEYDOWN(VK_RIGHT))
    {
        this->offsetX += STEP;
    }

    if (this->config->isSuperDown() && ISKEYDOWN(VK_UP))
    {
        this->offsetY -= STEP;
    }

    if (this->config->isSuperDown() && ISKEYDOWN(VK_DOWN))
    {
        this->offsetY += STEP;
    }
}
