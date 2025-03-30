#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Config.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include <memory>

struct WindowSize
{
    int w, h;
};

class MainWindow
{

  private:
    SDL_Renderer *renderer;
    SDL_Window *window;
    TTF_Font *Arial40;
    int offsetX = 0;
    int offsetY = 0;
    bool shouldDraw = true;
    bool shouldQuit = false;
    bool shouldDrawText = false;
    std::unique_ptr<CrosshairConfig> config;
    WindowSize size;

  public:
    MainWindow() {}
    ~MainWindow()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(Arial40);
        TTF_Quit();
        SDL_Quit();
    }

    int Init();
    void Loop();
    void Draw();
    void Logic();
};

#endif // MAINWINDOW_H