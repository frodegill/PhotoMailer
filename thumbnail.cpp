// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "thumbnail.h"
#endif

#include "thumbnail.h"


using namespace PhotoMailer;


ThumbnailClientData::ThumbnailClientData()
: wxClientData(),
  m_thumbnail_bitmap(nullptr)
{
}

ThumbnailClientData::~ThumbnailClientData()
{
	delete m_thumbnail_bitmap;
}

bool ThumbnailClientData::SetThumbnail(const wxImage& image)
{
	delete m_thumbnail_bitmap; m_thumbnail_bitmap=nullptr;
	if (!image.IsOk())
		return false;

	int image_width = image.GetWidth();
	int image_height = image.GetHeight();
	if (0>=image_width || 0>=image_height)
		return false;

	float image_ratio = static_cast<float>(image_width)/static_cast<float>(image_height);
	if (1.0>image_ratio) //Portrait
	{
		image_width = static_cast<float>(THUMBNAIL_SIZE)*image_ratio;
		image_height = THUMBNAIL_SIZE;
	}
	else //Landscape
	{
		image_width = THUMBNAIL_SIZE;
		image_height = static_cast<float>(THUMBNAIL_SIZE)/image_ratio;
	}

	m_thumbnail_bitmap = new wxBitmap(image.Scale(image_width, image_height, wxIMAGE_QUALITY_HIGH));
	if (!m_thumbnail_bitmap->IsOk())
	{
		delete m_thumbnail_bitmap; m_thumbnail_bitmap=nullptr;
		return false;
	}

	return true;
}


ThumbnailRenderer::ThumbnailRenderer()
: wxGridCellRenderer()
{
}

ThumbnailRenderer::~ThumbnailRenderer()
{
}

void ThumbnailRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect,
                             int row, int col, bool isSelected)
{
	wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

#ifndef CLIENTDATA_FIX
	wxGridCellAttr* attr2 = grid.GetOrCreateCellAttr(row, col);
	ThumbnailClientData* thumbnail_clientdata = static_cast<ThumbnailClientData*>(attr2?attr2->GetClientObject():nullptr);
#else
	ThumbnailClientData* thumbnail_clientdata = static_cast<ThumbnailClientData*>(attr.GetClientObject());
#endif
	if (thumbnail_clientdata)
	{
		const wxBitmap* thumbnail_bitmap = thumbnail_clientdata->GetThumbnail();
		if (thumbnail_bitmap && thumbnail_bitmap->IsOk())
		{
			dc.DrawBitmap(*thumbnail_bitmap,
										rect.GetLeft() + (THUMBNAIL_SIZE-thumbnail_bitmap->GetWidth())/2,
										rect.GetTop() + (THUMBNAIL_SIZE-thumbnail_bitmap->GetHeight())/2);
		}
	}
#ifndef CLIENTDATA_FIX
	attr2->DecRef();
#endif
}

wxGridCellRenderer* ThumbnailRenderer::Clone() const
{
	return new ThumbnailRenderer;
}
