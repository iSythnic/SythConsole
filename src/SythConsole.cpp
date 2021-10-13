#include "SythConsole.h"

//Protected
namespace Syth
{

    int consoleWindow::MakeError(const wchar_t *msg)
    {
        wchar_t buffer[256];  // Buffer for containing the error message 
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 256, NULL); // Format the Error message from GetLastError()
        SetConsoleActiveScreenBuffer(m_hConsoleOrgi);  // Set the screen buffer to the intial screen buffer so the error can be displayed without any interference from the current buffer
        std::wcout << "Error: " << msg << " " << buffer << std::endl;
        return 0;
    }


    // Public
    consoleWindow::consoleWindow() : m_ScreenWidth(80), m_ScreenHeight(30), m_MouseX(0), m_MouseY(0), m_WinName(L"Console"), m_CursorVis(false)
    {
        m_hConsoleOrgi = GetStdHandle(STD_OUTPUT_HANDLE);
        m_hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
        m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
    }

    int consoleWindow::ConstructConsole(int width, int height, int fontW, int fontH, std::wstring t_winName, bool t_CursorVis)
    {
        if (m_hConsole == INVALID_HANDLE_VALUE)
            return MakeError(L"INVALID HANDLE");

        m_ScreenWidth = width;
        m_ScreenHeight = height;
        m_WinName = t_winName;
        m_RectWindow = {0, 0, 1, 1};
        SetConsoleWindowInfo(m_hConsole, TRUE, &m_RectWindow);

        COORD xyCoord { (short)m_ScreenWidth, (short)m_ScreenHeight };

        if (!SetConsoleScreenBufferSize(m_hConsole, xyCoord))
            return MakeError(L"SetConsoleScreenBufferSize");
        
        if (!SetConsoleActiveScreenBuffer(m_hConsole))
            return MakeError(L"SetConsoleActiveScreenBuffer");
        
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = fontW;
        cfi.dwFontSize.Y = fontH;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        wcscpy_s(cfi.FaceName, L"Terminal");

        if (!SetCurrentConsoleFontEx(m_hConsole, false,&cfi))
            return MakeError(L"SetCurrentConsoleFontEx");
        
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        // Check if the size given by the programmer is bigger than the screen buffer
        if (!GetConsoleScreenBufferInfo(m_hConsole,&csbi))
            return MakeError(L"GetConsoleScreenBufferInfo");
        
        if (m_ScreenHeight > csbi.dwMaximumWindowSize.Y)
            return MakeError(L"Screen Height is too big");

        if (m_ScreenWidth > csbi.dwMaximumWindowSize.X)
            return MakeError(L"Screen Width is too big");

        short tempWidth = m_ScreenWidth-1;
        short tempHeight = m_ScreenHeight-1;
        m_RectWindow = {0, 0, tempWidth, tempHeight};
        
        // Changes the dimensions of the Window 
        if (!SetConsoleWindowInfo(m_hConsole, TRUE, &m_RectWindow))
            return MakeError(L"SetConsoleWindowInfo");
        
        if (!SetConsoleMode(m_hConsoleIn, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS))
            return MakeError(L"SetConsoleMode");
        
        wchar_t title[256];
        swprintf(title,256,m_WinName.c_str());
        SetConsoleTitle(title);

        CONSOLE_CURSOR_INFO cci;
        if (!GetConsoleCursorInfo(m_hConsole, &cci))
            return MakeError(L"GetConsoleCursorInfo");
        
        cci.bVisible = t_CursorVis;
        m_CursorVis = t_CursorVis;

        if (!SetConsoleCursorInfo(m_hConsole,&cci))
            return MakeError(L"SetCnsoleCursorInfo");
        
        // Create the screen buffer that stores data to be displayed on screen 
        m_ScreenBuf = new CHAR_INFO[m_ScreenWidth*m_ScreenHeight];
        memset(m_ScreenBuf, 0, sizeof(CHAR_INFO)*m_ScreenHeight*m_ScreenWidth);  // Sets all the unintialized 

        return 1;
    }

    int consoleWindow::getWidth() const { return m_ScreenWidth; }
    int consoleWindow::getHeight() const { return m_ScreenHeight; }
    int consoleWindow::getMouseX() const { return m_MouseX; }
    int consoleWindow::getMouseY() const { return m_MouseY; }
    
    void consoleWindow::handleMouseEvent(INPUT_RECORD &t_Record)
    {
        switch (t_Record.Event.MouseEvent.dwEventFlags)
        {
            case MOUSE_MOVED:
                m_MouseX = t_Record.Event.MouseEvent.dwMousePosition.X;
                m_MouseY = t_Record.Event.MouseEvent.dwMousePosition.Y;
                break;

            default:
                break;
        }
    }

    void consoleWindow::updateInputEvents()
    {
        DWORD numEvents {0};
        GetNumberOfConsoleInputEvents(m_hConsoleIn, &numEvents);

        if (numEvents == 0)
            return;

        INPUT_RECORD recordBuf[128];
        ReadConsoleInput(m_hConsoleIn,recordBuf, numEvents, &numEvents);

        for (DWORD i {0}; i < numEvents; i++)
        {
            switch (recordBuf[i].EventType)
            {
                case MOUSE_EVENT:
                    handleMouseEvent(recordBuf[i]);
                    break;
                
                default:
                    break;
            }
        }
    }

    void consoleWindow::clip(int &t_X, int &t_Y)
    {
        if (t_X < 0) t_X = 0;
        if (t_X >= m_ScreenWidth) t_X = m_ScreenWidth;
        
        if (t_Y < 0) t_Y = 0;
        if (t_Y >= m_ScreenHeight) t_Y = m_ScreenHeight;

    }

    void consoleWindow::draw(int t_X, int t_Y, short t_Char, short t_Attribute)
    {
        if (t_X >= 0 && t_X <= m_ScreenWidth && t_Y >= 0 && t_Y <= m_ScreenHeight)
        {
            m_ScreenBuf[m_ScreenWidth * t_Y + t_X].Char.UnicodeChar = t_Char;
            m_ScreenBuf[m_ScreenWidth * t_Y + t_X].Attributes = t_Attribute;
        }
    }

    void consoleWindow::fill(Syth::coord t_Start, Syth::coord t_End, short t_Char, short t_Attribute)
    {
        for (int i {t_Start.x}; i <= t_End.x; i++)
        {
            for (int z {t_Start.y}; z <= t_End.y; z++)
            {
                draw(i,z,t_Char,t_Attribute);
            }
        }
    }

    void consoleWindow::drawString(int t_X, int t_Y, std::wstring t_String, short t_Attribute)
    {
        for (int i {0}, width {t_X}; i < t_String.size(); i++, width++)
        {
            draw(width,t_Y,t_String.at(i), t_Attribute);
        }
    }

    void consoleWindow::drawTriangle(coord vertex1, coord vertex2, coord vertex3, short t_Char, short t_Attribute)
    {
        drawLine(vertex1, vertex2 ,t_Char, t_Attribute);
        drawLine(vertex2, vertex3, t_Char, t_Attribute);
        drawLine(vertex3, vertex1, t_Char, t_Attribute);
    }

     void consoleWindow::drawLine(coord t_p1, coord t_p2, short t_Char, short t_Attribute)
    {
        float slope = 0.0f;

        if (t_p1.x != t_p2.x)
            slope = ((float)t_p2.y - (float)t_p1.y) / ((float)t_p2.x - (float)t_p1.x);
        

        if (t_p1.x != t_p2.x && abs(slope) <= 1.0f)
        {

            if (t_p1.x > t_p2.x)
                std::swap(t_p1, t_p2);

            float c {t_p1.y - slope*t_p1.x};

            for (int i {t_p1.x}; i < t_p2.x; i++)
            {
                int bufferY = slope * i + c;
                draw(i,bufferY, t_Char, t_Attribute);
            }
            
        }
        else
        {
            if (t_p1.y > t_p2.y)
                std::swap(t_p1, t_p2);
            
            float slopeY { ((float)t_p1.x - (float)t_p2.x) / ((float)t_p1.y - (float)t_p2.y)};
            float cY {t_p1.x - slopeY*t_p1.y};

            for (int i {t_p1.y}; i < t_p2.y; i++)
            {
                int bufferX = slopeY * i + cY;
                draw(bufferX, i, t_Char, t_Attribute);
            }
        }


    }

    void consoleWindow::renderQuad(Syth::coord vertex1,Syth::coord vertex2, Syth::coord vertex3, Syth::coord vertex4, short t_Char, short t_Attribute)
    {
        drawLine(vertex1, vertex2, t_Char, t_Attribute);
        drawLine(vertex1, vertex3, t_Char, t_Attribute);
        drawLine(vertex1, vertex4, t_Char, t_Attribute);
        drawLine(vertex2, vertex3, t_Char, t_Attribute);
        drawLine(vertex2, vertex4, t_Char, t_Attribute);
        drawLine(vertex3, vertex4, t_Char, t_Attribute);

    }

    void consoleWindow::clear()
    {
        memset(m_ScreenBuf, 0, sizeof(CHAR_INFO)*m_ScreenHeight*m_ScreenWidth); 
    }

    void consoleWindow::update()
    {
        short tempW = m_ScreenWidth;
        short tempH = m_ScreenHeight;
        WriteConsoleOutput(m_hConsole,m_ScreenBuf, {tempW, tempH}, {0,0}, &m_RectWindow);
    }


    consoleWindow::~consoleWindow()
    {
        delete [] m_ScreenBuf;
    }
}