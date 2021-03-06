#include "TabControlDWrite.h"
#include "CustomText.h"
#include "CustomTextRenderer.h"

CustomText::CustomText()
  : hwnd_(NULL)
  , wszText_(NULL)
  , cTextLength_(NULL)
  , pD2DFactory_(NULL)
  , pRT_(NULL)
  , pBlackBrush_(NULL)
  , pBitmapBrush_(NULL)
  , pDWriteFactory_(NULL)
  , pTextFormat_(NULL)
  , pTextLayout_(NULL)
  , pTextRenderer_(NULL)
  , pWICFactory_(NULL)
{

}

CustomText::~CustomText()
{
  SafeRelease(&pD2DFactory_);
  SafeRelease(&pRT_);
  SafeRelease(&pBlackBrush_);
  SafeRelease(&pBitmapBrush_);
  SafeRelease(&pDWriteFactory_);
  SafeRelease(&pTextFormat_);
  SafeRelease(&pTextLayout_);
  SafeRelease(&pTextRenderer_);
  SafeRelease(&pWICFactory_);
}

HRESULT CustomText::Initialize(HWND hwndParent)
{
  WNDCLASSEX wcex;

  // Get the dpi information
  HDC screen = GetDC(NULL);
  dpiScaleX_ = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
  dpiScaleY_ = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
  ReleaseDC(0, screen);

  HRESULT hr = S_OK;
  ATOM atom;

  // Register window class.
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.lpfnWndProc = CustomText::WndProc;
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = sizeof(LONG_PTR);
  wcex.hInstance = HINST_THISCOMPONENT;
  wcex.hbrBackground = NULL;
  wcex.lpszMenuName = NULL;
  wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.lpszClassName = L"D2DCustomText";

  atom = RegisterClassEx(&wcex);

  hr = atom ? S_OK : E_FAIL;

  if (SUCCEEDED(hr))
  {
    // CreateWindow
    hwnd_ = CreateWindow(
      L"D2DCustomText",
      L"",
      WS_CHILD,
      0,
      0,
      static_cast<int>(640.0f / dpiScaleX_),
      static_cast<int>(480.0f / dpiScaleY_),
      hwndParent,
      NULL,
      HINST_THISCOMPONENT,
      this);
  }
  

  if (SUCCEEDED(hr))
  {
    hr = hwnd_ ? S_OK : E_FAIL;
  }

  if (SUCCEEDED(hr))
  {
    hr = CreateDeviceIndependentResources();
  }

  if (SUCCEEDED(hr))
  {
    DrawD2DContent();
  }

  return hr;
}

HRESULT CustomText::CreateDeviceIndependentResources()
{
  HRESULT hr;

  // Create WIC factory.
  hr = CoCreateInstance(
    CLSID_WICImagingFactory,
    NULL,
    CLSCTX_INPROC_SERVER,
    IID_IWICImagingFactory,
    reinterpret_cast<void**>(&pWICFactory_));

  // Create Direct2D factory.
  if (SUCCEEDED(hr))
  {
    hr = D2D1CreateFactory(
      D2D1_FACTORY_TYPE_SINGLE_THREADED,
      &pD2DFactory_);
  }

  // Create a shared DirectWrite factory.
  if (SUCCEEDED(hr))
  {
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
      __uuidof(IDWriteFactory),
      reinterpret_cast<IUnknown**>(&pDWriteFactory_));
  }

  // The string to display.
  wszText_ = L"Hello World using DirectWrite!";
  cTextLength_ = (UINT32)wcslen(wszText_);

  // Create a text format using Gabriola with a font size fo
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

  // Center align (vertically) the text.
  if (SUCCEEDED(hr))
  {
    hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  // Create a text layout using the text format.
  if (SUCCEEDED(hr))
  {
    hr = pDWriteFactory_->CreateTextLayout(
      wszText_,
      cTextLength_,
      pTextFormat_,
      640.0f,
      480.0f,
      &pTextLayout_);
  }

  // Format the "DirectWrite" substring to be of font size 100
  if (SUCCEEDED(hr))
  {
    DWRITE_TEXT_RANGE textRange = { 20, 6 };
    hr = pTextLayout_->SetFontSize(100.0f, textRange);
  }
  if (SUCCEEDED(hr))
  {
    DWRITE_TEXT_RANGE textRange = { 20, 11 };
    hr = pTextLayout_->SetUnderline(true, textRange);
  }

  if (SUCCEEDED(hr))
  {
    // Format the word "DWrite" to be bold.
    DWRITE_TEXT_RANGE textRange = { 20, 11 };
    hr = pTextLayout_->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, textRange);
  }

  // Declare a typography pointer.
  IDWriteTypography* pTypography = NULL;

  // Create a typography interface object.
  if (SUCCEEDED(hr))
  {
    hr = pDWriteFactory_->CreateTypography(&pTypography);
  }

  // Set the stylistic set.
  DWRITE_FONT_FEATURE fontFeature = { DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7, 1 };

  if (SUCCEEDED(hr))
  {
    hr = pTypography->AddFontFeature(fontFeature);
  }

  if (SUCCEEDED(hr))
  {
    // Set the typography for the entire string.
    DWRITE_TEXT_RANGE textRange = { 0, cTextLength_ };
    hr = pTextLayout_->SetTypography(pTypography, textRange);
  }

  SafeRelease(&pTypography);

  return hr;
}

