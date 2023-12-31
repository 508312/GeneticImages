/* FOLLOWING FILE IS TAKEN FROM :
                    https://github.com/openlab-vn-ua/ImageRotation/tree/master
   There may be small modifications in some functions.
*/


#include "rotate.h"
#include <math.h>

#define DEBUG_DRAW 0
#define DEBUG_MARK_COLOR ((RotatePixel_t)(0xFFFFFF))
#define DEBUG_BACK_COLOR ((RotatePixel_t)(0x3F6FCF))

#define BM_GET(src,stride,x,y) ((RotatePixel_t *)(((char*)(src)) + ((x)*sizeof(RotatePixel_t) + (y)*(stride))))[0]
#define BM_SET(dst,stride,x,y,c) ((RotatePixel_t *)(((char*)(dst)) + ((x)*sizeof(RotatePixel_t) + (y)*(stride))))[0] = (c)

#include <immintrin.h>
#include "utils.h"

#define BM_DATA_ADD_OFS(src,offset) ((RotatePixel_t *)(((char*)(src)) + (offset)))

/// <summary>
/// Checks if source value is power of 2
/// </summary>
static bool IsExp2(unsigned int value)
{
    return (value > 0 && (value & (value - 1)) == 0);
}

/// <summary>
/// Internal: Same as RotateDrawFill, but dimensions of the source image must be a power of two.
/// </summary>
static
void RotateDrawFillFastSrcSizeExp2(
    RotatePixel_t *pDstBase, int dstW, int dstH, int dstDelta,
    RotatePixel_t *pSrcBase, int srcW, int srcH, int srcDelta,
    float fDstCX, float fDstCY,
    float fSrcCX, float fSrcCY,
    float fAngle, float fScale)
{
    // IMPORTANT: This version assumes the dimensions of the source image to be a power of two.

    if (dstW <= 0) { return; }
    if (dstH <= 0) { return; }

    float duCol = (float)sin(-fAngle) * (1.0f / fScale);
    float dvCol = (float)cos(-fAngle) * (1.0f / fScale);
    float duRow = dvCol;
    float dvRow = -duCol;

    float startingu = fSrcCX - (fDstCX * dvCol + fDstCY * duCol);
    float startingv = fSrcCY - (fDstCX * dvRow + fDstCY * duRow);

    float rowu = startingu;
    float rowv = startingv;

    for(int y = 0; y < dstH; y++)
    {
        float u = rowu;
        float v = rowv;

        RotatePixel_t *pDst = BM_DATA_ADD_OFS(pDstBase, dstDelta * y);

        for(int x = 0; x < dstW ; x++)
        {
            #if DEBUG_DRAW
            if ((int(u) == int(fSrcCX)) && (int(v) == int(fSrcCY)))
            {
                *pDst++ = DEBUG_MARK_COLOR;
                u += duRow;
                v += dvRow;
                continue;
            }
            #endif

            int sx = (int)u;
            int sy = (int)v;

            // Negative u/v adjustement

            if (u < 0) { sx--; }
            if (v < 0) { sy--; }

            sx &= (srcW-1);
            sy &= (srcH-1);

            *pDst++ = BM_GET(pSrcBase, srcDelta, sx, sy);

            u += duRow;
            v += dvRow;
        }

        rowu += duCol;
        rowv += dvCol;
    }
}

