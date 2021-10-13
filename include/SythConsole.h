#pragma once

#ifndef UNICODE
    #error Please enable unicode for compiler!
#endif

#include <string>
#include <windows.h>
#include <iostream>
#include <cmath>
#include <vector>
#include "SythColour.h"
#include "SythKeyMap.h"
#include "SythUtilities.h"


namespace Syth
{

    class consoleWindow
    {
        protected:
            int m_ScreenWidth;
            int m_ScreenHeight;
            int m_MouseX;
            int m_MouseY;
            std::wstring m_WinName;

            CHAR_INFO *m_ScreenBuf;
            SMALL_RECT m_RectWindow;

            bool m_CursorVis;

            HANDLE m_hConsole;
            HANDLE m_hConsoleIn;
            HANDLE m_hConsoleOrgi;
            

        protected:

            int MakeError(const wchar_t *msg);
            void handleMouseEvent(INPUT_RECORD &t_Record);

        public:

            consoleWindow();
            ~consoleWindow();

        public:

            int ConstructConsole(int width, int height, int fontW, int fontH, std::wstring t_winName = L"Console", bool t_CursorVis = false);

            int getWidth() const;
            int getHeight() const;
            int getMouseX() const;
            int getMouseY() const;
            void updateInputEvents();

            void clip(int &t_X, int &t_Y);
            void draw (int t_X, int t_Y, short t_Char, short t_Attribute);
            void fill(coord t_Start, coord t_End, short t_Char, short t_Attribute);
            void drawString(int t_X, int t_Y, std::wstring t_String, short t_Attribute);
            void drawLine(coord t_p1, coord t_p2, short t_Char, short t_Attribute);
            void renderQuad(coord vertex1,coord vertex2, coord vertex3, coord vertex4, short t_Char, short t_Attribute);
            void drawTriangle(coord vertex1, coord vertex2, coord vertex3, short t_Char, short t_Attribute);
            void clear();
            void update();
    };

}
