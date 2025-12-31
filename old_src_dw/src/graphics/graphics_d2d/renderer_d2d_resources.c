#include <dwrite.h>
#include <d2d1.h>

#include "renderer_d2d_resources.h"

HRESULT rdr_create_device_indepedent_resources()
{
    HRESULT hr = S_OK;

    // Create D2D1 factory
    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &IID_ID2D1Factory,
        NULL,
        (void **)&applicationState->renderContext.d2dFactory);

    if (FAILED(hr))
        return hr;

    // Create DirectWrite factory
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        &IID_IDWriteFactory,
        (IUnknown **)&applicationState->textResources.dWriteFactory);

    if (FAILED(hr))
        return hr;

    {
        IDWriteTextFormat **textFormat = &applicationState->textResources.textFormat;

        hr = IDWriteFactory_CreateTextFormat(
            applicationState->textResources.dWriteFactory,
            L"Arial",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            15.0f,
            L"en-us",
            textFormat);

        if (SUCCEEDED(hr))
        {
            IDWriteTextFormat_SetTextAlignment(*textFormat, DWRITE_TEXT_ALIGNMENT_LEADING);
            IDWriteTextFormat_SetParagraphAlignment(*textFormat, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        }
    }

    return hr;
}