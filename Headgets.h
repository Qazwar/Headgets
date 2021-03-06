/*
	Headgets C++ library

	Author: MrOnlineCoder

	github.com/MrOnlineCoder/Headgets

	MIT License

	Copyright (c) 2017 Nikita Kogut (MrOnlineCoder)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef _HEADGETS_H
#define _HEADGETS_H

#if !defined(_WIN32) && !defined(_WIN64)
#error Headgets library supports only Windows platform
#endif

/*================== Headgets Configuration ==============*/

// If defined, Headgets will use Common Controls library (version 6)
// Without common controls, Progressbar widget is not avaliable.
#define HDG_USE_COMMONCTRLS 1


// Visual C++ only: If defined, comctl32.lib will be added to linker's input using #pragma directive (along with manifest for using Common Controls 6.0
// Comment if you are using GCC or just want to link library by yourself.
#define HDG_PRAGMA_COMMONCTRLS 1

/*========================================================*/

#include <Windows.h>
#include <Windowsx.h>

#include <string>
#include <functional>
#include <map>

#include <cassert>

#ifdef HDG_USE_COMMONCTRLS

#include <CommCtrl.h>

#if defined(_MSC_VER) && defined(HDG_PRAGMA_COMMONCTRLS)
//Require CommonControls ver. 6
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment( lib, "comctl32.lib" )
#endif

#endif

namespace hdg {
	//Win32 window class name, used in RegisterClassEx
	const char* HDG_CLASSNAME = "HEADGETSWINDOW";

	bool comctrlsInitalized = false;

	class Application;

	/*============== Utility ===========*/

	static void _reportLastError(std::string func) {
		std::string text = "";
		text = func+" call failed, GetLastError() = "+std::to_string(GetLastError());
		MessageBox(NULL, text.c_str(), "Headgets Error", MB_SYSTEMMODAL | MB_OK | MB_ICONERROR);
	}

