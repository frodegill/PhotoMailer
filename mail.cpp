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


Mail::Mail(PhotoMailerFrame* frame)
: wxThread(),
  m_frame(frame)
{
	m_certificate_verifier = vmime::create<vmime::security::cert::defaultCertificateVerifier>();
	vmime::ref<vmime::security::cert::X509Certificate> ca_certs = LoadCACertificateFile(CA_CERT_FILE);
	if (ca_certs)
	{
		std::vector<vmime::ref<vmime::security::cert::X509Certificate>> root_ca_list;
		root_ca_list.push_back(ca_certs);
		m_certificate_verifier->setX509RootCAs(root_ca_list);
	}
}

Mail::~Mail()
{
}

wxThread::ExitCode Mail::Entry()
{
	return nullptr;
}

void Mail::OnExit()
{
}

vmime::ref<vmime::security::cert::X509Certificate> Mail::LoadCACertificateFile(const std::string& filename)
{
	std::ifstream cert_file;
	cert_file.open(filename, std::ios::in|std::ios::binary);
	if (!cert_file)
		return nullptr;

	vmime::utility::inputStreamAdapter is(cert_file);
	return vmime::security::cert::X509Certificate::import(is);
}
