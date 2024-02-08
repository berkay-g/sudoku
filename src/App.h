#pragma once

#define SDL_MAIN_HANDLED
#include "SDL3/SDL.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <string>

#include "Sudoku.h"

class App
{
public:
	App(const char* window_title, int window_width, int window_height, Uint32 window_flags = SDL_WINDOW_RESIZABLE, Uint32 renderer_flags = SDL_RENDERER_ACCELERATED, Uint32 sdl_init_flags = SDL_INIT_VIDEO);
	~App();

    ImGuiIO& ImguiInit();
    ImGuiIO& ImguiNewFrame();
    void ImguiRender();

    int SetWindowMinimumSize(int min_width, int min_height);
    void SetWindowWidthHeight(int w, int h);

    int GetWindowWidth() { return window_width; }
    int GetWindowHeight() { return window_height; }
    void GetScaleFactors(float& x, float& y, float& factor);

    SDL_Window* GetSDLWindow() { return window; }
    SDL_Renderer* GetSDLRenderer() { return renderer; }

    bool GetIsMouseHidden() { return isMouseHidden; }
    void HideMouseCursor();
    void ShowMouseCursor();

    void DrawNumber(int number, const ImVec2& position, float size, const ImVec4& color, int x);
    bool IsMouseInsideRect(float mouseX, float mouseY, const SDL_FRect& rect);
    
private:
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    const char* window_title;
	int initial_window_width, initial_window_height;
    int window_width, window_height;
    float scale_factor_x{}, scale_factor_y{}, scale_factor{};
    bool isMouseHidden;

public:
    // Sudoku
    ImVec2 sudoku_window_pos, sudoku_window_size;
    ImVec4 check_color;
    bool valid = false;
    bool click = false;
    SDL_FRect rects[9 * 9]{};
    float rect_size = 60.f;
    int difficulty_level = 1;
    Sudoku sudoku;
    void sudokuStartGame();
    void sudokuDrawGrid();
    void sudokuDrawGridLines();
    void sudokuDrawActive();
    void sudokuDrawShadows(const Uint32 colors[3][4]);
    void sudokuDrawNumbers();
    void sudokuProcessKeyboardInput(const SDL_Keycode keycode);
    void sudokuProcessMouseMotionInput(const SDL_MouseMotionEvent& motion_event);
    void sudokuProcessMouseButtonDownInput();
    void sudokuDrawImguiWindow();
};

