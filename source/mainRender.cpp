#define NOMINMAX
#include "tgaimage.h"
#include "model.h"
#include"my_gl.h"
#include"createwindow.h"
#include"globals.h"

HWND renderwindow = 0;
const char* g_filePath = nullptr;
int width;
int height;
TGAImage* screen;
Vec3f cameracoord;
Vec3f center;
Vec3f position;
bool bculling = true;
Vec3f light;

struct complexshader
{
	vertex_shader* vs;
	frangment_shader* fs;
	HBITMAP map;

	~complexshader() {
		delete vs;
		delete fs;
		if (map) DeleteObject(map);
	}
};

void setmodel(complexshader* a);
void unloadmodel(complexshader* a);
void settex(complexshader* a);

LRESULT CALLBACK renderProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static float startlx = 0;
	static float startly = 0;
	static float startrx = 0;
	static float startry = 0;
	switch (uMsg)
	{
	case WM_LBUTTONDOWN: {
		startlx = (int)(short)LOWORD(lParam);
		startly = (int)(short)HIWORD(lParam);
		SetCapture(hwnd);
		break;
	}
	case WM_LBUTTONUP: {
		ReleaseCapture();
		break;
	}
	case WM_MOUSEMOVE: {
		if ((wParam & MK_LBUTTON) && (GetCapture() == hwnd)) {
			int currentX = (int)(short)LOWORD(lParam);
			int currentY = (int)(short)HIWORD(lParam);
			RECT rect;
			GetClientRect(hwnd, &rect);
			int screenWidth = rect.right - rect.left;
			int screenHeight = rect.bottom - rect.top;

			float movex = (currentX - startlx) / float(screenWidth);
			float movey = (currentY - startly) / float(screenHeight);

			cameracoord = CameraMoveByMouse(movex, movey, 180.0f, 0, cameracoord, center);
			startlx = currentX;
			startly = currentY;
		}
		else if ((wParam & MK_RBUTTON) && (GetCapture() == hwnd)) {
			// 鼠标移动时处理
			int currentX = (int)(short)LOWORD(lParam);
			int currentY = (int)(short)HIWORD(lParam);

			// 获取当前窗口大小，进行归一化处理
			RECT rect;
			GetClientRect(hwnd, &rect);
			int screenWidth = rect.right - rect.left;
			int screenHeight = rect.bottom - rect.top;

			// 根据鼠标的相对移动来调整 center（相机旋转中心）
			float movex = (currentX - startrx) / float(screenWidth);
			float movey = (currentY - startry) / float(screenHeight);

			// 更新相机的旋转中心 position
			center.x += movex * 1.0f;  // 水平调整旋转中心
			center.y -= movey * 1.0f;  // 垂直调整旋转中心

			// 更新起始位置，继续跟踪鼠标
			startrx = currentX;
			startry = currentY;
		}
		break;
	}
	case WM_RBUTTONDOWN: {
		// 获取鼠标按下的位置
		startrx = (int)(short)LOWORD(lParam);
		startry = (int)(short)HIWORD(lParam);
		SetCapture(hwnd);  // 捕获鼠标
		break;
	}
	case WM_RBUTTONUP: {
		ReleaseCapture();  // 释放鼠标捕获
		break;
	}
	case WM_KEYDOWN: {
		switch (wParam)
		{
		case VK_UP: {
			position.x += 0.1f;
			break;
		}
		case VK_DOWN: {
			position.x -= 0.1f;
			break;
		}
		case VK_LEFT: {
			position.y += 0.1f;
			break;
		}
		case VK_RIGHT: {
			position.y -= 0.1f;
			break;
		}
		case VK_NUMPAD1: {
			position.z += 0.1f;
			break;
		}
		case VK_NUMPAD2: {
			position.z -= 0.1f;
			break;
		}
		case VK_SPACE: {
			position = { 0.0f,0.0f,0.0f };
			center = position;
			break;
		}
		default:
			break;
		}
		break;
	}
	case WM_MOUSEWHEEL: {
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (zDelta > 0) {
			cameracoord = CameraMoveByMouse(0, 0, 0.1f, -0.1f, cameracoord, center);
		}
		else if (zDelta < 0) {
			cameracoord = CameraMoveByMouse(0, 0, -0.1f, 0.1f, cameracoord, center);
		}
		break;
	}
	case WM_COMMAND:
	{
		int mark = LOWORD(wParam);
		switch (mark)
		{
		case IDM_FILE_OPENMODEL:
			OpenFileDialog(hwnd);
			if (g_filePath) {
				setmodel((complexshader*)GetWindowLongPtr(hwnd, GWLP_USERDATA));
			}
			break;
		case IDM_FILE_UNLOADMODEL: {
			unloadmodel((complexshader*)GetWindowLongPtr(hwnd, GWLP_USERDATA));
			break;
		}
		case IDM_FILE_OPENTEXTURE: {
			OpenFileDialog(hwnd);
			complexshader* temp = nullptr;
			if (g_filePath) {
				temp = (complexshader*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if (temp && temp->vs) {
					settex(temp);
				}
				else {
					MessageBox(hwnd, L"请先载入模型！", L"对不起", MB_OK);
				}
			}
			break;
		}
		case IDM_SETTING_BCULLING: {
			bculling = !bculling;
			UpdateBackfaceCullingMenu(GetSubMenu(GetMenu(hwnd), 1), bculling);
			break;
		}
		default:
			break;
		}
		break;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		HBITMAP bmap = ((complexshader*)GetWindowLongPtr(renderwindow, GWLP_USERDATA))->map;
		if (bmap) {
			// 创建内存 DC 用于双缓冲
			HDC hdcMem = CreateCompatibleDC(hdc);
			SelectObject(hdcMem, bmap);

			// 获取客户区尺寸
			RECT rect;
			GetClientRect(hwnd, &rect);

			// 绘制位图（居中显示）
			int bmpWidth = width;
			int bmpHeight = height;
			int x = (rect.right - bmpWidth) / 2;
			int y = (rect.bottom - bmpHeight) / 2;
			BitBlt(hdc, x, y, bmpWidth, bmpHeight, hdcMem, 0, 0, SRCCOPY);

			DeleteDC(hdcMem);
		}
		else {
			DefWindowProc(hwnd, WM_PAINT, (WPARAM)hdc, 0);
		}

		EndPaint(hwnd, &ps);
		break;
	}
	case WM_SIZE: {
		if (screen) {
			delete(screen);
			screen = nullptr;
		}
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		width = std::max(width, height);
		height = width;
		screen = new TGAImage(width, height, 4);
		break;
	}
	case WM_CLOSE: {
		screen->write_tga_file("test.tga");
		if (MessageBox(hwnd, L"确认退出吗？", L"提示", MB_YESNO) == IDYES) {
			DestroyWindow(hwnd);
		}
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
		break;
	}
}

void setmodel(complexshader* a) {
	delete(a->vs);
	delete(a->fs);
	cameracoord = { 0.0f,0.0f,3.0f };
	center = { 0.0f,0.0f,0.0f };
	position = { 0.0f,0.0f,0.0f };
	vertex_shader* temp = new vertex_shader(g_filePath, false);
	frangment_shader* temp2 = new frangment_shader(temp->MVPtrans(position, Vec3f(0, 0, 0), Vec3f(0, 0, 0), cameracoord, center, -1, 1, -1, 1, 1.0f, 100.0f));
	a->vs = temp;
	a->fs = temp2;
}
void unloadmodel(complexshader* a) {
	delete(a->vs);
	delete(a->fs);
	a->vs = nullptr;
	a->fs = nullptr;
	screen->clear();
	g_filePath = nullptr;
}
void settex(complexshader* a) {
	a->vs->getmodel().settex(g_filePath);
}

int windowinit(HINSTANCE hInstance, int nCmdShow) {
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = renderProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"renderwin";
	wc.hIconSm = NULL;

	complexshader* a = new complexshader();

	RegisterClassEx(&wc);
	HMENU menu = createmianmenu();
	renderwindow = CreateWindowEx(
		0,
		L"renderwin",
		L"test",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		menu,
		hInstance,
		NULL);
	if (renderwindow == NULL) {
		return 1;
	}
	SetWindowLongPtr(renderwindow, GWLP_USERDATA, (LONG_PTR)a);
	ShowWindow(renderwindow, nCmdShow);
	UpdateWindow(renderwindow);
}

void Run(Vec3f light) {
	MSG msg = { 0 };
	float* _zbuffer = new float[2600 * 1500];
	constexpr float minuesmax = -std::numeric_limits<float>::max();
	for (int i = (2600 * 1500) - 1; i >= 0; i--)_zbuffer[i] = minuesmax;
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			complexshader* temp = (complexshader*)GetWindowLongPtr(renderwindow, GWLP_USERDATA);
			if (temp && temp->vs && temp->fs) {
				std::fill(_zbuffer, _zbuffer + (2600 * 1500), 2);
				screen->clear();
				temp->fs->vertex = temp->vs->MVPtrans(position, Vec3f(0, 0, 0), Vec3f(0, 0, 0), cameracoord, center, -1, 1, -1, 1, 1.0f, 100.0f);
				temp->fs->original_coords = temp->fs->vertex.vertex_coord;
				temp->fs->drawcall(width, height, _zbuffer, *screen, temp->vs->gettex(), light);
				if (temp->map) {
					DeleteObject(temp->map);
					temp->map = nullptr;
				}
				temp->map = CreateBitmapFromTga(*screen, width, height);
				InvalidateRect(renderwindow, NULL, FALSE);
			}
			else if (temp) {
				if (temp->map) {
					DeleteObject(temp->map);
					temp->map = nullptr;
				}
				InvalidateRect(renderwindow, NULL, TRUE);

			}
		}
	}
}

int  WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hRevInstance, _In_ PSTR pCmdLine, _In_ int nCmdShow) {
	windowinit(hInstance, nCmdShow);

	light = { 0,0,-1 };
	light.normalize();

	Run(light);
	return 0;
}