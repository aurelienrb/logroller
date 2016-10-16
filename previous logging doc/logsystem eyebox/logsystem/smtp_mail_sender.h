#ifndef SMTP_MAIL_SENDER_H
#define SMTP_MAIL_SENDER_H

#include <core_export.h>

// http://www.hochfeiler.it/alvise/smtp.htm

class LOGSYSTEM_EXPORT SMTPMailSender
{
public:
    SMTPMailSender();
    ~SMTPMailSender();

    bool open_smtp_connection( const char *destination_mail_address, const char *mail_subject, const char *sender_name_or_email = 0, unsigned short port = 25, int TimeOutSec = 5 );
    bool send_attached_rawdata( const char *AttachmentName, const char *data, size_t data_size, const char *MimeType = "application/octet-stream" );
    bool send_attached_rawdata( const char *AttachmentName, const unsigned char *data, size_t data_size, const char *MimeType = "application/octet-stream" );
    bool send_attached_file( const char *FileName, const char *FullPathFileName = 0, const char *MimeType = "application/octet-stream" );
    bool send_text_body( const char *Text, const char *ContentType = "text/plain", const char *Charset = "ISO-8859-1", size_t TextLength = 0 );
    bool terminate_and_close();
    void cancel_and_close();

protected:
    void close_connection();
    void wait_for_connection_closing();

private:
    int     socket_fd;
    int     timeout_sec;
};

#endif