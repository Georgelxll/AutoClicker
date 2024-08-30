#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>  // Para std::numeric_limits

#define IDD_SPEED_DIALOG 101
#define IDC_SPEED_SLIDER 1001
#define IDD_REPEAT_DIALOG 102
#define IDC_REPEAT_EDIT 1002
#define IDC_REPEAT_OK 1003
#define IDC_REPEAT_CANCEL 1004

using namespace std;

struct Coord {
    int x, y;
};

vector<Coord> coordinates;
bool captureCoords = false;
bool repeat = false;
int repeatCount = 1; // Quantidade de repetição padrão
bool isRunning = false;
int clickDelay = 2000;
HWND hwndList;
HANDLE hThread = NULL;  // Handle da thread

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

DWORD WINAPI RepeatLoop(LPVOID lpParam) {
    int count = repeatCount; // Usa a quantidade de repetições definida

    do {
        for (const auto& coord : coordinates) {
            if (!isRunning || count <= 0) {
                return 0;  // Finaliza a thread
            }

            SetCursorPos(coord.x, coord.y);
            Sleep(clickDelay);

            // Clicar com o botão direito
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
        }
        count--; // Decrementa o contador de repetições
    } while (count > 0 && isRunning); // Continua enquanto houver repetições e isRunning for verdadeiro

    MessageBox(NULL, L"Finalizado.", L"Informação", MB_OK | MB_ICONINFORMATION);
    return 0;
}


void StartLoop() {
    if (coordinates.empty()) {
        MessageBox(NULL, L"Você precisa adicionar as coordenadas primeiro.", L"Erro", MB_OK | MB_ICONERROR);
        return;
    }

    if (hThread != NULL) {
        CloseHandle(hThread);
    }

    isRunning = true;
    hThread = CreateThread(NULL, 0, RepeatLoop, NULL, 0, NULL);
}


void ShowSpeedDialog(HWND hwnd) {
    INT_PTR result;
    result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SPEED_DIALOG), hwnd, [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
        static HWND speedSlider;
        switch (message) {
        case WM_INITDIALOG:
            speedSlider = GetDlgItem(hDlg, IDC_SPEED_SLIDER);
            SendMessage(speedSlider, TBM_SETRANGE, TRUE, MAKELPARAM(1, 10)); // Ajusta o intervalo do slider
            SendMessage(speedSlider, TBM_SETPAGESIZE, 0, 1);
            SendMessage(speedSlider, TBM_SETPOS, TRUE, (clickDelay - 500) / 250); // Ajusta a posição inicial do slider
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                int pos = static_cast<int>(SendMessage(speedSlider, TBM_GETPOS, 0, 0));
                clickDelay = 500 + pos * 250; // Ajusta o delay baseado na posição do slider
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

INT_PTR CALLBACK RepeatDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        WCHAR buffer[4];
        swprintf_s(buffer, L"%d", repeatCount);
        SetDlgItemText(hDlg, IDC_REPEAT_EDIT, buffer);
        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_REPEAT_OK:
        {
            WCHAR buffer[4];
            GetDlgItemText(hDlg, IDC_REPEAT_EDIT, buffer, 4);
            int value = _wtoi(buffer);
            if (value >= 1 && value <= 999) {
                repeatCount = value;
                repeat = true;  // Ativa a repetição
                EndDialog(hDlg, IDOK);
            }
            else {
                MessageBox(hDlg, L"Por favor, insira um número entre 1 e 999.", L"Erro", MB_OK | MB_ICONERROR);
            }
        }
        return TRUE;
        case IDC_REPEAT_CANCEL:
            repeat = false;  // Desativa a repetição se o usuário cancelar
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}




LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case 1:  // Botão "Coordenadas"
            captureCoords = true;
            MessageBox(hwnd, L"Aperte F5 para capturar a posição do mouse.", L"Informação", MB_OK | MB_ICONINFORMATION);
            break;
        case 2:  // Botão "Iniciar"
            StartLoop();
            break;
        case 3:  // Botão "Limpar Coordenadas"
            coordinates.clear();
            SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
            MessageBox(hwnd, L"Coordenadas limpas.", L"Informação", MB_OK | MB_ICONINFORMATION);
            break;
        case 4:  // Configurações -> Repetir
            {
                // Exibe a caixa de diálogo para configurar o número de repetições
                if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REPEAT_DIALOG), hwnd, RepeatDialogProc) == IDOK) {
                    MessageBox(hwnd, repeat ? L"Repetição ativada." : L"Repetição desativada.", L"Configuração", MB_OK | MB_ICONINFORMATION);
                }
            }
            break;
        case 5:  // Configurações -> Velocidade
            ShowSpeedDialog(hwnd);
            break;
        case 6:  // Configurações -> Exportar
        {
            OPENFILENAME ofn;
            WCHAR szFile[260] = L"coordenadas.txt";

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
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
        case 7:  // Configurações -> Importar
        {
            OPENFILENAME ofn;
            WCHAR szFile[260] = L"coordenadas.txt";

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
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
            isRunning = false;  // Para o loop de execução
            if (hThread != NULL) {
                WaitForSingleObject(hThread, INFINITE);
                CloseHandle(hThread);
                hThread = NULL;
            }
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

        CreateWindow(L"BUTTON", L"Coordenadas",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 40, 50, 150, 30, hwnd, (HMENU)1,
            GetModuleHandle(NULL), NULL);
        CreateWindow(L"BUTTON", L"Iniciar",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 280, 50, 150, 30, hwnd, (HMENU)2,
            GetModuleHandle(NULL), NULL);
        CreateWindow(L"BUTTON", L"Limpar Coordenadas",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 100, 350, 30, hwnd, (HMENU)3,
            GetModuleHandle(NULL), NULL);

        // Cria a lista de coordenadas
        hwndList = CreateWindowEx(0, WC_LISTBOX, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
            10, 150, 460, 300, hwnd, NULL, GetModuleHandle(NULL), NULL);
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

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEX wcex;
    HWND hwnd;
    MSG msg;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"MainWndClass";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&wcex);

    hwnd = CreateWindow(L"MainWndClass", L"Automatizador de Clique", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 550, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        return FALSE;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
