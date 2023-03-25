#include <Windows.h>
#include <time.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#include "resource.h"

typedef struct NODE
{
	int x;
	int y;
	struct NODE *pLast;
	struct NODE *pNext;
}Snack,Apple;

Snack *pHead;
Snack *pEnd;
Apple apple = {5,6,NULL,NULL};
enum FX {UP,DOWN,LEFT,RIGHT};
enum FX fx = RIGHT;
BOOL g_flag = TRUE;                                                    //设置标记，避免在一个定时器周期内有两次动作

HBITMAP Hbitmap_BackGroup;
HBITMAP Hbitmap_Apple;
HBITMAP Hbitmap_SnackHead;
HBITMAP Hbitmap_SnackHead_Up;
HBITMAP Hbitmap_SnackHead_Down;
HBITMAP Hbitmap_SnackHead_Left;
HBITMAP Hbitmap_SnackHead_Right;

void ShowBackGround(HDC hdc);                                        //显示背景
void AddNode(int x,int y);                                            //添加蛇身
void ShowSnack(HDC hdc);                                            //显示蛇
void Move();                                                        //移动蛇
void ShowApple(HDC hdc);                                            //显示苹果
BOOL IfEatApple();                                                    //判断是否吃到苹果
void NewApple();                                                    //随机出现新苹果
BOOL IfBumpWall();                                                    //判断是否撞墙
BOOL IfEatSelf();                                                    //判断是否自咬