void RotateDrawFill(
    RotatePixel_t *pDstBase, int dstW, int dstH, int dstDelta,
    RotatePixel_t *pSrcBase, int srcW, int srcH, int srcDelta,
    float fDstCX, float fDstCY,
    float fSrcCX, float fSrcCY,
    float fAngle, float fScale)
{
    // This version takes any dimension source bitmap and wraps.
    if (IsExp2(srcW) && IsExp2(srcH))
    {
        RotateDrawFillFastSrcSizeExp2
        (
            pDstBase, dstW,dstH, dstDelta,
            pSrcBase,srcW, srcH, srcDelta,
            fDstCX, fDstCY,
            fSrcCX, fSrcCY,
            fAngle, fScale
        );
        return;
    }

    if (dstW <= 0) { return; }
    if (dstH <= 0) { return; }

    float duCol = (float)sin(-fAngle) * (1.0f / fScale);
    float dvCol = (float)cos(-fAngle) * (1.0f / fScale);
    float duRow = dvCol;
    float dvRow = -duCol;

    float startingu = fSrcCX - (fDstCX * dvCol + fDstCY * duCol);
    float startingv = fSrcCY - (fDstCX * dvRow + fDstCY * duRow);

    float rowu = startingu;
    float rowv = startingv;

    for(int y = 0; y < dstH; y++)
    {
        float u = rowu;
        float v = rowv;

        RotatePixel_t *pDst = BM_DATA_ADD_OFS(pDstBase, dstDelta * y);

        for(int x = 0; x < dstW ; x++)
        {
            #if DEBUG_DRAW
            if ((int(u) == int(fSrcCX)) && (int(v) == int(fSrcCY)))
            {
                *pDst++ = DEBUG_MARK_COLOR;
                u += duRow;
                v += dvRow;
                continue;
            }
            #endif

            int sx = (int)u;
            int sy = (int)v;

            // Negative u/v adjustement
            // We need some additional proccessing for negative u and v values
            // value in range (-0.99..;0) should be mapped to last source pixel, not zero
            // Because zero source pixel already drawn at [0;0.99..)
            // (else we will observe double line of same colored pixels in when u/v flips from + to -)
            // Example: without shift u = -0.25 becimes sx=0, we need sx=-1, since we already had sx=0 at u=+0.25

            if (u < 0)
            {
                // Negative u/v adjustement
                sx--;
                sx = -sx % srcW; sx = srcW - sx;
            }

            sx %= srcW;

            if (v < 0)
            {
                // Negative u/v adjustement
                sy--;
                sy = -sy % srcH; sy = srcH - sy;
            }

            sy %= srcH;

            *pDst++ = BM_GET(pSrcBase, srcDelta, sx, sy);

            u += duRow;
            v += dvRow;
        }

        rowu += duCol;
        rowv += dvCol;
    }
}

//#define VOID_COLOR 0

void RotateDrawClip1(
    RotatePixel_t *pDstBase, int dstW, int dstH, int dstDelta,
    RotatePixel_t *pSrcBase, int srcW, int srcH, int srcDelta,
    float fDstCX, float fDstCY,
    float fSrcCX, float fSrcCY,
    float fAngle, float fScale)
{
    if (dstW <= 0) { return; }
    if (dstH <= 0) { return; }

    float duCol = (float)sin(-fAngle) * (1.0f / fScale);
    float dvCol = (float)cos(-fAngle) * (1.0f / fScale);
    float duRow = dvCol;
    float dvRow = -duCol;

    float startingu = fSrcCX - (fDstCX * dvCol + fDstCY * duCol);
    float startingv = fSrcCY - (fDstCX * dvRow + fDstCY * duRow);

    float rowu = startingu;
    float rowv = startingv;

    for(int y = 0; y < dstH; y++)
    {
        float u = rowu;
        float v = rowv;

        RotatePixel_t *pDst = BM_DATA_ADD_OFS(pDstBase, dstDelta * y);

        for(int x = 0; x < dstW ; x++)
        {
            #if DEBUG_DRAW
            if ((int(u) == int(fSrcCX)) && (int(v) == int(fSrcCY)))
            {
                *pDst++ = DEBUG_MARK_COLOR;
                u += duRow;
                v += dvRow;
                continue;
            }
            #endif

            int sx = (int)u;
            int sy = (int)v;

            // For non-negative values we have to check u and v (not sx and sy)
            // since u = -0.25 gives sx=0 after rounsing, so 1 extra pixel line will be drawn
            // (we assume that u,v >= 0 will lead to sx,sy >= 0)

            if ((u >= 0) && (v >= 0) && (sx < srcW) && (sy < srcH))
            {
                *pDst++ = BM_GET(pSrcBase, srcDelta, sx, sy);
            }
            else
            {
                pDst++; // Skip
                //*pDst++ = VOID_COLOR; // Fill void (black)
            }

            u += duRow;
            v += dvRow;
        }

        rowu += duCol;
        rowv += dvCol;
    }
}

