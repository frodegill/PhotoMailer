#ifndef _PHOTOMAILER_MAIL_H_
#define _PHOTOMAILER_MAIL_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "mail.h"
#endif

#include <vmime/vmime.hpp>

#include "wx/event.h"
#include "wx/grid.h"
#include "wx/thread.h"


namespace PhotoMailer
{

class PhotoMailerFrame;
class MailThread : public wxThread, public vmime::utility::progressListener
{
public:
	MailThread(int row, const wxString& smtp_server, const wxString& smtp_port,
	           const wxString& smtp_username, const wxString& smtp_password,
	           const wxString& from, const wxString& to,
	           const wxString& subject, const wxString& filename);
	virtual ~MailThread();

public: //wxThread
	virtual wxThread::ExitCode Entry() wxOVERRIDE;

public: //vmime::utility::progressListener
	virtual void start(const size_t predictedTotal) override;
	virtual void progress(const size_t current, const size_t currentTotal) override;
	virtual void stop(const size_t total) override;

private:
	vmime::shared_ptr<vmime::security::cert::X509Certificate> LoadCACertificateFile(const std::string& filename);
	void GetSMTPUrl(std::string& url);

private:
	int m_row;
	int m_has_failed;
	wxString m_smtp_server;
	wxString m_smtp_port;
	wxString m_smtp_username;
	wxString m_smtp_password;
	wxString m_from;
	wxString m_to;
	wxString m_subject;
	wxString m_filename;
};



class MailProgressEventPayload
{
public:
	MailProgressEventPayload(int row, bool has_failed, double progress);
	virtual ~MailProgressEventPayload() {}

	int     GetRow() const {return m_row;}
	bool    HasFailed() const {return m_has_failed;}
	double  GetProgress() const {return m_progress;}

private:
	int     m_row;
	bool    m_has_failed;
	double  m_progress;
};

} //namespace


#endif // _PHOTOMAILER_MAIL_H_