App::App(const char* window_title, int window_width, int window_height, Uint32 window_flags, Uint32 renderer_flags, Uint32 sdl_init_flags)
	: window_title(window_title), initial_window_width(window_width), initial_window_height(window_height), window_width(window_width), window_height(window_height), isMouseHidden(false)
{
    if (SDL_Init(sdl_init_flags) != 0)
    {
        SDL_Log("Failed to initialize SDL: %s\n", SDL_GetError());
        return;
    }

    window = SDL_CreateWindow(window_title, initial_window_width, initial_window_height, window_flags);
    if (!window)
    {
        SDL_Log("Failed to create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, NULL, renderer_flags);
    if (!renderer)
    {
        SDL_Log("Failed to create SDL renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    SDL_Log("Current SDL_Renderer: %s", info.name);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(display);
    if (displayMode)
    {
        SDL_Log("%s %dx%d", SDL_GetDisplayName(display), displayMode->w, displayMode->h);
        SDL_Rect rect = { 0, 0, displayMode->w, displayMode->h };
        SDL_SetRenderViewport(renderer, &rect);
    }
    else
        SDL_SetRenderViewport(renderer, NULL);


    scale_factor_x = static_cast<float>(window_width) / initial_window_width;
    scale_factor_y = static_cast<float>(window_height) / initial_window_height;
    scale_factor = std::min(scale_factor_x, scale_factor_y);
}

App::~App()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

ImGuiIO& App::ImguiInit()
{
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

    return io;
}

ImGuiIO& App::ImguiNewFrame()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGuiIO& io = ImGui::GetIO();
   
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    return io;
}

void App::ImguiRender()
{
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());
}

int App::SetWindowMinimumSize(int min_width, int min_height)
{
    return SDL_SetWindowMinimumSize(window, min_width, min_height);
}

void App::SetWindowWidthHeight(int w, int h)
{
    window_width = w;
    window_height = h;
    scale_factor_x = static_cast<float>(window_width) / initial_window_width;
    scale_factor_y = static_cast<float>(window_height) / initial_window_height;
    scale_factor = std::min(scale_factor_x, scale_factor_y);
}

void App::GetScaleFactors(float& x, float& y, float& factor)
{
    x = scale_factor_x;
    y = scale_factor_y;
    factor = scale_factor;
}

void App::HideMouseCursor() {
    if (!isMouseHidden) {
        if (!SDL_HideCursor())
            SDL_Log("%s", SDL_GetError());
        isMouseHidden = true;
    }
}

void App::ShowMouseCursor() {
    if (isMouseHidden) {
        if (SDL_ShowCursor())
            SDL_Log("%s", SDL_GetError());
        isMouseHidden = false;
    }
}

void App::DrawNumber(int number, const ImVec2& position, float size, const ImVec4& color, int x) {
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

bool App::IsMouseInsideRect(float mouseX, float mouseY, const SDL_FRect& rect)
{
    return (mouseX >= rect.x && mouseX <= rect.x + rect.w &&
        mouseY >= rect.y && mouseY <= rect.y + rect.h);
}

// Sudoku
void App::sudokuStartGame()
{
    sudoku.setDifficulty(difficulty_level);
    sudoku.generateSudoku();
    sudoku.initializeCellNumbers();
    sudoku.initializeStates();
    sudoku.start = sudoku.grid;
    sudoku.solved = sudoku.grid;
    valid = false;
    check_color = { 1.f, 0.f, 0.f, 1.f };
}

void App::sudokuDrawGrid()
{
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            SDL_FRect rect{};
            rect.x = 10 + (j * (rect_size * scale_factor + 2));
            rect.y = 10 + (i * (rect_size * scale_factor + 2));

            rect.w = rect_size * scale_factor;
            rect.h = rect_size * scale_factor;
            rects[get1DIndex(i, j, 9)] = rect;
        }
    }
    SDL_RenderFillRects(renderer, rects, 9 * 9);
}

void App::sudokuDrawGridLines()
{
    SDL_FRect rect{};
    for (int i = 1; i <= 2; i++)
    {
        rect.x = 10 + ((i * 3) * (rect_size * scale_factor + 2)) - 2;
        rect.y = 10 + (0 * (rect_size * scale_factor + 2));
        rect.w = 2;
        rect.h = rect_size * scale_factor * 9 + 2 * 8;
        SDL_RenderFillRect(renderer, &rect);
    }
    for (int i = 1; i <= 2; i++)
    {
        rect.x = 10 + (0 * (rect_size * scale_factor + 2));
        rect.y = 10 + ((i * 3) * (rect_size * scale_factor + 2)) - 2;
        rect.w = rect_size * scale_factor * 9 + 2 * 8;
        rect.h = 2;
        SDL_RenderFillRect(renderer, &rect);
    }
}

void App::sudokuDrawActive()
{
    SDL_RenderRect(renderer, &rects[sudoku.active]);
}

void App::sudokuDrawShadows(const Uint32 colors[3][4])
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, colors[0][0], colors[0][1], colors[0][2], colors[0][3]);
    auto colored = sudoku.getAllColored();
    for (auto& i : colored)
        SDL_RenderFillRect(renderer, &rects[get1DIndex(i.first, i.second, 9)]);

    SDL_SetRenderDrawColor(renderer, colors[1][0], colors[1][1], colors[1][2], colors[1][3]);
    std::vector<int> allOcc = sudoku.findAll();
    for (int i = 0; i < allOcc.size(); i++)
    {
        if (allOcc[i] == sudoku.active)
            continue;
        SDL_RenderFillRect(renderer, &rects[allOcc[i]]);
    }

    // Overlap
    SDL_SetRenderDrawColor(renderer, colors[2][0], colors[2][1], colors[2][2], colors[2][3]);
    for (int i = 0; i < allOcc.size(); i++)
        for (int j = 0; j < colored.size(); j++)
            if (allOcc[i] == get1DIndex(colored[j].first, colored[j].second, 9))
                SDL_RenderFillRect(renderer, &rects[allOcc[i]]);
}

void App::sudokuDrawNumbers()
{
    for (int i = 0; i < 9 * 9; i++)
    {
        ImVec4 number_color(1.0f, 0.5f, 0.5f, 1.0f);
        if (sudoku.states[i] == State::Start)
            number_color = { 245.f / 255.f, 245 / 255.f, 225 / 255.f, 1.0f };

        DrawNumber(sudoku.cell_numbers[i], { rects[i].x + 11 * scale_factor, rects[i].y }, 300 * scale_factor, number_color, i);
    }
}

