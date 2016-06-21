#ifndef _PHOTOMAILER_MAIL_H_
#define _PHOTOMAILER_MAIL_H_

// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma interface "mail.h"
#endif

#include <vmime/vmime.hpp>


namespace PhotoMailer
{

class Mail
{
public:
	Mail();
	virtual ~Mail();

private:
	vmime::ref<vmime::security::cert::X509Certificate> LoadCACertificateFile(const std::string& filename);

private:
	vmime::ref<vmime::security::cert::defaultCertificateVerifier> m_certificate_verifier;
};

}

#endif // _PHOTOMAILER_MAIL_H_
