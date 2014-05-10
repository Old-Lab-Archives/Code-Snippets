#include<Windows.h>
long FAR PASCAL WndProc(HWND, WORD, WORD, LONG);
HANDLE hInst;
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	HWND hWnd;
	MSG msg;
	WNDCLASS w;
	if(!hPrevInstance)
	{
		w.style=CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
		w.lpfnWndProc=(WNDPROC)WndProc;
		w.cbClsExtra=0;
		w.cbWndExtra=0;
		w.hInstance=hInstance;
		w.hIcon=LoadIcon(NULL,IDI_WINLOGO);
		w.hCursor=LoadCursor(NULL,IDC_ARROW);
		w.hbrBackground=GetStockObject(NULL_BRUSH);
		w.lpszClassName="MYCLASS";
		if(!RegisterClass(&w))
			return 0;
	}
	hWnd=CreateWindow("MYCLASS","Action trap CAMERA", WS_OVERLAPPEDWINDOW,100,100,300,300,NULL,NULL, hInstance, NULL);
	ShowWindow(hWnd,SW_SHOWNORMAL);
	while(GetMessage(&msg,0,0,0))
	{
		DispatchMessage(&msg);
	}
	return(msg.wParam);
}
long FAR PASCAL WndProc(HWND hWnd,WORD wMessage, WORD wParam, LONG IParam)
{
	return(DefWindowProc(hWnd,wMessage,wParam,IParam));
	return(OL);
}
