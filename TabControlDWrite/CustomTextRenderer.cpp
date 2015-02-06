#include "TabControlDWrite.h"
#include "CustomTextRenderer.h"

CustomTextRenderer::CustomTextRenderer(
  ID2D1Factory* pD2DFactory,
  ID2D1HwndRenderTarget* pRT,
  ID2D1SolidColorBrush* pOutlineBrush,
  ID2D1BitmapBrush* pFillBrush)
  : cRefCount_(0)
  , pD2DFactory_(pD2DFactory)
  , pRT_(pRT)
  , pOutlineBrush_(pOutlineBrush)
  , pFillBrush_(pFillBrush)
{
  pD2DFactory_->AddRef();
  pRT_->AddRef();
  pOutlineBrush_->AddRef();
  pFillBrush_->AddRef();
}

CustomTextRenderer::~CustomTextRenderer()
{
  SafeRelease(&pD2DFactory_);
  SafeRelease(&pRT_);
  SafeRelease(&pOutlineBrush_);
  SafeRelease(&pFillBrush_);
}

IFACEMETHODIMP CustomTextRenderer::DrawGlyphRun(
  __maybenull void* clientDrawingContext,
  FLOAT baselineOriginX,
  FLOAT baselineOriginY,
  DWRITE_MEASURING_MODE measuingMode,
  __in DWRITE_GLYPH_RUN const* glyphRun,
  __in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
  IUnknown* clientDrawingEffect)
{
  HRESULT hr = S_OK;

  // Create the path geometry.
  ID2D1PathGeometry* pPathGeometry = NULL;
  hr = pD2DFactory_->CreatePathGeometry(&pPathGeometry);

  // Write to the path geometry using the geometry sink.
  ID2D1GeometrySink* pSink = NULL;
  if (SUCCEEDED(hr))
  {
    hr = pPathGeometry->Open(&pSink);
  }

  // Get the glyph run outline geometies back from DirectWrite
  // geometry sink.
  if (SUCCEEDED(hr))
  {
    hr = glyphRun->fontFace->GetGlyphRunOutline(
      glyphRun->fontEmSize,
      glyphRun->glyphIndices,
      glyphRun->glyphAdvances,
      glyphRun->glyphOffsets,
      glyphRun->glyphCount,
      glyphRun->isSideways,
      glyphRun->bidiLevel,
      pSink);
  }

  // Close the geometry sink.
  if (SUCCEEDED(hr))
  {
    hr = pSink->Close();
  }

  // Initialize a matrix to translate the origin of the glyph.
  D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
    1.0f, 0.0f,
    0.0f, 1.0f,
    baselineOriginX, baselineOriginY);

  // Create the transformed geometry.
  ID2D1TransformedGeometry* pTransformedGeometry = NULL;
  if (SUCCEEDED(hr))
  {
    hr = pD2DFactory_->CreateTransformedGeometry(
      pPathGeometry,
      &matrix,
      &pTransformedGeometry);
  }

  // Draw the outline of the glyph run.
  pRT_->DrawGeometry(
    pTransformedGeometry,
    pOutlineBrush_);

  // Fill in the glyph run.
  pRT_->FillGeometry(
    pTransformedGeometry,
    pFillBrush_);

  SafeRelease(&pPathGeometry);
  SafeRelease(&pSink);
  SafeRelease(&pTransformedGeometry);

  return hr;
}

