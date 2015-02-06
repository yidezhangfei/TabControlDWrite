#include "TabControlDWrite.h"
#include "TabWindow.h"

int WINAPI  wWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPWSTR lpCmdLine,
  int nCmdShow)
{
  HRESULT hr;

  hr = CoInitialize(NULL);

  if (SUCCEEDED(hr))
  {
    {
      TabWindow tabWindow;

      hr = tabWindow.Initialize();

      if (SUCCEEDED(hr))
      {
        MSG msg;

        while (GetMessage(&msg, NULL, 0, 0))
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
    }
    CoUninitialize();
  }

  return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}