HRESULT CustomText::CreateDeviceResources()
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
      hr = pRT_->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black),
        &pBlackBrush_);
    }

    ID2D1Bitmap* pBitmap = NULL;

    // Create the bitmap to be used by the bitmap brush.
    if (SUCCEEDED(hr))
    {
      hr = LoadResourceBitmap(pRT_,
        pWICFactory_,
        L"Tulip",
        L"Image",
        &pBitmap);
    }

    // Create the bitmap brush.
    D2D1_BITMAP_BRUSH_PROPERTIES properties = { D2D1_EXTEND_MODE_WRAP,
      D2D1_EXTEND_MODE_WRAP, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR };
    if (SUCCEEDED(hr))
    {
      hr = pRT_->CreateBitmapBrush(
        pBitmap,
        properties,
        &pBitmapBrush_);
    }

    if (SUCCEEDED(hr))
    {
      // Create the text renderer
      pTextRenderer_ = new (std::nothrow) CustomTextRenderer(
        pD2DFactory_,
        pRT_,
        pBlackBrush_,
        pBitmapBrush_);
    }
  }

  return hr;
}

void CustomText::DiscardDeviceResources()
{
  SafeRelease(&pRT_);
  SafeRelease(&pBlackBrush_);
  SafeRelease(&pBitmapBrush_);
  SafeRelease(&pTextRenderer_);
}

HRESULT CustomText::DrawTextW()
{
  HRESULT hr = S_OK;

  RECT rc;

  GetClientRect(hwnd_, &rc);

  D2D1_POINT_2F origin = D2D1::Point2F(
    static_cast<FLOAT>(rc.top / dpiScaleY_),
    static_cast<FLOAT>(rc.left / dpiScaleX_));

  // Draw the text layout using DirectWrite and the CustomText
  hr = pTextLayout_->Draw(NULL,
    pTextRenderer_,
    origin.x,
    origin.y);

  return hr;
}

