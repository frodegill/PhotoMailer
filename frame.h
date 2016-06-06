#ifndef _PHOTOMAILER_FRAME_H_
#define _PHOTOMAILER_FRAME_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "frame.h"
#endif

#include <wx/fswatcher.h>

#include "PhotoMailerGenerated.h"


namespace PhotoMailer
{

class PhotoMailerFrame : public PhotoMailerFrameGenerated
{
DECLARE_DYNAMIC_CLASS(PhotoMailerFrame)
public:
	PhotoMailerFrame(const wxString& title);
	virtual ~PhotoMailerFrame();

	void OnQuit(wxCommandEvent& event);
	void OnListen(wxCommandEvent& event);
	void OnDirectoryEvent(wxFileSystemWatcherEvent& event);

private:
	bool IsValidSettings() const;
	void RefreshPhotoList();
	bool AddPhoto();

private:
	wxFileSystemWatcher* m_filesystem_watcher;
	wxMutex m_photolist_mutex;

private:
	DECLARE_EVENT_TABLE()
};

}

#endif // _PHOTOMAILER_FRAME_H_