void RotateDrawClipExt1
    (
        RotatePixel_t *dst, int dstW, int dstH, int dstDelta,
        RotatePixel_t *src, int srcW, int srcH, int srcDelta,
        float ox, float oy,
        float px, float py,
        float angle, float scale,
        RotateColorMergerFunc_t mergeFunc,
        void *mergeParam
    )
{
    // Optimisation based on:
    // https://github.com/wernsey/bitmap/blob/master/bmp.cpp

    // Same as RotateDrawWithClipAltD but uses float [somehow more stable]
    // px, py = pivot point in source
    // ox, oy = place where pivot point will be located in target

    angle = -angle; // to made rules consistent with RotateDrawWithClip

    if (dstW <= 0) { return; }
    if (dstH <= 0) { return; }

    int x,y;

    // fill min/max reverced (invalid) values at first
    int minx = dstW, miny = dstH;
    int maxx = 0, maxy = 0;

    float sinAngle = sin(angle);
    float cosAngle = cos(angle);

    float dx, dy;
    // Compute the position of where each corner on the source bitmap
    // will be on the destination to get a bounding box for scanning
    dx = -cosAngle * px * scale + sinAngle * py * scale + ox;
    dy = -sinAngle * px * scale - cosAngle * py * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = cosAngle * (srcW - px) * scale + sinAngle * py * scale + ox;
    dy = sinAngle * (srcW - px) * scale - cosAngle * py * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = cosAngle * (srcW - px) * scale - sinAngle * (srcH - py) * scale + ox;
    dy = sinAngle * (srcW - px) * scale + cosAngle * (srcH - py) * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = -cosAngle * px * scale - sinAngle * (srcH - py) * scale + ox;
    dy = -sinAngle * px * scale + cosAngle * (srcH - py) * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    // Clipping
    if(minx < 0) { minx = 0; }
    if(maxx > dstW - 1) { maxx = dstW - 1; }
    if(miny < 0) { miny = 0; }
    if(maxy > dstH - 1) { maxy = dstH - 1; }

    #if DEBUG_DRAW
    //minx = 0;
    //miny = 0;
    //maxx = dstW-1;
    //maxy = dstH-1;
    #endif

    float dvCol = cos(angle) / scale;
    float duCol = sin(angle) / scale;

    float duRow = dvCol;
    float dvRow = -duCol;

    float startu = px - (ox * dvCol + oy * duCol);
    float startv = py - (ox * dvRow + oy * duRow);

    float rowu = startu + miny * duCol;
    float rowv = startv + miny * dvCol;

    for(y = miny; y <= maxy; y++)
    {
        float u = rowu + minx * duRow;
        float v = rowv + minx * dvRow;

        for(x = minx; x <= maxx; x++)
        {
            #if DEBUG_DRAW
            if ((int(u) == int(px)) && (int(v) == int(py)))
            {
                BM_SET(dst, dstDelta, x, y, DEBUG_MARK_COLOR);
                u += duRow;
                v += dvRow;
                continue;
            }
            #endif

            if(u >= 0 && u < srcW && v >= 0 && v < srcH)
            {
                RotatePixel_t c = BM_GET(src, srcDelta, (int)u, (int)v);

                if (mergeFunc != NULL)
                {
                    RotatePixel_t o = BM_GET(dst, dstDelta, x, y);
                    c = mergeFunc(c, o, mergeParam);
                }

                BM_SET(dst, dstDelta, x, y, c);
            }
            else
            {
                #if DEBUG_DRAW
                BM_SET(dst, dstDelta, x, y, DEBUG_BACK_COLOR);
                #endif
            }

            u += duRow;
            v += dvRow;
        }

        rowu += duCol;
        rowv += dvCol;
    }
}

void RotateDrawClipExt1D
    (
        RotatePixel_t *dst, int dstW, int dstH, int dstDelta,
        RotatePixel_t *src, int srcW, int srcH, int srcDelta,
        double ox, double oy,
        double px, double py,
        double angle, double scale,
        RotateColorMergerFunc_t mergeFunc,
        void* mergeParam
    )
{
    // Optimisation based on:
    // https://github.com/wernsey/bitmap/blob/master/bmp.cpp

    // Same as RotateDrawWithClipAlt but uses double [somewhat computationaly unstable]
    // px, py = pivot point in source
    // ox, oy = place where pivot point will be located in target

    angle = -angle; // to made rules consistent with RotateDrawWithClip

    if (dstW <= 0) { return; }
    if (dstH <= 0) { return; }

    int x,y;

    // fill min/max reverced (invalid) values at first
    int minx = dstW, miny = dstH;
    int maxx = 0, maxy = 0;

    double sinAngle = sin(angle);
    double cosAngle = cos(angle);

    double dx, dy;
    // Compute the position of where each corner on the source bitmap
    // will be on the destination to get a bounding box for scanning
    dx = -cosAngle * px * scale + sinAngle * py * scale + ox;
    dy = -sinAngle * px * scale - cosAngle * py * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = cosAngle * (srcW - px) * scale + sinAngle * py * scale + ox;
    dy = sinAngle * (srcW - px) * scale - cosAngle * py * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = cosAngle * (srcW - px) * scale - sinAngle * (srcH - py) * scale + ox;
    dy = sinAngle * (srcW - px) * scale + cosAngle * (srcH - py) * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = -cosAngle * px * scale - sinAngle * (srcH - py) * scale + ox;
    dy = -sinAngle * px * scale + cosAngle * (srcH - py) * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    // Clipping
    if(minx < 0) { minx = 0; }
    if(maxx > dstW - 1) { maxx = dstW - 1; }
    if(miny < 0) { miny = 0; }
    if(maxy > dstH - 1) { maxy = dstH - 1; }

    #if DEBUG_DRAW
    //minx = 0;
    //miny = 0;
    //maxx = dstW-1;
    //maxy = dstH-1;
    #endif

    double dvCol = cos(angle) / scale;
    double duCol = sin(angle) / scale;

    double duRow = dvCol;
    double dvRow = -duCol;

    double startu = px - (ox * dvCol + oy * duCol);
    double startv = py - (ox * dvRow + oy * duRow);

    double rowu = startu + miny * duCol;
    double rowv = startv + miny * dvCol;

    for(y = miny; y <= maxy; y++)
    {
        double u = rowu + minx * duRow;
        double v = rowv + minx * dvRow;

        for(x = minx; x <= maxx; x++)
        {
            #if DEBUG_DRAW
            if ((int(u) == int(px)) && (int(v) == int(py)))
            {
                BM_SET(dst, dstDelta, x, y, DEBUG_MARK_COLOR);
                u += duRow;
                v += dvRow;
                continue;
            }
            #endif

            if(u >= 0 && u < srcW && v >= 0 && v < srcH)
            {
                RotatePixel_t c = BM_GET(src, srcDelta, (int)u, (int)v);

                if (mergeFunc != NULL)
                {
                    RotatePixel_t o = BM_GET(dst, dstDelta, x, y);
                    c = mergeFunc(c, o, mergeParam);
                }

                BM_SET(dst, dstDelta, x, y, c);
            }
            else
            {
                #if DEBUG_DRAW
                BM_SET(dst, dstDelta, x, y, DEBUG_BACK_COLOR);
                #endif
            }

            u += duRow;
            v += dvRow;
        }

        rowu += duCol;
        rowv += dvCol;
    }
}

