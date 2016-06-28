// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "mail.h"
#endif

#include <fstream>
#include <ios>

#include "mail.h"


#define CA_CERT_FILE "/etc/ssl/certs/ca-certificates.crt"


using namespace PhotoMailer;


MailThread::MailThread(PhotoMailerFrame* frame, wxGrid* grid, int row)
: wxThread(),
  m_frame(frame),
  m_grid(grid),
  m_row(row)
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

	return static_cast<wxThread::ExitCode>(0);
}

void MailThread::OnExit()
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



wxDEFINE_EVENT(MAIL_PROGRESS_EVENT, PhotoMailer::MailProgressEvent);

MailProgressEvent::MailProgressEvent(wxGrid* grid, int row, bool has_failed, double progress, int winid, wxEventType command_type)
: wxEvent(winid, command_type),
  m_grid(grid),
  m_row(row),
  m_has_failed(has_failed),
  m_progress(progress)
{
}

wxEvent* MailProgressEvent::Clone() const
{
	return new MailProgressEvent(GetGrid(), GetRow(), HasFailed(), GetProgress());
}