void App::sudokuProcessKeyboardInput(const SDL_Keycode keycode)
{
    // SDLK_0 = 48
    for (int i = 0; i <= 9; i++)
    {
        if (keycode == 48 + i)
        {
            if (sudoku.states[sudoku.active] != State::Start)
            {
                sudoku.cell_numbers[sudoku.active] = i;
                sudoku.updateGrid();
            }
        }
    }

    // SDLK_KP_0 = 1073741922
    if (keycode == SDLK_KP_0)
    {
        if (sudoku.states[sudoku.active] != State::Start)
        {
            sudoku.cell_numbers[sudoku.active] = 0;
            sudoku.updateGrid();
        }
    }
    // SDLK_KP_1 = 1073741913
    for (int i = 0; i < 9; i++)
    {
        if (keycode == 1073741913 + i)
        {
            if (sudoku.states[sudoku.active] != State::Start)
            {
                sudoku.cell_numbers[sudoku.active] = i + 1;
                sudoku.updateGrid();
            }
        }
    }

    if (keycode == SDLK_RIGHT)
    {
        if ((sudoku.active + 1) % 9 == 0)
            sudoku.active -= 8;
        else
            sudoku.active += 1;
    }

    if (keycode == SDLK_LEFT)
    {
        if (sudoku.active % 9 == 0)
            sudoku.active += 8;
        else
            sudoku.active -= 1;
    }

    if (keycode == SDLK_DOWN)
    {
        if (sudoku.active + 9 > 80)
            sudoku.active -= 72;
        else
            sudoku.active += 9;
    }

    if (keycode == SDLK_UP)
    {
        if (sudoku.active - 9 < 0)
            sudoku.active += 72;
        else
            sudoku.active -= 9;
    }
}

void App::sudokuProcessMouseMotionInput(const SDL_MouseMotionEvent& motion_event)
{
    if (click)
        return;
    float mouseX = motion_event.x;
    float mouseY = motion_event.y;
    for (int i = 0; i < 9 * 9; i++)
    {
        bool isInside = IsMouseInsideRect(mouseX, mouseY, rects[i]);
        if (isInside)
            sudoku.active = i;
    }
}

void App::sudokuProcessMouseButtonDownInput()
{
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    for (int i = 0; i < 9 * 9; i++)
    {
        bool isInside = IsMouseInsideRect(mouseX, mouseY, rects[i]);
        if (isInside)
            sudoku.active = i;
    }
}

void App::sudokuDrawImguiWindow()
{
    static bool unsaved_document = false;
    static bool cheat = false;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
    if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;
    ImGui::SetNextWindowPos({ 650, 100 }, ImGuiCond_FirstUseEver);
    ImGui::Begin("sudoku", NULL, window_flags);
    sudoku_window_pos = ImGui::GetWindowPos();
    sudoku_window_size = ImGui::GetWindowSize();
    if (ImGui::Button("reset"))
    {
        unsaved_document = false;
        sudokuStartGame();
    }

    ImGui::SameLine();
    if (ImGui::Button("*##cheat"))
    {
        cheat = true;
        sudoku.solved = sudoku.start;
        sudoku.solveSudoku(sudoku.solved);
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
        valid = sudoku.isValidSudoku(sudoku.grid);
        valid ? check_color = { 0.f, 1.f, 0.f, 1.f } : check_color = { 1.f, 0.f, 0.f, 1.f };
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

    const char* items[] = { "Easy", "Medium", "Hard", "Evil" };
    static int item_current_idx = 1;
    const char* combo_preview_value = items[item_current_idx];
    if (ImGui::BeginCombo("##difficulty", combo_preview_value, ImGuiComboFlags_WidthFitPreview))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(items[n], is_selected))
            {
                item_current_idx = n;
                if (difficulty_level != item_current_idx)
                {
                    difficulty_level = item_current_idx;
                    unsaved_document = true;
                }
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }


    ImGui::Checkbox("Click", &click);
    if (click)
    {
        for (int i = 2; i >= 0; i--)
        {
            for (int j = 0; j < 3; j++)
            {
                if (ImGui::Button(std::to_string(3 * i + j + 1).c_str()))
                    if (sudoku.states[sudoku.active] != State::Start)
                        sudoku.cell_numbers[sudoku.active] = 3 * i + j + 1;
                if (j == 2)
                    break;
                ImGui::SameLine();
            }
        }
        if (ImGui::Button("   0   "))
            if (sudoku.states[sudoku.active] != State::Start)
                sudoku.cell_numbers[sudoku.active] = 0;
    }
    ImGui::End();

    if (cheat)
    {
        ImGui::SetNextWindowPos({ sudoku_window_pos.x - sudoku_window_size.y / 4, sudoku_window_pos.y + sudoku_window_size.y + 10 }, ImGuiCond_Appearing);
        ImGui::Begin("Cheat", &cheat, ImGuiWindowFlags_AlwaysAutoResize);
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                if (sudoku.states[get1DIndex(i, j, 9)] != State::Start)
                {
                    ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, std::to_string(sudoku.solved[i][j]).c_str());
                    if (j == 8) break;
                    ImGui::SameLine();
                }
                else
                {
                    ImGui::Text(std::to_string(sudoku.solved[i][j]).c_str());
                    if (j == 8) break;
                    ImGui::SameLine();
                }
            }
        }
        if (ImGui::Button("fill"))
        {
            sudoku.grid = sudoku.solved;
            sudoku.initializeCellNumbers();
        }


        ImGui::End();
    }
}