void RotateDrawClipExt2
    (
        RotatePixel_t *dst, int dstW, int dstH, int dstDelta,
        RotatePixel_t *src, int srcW, int srcH, int srcDelta,
        float ox, float oy,
        float px, float py,
        float angle, float scale,
        RotateColorMergerFunc_t mergeFunc,
        void *mergeParam
    )
{
    // Optimisation based on:
    // https://github.com/wernsey/bitmap/blob/master/bmp.cpp
    // Additional optimization inspired by:
    // http://www.gamedev.ru/code/forum/?id=156842
    // We assume that int is at least 32 bit integer here for all vars with i postfix

    angle = -angle; // to made rules consistent with RotateDrawWithClip

    if (dstW <= 0) { return; }
    if (dstH <= 0) { return; }

    int x,y;

    // fill min/max reverced (invalid) values at first
    int minx = dstW, miny = dstH;
    int maxx = 0, maxy = 0;

    float sinAngle = sin(angle);
    float cosAngle = cos(angle);

    float dx, dy;
    // Compute the position of where each corner on the source bitmap
    // will be on the destination to get a bounding box for scanning
    dx = -cosAngle * px * scale + sinAngle * py * scale + ox;
    dy = -sinAngle * px * scale - cosAngle * py * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = cosAngle * (srcW - px) * scale + sinAngle * py * scale + ox;
    dy = sinAngle * (srcW - px) * scale - cosAngle * py * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = cosAngle * (srcW - px) * scale - sinAngle * (srcH - py) * scale + ox;
    dy = sinAngle * (srcW - px) * scale + cosAngle * (srcH - py) * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = -cosAngle * px * scale - sinAngle * (srcH - py) * scale + ox;
    dy = -sinAngle * px * scale + cosAngle * (srcH - py) * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    // Clipping
    if(minx < 0) { minx = 0; }
    if(maxx > dstW - 1) { maxx = dstW - 1; }
    if(miny < 0) { miny = 0; }
    if(maxy > dstH - 1) { maxy = dstH - 1; }

    #if DEBUG_DRAW
    //minx = 0;
    //miny = 0;
    //maxx = dstW-1;
    //maxy = dstH-1;
    #endif

    // Let assume that image size is less that 16K x 16K
    // in that case,

    float dvCol = cosAngle / scale;
    float duCol = sinAngle / scale;

    float duRow = dvCol;
    float dvRow = -duCol;

    float startu = px - (ox * dvCol + oy * duCol);
    float startv = py - (ox * dvRow + oy * duRow);

    float rowu = startu + miny * duCol;
    float rowv = startv + miny * dvCol;

    // Scale all to 32 bit int

    #define OPT_INT_ROW // use int math on row step

    int    ISCALE_SHIFT  = 16;
    int    ISCALE_FACTOR = ((int)1) << (ISCALE_SHIFT); // ISCALE_SHIFT=16, ISCALE_FACTOR=65536

    #define OPT_INT_PRECALC

    #ifdef  OPT_INT_PRECALC

    int    pxi = px * ISCALE_FACTOR;
    int    pyi = py * ISCALE_FACTOR;

    //int  oxi = ox * ISCALE_FACTOR;
    //int  oyi = oy * ISCALE_FACTOR;

    int    dvColi = dvCol * ISCALE_FACTOR;
    int    duColi = duCol * ISCALE_FACTOR;
    int    duRowi = dvColi;
    int    dvRowi = -duColi;

    int    startui = pxi - (ox * dvColi + oy * duColi);
    int    startvi = pyi - (ox * dvRowi + oy * duRowi);

    int    rowui = startui + miny * duColi;
    int    rowvi = startvi + miny * dvColi;

    #else

    int    dvColi = dvCol * ISCALE_FACTOR;
    int    duColi = duCol * ISCALE_FACTOR;
    int    duRowi = duRow * ISCALE_FACTOR;
    int    dvRowi = dvRow * ISCALE_FACTOR;

    int    startui = startu * ISCALE_FACTOR;
    int    startvi = startv * ISCALE_FACTOR;

    int    rowui = rowu * ISCALE_FACTOR;
    int    rowvi = rowv * ISCALE_FACTOR;

    #endif

    #if DEBUG_DRAW
    int pxii = px;
    int pyii = py;
    #endif

    #define OPT_SRC_ADDR // use int add/sub instead of mul where possible
    #define OPT_DST_ADDR // use ++ in dst addr instead of BM_SET

    #ifdef OPT_SRC_ADDR
    RotatePixel_t *srcCurrent = src;
    int srcCurrentu = 0;
    int srcCurrentv = 0;
    #endif

    for(y = miny; y <= maxy; y++)
    {
        #ifdef OPT_INT_ROW
        int ui = rowui + minx * duRowi;
        int vi = rowvi + minx * dvRowi;
        #else
        float u = rowu + minx * duRow;
        float v = rowv + minx * dvRow;
        int  ui = u * ISCALE_FACTOR;
        int  vi = v * ISCALE_FACTOR;
        #endif

        #ifdef OPT_DST_ADDR
        RotatePixel_t *dstCurrent = BM_DATA_ADD_OFS(dst, (y * dstDelta));
        dstCurrent += minx;
        #endif

        for(x = minx; x <= maxx; x++)
        {
            int uii = ui >> ISCALE_SHIFT;
            int vii = vi >> ISCALE_SHIFT;

            #if DEBUG_DRAW
            if ((uii == pxii) && (vii == pyii))
            {
                BM_SET(dst, dstDelta, x, y, DEBUG_MARK_COLOR);
                ui += duRowi;
                vi += dvRowi;
                #ifdef OPT_DST_ADDR
                dstCurrent++;
                #endif
                continue;
            }
            #endif

            // For some reason (that needs more investigations) we may need to check for ui > 0 as well
            // else at angle 0 it draws 1 more pixel on the image left with zoom ~> 5
            // This check is not added here because it would not draw left column with it with zoom ~= 1

            if(uii >= 0 && uii < srcW && vii >= 0 && vii < srcH)
            {
                RotatePixel_t c;

                #ifdef OPT_SRC_ADDR
                int dv = vii - srcCurrentv;

                if (dv == 0)
                {
                    // Noithing to do
                }
                else if (dv == 1)
                {
                    srcCurrent = BM_DATA_ADD_OFS(srcCurrent, srcDelta);
                }
                else if (dv == -1)
                {
                    srcCurrent = BM_DATA_ADD_OFS(srcCurrent, -srcDelta);
                }
                else
                {
                    srcCurrent = BM_DATA_ADD_OFS(srcCurrent, srcDelta * dv);
                }

                srcCurrentv = vii;

                int du = uii - srcCurrentu;

                srcCurrent += du;

                srcCurrentu = uii;

                c = *srcCurrent;
                #else
                c = BM_GET(src, srcDelta, uii, vii);
                #endif

                if (mergeFunc != NULL)
                {
                    RotatePixel_t o = *dstCurrent;
                    c = mergeFunc(c, o, mergeParam);
                }

                RotatePixel_t o = *dstCurrent;
                //uint8_t mix = (((o) & 0xff) + ((c) & 0xff))/255;
                //uint32_t s = 255 - mix;
                /*
                c = ((((((c >> 0)  & 0xff) * s +
                   ((o >> 0)  & 0xff) * mix) >> 8)) |
                (((((c >> 8)  & 0xff) * s +
                   ((o >> 8)  & 0xff) * mix))  & ~0xff) |
                (((((c >> 16) & 0xff) * s +
                   ((o >> 16) & 0xff) * mix) << 8)  & ~0xffff) |
                (((((c >> 24) & 0xff) * s +
                   ((o >> 24) & 0xff) * mix) << 16) & ~0xffffff));
                   */
                #ifdef OPT_DST_ADDR
                *dstCurrent++ = c;
                #else
                BM_SET(dst, dstDelta, x, y, c);
                #endif
            }
            else
            {
                #if DEBUG_DRAW
                #ifdef OPT_DST_ADDR
                *dstCurrent++ = DEBUG_BACK_COLOR;
                #else
                BM_SET(dst, dstDelta, x, y, DEBUG_BACK_COLOR);
                #endif
                #else
                #ifdef OPT_DST_ADDR
                dstCurrent++;
                #endif
                #endif
            }

            ui += duRowi;
            vi += dvRowi;
        }

        #ifdef OPT_INT_ROW
        rowui += duColi;
        rowvi += dvColi;
        #else
        rowu += duCol;
        rowv += dvCol;
        #endif
    }

    #ifdef OPT_INT_ROW
    #undef OPT_INT_ROW
    #endif

    #ifdef OPT_SRC_ADDR
    #undef OPT_SRC_ADDR
    #endif

    #ifdef OPT_DST_ADDR
    #undef OPT_DST_ADDR
    #endif
}

