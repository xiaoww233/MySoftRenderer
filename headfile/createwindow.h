#ifndef CREATEWINDOW_H
#define CREATEWINDOW_H

#include<Windows.h>
#include <commdlg.h> 
#include <shlwapi.h>
#pragma comment(lib, "comdlg32.lib")

extern const char* g_filePath;

#define IDM_FILE_OPENMODEL  1001
#define IDM_FILE_OPENTEXTURE 1002
#define IDM_FILE_UNLOADMODEL 1003
#define IDM_HELP_ABOUT 2001

HMENU createmianmenu() {
	HMENU hmenu = CreateMenu();
	HMENU hfilemenu = CreatePopupMenu();
	AppendMenu(hfilemenu, MF_STRING, IDM_FILE_OPENMODEL, L"导入模型");
	AppendMenu(hfilemenu, MF_STRING, IDM_FILE_OPENTEXTURE, L"导入纹理");
	AppendMenu(hfilemenu, MF_STRING, IDM_FILE_UNLOADMODEL, L"卸载模型");
	AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hfilemenu, L"文件");
	return hmenu;
}

const char* WideToMultiByte(const wchar_t* wideStr) {
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
	char* buffer = new char[bufferSize];
	WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, buffer, bufferSize, nullptr, nullptr);
	return buffer;
}

void OpenFileDialog(HWND hwnd) {
	OPENFILENAMEW ofn = { 0 };
	wchar_t szFile[MAX_PATH] = { 0 };

	// 配置对话框
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = hwnd; // 父窗口句柄
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.txt\0*.obj\0*.tga\0"; // 文件类型过滤器
	ofn.nFilterIndex = 1; // 默认选中第一个过滤器
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER; // 关键标志

	if (GetOpenFileName(&ofn)) {
		// 转换并保存路径
		g_filePath = WideToMultiByte(szFile);
	}
	else {
		DWORD err = CommDlgExtendedError();
		if (err != 0) {
			wchar_t errorMsg[256];
			swprintf_s(errorMsg, L"错误代码: 0x%08X", err);
			MessageBoxW(hwnd, errorMsg, L"文件对话框错误", MB_ICONERROR);
		}
	}
}

void PrintLastError() {
	// 获取最后一次的错误代码
	DWORD errorCode = GetLastError();

	// 用于存储错误信息的缓冲区
	wchar_t* lpMsgBuf = nullptr;

	// 使用 FormatMessage 函数将错误代码转换为错误信息
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf,
		0, NULL);

	// 构造要显示的消息字符串
	wchar_t message[1024];
	swprintf_s(message, sizeof(message) / sizeof(wchar_t), L"Error Code: %lu\nError Message: %s", errorCode, lpMsgBuf);

	// 使用 MessageBox 显示错误信息
	MessageBoxW(NULL, message, L"Error Information", MB_OK | MB_ICONERROR);

	// 释放缓冲区
	LocalFree(lpMsgBuf);
}

HBITMAP CreateBitmapFromTga(TGAImage& image,int width,int height) {
	const uint8_t* pixels = image.buffer();

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;  // 负值表示从上到下的像素排列
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;     // 32 位颜色（BGRA）
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hdc = GetDC(nullptr);  // 获取屏幕 DC
	uint8_t* pBits = nullptr;
	HBITMAP hBitmap = CreateDIBSection(
		hdc,            // 设备上下文
		&bmi,           // 位图信息
		DIB_RGB_COLORS, // 颜色格式
		(void**)&pBits, // 接收像素数据指针
		nullptr,         // 不使用文件映射
		0               // 保留字段
	);
	ReleaseDC(nullptr, hdc);  // 释放 DC

	if (hBitmap && pBits) {
		memcpy(pBits, pixels, width * height * 4);  // 32 位 = 4 字节/像素
	}
	
	return hBitmap;
}

// 记录相机的水平角度（theta）和垂直角度（phi）
static float theta = 0.0f;  // 水平角度，绕 Y 轴旋转
static float phi = 0.0f;    // 垂直角度，绕 X 轴旋转
static float radius = 2.0f; // 相机到目标点的距离

// 修改后的 CameraMoveByMouse 使用球坐标方式
Vec3f CameraMoveByMouse(float delta_x, float delta_y, float sensitivity, float movespeed,Vec3f cameracoord, Vec3f center) {
	// 1. 更新球坐标角度
	theta += delta_x * sensitivity;  // 水平增量控制绕 Y 轴旋转
	phi -= delta_y * sensitivity;    // 垂直增量控制绕 X 轴旋转（负号调整方向）

	// 2. 限制 phi，防止摄像机绕 X 轴旋转过头（产生万向节锁）
	if (phi > 89.9f) phi = 89.9f;
	if (phi < -89.9f) phi = -89.9f;

	// 3. 将球坐标转换为笛卡尔坐标
	float radTheta = theta * 3.1415926f / 180.0f;  // 转为弧度
	float radPhi = phi * 3.1415926f / 180.0f;

	radius += movespeed;
	// 计算新的相机位置
	float x = radius * cos(radPhi) * sin(radTheta);
	float y = -radius * sin(radPhi);  // y 方向的旋转
	float z = radius * cos(radPhi) * cos(radTheta);

	// 4. 返回新的相机坐标（与目标点中心的相对位置）
	cameracoord = center + Vec3f(x, y, z);
	return cameracoord;
}
#endif // !CREATEWINDOW_H

