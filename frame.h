#ifndef _PHOTOMAILER_FRAME_H_
#define _PHOTOMAILER_FRAME_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "frame.h"
#endif

#include <wx/dir.h>

#include <unordered_set>

#include "PhotoMailerGenerated.h"

#include "ftp/CFtpServer.h"


#define ID_COLUMN        (0)
#define THUMBNAIL_COLUMN (1)
#define FILENAME_COLUMN  (2)
#define TIMESTAMP_COLUMN (3)
#define EMAIL_COLUMN     (4)
#define ACTION_COLUMN    (5)
#define COLUMN_COUNT     (ACTION_COLUMN+1)

#define FTP_UPLOAD_EVENT    (wxID_HIGHEST-4)
#define PREVIEW_EVENT       (wxID_HIGHEST-3)
#define THUMBNAIL_EVENT     (wxID_HIGHEST-2)
#define MAIL_PROGRESS_EVENT (wxID_HIGHEST-1)

typedef unsigned long RowId;
#define NO_ID (0L)

#define NO_ROW (-1)


void OnClientEventCallback(int event, CFtpServer::CClientEntry* client, void* arg);


namespace PhotoMailer
{

class PhotoMailerFrame : public PhotoMailerFrameGenerated, public wxDirTraverser
{
wxDECLARE_DYNAMIC_CLASS(PhotoMailerFrame);
public:
	PhotoMailerFrame(const wxString& title);
	virtual ~PhotoMailerFrame();

//wxDirTraverser
	virtual wxDirTraverseResult OnFile(const wxString& filename);
	virtual wxDirTraverseResult OnDir(const wxString& WXUNUSED(dirname)) {return wxDIR_CONTINUE;}
		
	void OnClose(wxCloseEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnFtpServerButton(wxCommandEvent& event);
	void OnGridSelectCell(wxGridEvent& event);
	void OnGridCellLeftClick(wxGridEvent& event);
	void OnGridMouseUp(wxEvent& event);
	void OnGridCellChanged(wxGridEvent& event);
	void OnMailProgress(wxThreadEvent& event);
	void OnThumbnailEvent(wxThreadEvent& event);
	void OnFtpUploadEvent(wxThreadEvent& event);

public:
	void RegisterThread(wxThread* thread);
	void UnregisterThread(wxThread* thread);
private:
	void DestroyThreads();
	void WaitForDestroyedThreads();

private:
	bool IsValidSettings();
	bool GetRelativeFilename(const wxString& absolute, wxString& relative);

	void InitPhotoList();
	void RefreshPhotoList();
	bool AddGridItem(const wxString& filename);

	void SelectPhoto(const RowId& row_id);
	void SendMail(const RowId& row_id);

public:
	static bool LoadImage(const wxString& filename, wxImage& image, wxDateTime* timestamp = nullptr);
	static bool GetExifInfo(const wxString& filename, unsigned char* orientation, wxDateTime* timestamp);
	static bool IsJpeg(const wxString& filename);

public:
	wxSemaphore* GetPhotolistSemaphore() const {return m_photolist_thread_semaphore;}
	bool GetSelectedPhoto(wxImage& image, wxString& filename);
	bool GetFilename(const RowId& row_id, wxString& filename);
  int GetRowWithId(const RowId& id);
  RowId GetRowId(int row);

private:
	bool StartFtpServer();
	bool StopFtpServer();
	
private:
	CFtpServer* m_ftp_server;

	wxMutex m_threadlist_mutex;
	std::unordered_set<wxThread*> m_threadlist;
	bool m_is_shutting_down;

	wxMutex m_photolist_mutex;
  int m_photolist_id;
	wxSemaphore* m_photolist_thread_semaphore;
	int     m_pending_thumbnail_count;

	RowId   m_selected_row_id;

	wxMutex m_selected_photo_mutex;
	wxImage m_selected_photo_image;
	wxString m_selected_photo_filename;

	RowId    m_pressed_send_button_row_id;

private:
	DECLARE_EVENT_TABLE()
};


class FtpUploadEventPayload
{
public:
	FtpUploadEventPayload(const char* path) {m_path=wxString::FromUTF8(path);}

	void GetPath(wxString& path) const {path=m_path;}

private:
	wxString m_path;
};
}

#endif // _PHOTOMAILER_FRAME_H_