sadPair RotateDrawClipSad
    (
        RotatePixel_t *dst, int dstW, int dstH, int dstDelta,
        RotatePixel_t *src, int srcW, int srcH, int srcDelta,
        RotatePixel_t *pst,
        float ox, float oy,
        float px, float py,
        float angle, float scale
    )
{
    //dst[0] = 0;
    int sad = 0;
    int sad2 = 0;
    int cnt = 0;
    const int SAD_VEC_SIZE = 256;

    angle = -angle; // to made rules consistent with RotateDrawWithClip

    if (dstW <= 0) { return sadPair{0, 0}; }
    if (dstH <= 0) { return sadPair{0, 0}; }

    int x,y;

    // fill min/max reverced (invalid) values at first
    int minx = dstW, miny = dstH;
    int maxx = 0, maxy = 0;

    float sinAngle = sin(angle);
    float cosAngle = cos(angle);

    float dx, dy;
    // Compute the position of where each corner on the source bitmap
    // will be on the destination to get a bounding box for scanning
    dx = -cosAngle * px * scale + sinAngle * py * scale + ox;
    dy = -sinAngle * px * scale - cosAngle * py * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = cosAngle * (srcW - px) * scale + sinAngle * py * scale + ox;
    dy = sinAngle * (srcW - px) * scale - cosAngle * py * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = cosAngle * (srcW - px) * scale - sinAngle * (srcH - py) * scale + ox;
    dy = sinAngle * (srcW - px) * scale + cosAngle * (srcH - py) * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    dx = -cosAngle * px * scale - sinAngle * (srcH - py) * scale + ox;
    dy = -sinAngle * px * scale + cosAngle * (srcH - py) * scale + oy;
    if(dx < minx) { minx = dx; }
    if(dx > maxx) { maxx = dx; }
    if(dy < miny) { miny = dy; }
    if(dy > maxy) { maxy = dy; }

    // Clipping
    if(minx < 0) { minx = 0; }
    if(maxx > dstW - 1) { maxx = dstW - 1; }
    if(miny < 0) { miny = 0; }
    if(maxy > dstH - 1) { maxy = dstH - 1; }

    #if DEBUG_DRAW
    //minx = 0;
    //miny = 0;
    //maxx = dstW-1;
    //maxy = dstH-1;
    #endif

    // Let assume that image size is less that 16K x 16K
    // in that case,

    float dvCol = cosAngle / scale;
    float duCol = sinAngle / scale;

    float duRow = dvCol;
    float dvRow = -duCol;

    float startu = px - (ox * dvCol + oy * duCol);
    float startv = py - (ox * dvRow + oy * duRow);

    float rowu = startu + miny * duCol;
    float rowv = startv + miny * dvCol;

    // Scale all to 32 bit int

    #define OPT_INT_ROW // use int math on row step

    int    ISCALE_SHIFT  = 16;
    int    ISCALE_FACTOR = ((int)1) << (ISCALE_SHIFT); // ISCALE_SHIFT=16, ISCALE_FACTOR=65536

    #define OPT_INT_PRECALC

    #ifdef  OPT_INT_PRECALC

    int    pxi = px * ISCALE_FACTOR;
    int    pyi = py * ISCALE_FACTOR;

    //int  oxi = ox * ISCALE_FACTOR;
    //int  oyi = oy * ISCALE_FACTOR;

    int    dvColi = dvCol * ISCALE_FACTOR;
    int    duColi = duCol * ISCALE_FACTOR;
    int    duRowi = dvColi;
    int    dvRowi = -duColi;

    int    startui = pxi - (ox * dvColi + oy * duColi);
    int    startvi = pyi - (ox * dvRowi + oy * duRowi);

    int    rowui = startui + miny * duColi;
    int    rowvi = startvi + miny * dvColi;

    #else

    int    dvColi = dvCol * ISCALE_FACTOR;
    int    duColi = duCol * ISCALE_FACTOR;
    int    duRowi = duRow * ISCALE_FACTOR;
    int    dvRowi = dvRow * ISCALE_FACTOR;

    int    startui = startu * ISCALE_FACTOR;
    int    startvi = startv * ISCALE_FACTOR;

    int    rowui = rowu * ISCALE_FACTOR;
    int    rowvi = rowv * ISCALE_FACTOR;

    #endif

    #if DEBUG_DRAW
    int pxii = px;
    int pyii = py;
    #endif

    #define OPT_SRC_ADDR // use int add/sub instead of mul where possible
    #define OPT_DST_ADDR // use ++ in dst addr instead of BM_SET

    #ifdef OPT_SRC_ADDR
    RotatePixel_t *srcCurrent = src;
    int srcCurrentu = 0;
    int srcCurrentv = 0;
    #endif

    // very fast
    uint32_t arrsad1[SAD_VEC_SIZE];
    uint32_t arrsad2[SAD_VEC_SIZE];
    uint32_t arrsad3[SAD_VEC_SIZE];
    uint32_t* arrsad1ptr = arrsad1;
    uint32_t* arrsad2ptr = arrsad2;
    uint32_t* arrsad3ptr = arrsad3;
    for(y = miny; y <= maxy; y++)
    {
        #ifdef OPT_INT_ROW
        int ui = rowui + minx * duRowi;
        int vi = rowvi + minx * dvRowi;
        #else
        float u = rowu + minx * duRow;
        float v = rowv + minx * dvRow;
        int  ui = u * ISCALE_FACTOR;
        int  vi = v * ISCALE_FACTOR;
        #endif

        #ifdef OPT_DST_ADDR
        RotatePixel_t *dstCurrent = BM_DATA_ADD_OFS(dst, (y * dstDelta));
        dstCurrent += minx;

        RotatePixel_t *pstCurrent = BM_DATA_ADD_OFS(pst, (y * dstDelta));
        pstCurrent += minx;
        #endif

        for(x = minx; x <= maxx; x++)
        {
            int uii = ui >> ISCALE_SHIFT;
            int vii = vi >> ISCALE_SHIFT;

            #if DEBUG_DRAW
            if ((uii == pxii) && (vii == pyii))
            {
                BM_SET(dst, dstDelta, x, y, DEBUG_MARK_COLOR);
                ui += duRowi;
                vi += dvRowi;
                #ifdef OPT_DST_ADDR
                dstCurrent++;
                #endif
                continue;
            }
            #endif

            // For some reason (that needs more investigations) we may need to check for ui > 0 as well
            // else at angle 0 it draws 1 more pixel on the image left with zoom ~> 5
            // This check is not added here because it would not draw left column with it with zoom ~= 1

            if(uii >= 0 && uii < srcW && vii >= 0 && vii < srcH)
            {
                RotatePixel_t c;

                #ifdef OPT_SRC_ADDR
                int dv = vii - srcCurrentv;

                if (dv == 0)
                {
                    // Noithing to do
                }
                else if (dv == 1)
                {
                    srcCurrent = BM_DATA_ADD_OFS(srcCurrent, srcDelta);
                }
                else if (dv == -1)
                {
                    srcCurrent = BM_DATA_ADD_OFS(srcCurrent, -srcDelta);
                }
                else
                {
                    srcCurrent = BM_DATA_ADD_OFS(srcCurrent, srcDelta * dv);
                }

                srcCurrentv = vii;

                int du = uii - srcCurrentu;

                srcCurrent += du;

                srcCurrentu = uii;
                c = *srcCurrent;
                #else
                c = BM_GET(src, srcDelta, uii, vii);
                #endif

                #ifdef OPT_DST_ADDR

                if (cnt != 0 && cnt%SAD_VEC_SIZE == 0) {
                    sad += compute_sad((uint8_t*)arrsad1, (uint8_t*)arrsad2, SAD_VEC_SIZE*4);
                    sad2 += compute_sad((uint8_t*)arrsad1, (uint8_t*)arrsad3, SAD_VEC_SIZE*4);
                    arrsad1ptr = arrsad1;
                    arrsad2ptr = arrsad2;
                    arrsad3ptr = arrsad3;
                }
                *arrsad1ptr++ = *dstCurrent;
                *arrsad2ptr++ = *srcCurrent;
                *arrsad3ptr++ = *pstCurrent;
                cnt++;

                //sad += compute_sad_naive((uint8_t*)dstCurrent, (uint8_t*)srcCurrent, 4);
                /*
                if (*((uint8_t*)dstCurrent) > *((uint8_t*)srcCurrent))
                    sad += *((uint8_t*)dstCurrent) - *((uint8_t*)srcCurrent);
                else
                    sad += *((uint8_t*)srcCurrent) - *((uint8_t*)dstCurrent);
                if (*((uint8_t*)dstCurrent) > *((uint8_t*)srcCurrent))
                    sad += *((uint8_t*)dstCurrent+1) - *((uint8_t*)srcCurrent+1);
                else
                    sad += *((uint8_t*)srcCurrent+1) - *((uint8_t*)dstCurrent+1);
                if (*((uint8_t*)dstCurrent) > *((uint8_t*)srcCurrent))
                    sad += *((uint8_t*)dstCurrent+2) - *((uint8_t*)srcCurrent+2);
                else
                    sad += *((uint8_t*)srcCurrent+2) - *((uint8_t*)dstCurrent+2);
                if (*((uint8_t*)dstCurrent) > *((uint8_t*)srcCurrent))
                    sad += *((uint8_t*)dstCurrent+3) - *((uint8_t*)srcCurrent+3);
                else
                    sad += *((uint8_t*)srcCurrent+3) - *((uint8_t*)dstCurrent+3);
                */

                dstCurrent++;
                pstCurrent++;
                #else

                dstCurrent++;
                BM_SET(dst, dstDelta, x, y, c);
                #endif
            }
            else
            {
                #if DEBUG_DRAW
                #ifdef OPT_DST_ADDR
                *dstCurrent++ = DEBUG_BACK_COLOR;
                #else
                BM_SET(dst, dstDelta, x, y, DEBUG_BACK_COLOR);
                #endif
                #else
                #ifdef OPT_DST_ADDR
                dstCurrent++;
                pstCurrent++;
                #endif
                #endif
            }

            ui += duRowi;
            vi += dvRowi;
        }

        #ifdef OPT_INT_ROW
        rowui += duColi;
        rowvi += dvColi;
        #else
        rowu += duCol;
        rowv += dvCol;
        #endif
    }

    #ifdef OPT_INT_ROW
    #undef OPT_INT_ROW
    #endif

    #ifdef OPT_SRC_ADDR
    #undef OPT_SRC_ADDR
    #endif

    #ifdef OPT_DST_ADDR
    #undef OPT_DST_ADDR
    #endif
    sad += compute_sad((uint8_t*)arrsad1, (uint8_t*)arrsad2, (cnt%SAD_VEC_SIZE)*4);
    sad2 += compute_sad((uint8_t*)arrsad1, (uint8_t*)arrsad3, (cnt%SAD_VEC_SIZE)*4);
    sadPair pr;
    pr.sad1 = sad;
    pr.sad2 = sad2;
    return pr;//compute_sad((uint8_t*)arrsad1, (uint8_t*)arrsad2, sad * 4);
}


