#include <Windows.h>
#include <string>

#include "SDL_syswm.h"
#include "MainWindow.h"

#define BLACK 0, 0, 0
#define WHEAT 245, 222, 179

#define ISKEYDOWN(key) ((GetAsyncKeyState(key) & 0x8000) == 0x8000)

constexpr int STEP = 5;
constexpr int FONT_W = 100;

void setWindowColorKey(SDL_Window *window)
{
    SDL_SysWMinfo wmInfo{};
    SDL_VERSION(&wmInfo.version);
    (void)SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    // Make Window Layered
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST);

    // Sets Color key to Wheat
    (void)SetLayeredWindowAttributes(hwnd, RGB(245, 222, 179), 0, LWA_COLORKEY);

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
        this->config = std::make_unique<CrosshairConfig>(screenX, screenY);
    }
    catch (std::exception &e)
    {
        std::string s("Config error: ");
        s.append(e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Config error", s.c_str(), NULL);
        return 1;
    }

    this->window = SDL_CreateWindow("Crosshair", 0, 0, screenX, screenY, 0);
    this->renderer = SDL_CreateRenderer(window, -1, 0);
    this->Arial40 = TTF_OpenFont("C:\\Windows\\Fonts\\arialbd.ttf", 40);

    this->size.w = screenX;
    this->size.h = screenY;
    this->size.half_x = screenX / 2;
    this->size.half_y = screenY / 2;

    setWindowColorKey(window);

    while (!this->shouldQuit)
    {
        this->Start();
    }

    return 0;
}

void MainWindow::Start()
{
    SDL_Event event;

    SDL_Delay(100);
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
    (void)SDL_SetRenderDrawColor(renderer, WHEAT, SDL_ALPHA_OPAQUE);
    (void)SDL_RenderClear(renderer);

    if (this->shouldDraw)
    {
        CrosshairColor color = this->config->getCrosshairColor();

        int wx = this->config->calcX() + this->offsetX;
        int wy = this->config->calcY() + this->offsetY;
        (void)SDL_SetRenderDrawColor(renderer, color.rgb[2], color.rgb[1], color.rgb[0], SDL_ALPHA_OPAQUE);
        (void)SDL_RenderDrawLine(renderer, wx - 5, wy, wx + 5, wy);
        (void)SDL_RenderDrawLine(renderer, wx, wy - 5, wx, wy + 5);

        wx++;
        wy++;
        (void)SDL_SetRenderDrawColor(renderer, BLACK, SDL_ALPHA_OPAQUE);
        (void)SDL_RenderDrawLine(renderer, wx - 5, wy, wx + 5, wy);
        (void)SDL_RenderDrawLine(renderer, wx, wy - 5, wx, wy + 5);
    }

    if (this->shouldDrawText)
    {
        char buf[8];
        SDL_snprintf(buf, 8, "X: %d", this->config->calcX() + this->offsetX);

        CrosshairColor color = this->config->getTextColor();

        SDL_Color c = {color.rgb[2], color.rgb[1], color.rgb[0], 0x99};
        SDL_Surface *surface = TTF_RenderText_Solid(Arial40, buf, c);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_Rect x_rect = {size.w - FONT_W - 25, 50, FONT_W, 50};
        SDL_RenderCopy(renderer, texture, NULL, &x_rect);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);

        SDL_snprintf(buf, 8, "Y: %d", this->config->calcY() + this->offsetY);

        surface = TTF_RenderText_Solid(Arial40, buf, c);
        texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_Rect y_rect = {size.w - FONT_W - 25, 100, FONT_W, 50};
        SDL_RenderCopy(renderer, texture, NULL, &y_rect);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    SDL_RenderPresent(renderer);
}

void MainWindow::Logic()
{
    this->shouldDrawText = this->config->isSuperDown();

    this->shouldQuit = this->config->isQuitDown();

    if (this->config->isToggleDown())
    {
        this->shouldDraw = !this->shouldDraw;
        Sleep(250);
    }
    if (this->config->isResetDown())
    {
        this->offsetX = 0;
        this->offsetY = 0;
    }
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
