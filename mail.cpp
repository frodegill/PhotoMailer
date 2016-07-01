// Copyright (C) 2016  Frode Roxrud Gill
// See LICENSE file for license

#ifdef __GNUG__
  #pragma implementation "mail.h"
#endif

#include <fstream>
#include <ios>

#include <wx/msgout.h>

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
	wxMessageOutputStderr logger;
	try {
		vmime::ref<vmime::security::cert::defaultCertificateVerifier> certificate_verifier = vmime::create<vmime::security::cert::defaultCertificateVerifier>();
		vmime::ref<vmime::security::cert::X509Certificate> ca_certs = LoadCACertificateFile(CA_CERT_FILE);
		if (ca_certs)
		{
			std::vector<vmime::ref<vmime::security::cert::X509Certificate>> root_ca_list;
			root_ca_list.push_back(ca_certs);
			certificate_verifier->setX509RootCAs(root_ca_list);
		}

		//Set up SMTP transport
		std::string smtp_url_string;
		GetSMTPUrl(smtp_url_string);
		vmime::ref<vmime::net::session> session = vmime::create<vmime::net::session>();
		vmime::ref<vmime::net::transport> transport = session->getTransport(smtp_url_string);
		transport->setCertificateVerifier(certificate_verifier);

		//Set up SMTP authentication
		std::string smtp_username_string;
		GetSMTPUsername(smtp_username_string);
		if (!smtp_username_string.empty())
		{
			std::string smtp_password_string;
			GetSMTPPassword(smtp_password_string);

			transport->setProperty("options.need−authentication", true);
			transport->setProperty("auth.username", smtp_username_string);
			transport->setProperty("auth.password", smtp_password_string);
		}

		//Connect
		transport->connect();

		//Create message
		vmime::messageBuilder mb;

		//FROM
		std::string from_string;
		GetFrom(from_string);
		mb.setExpeditor(vmime::mailbox(from_string));


		//TO
		std::string to_string;
		GetTo(to_string);
		mb.getRecipients().appendAddress(vmime::create<vmime::mailbox>(to_string));

		//SUBJECT
		std::string subject_string;
		GetSubject(subject_string);
		mb.setSubject(vmime::text(subject_string));

		//Attachment
		wxString wx_filename;
		if (!m_frame->GetRowFilename(m_row, wx_filename))
		{
			m_has_failed = true;
		}
		else
		{
			vmime::ref<vmime::fileAttachment> attachment =
				vmime::create<vmime::fileAttachment>(wx_filename.ToStdString(), vmime::mediaType("image/jpeg"));
			
			mb.appendAttachment(attachment);

			transport->send(mb.construct(), this);
		}

		transport->disconnect();
	}
	catch (vmime::exception& e)
	{
		m_has_failed = true;
		logger.Printf(_("Got exception: %s"), e.what());
	}
	catch (std::exception& e)
	{
		m_has_failed = true;
		logger.Printf(_("Got exception: %s"), e.what());
	}
	catch (...)
	{
		m_has_failed = true;
		logger.Output(_("Got unknown exception"));
	}

	//Send completed progress
	wxThreadEvent* threadEvent = new wxThreadEvent;
	MailProgressEventPayload* payload = new MailProgressEventPayload(m_row, m_has_failed, 100.0);
	threadEvent->SetPayload<MailProgressEventPayload*>(payload);
	::wxQueueEvent(::wxGetApp().GetMainFrame(), threadEvent);

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

void MailThread::GetSMTPUrl(std::string& url)
{
	url = (_("smtp://")+m_frame->GetSmtpServerCtrl()->GetValue()+_(":")+m_frame->GetSmtpPortCtrl()->GetValue()).ToUTF8();
}

void MailThread::GetSMTPUsername(std::string& username)
{
	username = m_frame->GetSmtpUsernameCtrl()->GetValue().ToUTF8();
}

void MailThread::GetSMTPPassword(std::string& password)
{
	password = m_frame->GetSmtpPasswordCtrl()->GetValue().ToUTF8();
}

void MailThread::GetFrom(std::string& from)
{
	from = m_frame->GetSenderCtrl()->GetValue().ToUTF8();
}

void MailThread::GetTo(std::string& to)
{
	wxGrid* grid = m_frame->GetPhotosGrid();
	to = grid->GetCellValue(m_row, EMAIL_COLUMN).ToUTF8();
}

void MailThread::GetSubject(std::string& subject)
{
	subject = m_frame->GetSubjectCtrl()->GetValue().ToUTF8();
}



MailProgressEventPayload::MailProgressEventPayload(int row, bool has_failed, double progress)
: m_row(row),
  m_has_failed(has_failed),
  m_progress(progress)
{
}