IFACEMETHODIMP CustomTextRenderer::DrawUnderline(
  __maybenull void* clientDrawingContext,
  FLOAT baselineOriginX,
  FLOAT baselineOriginY,
  __in DWRITE_UNDERLINE const* underline,
  IUnknown* clientDrawingEffect)
{
  HRESULT hr;

  D2D_RECT_F rect = D2D1::RectF(
    0,
    underline->offset,
    underline->width,
    underline->offset + underline->thickness);

  ID2D1RectangleGeometry* pRectangleGeometry = NULL;
  hr = pD2DFactory_->CreateRectangleGeometry(&rect,
    &pRectangleGeometry);

  // Initialize a matrix to translate the origin of the underline.
  D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
    1.0f, 0.0f,
    0.0f, 1.0f,
    baselineOriginX, baselineOriginY);

  ID2D1TransformedGeometry* pTransformedGeometry = NULL;
  if (SUCCEEDED(hr))
  {
    hr = pD2DFactory_->CreateTransformedGeometry(
      pRectangleGeometry,
      &matrix,
      &pTransformedGeometry);
  }

  // Draw the outline of the rectangle.
  pRT_->DrawGeometry(
    pTransformedGeometry,
    pOutlineBrush_);

  // Fill in the rectangle.
  pRT_->FillGeometry(
    pTransformedGeometry,
    pFillBrush_);

  SafeRelease(&pRectangleGeometry);
  SafeRelease(&pTransformedGeometry);

  return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::DrawStrikethrough(
  __maybenull void* clientDrawingContext,
  FLOAT baselineOriginX,
  FLOAT baselineOriginY,
  __in DWRITE_STRIKETHROUGH const* strikethrough,
  IUnknown* clientDrawingEffect)
{
  HRESULT hr;

  D2D1_RECT_F rect = D2D1::RectF(
    0,
    strikethrough->offset,
    strikethrough->width,
    strikethrough->offset + strikethrough->thickness);

  ID2D1RectangleGeometry* pRectangleGeometry = NULL;
  hr = pD2DFactory_->CreateRectangleGeometry(
    &rect,
    &pRectangleGeometry);

  // Initialize a matrix to translate the origin of the strikethough.
  D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
    1.0f, 0.0f,
    0.0f, 1.0f,
    baselineOriginX, baselineOriginY);

  ID2D1TransformedGeometry* pTransformedGeometry = NULL;
  if (SUCCEEDED(hr))
  {
    hr = pD2DFactory_->CreateTransformedGeometry(
      pRectangleGeometry,
      &matrix,
      &pTransformedGeometry);
  }

  // Draw the outline of the rectangle.
  pRT_->DrawGeometry(
    pTransformedGeometry,
    pOutlineBrush_);

  // Fill in the rectangle.
  pRT_->FillGeometry(
    pTransformedGeometry,
    pFillBrush_);

  SafeRelease(&pRectangleGeometry);
  SafeRelease(&pTransformedGeometry);

  return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::DrawInlineObject(
  __maybenull void* clientDrawingContext,
  FLOAT originX,
  FLOAT originY,
  IDWriteInlineObject* inlineObject,
  BOOL isSideways,
  BOOL isRightToLeft,
  IUnknown* clientDraingEffect)
{
  // Not implemented.
  return E_NOTIMPL;
}

IFACEMETHODIMP_(unsigned long) CustomTextRenderer::AddRef()
{
  return InterlockedIncrement(&cRefCount_);
}

IFACEMETHODIMP_(unsigned long) CustomTextRenderer::Release()
{
  unsigned long newCount = InterlockedDecrement(&cRefCount_);
  if (newCount == 0)
  {
    delete this;
    return 0;
  }

  return newCount;
}

IFACEMETHODIMP CustomTextRenderer::IsPixelSnappingDisabled(
  __maybenull void* clientDrawingContext,
  __out BOOL* isDisabled)
{
  *isDisabled = FALSE;
  return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::GetCurrentTransform(
  __maybenull void* clientDrawingContext,
  __out DWRITE_MATRIX* transform)
{
  // forward the render target's transform
  pRT_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
  return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::GetPixelsPerDip(
  __maybenull void* clientDrawingContext,
  __out FLOAT* pixelsPerDip)
{
  *pixelsPerDip = 1.0f;
  return S_OK;
}

IFACEMETHODIMP CustomTextRenderer::QueryInterface(
  IID const& riid,
  void** ppvObject)
{
  if (__uuidof(IDWriteTextRenderer) == riid)
  {
    *ppvObject = this;
  }
  else if (__uuidof(IDWritePixelSnapping) == riid)
  {
    *ppvObject = this;
  }
  else if (__uuidof(IUnknown) == riid)
  {
    *ppvObject = this;
  }
  else
  {
    *ppvObject = NULL;
    return E_FAIL;
  }

  this->AddRef();

  return S_OK;
}