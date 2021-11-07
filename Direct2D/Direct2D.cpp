/*##################################
Direct2D(수업용)
파일명: Driect2D.cpp (원본프로젝트 및 소스제공: http://vsts2010.tistory.com/) 
작성자: 김홍규 (downkhg@gmail.com)
마지막수정날짜: 2018.08.14
버전: 1.01
###################################*/
// Direct2D.cpp: 응용 프로그램의 진입점을 정의합니다.

#include "stdafx.h"
#include "Direct2D.h"
#include <string.h>
#include <conio.h>
#include <math.h>
//#include <stdio.h>

#define MAX_LOADSTRING 100
#define ANI_SIZE 4
#define _MAX_IMAGE_COUNT_ 1024	//	최대 로드할 수 있는 이미지 갯수.
#define PI 3.141592

class DrawObj;

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

TCHAR** g_ppImageNameArray = new TCHAR * [_MAX_IMAGE_COUNT_];

ID2D1Factory* g_ipD2DFactory = nullptr;
ID2D1HwndRenderTarget* g_ipRT = nullptr;
IWICImagingFactory* g_ipWICFactory = nullptr;
IWICFormatConverter* g_ipConvertedSrcBmp[ANI_SIZE]; //포맷변환기
ID2D1Bitmap* g_ipD2DBitmap[ANI_SIZE]; //비트맵
ID2D1SolidColorBrush* g_ipBlackBrush = nullptr;
ID2D1SolidColorBrush* g_ipRedBrush = nullptr;

D2D1_POINT_2F g_sPoint;
D2D1_POINT_2F g_sPointSize;
int g_iImageCount = 0;
int g_nAniIdx = 0;
float g_fRotAngle;
int timer;
bool CDFlag = false; // 충돌 감지 플래그


// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WindowProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void ManualLoadImage(HWND hWnd, const TCHAR* format, int idx);
void InitializeD2D(void); //D2D초기화
void InitializeRect(HWND hWnd); //랜더링할 뷰포트 타겟 생성
void CreateD2DBitmapFromFile(HWND hWnd, TCHAR* pImageFullPath, int idx); //비트맵디코딩
void Draw();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	AllocConsole(); //콘솔화면 만들기

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DIRECT2D, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 응용 프로그램 초기화를 수행합니다.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DIRECT2D));

    MSG msg;

    // 기본 메시지 루프입니다.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	assert(hr == S_OK);
	FreeConsole(); //콘솔화면 끄기

    return (int) msg.wParam;
}

//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECT2D));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DIRECT2D);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

    switch (message)
    {
	case WM_CREATE:
		InitializeD2D();
		InitializeRect(hWnd);

		for (int i = 0; i < ANI_SIZE; i++)
			ManualLoadImage(hWnd, L"Images\\sonic_%04d.png", i);

		SetTimer(hWnd, 1, 50, NULL); //타이머 시작
		break;
	case WM_TIMER:
		InvalidateRect(hWnd,NULL,FALSE); //타이머 호출시 마다, 윈도우영역을 초기화
		break;
	case WM_KEYDOWN:
		// 동시키가능
		if (GetAsyncKeyState(VK_UP))  g_sPoint.y -= 2; 
		if (GetAsyncKeyState(VK_DOWN))  g_sPoint.y += 2; 
		if (GetAsyncKeyState(VK_LEFT))  g_sPoint.x -= 2; 
		if (GetAsyncKeyState(VK_RIGHT))  g_sPoint.x += 2; 

		/*switch (wParam) {
		case VK_LEFT:
			g_sPoint.x -=2;
			break;
		case VK_RIGHT:
			g_sPoint.x += 2;
			break;
		case VK_UP:
			g_sPoint.y -= 2;
			break;
		case VK_DOWN:
			g_sPoint.y +=  2;
			break;
		case VK_SPACE:
			
			break;
		}*/
		return 0;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
	
		Draw();

		EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void InitializeD2D(void)
{
	HRESULT hr = E_FAIL;

	//-----------------------------------------------------------------------
	//	D2D Factory 를 생성한다.
	//-----------------------------------------------------------------------
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,&g_ipD2DFactory);
	assert(hr == S_OK);

	//-----------------------------------------------------------------------
	//	Windows Imaging Component Factory 를 생성한다.
	//-----------------------------------------------------------------------
	hr = CoCreateInstance(CLSID_WICImagingFactory,
		NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&g_ipWICFactory));

	assert(hr == S_OK);

}

