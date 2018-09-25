#include <windows.h>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include <gl/gl.h>

using namespace std;

int _w_width=600;
int _w_height=400;
vector< vector<int> > g_vec_board;
vector< vector<int> > g_vec_board_next;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);
bool init(void);
bool update(void);
bool draw(void);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GameOfLife";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    if(true)//if fullscreen
    {
        //Detect screen resolution
        RECT desktop;
        // Get a handle to the desktop window
        const HWND hDesktop = GetDesktopWindow();
        // Get the size of screen to the variable desktop
        GetWindowRect(hDesktop, &desktop);
        // The top left corner will have coordinates (0,0)
        // and the bottom right corner will have coordinates
        // (horizontal, vertical)
        _w_width  = desktop.right;
        _w_height = desktop.bottom;
    }

    hwnd = CreateWindowEx(0,
                          "GameOfLife",
                          "GameOfLife",
                          WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          _w_width,
                          _w_height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    //startup
    if(!init())
    {
        //cout<<"ERROR: Could not init\n";
        return 1;
    }

    while (!bQuit)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            update();
            draw();
            SwapBuffers(hDC);
        }
    }

    DisableOpenGL(hwnd, hDC, hRC);
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    *hDC = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    //set 2D mode
    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,_w_width,_w_height);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    //Set up the orthographic projection so that coordinates (0, 0) are in the top left
    //and the minimum and maximum depth is -10 and 10. To enable depth just put in
    //glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,_w_width,_w_height,0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /*//Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil(0);*/
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

bool init()
{
    srand(time(0));
    for(int x=0;x<_w_width;x++)
    {
        g_vec_board.push_back(vector<int>());
        g_vec_board_next.push_back(vector<int>());
        for(int y=0;y<_w_height;y++)
        {
            g_vec_board[x].push_back((rand()%20==0));
            g_vec_board_next[x].push_back(0);
        }
    }

    return true;
}

bool update()
{
    for(int x=0;x<_w_width;x++)
    {
        for(int y=0;y<_w_height;y++)
        {
            //check grid nearby
            int counter=0;
            for(int side_h=x-1;side_h<=x+1&&side_h<_w_width;side_h++)
            {
                if(side_h<0) continue;
                for(int side_v=y-1;side_v<=y+1&&side_v<_w_height;side_v++)
                {
                    if(side_v<0) continue;
                    if(side_h==x && side_v==y) continue;

                    if(g_vec_board[side_h][side_v]==1) counter++;
                }
            }
            //decide fate
            if(counter<2)       g_vec_board_next[x][y]=0;
            else if(counter==3) g_vec_board_next[x][y]=1;
            else if(counter>3)  g_vec_board_next[x][y]=0;
            else if(counter==2 && g_vec_board[x][y]==1) g_vec_board_next[x][y]=1;
            else                g_vec_board_next[x][y]=0;

        }
    }
    //transfer data
    for(int x=0;x<_w_width;x++)
    {
        for(int y=0;y<_w_height;y++)
        {
            g_vec_board[x][y]=g_vec_board_next[x][y];
        }
    }

    return true;
}

bool draw()
{
    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();

    glPushMatrix();
    glColor3f(1,1,1);
    glBegin(GL_POINTS);
    for(int x=0;x<_w_width;x++)
    {
        for(int y=0;y<_w_height;y++)
        {
            if(g_vec_board[x][y]==1)
            {
                //glColor3f(1,1,1);
                glVertex2f(x,y);
            }
            /*else
            {
                glColor3f(0,0,0);
                glVertex2f(x,y);
            }*/
        }
    }
    glEnd();
    glPopMatrix();

    return true;
}



