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

}

#endif // _PHOTOMAILER_MAIL_H_
