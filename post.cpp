#include <windows.h>
#include <wininet.h>
#include <commctrl.h>
#include <string>
#include <sstream>
using namespace std;
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
using namespace std;
// 全局变量
HWND 窗口句柄, 网址输入框, 请求数据框, 响应数据框, 发送按钮, 清除按钮, 复制按钮;
HINSTANCE 实例句柄;
const wchar_t* 窗口类名 = L"post请求工具";
// 编码转换辅助函数
wstring 多字节转宽字符(const string& 多字节文本, UINT 代码页 = CP_ACP) {
    if (多字节文本.empty()) return L"";
    int 长度 = MultiByteToWideChar(代码页, 0, 多字节文本.c_str(), -1, NULL, 0);
    wstring 宽字符文本(长度, 0);
    MultiByteToWideChar(代码页, 0, 多字节文本.c_str(), -1, &宽字符文本[0], 长度);
    return 宽字符文本;
}//应该是不行，我不知道为什么转不了码，建议自己研究一下
string 宽字符转多字节(const wstring& 宽字符文本, UINT 代码页 = CP_ACP) {
    if (宽字符文本.empty()) return "";
    int 长度 = WideCharToMultiByte(代码页, 0, 宽字符文本.c_str(), -1, NULL, 0, NULL, NULL);
    string 多字节文本(长度, 0);
    WideCharToMultiByte(代码页, 0, 宽字符文本.c_str(), -1, &多字节文本[0], 长度, NULL, NULL);
    return 多字节文本;
}
// 发送POST请求
string 发送HTTP请求(const wstring& 网址, const string& 请求数据 = "") {
    HINTERNET 会话句柄 = InternetOpenW(L"post请求工具", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!会话句柄) return "无法初始化网络会话";
    DWORD 标志 = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    if (wcscmp(L"https://", 网址.substr(0, 8).c_str()) == 0) {
        标志 |= INTERNET_FLAG_SECURE;
    }
    HINTERNET 连接句柄 = InternetOpenUrlW(会话句柄, 网址.c_str(), NULL, 0, 标志, 0);
    if (!连接句柄) {
        InternetCloseHandle(会话句柄);
        return "傻逼服务器炸了";
    }
    string 响应数据;
    char 缓冲区[4096];
    DWORD 已读取字节数;
    while (InternetReadFile(连接句柄, 缓冲区, sizeof(缓冲区), &已读取字节数) && 已读取字节数 > 0) {
        响应数据.append(缓冲区, 已读取字节数);
    }
    InternetCloseHandle(连接句柄);
    InternetCloseHandle(会话句柄);
    return 响应数据;
}
// 窗口过程
LRESULT CALLBACK 窗口过程(HWND 窗口句柄, UINT 消息, WPARAM w参数, LPARAM l参数) {
    switch (消息) {
    case WM_CREATE: {
        // 创建控件
        网址输入框 = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            10, 10, 600, 25, 窗口句柄, NULL, 实例句柄, NULL);

        请求数据框 = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            10, 45, 290, 300, 窗口句柄, NULL, 实例句柄, NULL);

        响应数据框 = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
            310, 45, 290, 300, 窗口句柄, NULL, 实例句柄, NULL);

        发送按钮 = CreateWindowW(L"BUTTON", L"发送请求", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 355, 100, 30, 窗口句柄, (HMENU)1, 实例句柄, NULL);

        清除按钮 = CreateWindowW(L"BUTTON", L"清除内容", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            120, 355, 100, 30, 窗口句柄, (HMENU)2, 实例句柄, NULL);

        复制按钮 = CreateWindowW(L"BUTTON", L"复制结果", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            230, 355, 100, 30, 窗口句柄, (HMENU)3, 实例句柄, NULL);

        // 设置字体
        HFONT 字体 = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

        SendMessage(网址输入框, WM_SETFONT, (WPARAM)字体, TRUE);
        SendMessage(请求数据框, WM_SETFONT, (WPARAM)字体, TRUE);
        SendMessage(响应数据框, WM_SETFONT, (WPARAM)字体, TRUE);
        SendMessage(发送按钮, WM_SETFONT, (WPARAM)字体, TRUE);
        SendMessage(清除按钮, WM_SETFONT, (WPARAM)字体, TRUE);
        SendMessage(复制按钮, WM_SETFONT, (WPARAM)字体, TRUE);

        break;
    }
    case WM_COMMAND: {
        if (LOWORD(w参数) == 1) { // 发送按钮
            wchar_t 网址缓冲区[1024];
            GetWindowTextW(网址输入框, 网址缓冲区, 1024);
            wstring 网址 = 网址缓冲区;

            if (网址.empty()) {
                MessageBoxW(窗口句柄, L"请输入URL", L"提示", MB_ICONINFORMATION);
                break;
            }

            // 获取请求数据
            wchar_t 请求数据缓冲区[8192];
            GetWindowTextW(请求数据框, 请求数据缓冲区, 8192);
            string 请求数据 = 宽字符转多字节(请求数据缓冲区);

            // 发送请求
            string 响应数据 = 发送HTTP请求(网址, 请求数据);

            // 显示响应
            SetWindowTextW(响应数据框, 多字节转宽字符(响应数据).c_str());
        }
        else if (LOWORD(w参数) == 2) { // 清除按钮
            SetWindowTextW(网址输入框, L"");
            SetWindowTextW(请求数据框, L"");
            SetWindowTextW(响应数据框, L"");
        }
        else if (LOWORD(w参数) == 3) { // 复制按钮
            if (OpenClipboard(窗口句柄)) {
                EmptyClipboard();
                wchar_t 响应数据缓冲区[32768];
                GetWindowTextW(响应数据框, 响应数据缓冲区, 32768);

                HGLOBAL 内存句柄 = GlobalAlloc(GMEM_MOVEABLE, (wcslen(响应数据缓冲区) + 1) * sizeof(wchar_t));
                if (内存句柄) {
                    wchar_t* 数据指针 = (wchar_t*)GlobalLock(内存句柄);
                    wcscpy_s(数据指针, wcslen(响应数据缓冲区) + 1, 响应数据缓冲区);
                    GlobalUnlock(内存句柄);
                    SetClipboardData(CF_UNICODETEXT, 内存句柄);
                }
                CloseClipboard();
            }
        }
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProcW(窗口句柄, 消息, w参数, l参数);
    }
    return 0;
}

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    实例句柄 = hInstance;

    // 初始化通用控件
    INITCOMMONCONTROLSEX 控件结构 = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&控件结构);

    // 注册窗口类
    WNDCLASSEXW 窗口类 = { 0 };
    窗口类.cbSize = sizeof(WNDCLASSEXW);
    窗口类.style = CS_HREDRAW | CS_VREDRAW;
    窗口类.lpfnWndProc = 窗口过程;
    窗口类.hInstance = 实例句柄;
    窗口类.hCursor = LoadCursor(NULL, IDC_ARROW);
    窗口类.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    窗口类.lpszClassName = 窗口类名;

    if (!RegisterClassExW(&窗口类)) {
        MessageBoxW(NULL, L"窗口类注册失败", L"错误", MB_ICONERROR);
        return 1;
    }

    // 创建窗口
    窗口句柄 = CreateWindowExW(0, 窗口类名, L"post请求工具", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, 实例句柄, NULL);

    if (!窗口句柄) {
        MessageBoxW(NULL, L"窗口创建失败", L"错误", MB_ICONERROR);
        return 1;
    }

    // 显示窗口
    ShowWindow(窗口句柄, nCmdShow);
    UpdateWindow(窗口句柄);

    // 消息循环
    MSG 消息;
    while (GetMessageW(&消息, NULL, 0, 0)) {
        TranslateMessage(&消息);
        DispatchMessageW(&消息);
    }

    return (int)消息.wParam;
}
