#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

// ── Colors ──────────────────────────────────────────────────────────────────
#define CLR_BG          RGB(18,  18,  20)
#define CLR_PANEL       RGB(26,  26,  30)
#define CLR_BORDER      RGB(45,  45,  52)
#define CLR_ACCENT      RGB(118, 185,  0)   // NVIDIA green
#define CLR_ACCENT_DIM  RGB(70,  110,  0)
#define CLR_TEXT        RGB(230, 230, 235)
#define CLR_SUBTEXT     RGB(140, 140, 150)
#define CLR_ERROR       RGB(220,  50,  50)
#define CLR_BAR_BG      RGB(35,  35,  40)

// ── IDs ─────────────────────────────────────────────────────────────────────
#define ID_TIMER        1001
#define ID_BTN_INSTALL  2001
#define ID_BTN_CANCEL   2002
#define ID_BTN_CLOSE    2003

// ── Window size ──────────────────────────────────────────────────────────────
#define WIN_W  620
#define WIN_H  420

// ── State ────────────────────────────────────────────────────────────────────
typedef enum { STATE_WELCOME, STATE_INSTALLING, STATE_ERROR } AppState;

static AppState  g_state       = STATE_WELCOME;
static int       g_progress    = 0;          // 0-100
static int       g_logLine     = 0;
static HWND      g_hwnd        = NULL;
static HFONT     g_fontTitle   = NULL;
static HFONT     g_fontMono    = NULL;
static HFONT     g_fontUI      = NULL;
static HFONT     g_fontSmall   = NULL;
static HBRUSH    g_brushBg     = NULL;
static HBRUSH    g_brushPanel  = NULL;

static const char *g_logLines[] = {
    "[0.001]  INIT      MFRTX Installer v0.0.1 starting...",
    "[0.045]  SYSTEM    Checking OS: Windows 10/11 detected.",
    "[0.102]  HARDWARE  Scanning PCI-e bus for NVIDIA Silicon...",
    "[0.251]  FOUND     NVIDIA GPU detected.",
    "[0.312]  COMPAT    Device architecture: verified.",
    "[0.460]  EXTRACT   Unpacking Metal-RTX kernel binaries...",
    "[1.203]  DEPLOY    Writing MFRTX.sys to System32\\drivers...",
    "[1.554]  REGISTRY  Registering MFRTX_Service...",
    "[1.901]  CONNECT   Contacting MFRTX authentication servers...",
    "[2.450]  CONNECT   Retrying... (attempt 2/3)",
    "[3.100]  CONNECT   Retrying... (attempt 3/3)",
    "[3.800]  ERROR     Cannot reach MFRTX servers.",
};
#define LOG_COUNT  (sizeof(g_logLines)/sizeof(g_logLines[0]))

// ── Forward declarations ──────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void OnPaint(HWND hwnd);
static void DrawWelcome(HDC hdc, RECT *rc);
static void DrawInstalling(HDC hdc, RECT *rc);
static void DrawError(HDC hdc, RECT *rc);
static void DrawProgressBar(HDC hdc, int x, int y, int w, int h, int pct);
static void SetBtnState(HWND hwnd);

