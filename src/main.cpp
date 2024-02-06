#define SDL_MAIN_HANDLED

#include "SDL2_framerate.h"
#include "SDL3/SDL.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_stdlib.h"

#include <string>
#include <unordered_map>

#include "sudoku.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 575
#define MIN_WIDTH 400
#define MIN_HEIGHT 305
int win_width = WINDOW_WIDTH, win_height = WINDOW_HEIGHT;

int quit = 0;

SDL_Window* window;
SDL_Renderer* renderer;
float scale_factor_x = static_cast<float>(win_width) / WINDOW_WIDTH;
float scale_factor_y = static_cast<float>(win_height) / WINDOW_HEIGHT;
float scale_factor = std::min(scale_factor_x, scale_factor_y);
ImVec2 sudoku_window_pos, sudoku_window_size;


void drawNumber(int number, ImVec2 position, float size, const ImVec4& color, int x) {
    if (number == 0)
        return;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::SetNextWindowPos(position);
    ImGui::Begin(std::string("Number " + std::to_string(x)).c_str(), nullptr, window_flags);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    float old_size = ImGui::GetFont()->Scale;
    ImGui::GetFont()->Scale *= size / 100;
    ImGui::PushFont(ImGui::GetFont());
    ImGui::Text(std::to_string(number).c_str());
    ImGui::GetFont()->Scale = old_size;
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::End();
}

inline int get1DIndex(int i, int j, int num_columns) {
    return i * num_columns + j;
}

std::pair<int, int> get2DIndex(int index, int size) {
    int i = index / size;
    int j = index % size;
    return {i, j};
}

inline bool isMouseInsideRect(float mouseX, float mouseY, const SDL_FRect& rect) {
    return (mouseX >= rect.x && mouseX <= rect.x + rect.w &&
            mouseY >= rect.y && mouseY <= rect.y + rect.h);
}

bool isMouseHidden = false;

void hideMouseCursor() {
    if (!isMouseHidden) {
        // SDL_Log("hidden");
        SDL_HideCursor();
        isMouseHidden = true;
    }
}

void showMouseCursor() {
    if (isMouseHidden) {
        // SDL_Log("shown");

        SDL_ShowCursor();
        isMouseHidden = false;
    }
}

int active = 0;
SDL_FRect rects[9*9];
int cell_numbers[9*9] = { 0 };
std::vector<std::vector<int>> grid, start, solved;
std::unordered_map<int, State> states;
ImVec4 check_color(1.f, 0.f, 0.f, 1.f);
bool valid = false;

void initializeCellNumbers(std::vector<std::vector<int>>& grid)
{  
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            cell_numbers[get1DIndex(i, j, 9)] = grid[i][j];
        }     
    }   
}

void initializeStates(int cell_numbers[9*9], std::unordered_map<int, State>& states)
{  
    for (int i = 0; i < 9*9; i++)
        cell_numbers[i] == 0 ? states[i] = State::Empty : states[i] = State::Start;
}

void updateGrid(int cell_numbers[9*9], std::vector<std::vector<int>>& grid)
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            grid[i][j] = cell_numbers[get1DIndex(i, j, 9)];
}

void startGame()
{
    generateSudoku(grid);
    initializeCellNumbers(grid);
    initializeStates(cell_numbers, states);
    start = grid;
    solved = grid;
    valid = false;
    check_color = {1.f, 0.f, 0.f, 1.f};
}

bool isValidSudoku(const std::vector<std::vector<int>>& board) {
    std::vector<std::vector<bool>> rowFlag(9, std::vector<bool>(9, false));
    std::vector<std::vector<bool>> colFlag(9, std::vector<bool>(9, false));
    std::vector<std::vector<bool>> boxFlag(9, std::vector<bool>(9, false));

    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (board[i][j] != 0) {
                int num = board[i][j] - 1;
                int k = i / 3 * 3 + j / 3;
                if (rowFlag[i][num] || colFlag[j][num] || boxFlag[k][num]) {
                    return false;
                }
                rowFlag[i][num] = colFlag[j][num] = boxFlag[k][num] = true;
            } else if (board[i][j] == 0) {
                // If there's an empty cell, it's not a complete solution yet, so return true
                return false;
            }
        }
    }
    return true;
}

bool click = false;

