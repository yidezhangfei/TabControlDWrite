#pragma once

class MultiformattedText
{
public:
  MultiformattedText();
  ~MultiformattedText();

  HRESULT Initialize(HWND hwndParent);

  HWND GetHwnd() { return hwnd_; }

private:
  HRESULT CreateDeviceIndependentResources();

  void DiscardDeviceIndependertResources();

  HRESULT CreateDeviceResources();

  void DiscardDeviceResources();

  HRESULT DrawD2DContent();

  HRESULT DrawText();

  void OnResize(UINT width, UINT height);

  static LRESULT CALLBACK WndProc(HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

private:
  HWND hwnd_;

  // How much o scale a design that assume 96-dpi pixels.
  float dpiScaleX_;
  float dpiScaleY_;

  // Direct2D.
  ID2D1Factory* pD2DFactory_;
  ID2D1HwndRenderTarget* pRT_;
  ID2D1SolidColorBrush* pBlackBrush_;

  // DirectWrite.
  IDWriteFactory* pDWriteFactory_;
  IDWriteTextFormat* pTextFormat_;

  IDWriteTextLayout* pTextLayout_;

  const wchar_t* wszText_;
  UINT32 cTextLength_;
};