// ────────────────────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
    (void)hPrev; (void)lpCmd;

    // Init common controls
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icc);

    // Brushes
    g_brushBg    = CreateSolidBrush(CLR_BG);
    g_brushPanel = CreateSolidBrush(CLR_PANEL);

    // Fonts
    g_fontTitle = CreateFont(28, 0, 0, 0, FW_BOLD, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    g_fontUI    = CreateFont(14, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    g_fontMono  = CreateFont(13, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas");
    g_fontSmall = CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");

    // Register class
    WNDCLASSEX wc = {0};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hbrBackground = g_brushBg;
    wc.lpszClassName = "MFRTX_Installer";
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(&wc);

    // Center window
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int wx = (sw - WIN_W) / 2;
    int wy = (sh - WIN_H) / 2;

    g_hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        "MFRTX_Installer",
        "MFRTX Setup",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        wx, wy, WIN_W, WIN_H,
        NULL, NULL, hInst, NULL
    );

    ShowWindow(g_hwnd, nShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(g_brushBg);
    DeleteObject(g_brushPanel);
    DeleteObject(g_fontTitle);
    DeleteObject(g_fontUI);
    DeleteObject(g_fontMono);
    DeleteObject(g_fontSmall);
    return (int)msg.wParam;
}

// ────────────────────────────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // ── Install button ────────────────────────────────────────────────
        CreateWindow("BUTTON", "Install",
            WS_CHILD | WS_VISIBLE | BS_FLAT,
            WIN_W - 230, WIN_H - 62, 100, 34,
            hwnd, (HMENU)ID_BTN_INSTALL, NULL, NULL);

        // ── Cancel button ────────────────────────────────────────────────
        CreateWindow("BUTTON", "Cancel",
            WS_CHILD | WS_VISIBLE | BS_FLAT,
            WIN_W - 120, WIN_H - 62, 100, 34,
            hwnd, (HMENU)ID_BTN_CANCEL, NULL, NULL);

        // Style buttons via subclass is complex; we'll just paint over
        SetBtnState(hwnd);
        return 0;
    }

    case WM_CTLCOLORBTN:
    {
        HDC hdc = (HDC)wp;
        SetBkColor(hdc, CLR_PANEL);
        SetTextColor(hdc, CLR_TEXT);
        return (LRESULT)g_brushPanel;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wp);
        if (id == ID_BTN_INSTALL && g_state == STATE_WELCOME) {
            g_state    = STATE_INSTALLING;
            g_progress = 0;
            g_logLine  = 0;
            SetBtnState(hwnd);
            // Timer: fires every 350ms to advance progress
            SetTimer(hwnd, ID_TIMER, 350, NULL);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if ((id == ID_BTN_CANCEL || id == ID_BTN_CLOSE)) {
            DestroyWindow(hwnd);
        }
        return 0;
    }

    case WM_TIMER:
    {
        if (g_state != STATE_INSTALLING) break;

        // Advance log lines
        if (g_logLine < (int)LOG_COUNT) g_logLine++;

        // Progress: map log lines to percentage
        // Lines 0-8 go 0→85, lines 9-10 slow, 11 = error
        if (g_logLine < 9)
            g_progress = (int)(g_logLine * (85.0f / 9.0f));
        else if (g_logLine < 12)
            g_progress = 85 + (g_logLine - 9) * 2;
        else
            g_progress = 91;

        if (g_logLine >= (int)LOG_COUNT) {
            // Show error
            KillTimer(hwnd, ID_TIMER);
            g_state = STATE_ERROR;
            SetBtnState(hwnd);
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        OnPaint(hwnd);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1; // we handle background in WM_PAINT

    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wp;
        SetBkColor(hdc, CLR_BG);
        SetTextColor(hdc, CLR_TEXT);
        return (LRESULT)g_brushBg;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// ────────────────────────────────────────────────────────────────────────────
static void OnPaint(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    // Double-buffer
    HDC hdc    = GetDC(hwnd);
    HDC memDC  = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    SelectObject(memDC, bmp);

    // Background
    FillRect(memDC, &rc, g_brushBg);

    // Top accent bar (thin green line at very top)
    HBRUSH accentBrush = CreateSolidBrush(CLR_ACCENT);
    RECT topBar = {0, 0, rc.right, 3};
    FillRect(memDC, &topBar, accentBrush);
    DeleteObject(accentBrush);

    // Header panel
    RECT hdrRect = {0, 3, rc.right, 80};
    FillRect(memDC, &hdrRect, g_brushPanel);

    // Border under header
    HPEN borderPen = CreatePen(PS_SOLID, 1, CLR_BORDER);
    HPEN oldPen    = SelectObject(memDC, borderPen);
    MoveToEx(memDC, 0, 80, NULL);
    LineTo(memDC, rc.right, 80);
    SelectObject(memDC, oldPen);
    DeleteObject(borderPen);

    // Title
    SelectObject(memDC, g_fontTitle);
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, CLR_TEXT);
    TextOut(memDC, 24, 16, "MFRTX", 5);

    // Green "M" accent dot
    HBRUSH gb = CreateSolidBrush(CLR_ACCENT);
    RECT dot = {20, 14, 22, 16};
    (void)dot;
    DeleteObject(gb);

    // Subtitle
    SelectObject(memDC, g_fontSmall);
    SetTextColor(memDC, CLR_SUBTEXT);
    TextOut(memDC, 24, 50, "Metal API Driver Suite  |  v0.0.1 Closed Dev Beta", 49);

    // Version badge (small pill)
    HBRUSH dimBrush = CreateSolidBrush(CLR_ACCENT_DIM);
    RECT badge = {rc.right - 200, 18, rc.right - 20, 38};
    FillRect(memDC, &badge, dimBrush);
    DeleteObject(dimBrush);
    SelectObject(memDC, g_fontSmall);
    SetTextColor(memDC, CLR_ACCENT);
    SetBkMode(memDC, TRANSPARENT);
    DrawText(memDC, "SERVERS: DOWN", -1, &badge, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Content area
    RECT content = {0, 80, rc.right, rc.bottom - 70};

    switch (g_state) {
    case STATE_WELCOME:    DrawWelcome(memDC, &content);    break;
    case STATE_INSTALLING: DrawInstalling(memDC, &content); break;
    case STATE_ERROR:      DrawError(memDC, &content);      break;
    }

    // Bottom bar
    RECT botBar = {0, rc.bottom - 70, rc.right, rc.bottom};
    FillRect(memDC, &botBar, g_brushPanel);

    HPEN bPen = CreatePen(PS_SOLID, 1, CLR_BORDER);
    HPEN bOld = SelectObject(memDC, bPen);
    MoveToEx(memDC, 0, rc.bottom - 70, NULL);
    LineTo(memDC, rc.right, rc.bottom - 70);
    SelectObject(memDC, bOld);
    DeleteObject(bPen);

    // Footer text
    SelectObject(memDC, g_fontSmall);
    SetTextColor(memDC, CLR_SUBTEXT);
    TextOut(memDC, 20, rc.bottom - 40, "The MFRTX Project  |  github.com/The-MFRTX-Project", 51);

    // Blit
    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
    DeleteObject(bmp);
    DeleteDC(memDC);
    ReleaseDC(hwnd, hdc);
}

// ────────────────────────────────────────────────────────────────────────────
static void DrawWelcome(HDC hdc, RECT *rc)
{
    int x = 40, y = rc->top + 30;

    SelectObject(hdc, g_fontUI);
    SetTextColor(hdc, CLR_TEXT);
    SetBkMode(hdc, TRANSPARENT);

    TextOut(hdc, x, y, "Welcome to the MFRTX Driver Setup Wizard.", 41);
    y += 30;

    SetTextColor(hdc, CLR_SUBTEXT);
    TextOut(hdc, x, y, "This wizard will install the MFRTX driver suite on your system.", 63);
    y += 18;
    TextOut(hdc, x, y, "Please ensure you have Administrator privileges before proceeding.", 65);
    y += 40;

    // Info box
    RECT infoBox = {x, y, rc->right - 40, y + 90};
    HBRUSH infoBrush = CreateSolidBrush(RGB(28, 30, 34));
    FillRect(hdc, &infoBox, infoBrush);
    DeleteObject(infoBrush);

    HPEN infoPen = CreatePen(PS_SOLID, 1, CLR_BORDER);
    HPEN oldP    = SelectObject(hdc, infoPen);
    Rectangle(hdc, infoBox.left, infoBox.top, infoBox.right, infoBox.bottom);
    SelectObject(hdc, oldP);
    DeleteObject(infoPen);

    SelectObject(hdc, g_fontSmall);
    SetTextColor(hdc, CLR_SUBTEXT);
    int ix = x + 16, iy = y + 14;
    TextOut(hdc, ix, iy, "Supported:  NVIDIA GTX (2018+)  |  RTX 20 / 30 / 40 series", 58);
    iy += 20;
    TextOut(hdc, ix, iy, "Requires:   Windows 10 / 11  (64-bit)", 37);
    iy += 20;
    TextOut(hdc, ix, iy, "Note:       An internet connection is required for server auth.", 61);
}

// ────────────────────────────────────────────────────────────────────────────
static void DrawInstalling(HDC hdc, RECT *rc)
{
    int x = 30, y = rc->top + 20;

    SelectObject(hdc, g_fontUI);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, CLR_TEXT);
    TextOut(hdc, x, y, "Installing MFRTX Driver Suite...", 32);
    y += 30;

    // Progress bar
    DrawProgressBar(hdc, x, y, WIN_W - 60, 18, g_progress);
    y += 30;

    // Percentage
    char pctStr[16];
    sprintf(pctStr, "%d%%", g_progress);
    SelectObject(hdc, g_fontSmall);
    SetTextColor(hdc, CLR_SUBTEXT);
    TextOut(hdc, x, y, pctStr, (int)strlen(pctStr));
    y += 24;

    // Log panel
    RECT logBox = {x, y, WIN_W - 30, rc->bottom - 10};
    HBRUSH logBrush = CreateSolidBrush(RGB(13, 13, 15));
    FillRect(hdc, &logBox, logBrush);
    DeleteObject(logBrush);

    HPEN lp = CreatePen(PS_SOLID, 1, CLR_BORDER);
    HPEN op = SelectObject(hdc, lp);
    Rectangle(hdc, logBox.left, logBox.top, logBox.right, logBox.bottom);
    SelectObject(hdc, op);
    DeleteObject(lp);

    SelectObject(hdc, g_fontMono);
    int ly = logBox.top + 8;
    for (int i = 0; i < g_logLine && i < (int)LOG_COUNT; i++) {
        // Color code: ERROR lines red, CONNECT lines yellow, rest green
        if (strstr(g_logLines[i], "ERROR"))
            SetTextColor(hdc, CLR_ERROR);
        else if (strstr(g_logLines[i], "CONNECT"))
            SetTextColor(hdc, RGB(220, 180, 50));
        else if (strstr(g_logLines[i], "FOUND") || strstr(g_logLines[i], "SUCCESS"))
            SetTextColor(hdc, CLR_ACCENT);
        else
            SetTextColor(hdc, CLR_SUBTEXT);

        TextOut(hdc, logBox.left + 10, ly, g_logLines[i], (int)strlen(g_logLines[i]));
        ly += 16;
    }
}

// ────────────────────────────────────────────────────────────────────────────
static void DrawError(HDC hdc, RECT *rc)
{
    int x = 40, y = rc->top + 28;

    // Red error icon area
    HBRUSH redBrush = CreateSolidBrush(RGB(80, 20, 20));
    RECT errBox = {x, y, WIN_W - 40, y + 60};
    FillRect(hdc, &errBox, redBrush);
    DeleteObject(redBrush);

    HPEN rp = CreatePen(PS_SOLID, 1, CLR_ERROR);
    HPEN rpo = SelectObject(hdc, rp);
    Rectangle(hdc, errBox.left, errBox.top, errBox.right, errBox.bottom);
    SelectObject(hdc, rpo);
    DeleteObject(rp);

    SelectObject(hdc, g_fontUI);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, CLR_ERROR);
    TextOut(hdc, x + 16, y + 10, "Installation Failed", 19);

    SelectObject(hdc, g_fontSmall);
    SetTextColor(hdc, RGB(200, 120, 120));
    TextOut(hdc, x + 16, y + 34, "Cannot connect to MFRTX servers. Driver could not be authenticated.", 66);

    y += 80;

    // Detail text
    SelectObject(hdc, g_fontUI);
    SetTextColor(hdc, CLR_TEXT);
    SetBkMode(hdc, TRANSPARENT);

    const char *line1 = "The MFRTX installer could not reach the authentication";
    const char *line2 = "servers required to complete this installation.";
    TextOut(hdc, x, y, line1, (int)strlen(line1));
    y += 22;
    TextOut(hdc, x, y, line2, (int)strlen(line2));
    y += 36;

    SelectObject(hdc, g_fontSmall);
    SetTextColor(hdc, CLR_SUBTEXT);
    const char *hint = "To check if servers are UP or DOWN, please visit the GitHub page:";
    TextOut(hdc, x, y, hint, (int)strlen(hint));
    y += 20;

    SetTextColor(hdc, RGB(100, 160, 255));
    const char *link = "https://github.com/The-MFRTX-Project/MetalForRTX/tree/main";
    TextOut(hdc, x, y, link, (int)strlen(link));

    // Underline the link
    SIZE sz;
    GetTextExtentPoint32(hdc, link, (int)strlen(link), &sz);
    HPEN ulp = CreatePen(PS_SOLID, 1, RGB(100, 160, 255));
    HPEN ulpo = SelectObject(hdc, ulp);
    MoveToEx(hdc, x, y + sz.cy - 1, NULL);
    LineTo(hdc, x + sz.cx, y + sz.cy - 1);
    SelectObject(hdc, ulpo);
    DeleteObject(ulp);
}

// ────────────────────────────────────────────────────────────────────────────
static void DrawProgressBar(HDC hdc, int x, int y, int w, int h, int pct)
{
    // Background
    HBRUSH bgBrush = CreateSolidBrush(CLR_BAR_BG);
    RECT bgRect = {x, y, x + w, y + h};
    FillRect(hdc, &bgRect, bgBrush);
    DeleteObject(bgBrush);

    // Fill
    if (pct > 0) {
        int fillW = (w * pct) / 100;
        HBRUSH fillBrush = CreateSolidBrush(CLR_ACCENT);
        RECT fillRect = {x, y, x + fillW, y + h};
        FillRect(hdc, &fillRect, fillBrush);
        DeleteObject(fillBrush);
    }

    // Border
    HPEN pen = CreatePen(PS_SOLID, 1, CLR_BORDER);
    HPEN op  = SelectObject(hdc, pen);
    Rectangle(hdc, x, y, x + w, y + h);
    SelectObject(hdc, op);
    DeleteObject(pen);
}

// ────────────────────────────────────────────────────────────────────────────
static void SetBtnState(HWND hwnd)
{
    HWND hInstall = GetDlgItem(hwnd, ID_BTN_INSTALL);
    HWND hCancel  = GetDlgItem(hwnd, ID_BTN_CANCEL);

    switch (g_state) {
    case STATE_WELCOME:
        EnableWindow(hInstall, TRUE);
        SetWindowText(hInstall, "Install");
        SetWindowText(hCancel, "Cancel");
        EnableWindow(hCancel, TRUE);
        break;
    case STATE_INSTALLING:
        EnableWindow(hInstall, FALSE);
        SetWindowText(hInstall, "Installing...");
        SetWindowText(hCancel, "Cancel");
        EnableWindow(hCancel, FALSE);
        break;
    case STATE_ERROR:
        EnableWindow(hInstall, FALSE);
        SetWindowText(hInstall, "Failed");
        SetWindowText(hCancel, "Close");
        EnableWindow(hCancel, TRUE);
        break;
    }
}
