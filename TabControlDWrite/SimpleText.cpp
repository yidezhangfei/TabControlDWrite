#include "TabControlDWrite.h"
#include "SimpleText.h"

SimpleText::SimpleText()
  : hwnd_(NULL)
  , wszText_(NULL)
  , cTextLength_(0)
  , pD2DFactory_(NULL)
  , pRT_(NULL)
  , pBlackBrush_(NULL)
  , pDWriteFactory_(NULL)
  , pTextFormat_(NULL)
{

}

SimpleText::~SimpleText()
{
  SafeRelease(&pD2DFactory_);
  SafeRelease(&pRT_);
  SafeRelease(&pBlackBrush_);
  SafeRelease(&pDWriteFactory_);
  SafeRelease(&pTextFormat_);
}

HRESULT SimpleText::Initialize(HWND hwndParent)
{
  WNDCLASSEX wcex;

  // get the dpi information
  HDC screen = GetDC(NULL);
  dpiScaleX_ = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
  dpiScaleY_ = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
  ReleaseDC(0, screen);

  HRESULT hr = S_OK;

  ATOM atom;

  // Register window class.
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = SimpleText::WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = sizeof(LONG_PTR);
  wcex.hInstance = HINST_THISCOMPONENT;
  wcex.hbrBackground = NULL;
  wcex.lpszMenuName = NULL;
  wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.lpszClassName = L"D2DSimpleText";
  wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  atom = RegisterClassEx(&wcex);

  hr = atom ? S_OK : E_FAIL;

  // Create window.
  hwnd_ = CreateWindow(
    L"D2DSimpleText",
    L"",
    WS_CHILD,
    0,
    0,
    static_cast<int>(640.0f / dpiScaleX_),
    static_cast<int>(480.0f / dpiScaleY_),
    hwndParent,
    NULL,
    HINST_THISCOMPONENT,
    this
    );

  if (SUCCEEDED(hr))
  {
    hr = hwnd_ ? S_OK : E_FAIL;
  }

  if (SUCCEEDED(hr))
  {
    hr = CreateDeviceIndependentResources();
  }

  //if (SUCCEEDED(hr))
  //{
  //  ShowWindow(hwnd_, SW_SHOWNORMAL);

  //  UpdateWindow(hwnd_);
  //}

  if (SUCCEEDED(hr))
  {
    DrawD2DContent();
  }

  return hr;
}

HRESULT SimpleText::CreateDeviceIndependentResources()
{
  HRESULT hr;

  // Create Direct2D factory.

  hr = D2D1CreateFactory(
    D2D1_FACTORY_TYPE_SINGLE_THREADED,
    &pD2DFactory_);

  // Create a shared DirectWrite factory.

  if (SUCCEEDED(hr))
  {
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
      __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory_));
  }

  // The string to display.

  wszText_ = L"Hello World using DirectWrite";
  cTextLength_ = (UINT32)wcslen(wszText_);

  // Create a text format using Gabriola with a font size of 
  // This sets the default font, weight, stretch, style, and

  if (SUCCEEDED(hr))
  {
    hr = pDWriteFactory_->CreateTextFormat(
      L"Gabriola",
      NULL,
      DWRITE_FONT_WEIGHT_REGULAR,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      72.0f,
      L"en-us",
      &pTextFormat_);
  }

  // Center align (horizontally) the text.
  if (SUCCEEDED(hr))
  {
    hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  }

  if (SUCCEEDED(hr))
  {
    hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  return hr;
}

HRESULT SimpleText::CreateDeviceResources()
{
  HRESULT hr = S_OK;

  RECT rc;
  GetClientRect(hwnd_, &rc);

  D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

  if (!pRT_)
  {
    // Create a Direct2D render target.
    hr = pD2DFactory_->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(hwnd_, size),
      &pRT_);

    // Create a black brush.
    if (SUCCEEDED(hr))
    {
      hr = pRT_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),
        &pBlackBrush_);
    }
  }

  return hr;
}

void SimpleText::DiscardDeviceResources()
{
  SafeRelease(&pRT_);
  SafeRelease(&pBlackBrush_);
}

HRESULT SimpleText::DrawTextW()
{
  RECT rc;

  GetClientRect(hwnd_, &rc);

  // Create a D2D rect that is the same size as the window.

  D2D1_RECT_F layoutRect = D2D1::RectF(
    static_cast<FLOAT>(rc.top) / dpiScaleY_,
    static_cast<FLOAT>(rc.left) / dpiScaleX_,
    static_cast<FLOAT>(rc.right - rc.left) / dpiScaleX_,
    static_cast<FLOAT>(rc.bottom - rc.top) / dpiScaleY_);

  // Use the DrawText method of the D2D render target interface.
  pRT_->DrawText(
    wszText_,
    cTextLength_,
    pTextFormat_,
    layoutRect,
    pBlackBrush_);

  return S_OK;
}

HRESULT SimpleText::DrawD2DContent()
{
  HRESULT hr;

  hr = CreateDeviceResources();

  if (!(pRT_->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
  {
    pRT_->BeginDraw();
    pRT_->SetTransform(D2D1::IdentityMatrix());
    pRT_->Clear(D2D1::ColorF(D2D1::ColorF::White));

    // Call the DraText method of this class.
    hr = DrawText();

    if (SUCCEEDED(hr))
    {
      hr = pRT_->EndDraw();
    }
  }

  if (FAILED(hr))
  {
    DiscardDeviceResources();
  }

  return hr;
}

void SimpleText::OnResize(UINT width, UINT height)
{
  if (pRT_)
  {
    D2D1_SIZE_U size;
    size.width = width;
    size.height = height;
    pRT_->Resize(size);
  }
}

LRESULT CALLBACK SimpleText::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_CREATE)
  {
    LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
    SimpleText* pSimpleText = (SimpleText*)pcs->lpCreateParams;

    ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(pSimpleText));

    return 1;
  }

  SimpleText* pSimpleText = reinterpret_cast<SimpleText*>(
    ::GetWindowLongPtr(hWnd, GWLP_USERDATA));

  if (pSimpleText)
  {
    switch (message)
    {
    case WM_SIZE:
    {
                  UINT width = LOWORD(lParam);
                  UINT height = HIWORD(lParam);
                  pSimpleText->OnResize(width, height);
    }
      return 0;
    case WM_PAINT:
    case WM_DISPLAYCHANGE:
    {
                           PAINTSTRUCT ps;
                           BeginPaint(hWnd, &ps);
                           pSimpleText->DrawD2DContent();
                           EndPaint(hWnd, &ps);
    }
      return 0;
    case WM_DESTROY:
    {
                     PostQuitMessage(0);
    }
      return 1;
    }
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}