LRESULT CALLBACK MyWNDPROC(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

int CALLBACK WinMain(
					 HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR lpCmdLine,
					 int nCmdShow
					 )
{
	HWND hwnd;
	MSG msg;
	//1.窗口设计
	WNDCLASSEX ex;
	ex.style = (UINT)NULL;
	ex.cbSize = sizeof(ex);
	ex.cbClsExtra = 0;
	ex.cbWndExtra = 0;
	ex.hInstance = hInstance;
	ex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));;
	ex.hCursor = NULL;
	ex.hbrBackground = CreateSolidBrush(RGB(0,255,0));
	ex.hIconSm = NULL;
	ex.lpfnWndProc = &MyWNDPROC;
	ex.lpszMenuName = NULL;
	ex.lpszClassName = "aa";

	Hbitmap_BackGroup = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP8));
	Hbitmap_Apple = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP2));
	Hbitmap_SnackHead = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP3));
	Hbitmap_SnackHead_Up = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP4));
	Hbitmap_SnackHead_Down = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP5));
	Hbitmap_SnackHead_Left = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP6));
	Hbitmap_SnackHead_Right = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP7));

	AddNode(5,5);
	AddNode(4,5);
	AddNode(3,5);

	//2.注册
	RegisterClassEx(&ex);
	//3.创建
	hwnd = CreateWindow(ex.lpszClassName,L"贪吃坤",WS_OVERLAPPEDWINDOW,300,50,915,638,NULL,NULL,hInstance,NULL);
	//4.显示
	ShowWindow(hwnd,SW_SHOW);

	SetTimer(hwnd,1,120,NULL);

	srand((unsigned int)time(0));

	//消息循环
	while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK MyWNDPROC(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;
	switch(message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd,&paintstruct);
		ShowBackGround(hdc);
		ShowSnack(hdc);
		ShowApple(hdc);

		EndPaint(hWnd,&paintstruct);
		break;
	case WM_TIMER:
		g_flag = TRUE;
		Move();
		if(IfBumpWall() || IfEatSelf())
		{
			KillTimer(hWnd,1);
			MessageBox(hWnd,L"GAME OVER",L"提示",MB_OK);
		}
		if(IfEatApple())                                        //判断是否吃到苹果
		{
			PlaySound(TEXT ("amagi.wav"), NULL, SND_FILENAME | SND_ASYNC);
			NewApple();                                            //随机一个新苹果
			AddNode(-10,-10);                                    //长个
		}
		hdc = GetDC(hWnd);
		ShowBackGround(hdc);                                    //层层覆盖
		ShowSnack(hdc);
		ShowApple(hdc);
		ReleaseDC(hWnd,hdc);
		break;
	case WM_KEYDOWN:
		if(g_flag == TRUE)
		{
			switch(wParam)
			{
			case VK_UP:
				if(fx != DOWN)
				{
					fx = UP;
				}
				break;
			case VK_DOWN:
				if(fx != UP)
				{
					fx = DOWN;
				}
				break;
			case VK_LEFT:
				if(fx != RIGHT)
				{
					fx = LEFT;
				}
				break;
			case VK_RIGHT:
				if(fx != LEFT)
				{
					fx = RIGHT;
				}
				break;
			case VK_SPACE:
				KillTimer(hWnd, 1);
				MessageBox(hWnd,L"您已暂停游戏，关闭可继续游戏",L"提示",MB_OK);
				SetTimer(hWnd,1,120,NULL);
			}
		}
		g_flag = FALSE;
		hdc = GetDC(hWnd);
		ShowBackGround(hdc);
		ShowSnack(hdc);
		ShowApple(hdc);
		ReleaseDC(hWnd,hdc);
		break;
	}

	return DefWindowProc(hWnd,message,wParam,lParam);
}
void ShowBackGround(HDC hdc)
{
	HDC hdc_compatible;
	hdc_compatible = CreateCompatibleDC(hdc);                                        //创建兼容性DC
	SelectObject(hdc_compatible,Hbitmap_BackGroup);                                    //为兼容性DC选择背景位图
	BitBlt(hdc,0,0,900,600,hdc_compatible,0,0,SRCCOPY);                                //按像素点进行传输
	DeleteDC(hdc_compatible);                                                        //删除兼容性DC
	return;
}
void AddNode(int x,int y)
{
	Snack *pTemp = (Snack *)malloc(sizeof(Snack));
	pTemp->x = x;
	pTemp->y = y;
	pTemp->pLast = NULL;
	pTemp->pNext = NULL;

	if(pHead == NULL)
	{
		pHead = pTemp;
	}
	else
	{
		pEnd->pNext = pTemp;
		pTemp->pLast = pEnd;
	}
	pEnd = pTemp;
}
void ShowSnack(HDC hdc)
{
	Snack *pMark = pHead->pNext;
	HDC hdc_compatible;
	hdc_compatible = CreateCompatibleDC(hdc);
	switch (fx)
	{
	case UP:
		SelectObject(hdc_compatible,Hbitmap_SnackHead_Up);
		break;
	case DOWN:
		SelectObject(hdc_compatible,Hbitmap_SnackHead_Down);
		break;
	case LEFT:
		SelectObject(hdc_compatible,Hbitmap_SnackHead_Left);
		break;
	case RIGHT:
		SelectObject(hdc_compatible,Hbitmap_SnackHead_Right);
		break;
	default:
		break;
	}
	BitBlt(hdc,pHead->x*30,pHead->y*30,30,30,hdc_compatible,0,0,SRCCOPY);

	while(pMark)
	{
		SelectObject(hdc_compatible,Hbitmap_SnackHead);
		BitBlt(hdc,pMark->x*30,pMark->y*30,30,30,hdc_compatible,0,0,SRCCOPY);
		pMark = pMark->pNext;
	}
	DeleteDC(hdc_compatible);
}
void Move()
{
	Snack *pMark = pEnd;
	while(pMark != pHead)
	{
		pMark->x = pMark->pLast->x;
		pMark->y = pMark->pLast->y;
		pMark = pMark->pLast;
	}
	switch (fx)
	{
	case UP:
		pHead->y--;
		break;
	case DOWN:
		pHead->y++;
		break;
	case LEFT:
		pHead->x--;
		break;
	case RIGHT:
		pHead->x++;
		break;
	}
}
void ShowApple(HDC hdc)
{
	HDC hdc_compatible;
	hdc_compatible = CreateCompatibleDC(hdc);
	SelectObject(hdc_compatible,Hbitmap_Apple);
	BitBlt(hdc,apple.x*30,apple.y*30,30,30,hdc_compatible,0,0,SRCCOPY);
	DeleteDC(hdc_compatible);
}
BOOL IfEatApple()
{
	if(pHead->x == apple.x && pHead->y == apple.y)
		return TRUE;
	return FALSE;
}
void NewApple()
{
	Snack *pMark = pHead;
	int x;
	int y;
	do
	{
		x = rand() % 28 + 1;
		y = rand() % 18 + 1;
		pMark = pHead;
		while(pMark)
		{
			if(pMark->x == x && pMark->y == y)
				break;
			pMark = pMark->pNext;
		}
	}while(pMark);
	apple.x = x;
	apple.y = y;
}
BOOL IfBumpWall()
{
	if(pHead->x == -1 || pHead->x == 30 || pHead->y == -1 || pHead->y == 20)
		return TRUE;
	return FALSE;
}
BOOL IfEatSelf()
{
	Snack *pMark = pHead->pNext;
	while(pMark)
	{
		if(pMark->x == pHead->x && pMark->y == pHead->y)
			return TRUE;
		pMark = pMark->pNext;
	}
	return FALSE;
}