void draw()
{
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    static float rect_size = 60.f;
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            SDL_FRect rect;
            rect.x = 10 + (j * (rect_size * scale_factor + 2));
            rect.y = 10 + (i * (rect_size * scale_factor + 2));

            rect.w = rect_size * scale_factor;
            rect.h = rect_size * scale_factor;
            rects[get1DIndex(i, j, 9)] = rect;
        }
    }
    SDL_SetRenderDrawColor(renderer, 100, 100, 155, 255);
    SDL_RenderFillRects(renderer, rects, 9*9);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderRect(renderer, &rects[active]);

    for (int i = 0; i < 9*9; i++)
    {
        ImVec4 number_color(1.0f, 0.5f, 0.5f, 1.0f);
        if (states[i] == State::Start)
            number_color = {245.f / 255.f, 245 / 255.f, 225 / 255.f, 1.0f};

        drawNumber(cell_numbers[i], {rects[i].x + 11*scale_factor, rects[i].y}, 300 * scale_factor, number_color, i);
    }

    SDL_FRect rect;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 1; i <= 2; i++)
    {
        rect.x = 10 + ((i * 3) * (rect_size * scale_factor + 2)) - 2;
        rect.y = 10 + (0 * (rect_size * scale_factor + 2));
        rect.w = 2;
        rect.h = rect_size * scale_factor * 9 + 2*8;
        SDL_RenderFillRect(renderer, &rect);
    }
    for (int i = 1; i <= 2; i++)
    {
        rect.x = 10 + (0 * (rect_size * scale_factor + 2));
        rect.y = 10 + ((i * 3) * (rect_size * scale_factor + 2)) - 2;
        rect.w = rect_size * scale_factor * 9 + 2*8;
        rect.h = 2;
        SDL_RenderFillRect(renderer, &rect);
    }

    static bool cheat = false;
    ImGui::SetNextWindowPos({650, 100}, ImGuiCond_FirstUseEver);
    ImGui::Begin("sudoku", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    sudoku_window_pos = ImGui::GetWindowPos();
    sudoku_window_size = ImGui::GetWindowSize();
    if (ImGui::Button("reset"))
        startGame();

    ImGui::SameLine();
    if (ImGui::Button("*##cheat"))
    {
        cheat = true;
        solved = start;
        solveSudoku(solved);
    }
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted("cheat");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }

    if (ImGui::Button("check"))
    {
        valid = isValidSudoku(grid);
        valid ? check_color = {0.f, 1.f, 0.f, 1.f} : check_color = {1.f, 0.f, 0.f, 1.f};
    }
    ImGui::SameLine();
    ImGui::ColorButton("check", check_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip);
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(valid ? "valid solution" : "invalid solution");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    ImGui::Checkbox("Click", &click);
    if (click)
    {
        for (int i = 2; i >= 0; i--)
        {
            for (int j = 0; j < 3; j++)
            {
                if (ImGui::Button(std::to_string(3*i + j + 1).c_str()))
                    if (states[active] != State::Start)
                        cell_numbers[active] = 3*i + j + 1;
                if (j == 2)
                    break;
                ImGui::SameLine();
            }
        }
        if (ImGui::Button("   0   "))
            if (states[active] != State::Start)
                cell_numbers[active] = 0;
    }
    ImGui::End();

    if (cheat)
    {
        ImGui::SetNextWindowPos({sudoku_window_pos.x - sudoku_window_size.y / 4, sudoku_window_pos.y + sudoku_window_size.y + 10}, ImGuiCond_Appearing);
        ImGui::Begin("Cheat", &cheat, ImGuiWindowFlags_AlwaysAutoResize);
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                if (states[get1DIndex(i, j, 9)] != State::Start)
                {
                    ImGui::TextColored({1.f, 0.f, 0.f, 1.f}, std::to_string(solved[i][j]).c_str()); 
                    if (j == 8) break;
                    ImGui::SameLine();
                }
                else
                {
                    ImGui::Text(std::to_string(solved[i][j]).c_str()); 
                    if (j == 8) break;
                    ImGui::SameLine();
                }            
            }    
        }
        if (ImGui::Button("fill"))
        {
            grid = solved;
            initializeCellNumbers(grid);
        }
        
        
        ImGui::End();
    } 
    
    ImGui::Render();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());
    // Update the screen
    SDL_RenderPresent(renderer);
}

