#ifndef _PHOTOMAILER_THUMBNAIL_H_
#define _PHOTOMAILER_THUMBNAIL_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "thumbnail.h"
#endif

#include <memory>

#include <wx/bitmap.h>
#include <wx/clntdata.h>
#include <wx/dc.h>
#include <wx/grid.h>
#include <wx/image.h>

#include "frame.h"


#define THUMBNAIL_SIZE (64)


namespace PhotoMailer
{

class ThumbnailClientData : public wxClientData
{
public:
	ThumbnailClientData();

	void SetThumbnail(const wxBitmap& bitmap) {m_thumbnail_bitmap=bitmap;}
	void GetThumbnail(wxBitmap& bitmap) const {bitmap=m_thumbnail_bitmap;}

private:
	wxBitmap m_thumbnail_bitmap;
};


class ThumbnailRenderer : public wxGridCellRenderer
{
public:
	ThumbnailRenderer();
	
	virtual void Draw(wxGrid& grid,
										wxGridCellAttr& attr,
										wxDC& dc,
										const wxRect& rect,
										int row, int col,
										bool isSelected) wxOVERRIDE;

	virtual int GetBestWidth() const {return THUMBNAIL_SIZE+1;}
	virtual int GetBestHeight() const {return THUMBNAIL_SIZE+1;}
	virtual wxSize GetBestSize(wxGrid& WXUNUSED(grid), wxGridCellAttr& WXUNUSED(attr), wxDC& WXUNUSED(dc),
	                           int WXUNUSED(row), int WXUNUSED(col)) wxOVERRIDE {return wxSize(GetBestWidth(), GetBestHeight());}

	virtual wxGridCellRenderer* Clone() const;
};



class PhotoMailerFrame;
class ThumbnailThread : public wxThread
{
public:
	ThumbnailThread(wxSemaphore* semaphore, const RowId& row_id, const wxString& filename);
	virtual ~ThumbnailThread();

public: //wxThread
	virtual wxThread::ExitCode Entry() wxOVERRIDE;

private:
	wxThread::ExitCode CleanupAndExit(int exit_code);
	bool CreateThumbnailBitmap(const wxImage& image, wxBitmap& bitmap);	
	
private:
	wxSemaphore* m_semaphore;
	RowId m_row_id;
	wxString m_filename;
};

class ThumbnailEventPayload
{
public:
	ThumbnailEventPayload(const RowId& row_id, const wxBitmap& bitmap, const wxDateTime& exif_timestamp);

	RowId GetRowId() const {return m_row_id;}
	const wxBitmap* GetBitmap() const {return &m_bitmap;}
	wxDateTime GetTimestamp() const {return m_exif_timestamp;}

private:
	RowId m_row_id;
	wxBitmap m_bitmap;
	wxDateTime m_exif_timestamp;
};

}

#endif // _PHOTOMAILER_THUMBNAIL_H_
