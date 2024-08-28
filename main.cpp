#include <windows.h>
#include <commdlg.h>  // Para caixas de diálogo de arquivos
#include <commctrl.h> // Para controles comuns, incluindo sliders
#include "resource.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#define IDD_SPEED_DIALOG 101
#define IDC_SPEED_SLIDER 1001


using namespace std;

struct Coord {
    int x, y;
};

vector<Coord> coordinates;
bool captureCoords = false;
bool repeat = false;
int clickDelay = 2500;
HWND hwndList;

void SaveCoordinatesToFile(const std::wstring& filename) {
    ofstream outFile(filename.c_str());
    for (const auto& coord : coordinates) {
        outFile << coord.x << " " << coord.y << endl;
    }
}

void LoadCoordinatesFromFile(const std::wstring& filename) {
    ifstream inFile(filename.c_str());
    if (inFile) {
        coordinates.clear();
        SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
        Coord coord;
        while (inFile >> coord.x >> coord.y) {
            coordinates.push_back(coord);
            std::wostringstream coordText;
            coordText << L"X: " << coord.x << L", Y: " << coord.y;
            SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)coordText.str().c_str());
        }
    }
}

void ShowSpeedDialog(HWND hwnd) {
    // Cria uma caixa de diálogo para configurar a velocidade
    INT_PTR result;
    result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SPEED_DIALOG), hwnd, [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
        static HWND speedSlider;
        switch (message) {
        case WM_INITDIALOG:
            speedSlider = GetDlgItem(hDlg, IDC_SPEED_SLIDER);
            SendMessage(speedSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 10));
            SendMessage(speedSlider, TBM_SETPAGESIZE, 0, 1);
            SendMessage(speedSlider, TBM_SETPOS, TRUE, (clickDelay - 500) / 500); // Ajusta a posição do slider
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                int pos = static_cast<int>(SendMessage(speedSlider, TBM_GETPOS, 0, 0));
                clickDelay = 5000 - pos * 500; // Ajusta o delay baseado na posição do slider
                EndDialog(hDlg, IDOK);
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
        }
        return FALSE;
        });
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case 1: {  // Botão "Coordenadas"
            captureCoords = true;
            MessageBox(hwnd, L"Aperte F5 para capturar a posição do mouse.", L"Informação", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case 2: {  // Botão "Iniciar"
            if (coordinates.empty()) {
                MessageBox(hwnd, L"Você precisa adicionar as coordenadas primeiro.", L"Erro", MB_OK | MB_ICONERROR);
            }
            else {
                do {
                    for (const auto& coord : coordinates) {
                        SetCursorPos(coord.x, coord.y);
                        Sleep(clickDelay);

                        // Clicar com o botão direito
                        mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
                        mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
                    }
                } while (repeat);
            }
            break;
        }
        case 3: {  // Botão "Limpar Coordenadas"
            coordinates.clear();
            SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
            MessageBox(hwnd, L"Coordenadas limpas.", L"Informação", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case 4: {  // Configurações -> Repetir
            repeat = !repeat;
            MessageBox(hwnd, repeat ? L"Repetição ativada." : L"Repetição desativada.", L"Configuração", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case 5: {  // Configurações -> Velocidade
            ShowSpeedDialog(hwnd);
            break;
        }
        case 6: {  // Configurações -> Exportar
            OPENFILENAME ofn;
            WCHAR szFile[260] = L"coordenadas.txt";

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Text Files\0*.TXT\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetSaveFileName(&ofn)) {
                SaveCoordinatesToFile(ofn.lpstrFile);
                MessageBox(hwnd, L"Coordenadas exportadas com sucesso.", L"Exportar", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case 7: {  // Configurações -> Importar
            OPENFILENAME ofn;
            WCHAR szFile[260] = L"coordenadas.txt";

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Text Files\0*.TXT\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn)) {
                LoadCoordinatesFromFile(ofn.lpstrFile);
                MessageBox(hwnd, L"Coordenadas importadas com sucesso.", L"Importar", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        }
        break;
    }
    case WM_KEYDOWN: {
        if (captureCoords && wParam == VK_F5) {
            POINT p;
            if (GetCursorPos(&p)) {
                Coord coord = { p.x, p.y };
                coordinates.push_back(coord);

                std::wostringstream coordText;
                coordText << L"X: " << coord.x << L", Y: " << coord.y;
                SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)coordText.str().c_str());
            }
        }
        else if (wParam == VK_F4) {
            repeat = false;
        }
        break;
    }
    case WM_CREATE: {
        // Cria o menu suspenso
        HMENU hMenu = CreateMenu();
        HMENU hSubMenu = CreatePopupMenu();

        AppendMenu(hSubMenu, MF_STRING, 4, L"Repetir");
        AppendMenu(hSubMenu, MF_STRING, 5, L"Velocidade");
        AppendMenu(hSubMenu, MF_STRING, 6, L"Exportar");
        AppendMenu(hSubMenu, MF_STRING, 7, L"Importar");

        AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, L"Configurações");
        SetMenu(hwnd, hMenu);

        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES; // Para barras de progresso, sliders, etc.
    InitCommonControlsEx(&icex);

    const wchar_t CLASS_NAME[] = L"MouseMoverWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Mouse Mover",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    CreateWindow(L"BUTTON", L"Coordenadas", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        50, 50, 120, 30, hwnd, (HMENU)1, hInstance, NULL);

    CreateWindow(L"BUTTON", L"Iniciar", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 50, 120, 30, hwnd, (HMENU)2, hInstance, NULL);

    CreateWindow(L"BUTTON", L"Limpar Coordenadas", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        50, 100, 120, 30, hwnd, (HMENU)3, hInstance, NULL);

    hwndList = CreateWindow(L"LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
        50, 150, 300, 200, hwnd, NULL, hInstance, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
