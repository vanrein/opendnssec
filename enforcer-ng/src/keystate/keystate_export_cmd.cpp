#include <ctime>
#include <iostream>
#include <cassert>

// Interface of this cpp file is used by C code, we need to declare 
// extern "C" to prevent linking errors.
extern "C" {
#include "keystate/keystate_export_cmd.h"
#include "keystate/keystate_export_task.h"
#include "shared/duration.h"
#include "shared/file.h"
#include "shared/str.h"
#include "daemon/engine.h"
}

static const char *module_str = "keystate_export_cmd";

void help_keystate_export_cmd(int sockfd)
{
    char buf[ODS_SE_MAXLINE];
    (void) snprintf(buf, ODS_SE_MAXLINE,
        "key export        export all the keys used by a zone\n"
        "  --zone <zone>   (aka -z) export for the specified zone.\n"
        );
    ods_writen(sockfd, buf, strlen(buf));
}

int handled_keystate_export_cmd(int sockfd, engine_type* engine, const char *cmd,
                              ssize_t n)
{
    char buf[ODS_SE_MAXLINE];
    const char *argv[8];
    const int NARGV = sizeof(argv)/sizeof(char*);
    int argc;
    const char *scmd = "key export";

    cmd = ods_check_command(cmd,n,scmd);
    if (!cmd)
        return 0; // not handled
    
    ods_log_debug("[%s] %s command", module_str, scmd);
    
    // Use buf as an intermediate buffer for the command.
    strncpy(buf,cmd,sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    
    // separate the arguments
    argc = ods_str_explode(buf,NARGV,argv);
    if (argc > NARGV) {
        ods_log_warning("[%s] too many arguments for %s command",
                        module_str,scmd);
        (void)snprintf(buf, ODS_SE_MAXLINE,"too many arguments\n");
        ods_writen(sockfd, buf, strlen(buf));
        return 1; // errors, but handled
    }
    
    const char *zone = NULL;
    ods_find_arg_and_param(&argc,argv,"zone","z",&zone);
    if (!zone) {
        ods_log_warning("[%s] expected option --zone <zone> for %s command",
                        module_str,scmd);
        (void)snprintf(buf, ODS_SE_MAXLINE,"expected --zone <zone> option\n");
        ods_writen(sockfd, buf, strlen(buf));
        return 1; // errors, but handled
    }

    if (argc) {
        ods_log_warning("[%s] unknown arguments for %s command",
                        module_str,scmd);
        (void)snprintf(buf, ODS_SE_MAXLINE,"unknown arguments\n");
        ods_writen(sockfd, buf, strlen(buf));
        return 1; // errors, but handled
    }
    
    /* perform task directly */
    time_t tstart = time(NULL);
    perform_keystate_export(sockfd,engine->config,zone);
    (void)snprintf(buf, ODS_SE_MAXLINE, "%s completed in %ld seconds.\n",
                   scmd,time(NULL)-tstart);
    ods_writen(sockfd, buf, strlen(buf));
    
    return 1;
}