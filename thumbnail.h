#ifndef _PHOTOMAILER_THUMBNAIL_H_
#define _PHOTOMAILER_THUMBNAIL_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "thumbnail.h"
#endif

#include <wx/bitmap.h>
#include <wx/clntdata.h>
#include <wx/dc.h>
#include <wx/grid.h>
#include <wx/image.h>


#define THUMBNAIL_SIZE (64)


namespace PhotoMailer
{

class ThumbnailClientData : public wxClientData
{
public:
	ThumbnailClientData();
	virtual ~ThumbnailClientData();

	bool SetThumbnail(const wxImage& image);
	const wxBitmap* GetThumbnail() const {return m_thumbnail_bitmap;}

private:
	wxBitmap* m_thumbnail_bitmap;
};


class ThumbnailRenderer : public wxGridCellRenderer
{
public:
	ThumbnailRenderer();
	virtual ~ThumbnailRenderer();
	
	virtual void Draw(wxGrid& grid,
										wxGridCellAttr& attr,
										wxDC& dc,
										const wxRect& rect,
										int row, int col,
										bool isSelected) wxOVERRIDE;

	virtual int GetBestHeight() const {return THUMBNAIL_SIZE+1;}
	virtual int GetBestWidth() const {return THUMBNAIL_SIZE+1;}
	virtual wxSize GetBestSize(wxGrid& WXUNUSED(grid), wxGridCellAttr& WXUNUSED(attr), wxDC& WXUNUSED(dc),
	                           int WXUNUSED(row), int WXUNUSED(col)) wxOVERRIDE {return wxSize(GetBestWidth(), GetBestHeight());}

	virtual wxGridCellRenderer* Clone() const;
};

}

#endif // _PHOTOMAILER_THUMBNAIL_H_