void CreateD2DBitmapFromFile(HWND hWnd, TCHAR* pImageFullPath, int idx)
{

	assert(pImageFullPath != nullptr);
	assert(g_ipWICFactory != nullptr);
	assert(g_ipRT != nullptr);

	HRESULT hr = E_FAIL;

	//----------------------------------------------------------------
	//	디코더를 생성한다.
	//----------------------------------------------------------------
	IWICBitmapDecoder* ipDecoderPtr = nullptr;
	hr = g_ipWICFactory->CreateDecoderFromFilename(pImageFullPath, nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&ipDecoderPtr);
	assert(hr == S_OK);

	//----------------------------------------------------------------
	//	디코더에서 프레임을 얻는다.
	//----------------------------------------------------------------
	IWICBitmapFrameDecode* ipFramePtr = nullptr;
	hr = ipDecoderPtr->GetFrame(0, &ipFramePtr);
	assert(hr == S_OK);


	//----------------------------------------------------------------
	//	프레임을 기반으로 해서 포맷 컨버터를 만든다.
	//----------------------------------------------------------------
	SafeRelease(g_ipConvertedSrcBmp[idx]);
	hr = g_ipWICFactory->CreateFormatConverter(&g_ipConvertedSrcBmp[idx]);
	assert(hr == S_OK);


	hr = g_ipConvertedSrcBmp[idx]->Initialize(ipFramePtr, GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom);
	assert(hr == S_OK);

	//----------------------------------------------------------------
	//	컨버트된 데이터를 기반으로 해서 실제 비트맵을 만든다.
	//----------------------------------------------------------------
	SafeRelease(g_ipD2DBitmap[idx]);
	hr = g_ipRT->CreateBitmapFromWicBitmap(g_ipConvertedSrcBmp[idx], nullptr, &g_ipD2DBitmap[idx]);
	assert(hr == S_OK);

	SafeRelease(ipDecoderPtr);
	SafeRelease(ipFramePtr);
}

class DrawObj 
{
	D2D1::Matrix3x2F omatT;
	D2D1::Matrix3x2F omatR;
	D2D1::Matrix3x2F omatS;
	D2D1::Matrix3x2F omatTrans; // = matT * matR * matS;

	D2D1_POINT_2F osTL;
	D2D1_POINT_2F osTR;
	D2D1_POINT_2F osBL;
	D2D1_POINT_2F osBR;
	D2D1_POINT_2F osCenter;
	D2D1_POINT_2F osRectTL;
	D2D1_POINT_2F osRectBR;
	D2D1_RECT_F osArea;

public: 
	DrawObj() {}

	D2D1::Matrix3x2F GetomatT() { return omatT; }

	void Draw(float _px, float _py, float _sx, float _sy) {
		omatT = D2D1::Matrix3x2F::Translation(_px, _py);
		omatR = D2D1::Matrix3x2F::Rotation(0, D2D1::Point2F(_sx, _sy) * omatT);
		omatS = D2D1::Matrix3x2F::Scale(1,1);
		omatTrans = omatT * omatR * omatS;

		osTL = D2D1::Point2F(0, 0);
		osTR = D2D1::Point2F(0 + _sx, 0);
		osBL = D2D1::Point2F(0, 0 + _sy);
		osBR = D2D1::Point2F(0 + _sx, 0 + _sy);
		osCenter = D2D1::Point2F( _sx / 2,  _sy / 2);

		osRectTL = osTL * omatT;
		osRectBR = osBR * omatT;
		osArea = D2D1::RectF(osRectTL.x, osRectTL.y, osRectBR.x, osRectBR.y);
		g_ipRT->DrawBitmap(g_ipD2DBitmap[g_nAniIdx], osArea);

		g_ipRT->FillEllipse(D2D1::Ellipse(osTL * omatTrans, 2.0f, 2.0), g_ipBlackBrush);
		g_ipRT->FillEllipse(D2D1::Ellipse(osTR * omatTrans, 2.0f, 2.0), g_ipBlackBrush);
		g_ipRT->FillEllipse(D2D1::Ellipse(osBL * omatTrans, 2.0f, 2.0), g_ipBlackBrush);
		g_ipRT->FillEllipse(D2D1::Ellipse(osBR * omatTrans, 2.0f, 2.0), g_ipBlackBrush);

		g_ipRT->DrawLine(osTL * omatTrans, osTR * omatTrans, g_ipBlackBrush);
		g_ipRT->DrawLine(osTL * omatTrans, osBL * omatTrans, g_ipBlackBrush);
		g_ipRT->DrawLine(osBR * omatTrans, osTR * omatTrans, g_ipBlackBrush);
		g_ipRT->DrawLine(osBR * omatTrans, osBL * omatTrans, g_ipBlackBrush);
	}