HRESULT CustomText::DrawD2DContent()
{
  HRESULT hr;

  hr = CreateDeviceResources();

  if (!(pRT_->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
  {
    pRT_->BeginDraw();
    pRT_->SetTransform(D2D1::IdentityMatrix());
    pRT_->Clear(D2D1::ColorF(D2D1::ColorF::White));

    // Call the DrawText method of this class.
    if (SUCCEEDED(hr))
    {
      hr = DrawText();
    }

    hr = pRT_->EndDraw();
  }

  if (FAILED(hr))
  {
    DiscardDeviceResources();
  }

  return hr;
}

void CustomText::OnResize(UINT width, UINT height)
{
  if (pTextLayout_)
  {
    pTextLayout_->SetMaxWidth(static_cast<FLOAT>(width / dpiScaleX_));
    pTextLayout_->SetMaxHeight(static_cast<FLOAT>(height / dpiScaleY_));
  }

  if (pRT_)
  {
    D2D1_SIZE_U size;
    size.width = width;
    size.height = height;
    pRT_->Resize(size);
  }
}

HRESULT CustomText::LoadResourceBitmap(
  ID2D1RenderTarget* pRT_,
  IWICImagingFactory* pIWICFactory,
  PCWSTR resourceName,
  PCWSTR resourceType,
  __deref_out ID2D1Bitmap** ppBitmap)
{
  HRESULT hr = S_OK;
  
  IWICBitmapDecoder* pDecoder = NULL;
  IWICBitmapFrameDecode* pSource = NULL;
  IWICStream* pStream = NULL;
  IWICFormatConverter* pConverter = NULL;

  HRSRC imageResHandle = NULL;
  HGLOBAL imageResDataHandle = NULL;
  void* pImageFile = NULL;
  DWORD imageFileSize = 0;

  // Locate the resource handle in out dll
  imageResHandle = FindResourceW(
    HINST_THISCOMPONENT,
    resourceName,
    resourceType);

  hr = imageResHandle ? S_OK : E_FAIL;

  // Load the resource
  imageResDataHandle = LoadResource(HINST_THISCOMPONENT,
    imageResHandle);

  if (SUCCEEDED(hr))
  {
    hr = imageResDataHandle ? S_OK : E_FAIL;
  }

  // Lock it to get a system memory pointer
  pImageFile = LockResource(imageResDataHandle);

  if (SUCCEEDED(hr))
  {
    hr = pImageFile ? S_OK : E_FAIL;
  }

  // Calculate the size.
  imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);

  if (SUCCEEDED(hr))
  {
    hr = imageFileSize ? S_OK : E_FAIL;
  }

  // Create a WIC stream to map onto the memory
  if (SUCCEEDED(hr))
  {
    hr = pIWICFactory->CreateStream(&pStream);
  }

  // Initialalize the stream with the memory pointer and size
  if (SUCCEEDED(hr))
  {
    hr = pStream->InitializeFromMemory(
      reinterpret_cast<BYTE*>(pImageFile),
      imageFileSize);
  }

  // Create a decoder for the stream.
  if (SUCCEEDED(hr))
  {
    hr = pIWICFactory->CreateDecoderFromStream(
      pStream,
      NULL,
      WICDecodeMetadataCacheOnLoad,
      &pDecoder);
  }

  // Create the initial frame.
  if (SUCCEEDED(hr))
  {
    hr = pDecoder->GetFrame(0,
      &pSource);
  }

  // Format convert to 32bppPBGRA -- which Direct2D expects
  if (SUCCEEDED(hr))
  {
    hr = pIWICFactory->CreateFormatConverter(&pConverter);
  }

  if (SUCCEEDED(hr))
  {
    hr = pConverter->Initialize(
      pSource,
      GUID_WICPixelFormat32bppPBGRA,
      WICBitmapDitherTypeNone,
      NULL,
      0.f,
      WICBitmapPaletteTypeMedianCut);
  }

  // Create a Direct2D bitmap from the WIC bitmap.
  if (SUCCEEDED(hr))
  {
    hr = pRT_->CreateBitmapFromWicBitmap(
      pConverter,
      NULL,
      ppBitmap);
  }

  SafeRelease(&pDecoder);
  SafeRelease(&pSource);
  SafeRelease(&pStream);
  SafeRelease(&pConverter);

  return hr;
}

LRESULT CALLBACK CustomText::WndProc(HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  if (message == WM_CREATE)
  {
    LPCREATESTRUCT  pcs = (LPCREATESTRUCT)lParam;
    CustomText* pCustomText = (CustomText*)pcs->lpCreateParams;

    ::SetWindowLongPtrW(
      hWnd,
      GWLP_USERDATA,
      PtrToUlong(pCustomText));

    return 1;
  }

  CustomText* pCustomText = reinterpret_cast<CustomText*>(
    ::GetWindowLongPtr(hWnd, GWLP_USERDATA));

  if (pCustomText)
  {
    switch (message)
    {
    case WM_SIZE:
    {
                  UINT width = LOWORD(lParam);
                  UINT height = HIWORD(lParam);
                  pCustomText->OnResize(width, height);
    }
      return 0;
    case WM_PAINT:
    case WM_DISPLAYCHANGE:
    {
                           PAINTSTRUCT ps;
                           BeginPaint(hWnd, &ps);
                           pCustomText->DrawD2DContent();
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