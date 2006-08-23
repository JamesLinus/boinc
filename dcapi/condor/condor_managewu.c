/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib.h>

#include "dc.h"

#include "condor_log.h"
#include "condor_wu.h"


/*************************************************************** Manage WUs */

/* Submits a work unit. */
int
DC_submitWU(DC_Workunit *wu)
{
	int ret;
	//GString *fn;
	GString *cmd;
	gchar *act, *act2;
	char *id;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	DC_log(LOG_DEBUG, "DC_submitWU(%p-\"%s\")", wu, wu->data.name);

	if (wu->state != DC_WU_READY)
	{
		DC_log(LOG_INFO, "Re-submission of %s", wu->data.name);
		return(DC_ERR_BADPARAM);
	}

	ret= _DC_wu_gen_condor_submit(wu);
	if (ret)
	{
		DC_log(LOG_ERR, "Submit file generation failed");
		return(ret);
	}

	//fn= g_string_new(wu->workdir);
	//fn= g_string_append(fn, "/condor_submit.txt");
	cmd= g_string_new("condor_submit");
	cmd= g_string_append(cmd, " condor_submit.txt");
	act= getcwd(NULL, 0);
	chdir(wu->data.workdir);
	act2= getcwd(NULL, 0);
	DC_log(LOG_DEBUG, "Calling \"%s\" in %s...",
	       cmd->str, act2);
	ret= system(cmd->str);
	DC_log(LOG_DEBUG, "Returned %d", ret);
	chdir(act);
	g_free(act);
	g_free(act2);
	g_string_free(cmd, TRUE);
	if (ret == 0)
	{
		_DC_wu_update_condor_events(wu);
		while (wu->condor_events->len == 0)
		{
			sleep(1);
			_DC_wu_update_condor_events(wu);
		}
		wu->state= DC_WU_RUNNING;
		id= DC_getWUId(wu);
		DC_log(LOG_INFO, "Condor id of wu's job: %s", id);
		g_free(id);
	}
	return(DC_OK);
}


/* Cancels all computations for a given work unit. */
int
DC_cancelWU(DC_Workunit *wu)
{
	char *id;
	GString *cmd;
	int ret;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	DC_log(LOG_DEBUG, "DC_cancelWU(%p-\"%s\")", wu, wu->data.name);

	id= DC_getWUId(wu);
	if (!id)
		return(DC_ERR_UNKNOWN_WU);
	cmd= g_string_new("condor_rm ");
	cmd= g_string_append(cmd, id);
	DC_log(LOG_DEBUG, "Calling \"%s\"...", cmd->str);
	ret= system(cmd->str);
	DC_log(LOG_DEBUG, "Returned %d", ret);
	g_string_free(cmd, TRUE);
	g_free(id);
	wu->state= DC_WU_ABORTED;
	return((ret==0)?DC_OK:DC_ERR_BADPARAM);
}


/* Temporarily suspends the execution of a work unit. */
int
DC_suspendWU(DC_Workunit *wu)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	return(DC_ERR_NOTIMPL);
}


/* Resumes computation of a previously suspended work unit. */
int
DC_resumeWU(DC_Workunit *wu)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	return(DC_ERR_NOTIMPL);
}


/* End of condor/condor_managewu.c */
