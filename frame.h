#ifndef _PHOTOMAILER_FRAME_H_
#define _PHOTOMAILER_FRAME_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "frame.h"
#endif

#include <wx/dir.h>
#include <wx/fswatcher.h>

#include "PhotoMailerGenerated.h"


namespace PhotoMailer
{

class PhotoMailerFrame : public PhotoMailerFrameGenerated, public wxDirTraverser
{
DECLARE_DYNAMIC_CLASS(PhotoMailerFrame)
public:
	PhotoMailerFrame(const wxString& title);
	virtual ~PhotoMailerFrame();

//wxDirTraverser
	virtual wxDirTraverseResult OnFile(const wxString& filename);
	virtual wxDirTraverseResult OnDir(const wxString& WXUNUSED(dirname)) {return wxDIR_CONTINUE;}
		
	void OnQuit(wxCommandEvent& event);
	void OnListen(wxCommandEvent& event);
	void OnDirectoryEvent(wxFileSystemWatcherEvent& event);
	void OnGridSelectCell(wxGridEvent& event);

private:
	bool IsValidSettings() const;
	bool IsJpeg(const wxString& filename) const;
	bool GetRelativeFilename(const wxString& absolute, wxString& relative);

	void InitPhotoList();
	void RefreshPhotoList();
	bool AddPhoto(const wxString& filename);

private:
	wxFileSystemWatcher* m_filesystem_watcher;
	wxMutex m_photolist_mutex;
	
	bool m_is_batch_updating;
	int  m_selected_row;

private:
	DECLARE_EVENT_TABLE()
};

}

#endif // _PHOTOMAILER_FRAME_H_