	//Shows fatal error message and exits the application
	//TODO: more polite error handling
	static void _fatal(std::string msg) {
		MessageBox(NULL, msg.c_str(), "Headgets Error", MB_SYSTEMMODAL | MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	enum class MessageBoxType {
		Empty = 0,
		Information = MB_ICONINFORMATION,
		Warning = MB_ICONWARNING,
		Error = MB_ICONERROR
	};

	enum class MessageBoxButtons {
		Ok = 0,
		OkCancel = 1,
		AbortRetryIgnore = 2,
		YesNoCancel = 3,
		YesNo = 4,
		RetryCancel = 5,
		CancelTryContinue = 6
	};

	static void showMessageBox(std::string text, std::string caption="Information", hdg::MessageBoxType type = hdg::MessageBoxType::Information, hdg::MessageBoxButtons buttons = hdg::MessageBoxButtons::Ok) {

		MessageBox(NULL, text.c_str(), caption.c_str(), static_cast<UINT>(type) | static_cast<UINT>(buttons) );
	}

	static SIZE computeTextSize(HWND wnd, std::string str) {
		HDC hdc = GetDC(wnd); 
		SIZE size;
		if (GetTextExtentPoint32(hdc, str.c_str(), str.size(), &size) == TRUE) {
			return size;
		} else {
			size.cx = 0;
			size.cy = 0;
			return size;
		}
	}

	const std::string getEnvironmentVariable(const std::string var) {
		char buffer[MAX_PATH];
		if (GetEnvironmentVariable(var.c_str(), &buffer[0], MAX_PATH) == 0) {
			_reportLastError("getEnvironmentVariable() => GetEnvironmentVariable ");
			return "";
		}

		return std::string(buffer);
	}

	namespace FileFilters {
		const static char* AllFiles = "All Files\0*.*\0\0";
		const static char* TextFiles = "Text Files\0*.txt\0\0";
		const static char* ImageFiles = "Images\0*.jpg;*.png;*.bmp\0\0";
		const static char* VideoFiles = "Video files\0*.mp4;*.avi\0\0";
		const static char* AudioFiles = "Audio files\0*.mp3;*.ogg;*.wav;*.flac\0\0";
		const static char* ExecutableFiles = "Executables\0*.exe";
		const static char* DLLFiles = "DLLs\0*.dll\0\0";
	};

	class OpenDialog {
	public:
		OpenDialog() {
			filename[0] = '\0';

			ctx.lStructSize = sizeof(ctx);
			ctx.hwndOwner = NULL;
			ctx.hInstance = GetModuleHandle(NULL);
			ctx.lpstrFilter = "All Files\0*.*\0\0";
			ctx.lpstrCustomFilter = NULL;
			ctx.nMaxCustFilter = 0;
			ctx.nFilterIndex = 1;
			ctx.lpstrFile = &filename[0];
			ctx.nMaxFile = MAX_PATH;
			ctx.lpstrFileTitle = NULL;
			ctx.lpstrInitialDir = NULL;
			ctx.lpstrTitle = NULL;
			ctx.Flags = OFN_EXPLORER;
			ctx.lpstrDefExt = NULL;
			ctx.FlagsEx = 0;
		}

		void setRawFilter(const char* str) {
			filter = str;
			ctx.lpstrFilter = str;
		}

		void setTitle(std::string _title) {
			title = _title;
			ctx.lpstrTitle = title.c_str();
		}

		void setInitialPath(std::string dir) {
			initPath = dir;
			ctx.lpstrInitialDir = initPath.c_str();
		}

		bool open() {
			return GetOpenFileName(&ctx) != 0;
		}

		const std::string getFilename() {
			return std::string(filename);
		}
	private:
		OPENFILENAME ctx;

		std::string filter;
		std::string title;
		std::string initPath;

		char filename[MAX_PATH];
	};

	class SaveDialog {
	public:
		SaveDialog() {
			filename[0] = '\0';

			ctx.lStructSize = sizeof(ctx);
			ctx.hwndOwner = NULL;
			ctx.hInstance = GetModuleHandle(NULL);
			ctx.lpstrFilter = "All Files\0*.*\0\0";
			ctx.lpstrCustomFilter = NULL;
			ctx.nMaxCustFilter = 0;
			ctx.nFilterIndex = 1;
			ctx.lpstrFile = &filename[0];
			ctx.nMaxFile = MAX_PATH;
			ctx.lpstrFileTitle = NULL;
			ctx.lpstrInitialDir = NULL;
			ctx.lpstrTitle = NULL;
			ctx.Flags = OFN_EXPLORER;
			ctx.lpstrDefExt = NULL;
			ctx.FlagsEx = 0;
		}

		void setRawFilter(const char* str) {
			filter = str;
			ctx.lpstrFilter = str;
		}

		void setTitle(std::string _title) {
			title = _title;
			ctx.lpstrTitle = title.c_str();
		}

		void setInitialPath(std::string dir) {
			initPath = dir;
			ctx.lpstrInitialDir = initPath.c_str();
		}

		bool open() {
			return GetSaveFileName(&ctx) != 0;
		}

		const std::string getFilename() {
			return std::string(filename);
		}
	private:
		OPENFILENAME ctx;

		std::string filter;
		std::string title;
		std::string initPath;

		char filename[MAX_PATH];
	};

	/*============== Events ============*/
	enum class EventType {
		Created = 0,

		Resized,
		Moved,

		MouseEvent,
		Command,

		Closed,
		Destroyed
	};

	enum class MouseEvent {
		Nothing = 0,
		LeftPressed = WM_LBUTTONDOWN,
		RightPressed = WM_RBUTTONDOWN,
		LeftReleased = WM_LBUTTONUP,
		RightReleased = WM_RBUTTONUP
	};

	//Event struct
	//Contains:
	//type - type of event
	//num1, num2 - possible integer values, can be used to store native event data. By default they should be 0
	//handle - handle to window, which send the event, can be NULL
	struct Event {
		hdg::EventType type;

		int num1;
		int num2;

		hdg::MouseEvent mouse;

		HWND handle;

		hdg::Application* app;
	};


	/*============== Application ============*/
	class Application {
	public:
		Application(HINSTANCE _instance, std::string _title, int _width, int _height) {
			if (instance != NULL) {
				_fatal("Only 1 instance of hdg::Application is allowed at any time. Destruct the another one.");
			}

			#ifdef HDG_USE_COMMONCTRLS
			if (!comctrlsInitalized) {
				INITCOMMONCONTROLSEX cmcex;
				cmcex.dwSize = sizeof(INITCOMMONCONTROLSEX);
				cmcex.dwICC = ICC_LINK_CLASS | ICC_NATIVEFNTCTL_CLASS | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;
				if (InitCommonControlsEx(&cmcex) == FALSE) {
					_reportLastError("InitCommonControlsEx() ");
				} else {
					comctrlsInitalized = true;
				}
			}
			#endif
			
			hinstance = _instance;
			title = _title;
			width = _width;
			height = _height;

			nextId = 1;

			window = NULL;

			open = false;

			instance = this;

			registerWindowClass();

			DWORD dwStyle= (WS_OVERLAPPEDWINDOW);

			this->window = CreateWindow(
				HDG_CLASSNAME,
				title.c_str(),
				dwStyle,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				width,
				height,
				NULL,
				NULL,
				hinstance,
				NULL
				);

			if(window == NULL)
			{
				_fatal("Failed to create window!");
			}
		}

		~Application() {
			instance = NULL;
		}

		int run() {
			ShowWindow(window, SW_SHOW);
			UpdateWindow(window);

			open = true;

			MSG msg;
			while(GetMessage(&msg, NULL, 0, 0) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return msg.wParam;
		}

		static LRESULT CALLBACK _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			return instance->RealWndProc(hwnd, msg, wParam, lParam); 
		}

		LRESULT CALLBACK RealWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			switch(msg) {
				//Create and destroy events
				case WM_CREATE:
					postSimpleEvent(hdg::EventType::Created, hwnd);
					return 0;
					break;
				case WM_CLOSE:
					postSimpleEvent(hdg::EventType::Closed, hwnd);
					DestroyWindow(hwnd);
					break;
				case WM_DESTROY:
					postSimpleEvent(hdg::EventType::Destroyed);
					open = false;
					PostQuitMessage(0);
					break;
				//Mouse events
				case WM_LBUTTONUP:
				case WM_LBUTTONDOWN:
				case WM_RBUTTONUP:
				case WM_RBUTTONDOWN: {
					hdg::Event ev = {
						hdg::EventType::MouseEvent,
						GET_X_LPARAM(lParam),
						GET_Y_LPARAM(lParam),
						static_cast<hdg::MouseEvent>(msg),
						hwnd,
						this
					};
					postEvent(ev);
					break;
				}
				//Move and size events
				case WM_MOVE: {
					hdg::Event ev = {
						hdg::EventType::Moved,
						(int)(short) LOWORD(lParam),
						(int)(short) HIWORD(lParam),
						hdg::MouseEvent::Nothing,
						hwnd,
						this
					};
					this->x = ev.num1;
					this->y = ev.num2;
					postEvent(ev);
					break;
				}
				case WM_SIZE: {
					hdg::Event ev = {
						hdg::EventType::Resized,
						(int)(short) LOWORD(lParam),
						(int)(short) HIWORD(lParam),
						hdg::MouseEvent::Nothing,
						hwnd,
						this
					};
					this->width = ev.num1;
					this->height = ev.num2;
					postEvent(ev);
					break;
				}
				case WM_COMMAND: {
					postSimpleEvent(hdg::EventType::Command, hwnd, LOWORD(wParam), 0);
					break;
				}

				default:
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}
		}

		void setUserCallback(std::function<void(hdg::Event)> func) {
			eventCallback = func;

			if (!eventCallback) _fatal("Failed to set event callback");
		}

		int getX() {
			return x;
		}

		int getY() {
			return y;
		}

		void moveTo(int newX, int newY) {
			x = newX;
			y = newY;

			if (SetWindowPos(window, (HWND) -1, x, y, -1, -1, SWP_NOZORDER | SWP_NOSIZE) == 0) {
				_reportLastError("Application::moveTo()");
			}
		}

		void moveBy(int dX, int dY) {
			moveTo(x+dX, y+dY);
		}

		int getWidth() {
			return width;
		}

		int getHeight() {
			return height;
		}

		HWND getNativeHandle() {
			return window;
		}

		bool isOpen() {
			return open;
		}

		void close() {
			SendMessage(window, WM_CLOSE, 0, 0);
		}

		void setTitle(std::string str) {
			title = str;
			SetWindowText(window, title.c_str());
		}

		UINT getNextControlID() {
			UINT id = WM_USER + nextId;
			nextId++;
			return id;
		}

		static Application *instance;
	private:

		//Registers Win32 window class.
		void registerWindowClass() {
			WNDCLASSEX wc;

			wc.cbSize        = sizeof(WNDCLASSEX);

			wc.style         = 0;

			wc.lpfnWndProc   = this->_WndProc;

			wc.cbClsExtra    = 0;
			wc.cbWndExtra    = 0;

			wc.hInstance     = hinstance;

			wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
			wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
			wc.lpszMenuName  = NULL;
			wc.lpszClassName = HDG_CLASSNAME;
			wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

			if(!RegisterClassEx(&wc)) {
				_fatal("Failed to register Headgets Win32 window class.");
			}
		}

		//Helper function to send an event to user quickly
		void postSimpleEvent(hdg::EventType type, HWND wnd=NULL, int n1=0, int n2=0) {
			hdg::Event ev;
			ev.type = type;
			ev.handle = wnd;
			ev.num1 = n1;
			ev.num2 = n2;
			ev.mouse = hdg::MouseEvent::Nothing;
			ev.app = this;


			if (eventCallback) eventCallback(ev);
		}

		void postEvent(hdg::Event ev) {
			if (eventCallback) eventCallback(ev);
		}

		//Handle of main window
		HWND window;

		//Instance of the application in OS
		HINSTANCE hinstance;
		
		//Window title
		std::string title;

		//Size
		int width;
		int height;

		//Pos
		int x;
		int y;

		//Is window open?
		bool open;

		UINT nextId;

		//Event callback
		//Used to send user (library user) an hdg::Event so he can process it.
		std::function<void(hdg::Event)> eventCallback;
	};
	
	enum FontWeight {
		Default = FW_DONTCARE,
		Thin = FW_THIN,
		Extralight = FW_EXTRALIGHT,
		Light = FW_LIGHT,
		Normal = FW_NORMAL,
		Medium = FW_MEDIUM,
		Semibold = FW_SEMIBOLD, 
		Bold = FW_BOLD,
		Extrabold = FW_EXTRABOLD,
		Heavy = FW_HEAVY
	};

	class Font {
	public:
		Font(const std::string family, int weight=FontWeight::Default, int size = 0, bool italic=false) {
			this->family = family;
			this->weight = weight;
			this->italic = italic;
			this->size = size;

			this->underline = false;
			this->striked = false;
		}

		void setItalic(bool arg) {
			italic = arg;
		}

		void setStriked(bool arg) {
			striked = arg;
		}

		void setUnderline(bool arg) {
			underline = arg;
		}

		void setSize(int sz) {
			size = sz;
		}

		void setWeight(int arg) {
			weight = arg;
		}

		void setFamily(const std::string arg) {
			family = arg;
		}

		HFONT createHandle() {
			hf = CreateFont(
				size, // Title font size
				0, // Width (default is used)
				0, //0 now
				0, // 0 now
				weight, //weight
				(BOOL) italic, //Is italic?
				(BOOL) underline, //Is underlined?
				(BOOL) striked, //Is striked out?
				ANSI_CHARSET, //Charset
				OUT_DEFAULT_PRECIS, //Default precision
				CLIP_DEFAULT_PRECIS, //Default
				DEFAULT_QUALITY, //Default quality
				DEFAULT_PITCH | FF_SWISS, //?
				family.c_str() // Font family
			);

			if (hf == NULL) _reportLastError("Font::createHandle() => CreateFont");

			return hf;
		}
	private:
		HFONT hf;

		std::string family;
		int weight;
		bool italic;
		bool underline;
		bool striked;
		int size;
	};

	/*============== Widgets ================*/


	class Widget {
	public:
		Widget(HWND _parent) {
			assert(_parent != NULL);

			parent = _parent;
			hinstance = (HINSTANCE) GetWindowLong (parent, GWL_HINSTANCE);
		}

		Widget(Application* _app) {
			assert(_app != NULL);

			app = _app;

			parent = app->getNativeHandle();
			hinstance = (HINSTANCE) GetWindowLong (parent, GWL_HINSTANCE);
		}

		~Widget() {
			DestroyWindow(window);
		}

		void hide() {
			ShowWindow(window, SW_HIDE);
		}

		void show() {
			ShowWindow(window, SW_SHOW);
		}

		void setPosition(int x, int y) {
			if (SetWindowPos(window, (HWND) -1, x, y, -1, -1, SWP_NOZORDER | SWP_NOSIZE) == 0) {
				_reportLastError("Widget::setPosition()");
			}
		}

		void setSize(int w, int h) {
			if (SetWindowPos(window, (HWND) -1, -1, -1, w, h, SWP_NOZORDER | SWP_NOMOVE) == 0) {
				_reportLastError("Widget::setSize()");
			}
		}

		void setFont(hdg::Font& font) {
			HFONT hf = font.createHandle();
			if (hf == NULL) {
				return;
			}


			SendMessage(window, WM_SETFONT, (WPARAM) hf, TRUE);
		} 
	protected:
		HWND parent;
		HWND window;

		Application* app;

		HINSTANCE hinstance;
	};

	class Label : public hdg::Widget {
	public:
		Label(std::string _text, int x=0, int y=0, int w=100, int h=50)
		: Widget(hdg::Application::instance){
			text = _text;

			window = CreateWindow("STATIC", text.c_str(),  WS_CHILD | WS_VISIBLE | WS_TABSTOP, x, y, w, h, parent, NULL, hinstance, NULL);

			if (window == NULL) _reportLastError("Label::Label() => CreateWindow");
		}

		void setText(std::string txt) {
			text = txt;
			SetWindowText(window, text.c_str());
		}
	private:
		std::string text;
	};

	class Button : public hdg::Widget {
	public:
		Button(std::string _text, int x=0, int y=0)
		: Widget(hdg::Application::instance){
			text = _text;

			id = app->getNextControlID();

			window = CreateWindow("BUTTON", text.c_str(),  WS_CHILD | WS_VISIBLE | WS_TABSTOP, x, y, 100, 50, parent, (HMENU) id, hinstance, NULL);

			if (window == NULL) _reportLastError("Button::Button() => CreateWindow");

			setText(text);
		}

		void setText(std::string txt) {
			text = txt;
			SetWindowText(window, text.c_str());

			SIZE sz = hdg::computeTextSize(window, text);

			setSize(sz.cx+50, sz.cy+14);
		}

		void disable() {
			EnableWindow(window, FALSE);
		}

		void enable() {
			EnableWindow(window, TRUE);
		}

		void setDisabled(bool arg) {
			EnableWindow(window, (BOOL) arg);
		}

		UINT getID() {
			return id;
		}
	private:
		std::string text;

		int id;
	};

	class EditboxStyle {
	public:
		const static UINT None = 0x0; 
		const static UINT Center = ES_CENTER;
		const static UINT Left = ES_LEFT;
		const static UINT Lowercase = ES_LOWERCASE;
		const static UINT Multiline = ES_MULTILINE;
		const static UINT Number = ES_NUMBER;
		const static UINT Password = ES_PASSWORD;
		const static UINT Right = ES_RIGHT;
		const static UINT Uppercase = ES_UPPERCASE;
	};

	class Editbox : public hdg::Widget {
	public:
		Editbox(UINT st = hdg::EditboxStyle::None, int x=0, int y=0, int w=100, int h=14)
		: Widget(hdg::Application::instance){

			window = CreateWindow("EDIT", "",  WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | (UINT) (st), x, y, w, h+14, parent, NULL, hinstance, NULL);

			if (window == NULL) _reportLastError("Editbox::Editbox() => CreateWindow");
		}

		void setText(std::string txt) {
			SetWindowText(window, txt.c_str());
		}

		std::string value() {
			int sz = GetWindowTextLength(window) + 1;

			char* rstr = new char[sz];
			GetWindowText(window, rstr, sz);

			std::string val(rstr);
			delete [] rstr;

			return val;
		}

		void setReadonly(bool arg) {
			SendMessage(window, EM_SETREADONLY, (BOOL) arg, 0);
		}

		bool isEmpty() {
			return value().size() == 0;
		}
	};

	class Progressbar : public hdg::Widget {
	public:
		Progressbar(bool isMarquee, int x=0, int y=0, int w=100, int h=14)
		: Widget(hdg::Application::instance){

			int style = isMarquee ? PBS_MARQUEE : 0x0;

			if (!comctrlsInitalized) _fatal("Progressbar widgets is avaliable only with Common Controls!");
			window = CreateWindow(PROGRESS_CLASS, "",  WS_CHILD | WS_VISIBLE | WS_TABSTOP | style , x, y, w, h+14, parent, NULL, hinstance, NULL);

			if (window == NULL) _reportLastError("Progressbar::Progressbar() => CreateWindow");

			min = 0;
			max = 100;
			barStep = 10;
			marquee = isMarquee;
		}

		void setRange(int _min, int _max) {
			if (marquee) return;

			min = _min;
			max = _max;

			SendMessage(window, PBM_SETRANGE, 0, MAKELPARAM(min, max));
		}

		void setStep(int _step) {
			if (marquee) return;

			barStep = _step;

			SendMessage(window, PBM_SETSTEP, _step, 0);
		}

		void step(int amount=0) {
			if (marquee) return;

			if (amount != 0) {
				SendMessage(window, PBM_DELTAPOS, amount, 0);
			} else {
				SendMessage(window, PBM_STEPIT, 0, 0);
			}
		}

		void toggleMarquee(bool mode, int time=30) {
			if (!marquee) return;

			if (!mode) {
				SendMessage(window, PBM_SETMARQUEE, FALSE, 0);
			} else {
				SendMessage(window, PBM_SETMARQUEE, TRUE, time);
			}
		}
	private:
		int min;
		int max;
		int barStep;

		bool marquee;
	};


	//Instance of the application
	Application * Application::instance = NULL;
};

#endif