/////////////////////////////////////////////////////////////////////////
// Image.cpp

#include <Framework/Controls/Image.h>
#include <System/Graphics/Solidcolorbrush.h>

namespace suic
{

ImplementRTTIOfClass(Image, FrameworkElement)

DpProperty* Image::SourceProperty;
DpProperty* Image::StretchProperty;
DpProperty* Image::ZoomRatioProperty;
DpProperty* Image::StretchDirectionProperty;
DpProperty* Image::EnableGreyProperty;

AutoObj StretchConvert::ConvertValue(DpProperty* dp, Object* prop)
{
    String val = prop->ToString();

    if (val.Equals(_T("Uniform")))
    {
        return StretchBox::UniformBox;;
    }
    else if (val.Equals(_T("Fill")))
    {
        return StretchBox::FillBox;;
    }
    else if (val.Equals(_T("UniformToFill")))
    {
        return StretchBox::UniformToFillBox;;
    }
    else
    {
        return StretchBox::NoneBox;
    }
}

//String StretchConvert::ToString(DpProperty* dp, Object* prop)
//{
//    String strVal;
//    int iVal = prop->ToInt();
//
//    switch (iVal)
//    {
//    case Stretch::stUniform:
//        strVal = _U("Uniform");
//        break;
//
//    case Stretch::stUniformToFill:
//        strVal = _U("UniformToFill");
//        break;
//
//    case Stretch::stFill:
//        strVal = _U("Fill");
//        break;
//
//    default:
//
//        strVal = _U("None");
//    }
//
//    return strVal;
//}

StretchConvert* StretchConvert::Convert = new StretchConvert();

class StretchDirectionConvert : public IConvertValue
{
public:

    AutoObj ConvertValue(DpProperty* dp, Object* prop)
    {
        String val = prop->ToString();

        if (val.Equals(_T("UpOnly")))
        {
            return StretchDirectionBox::UpOnlyBox;
        }
        else if (val.Equals(_T("DownOnly")))
        {
            return StretchDirectionBox::DownOnlyBox;
        }
        else
        {
            return StretchDirectionBox::BothBox;
        }
    }

