/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_mobile_test.c
 * Description:  EC718
 * History:      Rev1.0   2024-09-24
 *
 ****************************************************************************/

#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "rtthread.h"
#include "teleservice.h"

extern int skip_atoi(const char **s);

int cmd_mobile(int argc, char **argv)
{

	char *sub_cmd = argv[1];
	// mobile dial 10010
	if (strcmp(sub_cmd, "dial") == 0)
	{
		char *phoneNumber = argv[2];
		if (phoneDialled(phoneNumber) != 0)
		{
			rt_kprintlnf("DIAL FAIL");
			
			return 1;
		}
	}
	// mobile vts 1
	else if (strcmp(sub_cmd, "vts") == 0)
	{
		char *character = argv[2];
		if (vtsSend(character) != 0)
		{
			rt_kprintlnf("VTS FAIL");
			
			return 1;
		}
	}
	// mobile hang_up
	else if (strcmp(sub_cmd, "hang_up") == 0)
	{
		if (phoneHangUp() != 0)
		{
			rt_kprintlnf("HANGUP FAIL");
			
			return 1;
		}
	}
	// mobile answer
	else if (strcmp(sub_cmd, "answer") == 0)
	{
		if (phoneAnswer() != 0)
		{
			rt_kprintlnf("ANSWER FAIL");
			
			return 1;
		}
	}
	// mobile sms 10010 10010
	else if (strcmp(sub_cmd, "sms") == 0)
	{
		char *phoneNumber = argv[2];
		char *text = argv[3];
		if (smsSendText(phoneNumber, text) != 0)
		{
			rt_kprintlnf("SMS FAIL");
			
			return 1;
		}
	}
	else
	{
		rt_kprintlnf("Usage: mobile [options]");
		rt_kprintlnf("[options]:");
		rt_kprintlnf("    %-10s - dial phoneNumber", "dial");
		rt_kprintlnf("    %-10s - hang up the call ", "hang_up");
		rt_kprintlnf("    %-10s - answer the call ", "answer");
		rt_kprintlnf("    %-10s - send text message to the phone number", "sms");
	}
	
	return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_mobile, mobile, mobile test);

#endif
