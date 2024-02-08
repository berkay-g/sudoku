#include "App.h"

#include "SDL2_framerate.h"

void draw(App* app)
{
    SDL_Renderer* renderer = app->GetSDLRenderer();
    ImGuiIO& io = app->ImguiNewFrame(); (void)io;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);


    SDL_SetRenderDrawColor(renderer, 100, 100, 155, 255);
    app->sudokuDrawGrid();
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    app->sudokuDrawGridLines();
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    app->sudokuDrawActive();
    const Uint32 colors[3][4] = { {0, 0, 255, 100}, {255, 255, 0, 100}, {255, 0, 0, 100} };
    app->sudokuDrawShadows(colors);

    app->sudokuDrawNumbers();

    app->sudokuDrawImguiWindow();


    app->ImguiRender();
    // Update the screen
    SDL_RenderPresent(renderer);
}

int main()
{
    App app("Sudoku", 800, 575);
    app.SetWindowMinimumSize(400, 305);

    SDL_AddEventWatch([](void* userdata, SDL_Event* event) -> int {

        if (event->type == SDL_EVENT_WINDOW_RESIZED)
        {
            // Handle window resize event here
            App* pThis = reinterpret_cast<App*>(userdata);

            pThis->SetWindowWidthHeight(event->window.data1, event->window.data2);

            draw(pThis);

            //SDL_Log("Window resized: %dx%d\n", event->window.data1, event->window.data2);
        }

        return 1;
    }, &app);

    ImGuiIO& io = app.ImguiInit();
    app.sudokuStartGame();

    FPSmanager fps;
    SDL_initFramerate(&fps);
    SDL_setFramerate(&fps, 100);

    SDL_Event event;
    int quit = 0;
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                quit = 1;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    quit = 1;
                if (event.key.keysym.sym == SDLK_k) 
                    SDL_Log("fps = %f", io.Framerate);

                app.sudokuProcessKeyboardInput(event.key.keysym.sym);

                break;
            case SDL_EVENT_MOUSE_MOTION:
                app.sudokuProcessMouseMotionInput(event.motion);

                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                app.sudokuProcessMouseButtonDownInput();

                break;
            }
        }

        draw(&app);

        SDL_framerateDelay(&fps);
    }
 
    return 0;
}