    String ToString(DpProperty* dp, Object* prop)
    {
        String strVal;
        int iVal = prop->ToInt();

        switch (iVal)
        {
        case StretchDirection::sdDownOnly:
            strVal = _U("DownOnly");
            break;

        case StretchDirection::sdUpOnly:
            strVal = _U("UpOnly");
            break;

        default:

            strVal = _U("Both");
        }

        return strVal;
    }
};


Image::Image()
{
}

Image::~Image()
{
}

void Image::OnSourcePropChanged(DpObject* d, DpPropChangedEventArg* e)
{

}

bool Image::StaticInit()
{
    if (SourceProperty == NULL)
    {
        FrameworkElement::StaticInit();

        SourceProperty = DpProperty::Register(_U("Source"), RTTIType(), ImageSource::RTTIType()
            , DpPropMemory::GetPropMeta(NULL, PropMetadataOptions::AffectsMeasure, &Image::OnSourcePropChanged));
        StretchProperty = DpProperty::Register(_U("Stretch"), RTTIType(), Integer::RTTIType()
            , DpPropMemory::GetPropMeta(StretchBox::UniformBox, PropMetadataOptions::AffectsMeasure));
        StretchDirectionProperty = DpProperty::Register(_U("StretchDirection"), RTTIType(), Integer::RTTIType()
            , DpPropMemory::GetPropMeta(StretchDirectionBox::BothBox, PropMetadataOptions::AffectsMeasure));
        EnableGreyProperty = DpProperty::Register(_U("EnableGrey"), RTTIType(), Boolean::RTTIType()
            , DpPropMemory::GetPropMeta(Boolean::False, PropMetadataOptions::AffectsRender));

        ZoomRatioProperty = DpProperty::Register(_U("ZoomRatio"), RTTIType(), OFloat::RTTIType()
            , DpPropMemory::GetPropMeta(new OFloat(1.0), PropMetadataOptions::AffectsMeasure));

        StretchProperty->SetConvertValueCb(StretchConvert::Convert);
        StretchDirectionProperty->SetConvertValueCb(new StretchDirectionConvert());
    }

    return true;
}

static fSize ComputeScaleFactor(Size availableSize, Size contentSize, int stretch, int stretchDirection)
{
    Float width = 1.0;
    Float height = 1.0;
    bool maxWid = Numeric::IsMeasureInt(availableSize.Width());
    bool maxHei = Numeric::IsMeasureInt(availableSize.Height());

    if (maxWid && maxHei)
    {
        return fSize(width, height);
    }

    width = contentSize.Width() == 0 ? 0.0 : ((Float) availableSize.Width() / (Float) contentSize.Width());
    height = contentSize.Height() == 0 ? 0.0 : ((Float) availableSize.Height() / (Float) contentSize.Height());

    if (maxWid)
    {
        width = height;
    }
    else if (maxHei)
    {
        height = width;
    }
    else
    {
        switch (stretch)
        {
        // 拉伸至整个区域
        case Stretch::stFill:
            break;

        // 按照高和框最大值缩放显示
        case Stretch::stUniform:
            {
                Float num3 = (width < height) ? width : height;
                width = height = num3;
                break;
            }
        // 按照高和框最小值缩放显示（图像显示可能会超过显示区域）
        case Stretch::stUniformToFill:
            {
                Float num4 = (width > height) ? width : height;
                width = height = num4;
                break;
            }
        }
    }

    switch (stretchDirection)
    {
    case StretchDirection::sdUpOnly:
        if (width < 1.0)
        {
            width = 1.0;
        }
        if (height < 1.0)
        {
            height = 1.0;
        }
        break;

    case StretchDirection::sdDownOnly:
        if (width > 1.0)
        {
            width = 1.0;
        }
        if (height > 1.0)
        {
            height = 1.0;
        }
        break;

    case StretchDirection::sdBoth:
        break;
    }

    return fSize(width, height);
}

Size Image::InternalMeasureArrange(Size inputSize)
{
    ImageSource* source = GetSource();
    Size contentSize;

    if (source == NULL)
    {
        return contentSize;
    }

    // 下载图像
    contentSize.cx = source->GetBitmap()->Width();
    contentSize.cy = source->GetBitmap()->Height();

    Float rzoom = Math::Max(0.2f, GetZoomRatio());
    fSize fRadio = ComputeScaleFactor(inputSize, contentSize, GetStretch(), GetStretchDirection());

    fRadio.cx *= rzoom;
    fRadio.cy *= rzoom;

    return Size(contentSize.Width() * fRadio.Width(), contentSize.Height() * fRadio.Height());
}

Size Image::OnMeasure(const Size& constraint)
{
    return InternalMeasureArrange(constraint);
}

void Image::OnArrange(const Size& arrangeSize)
{
    FrameworkElement::OnArrange(arrangeSize);
}

ImageSource* Image::GetSource()
{
    return RTTICast<ImageSource>(GetValue(SourceProperty));
}

void Image::SetSource(ImageSource* val)
{
    SetValue(SourceProperty, val);
}

void Image::OnRender(Drawing * drawing)
{
    ImageSource* img = GetSource();
    if (img != NULL)
    {
        Bitmap* bmp = img->GetBitmap();
        bool bGrey = GetValue(EnableGreyProperty)->ToBool();

        if (bGrey)
        {
            bmp->Backup();
            bmp->EraseGray();
        }

        fRect rect(Point(), GetRenderSize());
        fRect rcimg(0, 0, (Float)bmp->Width(), (Float)bmp->Height());

        drawing->DrawImage(DrawCtx::DefDraw, bmp, &rect, &rcimg, 255);

        if (bGrey)
        {
            bmp->Restore();
            bmp->ResetBackup();
        }
    }
}

}