	bool CollisionDetect(DrawObj target) {
		if (fabs(omatT.dx - target.GetomatT().dx) < g_sPointSize.x
			&& fabs(omatT.dy - target.GetomatT().dy) < g_sPointSize.y)
		{ // fabs : (실수끼리 계산) 절대값 반환
			return true;
		}
		else return false;
	}

	~DrawObj() { }
};

DrawObj player;
DrawObj child;
DrawObj target1;

void Draw()
{
	HRESULT hr = E_FAIL;
	g_ipRT->BeginDraw();
	g_ipRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

	player.Draw(g_sPoint.x + 100, g_sPoint.y + 100 , g_sPointSize.x, g_sPointSize.y);

	if (timer < 360) timer += 3;
	else timer = 0;			// 360프레임 때 원위치되므로 계속 도는 것처럼 느낌
	float roundX = cos(timer * PI / 180) * 100; // 코사인 : 1 -> 0 -> -1 -> 0 -> 1 (반복)
	float roundY = sin(timer * PI / 180) * 100;  // 사인 : 0 -> 1  -> 0 -> -1 -> 0 (반복)
	child.Draw(player.GetomatT().dx + roundX,
		player.GetomatT().dy + roundY
		, g_sPointSize.x, g_sPointSize.y);

	target1.Draw(400, 200, g_sPointSize.x, g_sPointSize.y);

	CDFlag = player.CollisionDetect(target1);
	if(!CDFlag)	CDFlag = child.CollisionDetect(target1);
	

	for (int i = 0; i < 10; i++)
		g_ipRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(0, i * 10), 1.0f, 1.0), g_ipBlackBrush);

	if (g_nAniIdx < ANI_SIZE - 1)
		g_nAniIdx++;
	else
		g_nAniIdx = 0;

	g_fRotAngle += 1.0f;

	if (CDFlag)
	{
		_cprintf("collision Detected!!!\n");
	}
	else _cprintf("Anlge: %f\n", g_fRotAngle);

	hr = g_ipRT->EndDraw();
}

void InitializeRect(HWND hWnd)
{
	assert(g_ipRT == nullptr);
	assert(hWnd != 0);

	HRESULT hr = E_FAIL;
	RECT rc;
	GetClientRect(hWnd, &rc);

	D2D1_RENDER_TARGET_PROPERTIES dxRTProperties = D2D1::RenderTargetProperties();
	D2D1_SIZE_U dxSize = D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top);

	hr = g_ipD2DFactory->CreateHwndRenderTarget(dxRTProperties,D2D1::HwndRenderTargetProperties(hWnd, dxSize),&g_ipRT);

	hr = g_ipRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &g_ipBlackBrush);
	hr = g_ipRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &g_ipRedBrush);

	assert(hr == S_OK);
}

void ManualLoadImage(HWND hWnd, const TCHAR* format, int idx)
{
	TCHAR szFullPath[1024];
	wsprintf(szFullPath, format, idx);
	CreateD2DBitmapFromFile(hWnd, szFullPath, idx);

	RECT rc;
	g_sPointSize = D2D1::Point2F(g_ipD2DBitmap[idx]->GetSize().width, g_ipD2DBitmap[idx]->GetSize().height);
	SetRect(&rc, 0, 0, g_sPointSize.x, g_sPointSize.y);
	InvalidateRect(hWnd, &rc, TRUE);
}

