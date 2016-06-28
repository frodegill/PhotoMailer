#ifndef _PHOTOMAILER_MAIL_H_
#define _PHOTOMAILER_MAIL_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "mail.h"
#endif

#include <vmime/vmime.hpp>

#include "wx/grid.h"
#include "wx/thread.h"


namespace PhotoMailer
{

class PhotoMailerFrame;
class MailThread : public wxThread
{
public:
	MailThread(PhotoMailerFrame* frame, wxGrid* grid, int row);
	virtual ~MailThread();

public: //wxThread
	virtual wxThread::ExitCode Entry() wxOVERRIDE;
	virtual void OnExit() wxOVERRIDE;

private:
	vmime::ref<vmime::security::cert::X509Certificate> LoadCACertificateFile(const std::string& filename);

private:
	PhotoMailerFrame* m_frame;
	wxGrid* m_grid;
	int m_row;	

	vmime::ref<vmime::security::cert::defaultCertificateVerifier> m_certificate_verifier;
};



class MailProgressEvent : public wxEvent
{
public:
	MailProgressEvent(wxGrid* grid, int row, bool has_failed, double progress, int winid=0, wxEventType command_type=wxEVT_NULL);
	virtual ~MailProgressEvent() {}

	virtual wxEvent* Clone() const;

	wxGrid* GetGrid() const {return m_grid;}
	int     GetRow() const {return m_row;}
	bool    HasFailed() const {return m_has_failed;}
	double  GetProgress() const {return m_progress;}

private:
	wxGrid* m_grid;
	int     m_row;
	bool    m_has_failed;
	double  m_progress;
};

} //namespace

wxDECLARE_EVENT(MAIL_PROGRESS_EVENT, PhotoMailer::MailProgressEvent);
#define MailProgressEventHandler(func) (&func)
#define EVT_MAIL_PROGRESS(id, func) wx__DECLARE_EVT1(MAIL_PROGRESS_EVENT, id, &func)


#endif // _PHOTOMAILER_MAIL_H_
