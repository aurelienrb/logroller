@echo off

if not "%1"=="" goto copy_param1

call %0 logsystem.h
call %0 log_mgr.h
call %0 log_message.h
call %0 log_thread_message_mgr.h
call %0 log_message_scope.h
call %0 log_message_builder.h
call %0 smtp_mail_sender.h

goto end

:copy_param1

xcopy %1 ..\ /R /Y
attrib +R ..\%1

:end
