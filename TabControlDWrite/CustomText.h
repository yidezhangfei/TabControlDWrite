#pragma once

class CustomText
{
public:
  CustomText();
  ~CustomText();

  HRESULT Initialize(HWND hwndParent);

  HWND GetHwnd() { return hwnd_; }

private:
  HRESULT CreateDeviceIndependentResources();

  void DiscardDeviceIndependentResources();

  HRESULT CreateDeviceResources();

  void DiscardDeviceResources();

  HRESULT DrawD2DContent();

  HRESULT LoadResourceBitmap(ID2D1RenderTarget* pRT,
    IWICImagingFactory* pIWICFactory,
    PCWSTR resourceName,
    PCWSTR resourceType,
    __deref_out ID2D1Bitmap **ppBitmap);

  HRESULT DrawText();

  void OnResize(UINT width, UINT height);

  static LRESULT CALLBACK WndProc(HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

private:
  HWND hwnd_;

  // how much to scale a design that assume 96-dpi pixels.
  float dpiScaleX_;
  float dpiScaleY_;

  // Direct2D
  ID2D1Factory* pD2DFactory_;
  ID2D1HwndRenderTarget* pRT_;
  ID2D1SolidColorBrush* pBlackBrush_;
  ID2D1BitmapBrush* pBitmapBrush_;

  // DirectWrite
  IDWriteFactory* pDWriteFactory_;
  IDWriteTextFormat* pTextFormat_;
  IDWriteTextLayout* pTextLayout_;
  IDWriteTextRenderer* pTextRenderer_;

  // WIC
  IWICImagingFactory* pWICFactory_;

  const wchar_t* wszText_;
  UINT32 cTextLength_;
};