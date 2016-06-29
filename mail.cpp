// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "mail.h"
#endif

#include <fstream>
#include <ios>

#include "app.h"
#include "frame.h"
#include "mail.h"


#define CA_CERT_FILE "/etc/ssl/certs/ca-certificates.crt"


using namespace PhotoMailer;


MailThread::MailThread(PhotoMailerFrame* frame, int row)
: wxThread(),
  m_frame(frame),
  m_row(row),
  m_has_failed(false)
{
}

MailThread::~MailThread()
{
}

wxThread::ExitCode MailThread::Entry()
{
	m_certificate_verifier = vmime::create<vmime::security::cert::defaultCertificateVerifier>();
	vmime::ref<vmime::security::cert::X509Certificate> ca_certs = LoadCACertificateFile(CA_CERT_FILE);
	if (ca_certs)
	{
		std::vector<vmime::ref<vmime::security::cert::X509Certificate>> root_ca_list;
		root_ca_list.push_back(ca_certs);
		m_certificate_verifier->setX509RootCAs(root_ca_list);
	}

	wxThreadEvent* threadEvent = new wxThreadEvent;
	MailProgressEventPayload* payload = new MailProgressEventPayload(m_row, m_has_failed, 100.0);
	threadEvent->SetPayload<MailProgressEventPayload*>(payload);
	::wxQueueEvent(::wxGetApp().GetMainFrame(), threadEvent);

//TODO
	
	return static_cast<wxThread::ExitCode>(0);
}

void MailThread::OnExit()
{
}

bool MailThread::cancel() const
{
	return false;
}

void MailThread::start(const int WXUNUSED(predictedTotal))
{
}

void MailThread::progress(const int current, const int currentTotal)
{
	if (0 != currentTotal)
	{
		wxThreadEvent* threadEvent = new wxThreadEvent;
		MailProgressEventPayload* payload = new MailProgressEventPayload(m_row, m_has_failed,
																																		 static_cast<double>(current)/static_cast<double>(currentTotal));
		threadEvent->SetPayload<MailProgressEventPayload*>(payload);
		::wxQueueEvent(::wxGetApp().GetMainFrame(), threadEvent);
	}
}

void MailThread::stop(const int WXUNUSED(total))
{
}

vmime::ref<vmime::security::cert::X509Certificate> MailThread::LoadCACertificateFile(const std::string& filename)
{
	std::ifstream cert_file;
	cert_file.open(filename, std::ios::in|std::ios::binary);
	if (!cert_file)
		return nullptr;

	vmime::utility::inputStreamAdapter is(cert_file);
	return vmime::security::cert::X509Certificate::import(is);
}



MailProgressEventPayload::MailProgressEventPayload(int row, bool has_failed, double progress)
: m_row(row),
  m_has_failed(has_failed),
  m_progress(progress)
{
}