int EventFilter(void* userdata, SDL_Event* event)
{
    if (event->type == SDL_EVENT_WINDOW_RESIZED)
    {
        // Handle window resize event here

        win_width = event->window.data1;
        win_height = event->window.data2;
        scale_factor_x = static_cast<float>(win_width) / WINDOW_WIDTH;
        scale_factor_y = static_cast<float>(win_height) / WINDOW_HEIGHT;
        scale_factor = std::min(scale_factor_x, scale_factor_y);

        SDL_SetRenderViewport(renderer, NULL);

        draw();

        // SDL_Log("Window resized: %dx%d\n", event->window.data1, event->window.data2);
    }
    if (event->type == SDL_EVENT_WINDOW_MAXIMIZED)
    {
        SDL_Log("Maximized");
        SDL_RestoreWindow(window);
    }
    
    return 1;
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Sudoku", WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE & ~SDL_WINDOW_MAXIMIZED);
    if (!window)
    {
        SDL_Log("Failed to create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    bool vsync = true;
    renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        SDL_Log("Failed to create SDL renderer: %s\n", SDL_GetError());

        renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer)
        {
            SDL_Log("Failed to create SDL renderer: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
        }
    }
    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    SDL_Log("Current SDL_Renderer: %s", info.name);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    

    SDL_SetEventFilter(EventFilter, NULL);

    SDL_SetWindowMinimumSize(window, MIN_WIDTH, MIN_HEIGHT);

    startGame();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = NULL;
    io.LogFilename = NULL;
    ImGui::StyleColorsClassic();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    FPSmanager fps;
    SDL_initFramerate(&fps);
    SDL_setFramerate(&fps, 100);

    SDL_Event event;
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
                hideMouseCursor();
                if (event.key.keysym.sym == SDLK_ESCAPE) 
                    quit = 1;

                if (event.key.keysym.sym == SDLK_k) SDL_Log("fps = %f", io.Framerate);

                if (event.key.keysym.sym == SDLK_v)
                {
                    vsync = !vsync;
                    SDL_SetRenderVSync(renderer, vsync);
                }

                // SDLK_0 = 48
                for (int i = 0; i <= 9; i++)
                {
                    if (event.key.keysym.sym == 48 + i)
                    {
                        if (states[active] != State::Start)
                        {  
                            cell_numbers[active] = i;
                            updateGrid(cell_numbers, grid);
                        }
                    }
                }

                // SDLK_KP_0 = 1073741922
                if (event.key.keysym.sym == SDLK_KP_0)
                {
                    if (states[active] != State::Start)
                    {  
                        cell_numbers[active] = 0;
                        updateGrid(cell_numbers, grid);
                    }
                }
                // SDLK_KP_1 = 1073741913
                for (int i = 0; i < 9; i++)
                {    
                    if (event.key.keysym.sym == 1073741913 + i)
                    {
                        if (states[active] != State::Start)
                        {  
                            cell_numbers[active] = i + 1;
                            updateGrid(cell_numbers, grid);
                        }
                    }
                }

                {
                    if (event.key.keysym.sym == SDLK_RIGHT)
                    {
                        if ((active + 1) % 9 == 0)
                            active -= 8;
                        else
                            active += 1;
                    }

                    if (event.key.keysym.sym == SDLK_LEFT)
                    {
                        if (active % 9 == 0)
                            active += 8;
                        else
                            active -= 1;
                    }

                    if (event.key.keysym.sym == SDLK_DOWN)
                    {
                        if (active + 9 > 80)
                            active -= 72;
                        else
                            active += 9;
                    }

                    if (event.key.keysym.sym == SDLK_UP)
                    {
                        if (active - 9 < 0)
                            active += 72;
                        else
                            active -= 9;
                    }
                }
                break;
            case SDL_EVENT_MOUSE_MOTION:
                {
                    showMouseCursor();
                    if (click) break;
                    float mouseX = event.motion.x;
                    float mouseY = event.motion.y;
                    for (int i = 0; i < 9*9; i++)
                    {
                        bool isInside = isMouseInsideRect(mouseX, mouseY, rects[i]);
                        if (isInside) {
                            active = i;
                        }
                    }
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                float mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                for (int i = 0; i < 9*9; i++)
                {
                    bool isInside = isMouseInsideRect(mouseX, mouseY, rects[i]);
                    if (isInside) {
                        active = i;
                    }
                }
                break;
            }
        }

        draw();

        SDL_framerateDelay(&fps);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}