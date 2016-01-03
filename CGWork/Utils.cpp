#include "Utils.h"

void CopyImage(CImage& src, CImage& dst)
{
	if (!dst.IsNull())
		dst.Destroy();
	dst.Create(src.GetWidth(), src.GetHeight(), 32);

	// copy image:
	HDC bgdc = dst.GetDC();
	src.BitBlt(bgdc, 0, 0, src.GetWidth(), src.GetHeight(), 0, 0);
	dst.ReleaseDC();
}