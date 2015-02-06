#pragma once

#include <commctrl.h>
#include "SimpleText.h"
#include "MultiformattedText.h"
#include "CustomText.h"


class TabWindow
{
public:
  TabWindow();
  ~TabWindow();

  HRESULT Initialize();
  HWND GetHwnd() { return hwnd_; }

private:

  HWND CreateTabControl();
  HRESULT CreateChildWindows();

  HRESULT OnPaint(const PAINTSTRUCT &ps);

  void OnResize(UINT width, UINT height);

  static LRESULT CALLBACK WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

private:
  HWND hwnd_;
  HWND hwndTab_;
  HWND hwndChild_;

  SimpleText simpleText_;
  MultiformattedText multiformattedText_;
  CustomText customText_;
};