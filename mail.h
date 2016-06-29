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
class MailThread : public wxThread
{
public:
	MailThread(PhotoMailerFrame* frame, int row);
	virtual ~MailThread();

public: //wxThread
	virtual wxThread::ExitCode Entry() wxOVERRIDE;
	virtual void OnExit() wxOVERRIDE;

private:
	vmime::ref<vmime::security::cert::X509Certificate> LoadCACertificateFile(const std::string& filename);

private:
	PhotoMailerFrame* m_frame;
	int m_row;
	int m_has_failed;

	vmime::ref<vmime::security::cert::defaultCertificateVerifier> m_certificate_verifier;
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