// External default functions
// --------------------------------------------------------

void RotateDrawClip
(
    RotatePixel_t* pDstBase, int dstW, int dstH, int dstDelta,
    RotatePixel_t* pSrcBase, int srcW, int srcH, int srcDelta,
    float fDstRotCenterX, float fDstRotCenterY,
    float fSrcRotCenterX, float fSrcRotCenterY,
    float fAngle, float fScale
)
{
    RotateDrawClipExt2
    (
        pDstBase, dstW, dstH, dstDelta,
        pSrcBase, srcW, srcH, srcDelta,
        fDstRotCenterX, fDstRotCenterY,
        fSrcRotCenterX, fSrcRotCenterY,
        fAngle, fScale
    );
}

void RotateDrawClipExt
(
    RotatePixel_t* pDstBase, int dstW, int dstH, int dstDelta,
    RotatePixel_t* pSrcBase, int srcW, int srcH, int srcDelta,
    float fDstRotCenterX, float fDstRotCenterY,
    float fSrcRotCenterX, float fSrcRotCenterY,
    float fAngle, float fScale,
    RotateColorMergerFunc_t mergeFunc,
    void* mergeParam
)
{
    RotateDrawClipExt2
    (
        pDstBase, dstW, dstH, dstDelta,
        pSrcBase, srcW, srcH, srcDelta,
        fDstRotCenterX, fDstRotCenterY,
        fSrcRotCenterX, fSrcRotCenterY,
        fAngle, fScale,
        mergeFunc,
        mergeParam
    );
}
