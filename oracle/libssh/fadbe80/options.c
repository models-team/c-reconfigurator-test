int _reconfig_WITH_SSH1;
int _reconfig__WIN32;
int _reconfig_WITH_SERVER;
int _reconfig__MSC_VER;


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include "libssh/priv.h"
#include "libssh/session.h"
#include "libssh/misc.h"
#include "libssh/options.h"

// defined(WITH_SERVER)
#if defined(WITH_SERVER)
#include "libssh/server.h"
#endif

// defined(WITH_SERVER)

#if defined(WITH_SERVER)
#include "libssh/bind.h"
#endif

int ssh_options_copy (ssh_session src , ssh_session* dest)
{
    ssh_session new;
    int i;
    if (src == NULL || dest == NULL)
    {
        return - 1;
    }
    new = ssh_new ();
    if (new == NULL)
    {
        return - 1;
    }
    if (src -> opts . username)
    {
        new -> opts . username = strdup (src -> opts . username);
        if (new -> opts . username == NULL)
        {
            ssh_free (new);
            return - 1;
        }
    }
    if (src -> opts . host)
    {
        new -> opts . host = strdup (src -> opts . host);
        if (new -> opts . host == NULL)
        {
            ssh_free (new);
            return - 1;
        }
    }
    if (src -> opts . identity)
    {
        struct ssh_iterator* it;
        new -> opts . identity = ssh_list_new ();
        if (new -> opts . identity == NULL)
        {
            ssh_free (new);
            return - 1;
        }
        it = ssh_list_get_iterator (src -> opts . identity);
        while (it)
        {
            char* id;
            int rc;
            id = strdup ((char*) it -> data);
            if (id == NULL)
            {
                ssh_free (new);
                return - 1;
            }
            rc = ssh_list_append (new -> opts . identity , id);
            if (rc < 0)
            {
                free (id);
                ssh_free (new);
                return - 1;
            }
            it = it -> next;
        }
    }
    if (src -> opts . sshdir)
    {
        new -> opts . sshdir = strdup (src -> opts . sshdir);
        if (new -> opts . sshdir == NULL)
        {
            ssh_free (new);
            return - 1;
        }
    }
    if (src -> opts . knownhosts)
    {
        new -> opts . knownhosts = strdup (src -> opts . knownhosts);
        if (new -> opts . knownhosts == NULL)
        {
            ssh_free (new);
            return - 1;
        }
    }
    for (i = 0; i < 10; ++ i)
    {
        if (src -> opts . wanted_methods [ i ])
        {
            new -> opts . wanted_methods [ i ] = strdup (src -> opts . wanted_methods [ i ]);
            if (new -> opts . wanted_methods [ i ] == NULL)
            {
                ssh_free (new);
                return - 1;
            }
        }
    }
    if (src -> opts . ProxyCommand)
    {
        new -> opts . ProxyCommand = strdup (src -> opts . ProxyCommand);
        if (new -> opts . ProxyCommand == NULL)
        {
            ssh_free (new);
            return - 1;
        }
    }
    new -> opts . fd = src -> opts . fd;
    new -> opts . port = src -> opts . port;
    new -> opts . timeout = src -> opts . timeout;
    new -> opts . timeout_usec = src -> opts . timeout_usec;
    new -> opts . ssh2 = src -> opts . ssh2;
    new -> opts . ssh1 = src -> opts . ssh1;
    new -> opts . compressionlevel = src -> opts . compressionlevel;
    new -> common . log_verbosity = src -> common . log_verbosity;
    new -> common . callbacks = src -> common . callbacks;
    * dest = new;
    return 0;
}

int ssh_options_set_algo (ssh_session session , int algo , const char* list)
{
    if (!verify_existing_algo (algo , list))
    {
        ssh_set_error (session , SSH_REQUEST_DENIED , "Setting method: no algorithm for method \"%s\" (%s)\n" , ssh_kex_get_description (algo) , list);
        return - 1;
    }
    SAFE_FREE (session -> opts . wanted_methods [ algo ]);
    session -> opts . wanted_methods [ algo ] = strdup (list);
    if (session -> opts . wanted_methods [ algo ] == NULL)
    {
        ssh_set_error_oom (session);
        return - 1;
    }
    return 0;
}

int ssh_options_set (ssh_session session , enum ssh_options_e type , const void* value)
{
    const char* v;
    char* p;
    char* q;
    long int i;
    int rc;
    if (session == NULL)
    {
        return - 1;
    }
    switch (type)
    {
        case SSH_OPTIONS_HOST : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            q = strdup (value);
            if (q == NULL)
            {
                ssh_set_error_oom (session);
                return - 1;
            }
            p = strchr (q , '@');
            SAFE_FREE (session -> opts . host);
            if (p)
            {
                * p = '\0';
                session -> opts . host = strdup (p + 1);
                if (session -> opts . host == NULL)
                {
                    SAFE_FREE (q);
                    ssh_set_error_oom (session);
                    return - 1;
                }
                SAFE_FREE (session -> opts . username);
                session -> opts . username = strdup (q);
                SAFE_FREE (q);
                if (session -> opts . username == NULL)
                {
                    ssh_set_error_oom (session);
                    return - 1;
                }
            }
            else
            {
                session -> opts . host = q;
            }
        }
        break;
        case SSH_OPTIONS_PORT : if (value == NULL)
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            int* x;
            x = (int*) value;
            if (* x <= 0)
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> opts . port = * x & 0xffff;
        }
        break;
        case SSH_OPTIONS_PORT_STR : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            q = strdup (v);
            if (q == NULL)
            {
                ssh_set_error_oom (session);
                return - 1;
            }
            i = strtol (q , & p , 10);
            if (q == p)
            {
                SAFE_FREE (q);
            }
            SAFE_FREE (q);
            if (i <= 0)
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> opts . port = i & 0xffff;
        }
        break;
        case SSH_OPTIONS_FD : if (value == NULL)
        {
            session -> opts . fd = SSH_INVALID_SOCKET;
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            socket_t* x;
            x = (socket_t*) value;
            if (* x < 0)
            {
                session -> opts . fd = SSH_INVALID_SOCKET;
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> opts . fd = * x & 0xffff;
        }
        break;
        case SSH_OPTIONS_BINDADDR : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        q = strdup (v);
        if (q == NULL)
        {
            return - 1;
        }
        SAFE_FREE (session -> opts . bindaddr);
        session -> opts . bindaddr = q;
        break;
        case SSH_OPTIONS_USER : v = value;
        SAFE_FREE (session -> opts . username);
        if (v == NULL)
        {
            q = ssh_get_local_username ();
            if (q == NULL)
            {
                ssh_set_error_oom (session);
                return - 1;
            }
            session -> opts . username = q;
        }
        else
        {
            if (v [ 0 ] == '\0')
            {
                ssh_set_error_oom (session);
                return - 1;
            }
            else
            {
                session -> opts . username = strdup (value);
                if (session -> opts . username == NULL)
                {
                    ssh_set_error_oom (session);
                    return - 1;
                }
            }
        }
        break;
        case SSH_OPTIONS_SSH_DIR : v = value;
        SAFE_FREE (session -> opts . sshdir);
        if (v == NULL)
        {
            session -> opts . sshdir = ssh_path_expand_tilde ("~/.ssh");
            if (session -> opts . sshdir == NULL)
            {
                return - 1;
            }
        }
        else
        {
            if (v [ 0 ] == '\0')
            {
                ssh_set_error_oom (session);
                return - 1;
            }
            else
            {
                session -> opts . sshdir = ssh_path_expand_tilde (v);
                if (session -> opts . sshdir == NULL)
                {
                    ssh_set_error_oom (session);
                    return - 1;
                }
            }
        }
        break;
        case SSH_OPTIONS_IDENTITY : case SSH_OPTIONS_ADD_IDENTITY : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        q = strdup (v);
        if (q == NULL)
        {
            return - 1;
        }
        rc = ssh_list_prepend (session -> opts . identity , q);
        if (rc < 0)
        {
            free (q);
            return - 1;
        }
        break;
        case SSH_OPTIONS_KNOWNHOSTS : v = value;
        SAFE_FREE (session -> opts . knownhosts);
        if (v == NULL)
        {
            session -> opts . knownhosts = ssh_path_expand_escape (session , "%d/known_hosts");
            if (session -> opts . knownhosts == NULL)
            {
                ssh_set_error_oom (session);
                return - 1;
            }
        }
        else
        {
            if (v [ 0 ] == '\0')
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            else
            {
                session -> opts . knownhosts = strdup (v);
                if (session -> opts . knownhosts == NULL)
                {
                    ssh_set_error_oom (session);
                    return - 1;
                }
            }
        }
        break;
        case SSH_OPTIONS_TIMEOUT : if (value == NULL)
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            long* x;
            x = (long*) value;
            if (* x < 0)
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> opts . timeout = * x & 0xffffffff;
        }
        break;
        case SSH_OPTIONS_TIMEOUT_USEC : if (value == NULL)
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            long* x;
            x = (long*) value;
            if (* x < 0)
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> opts . timeout_usec = * x & 0xffffffff;
        }
        break;
        case SSH_OPTIONS_SSH1 : if (value == NULL)
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            int* x;
            x = (int*) value;
            if (* x < 0)
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> opts . ssh1 = * x;
        }
        break;
        case SSH_OPTIONS_SSH2 : if (value == NULL)
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            int* x;
            x = (int*) value;
            if (* x < 0)
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> opts . ssh2 = * x & 0xffff;
        }
        break;
        case SSH_OPTIONS_LOG_VERBOSITY : if (value == NULL)
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            int* x;
            x = (int*) value;
            if (* x < 0)
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> common . log_verbosity = * x & 0xffff;
        }
        break;
        case SSH_OPTIONS_LOG_VERBOSITY_STR : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            session -> common . log_verbosity = 0;
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            q = strdup (v);
            if (q == NULL)
            {
                ssh_set_error_oom (session);
                return - 1;
            }
            i = strtol (q , & p , 10);
            if (q == p)
            {
                SAFE_FREE (q);
            }
            SAFE_FREE (q);
            if (i < 0)
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> common . log_verbosity = i & 0xffff;
        }
        break;
        case SSH_OPTIONS_CIPHERS_C_S : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            if (ssh_options_set_algo (session , SSH_CRYPT_C_S , v) < 0)
         return - 1;
        }
        break;
        case SSH_OPTIONS_CIPHERS_S_C : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            if (ssh_options_set_algo (session , SSH_CRYPT_S_C , v) < 0)
         return - 1;
        }
        break;
        case SSH_OPTIONS_KEY_EXCHANGE : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            if (ssh_options_set_algo (session , SSH_KEX , v) < 0)
         return - 1;
        }
        break;
        case SSH_OPTIONS_HOSTKEYS : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            if (ssh_options_set_algo (session , SSH_HOSTKEYS , v) < 0)
         return - 1;
        }
        break;
        case SSH_OPTIONS_COMPRESSION_C_S : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            if (strcasecmp (value , "yes") == 0)
            {
                if (ssh_options_set_algo (session , SSH_COMP_C_S , "zlib@openssh.com,zlib") < 0)
         return - 1;
            }
            else
            {
                if (strcasecmp (value , "no") == 0)
                {
                    if (ssh_options_set_algo (session , SSH_COMP_C_S , "none") < 0)
         return - 1;
                }
                else
                {
                    if (ssh_options_set_algo (session , SSH_COMP_C_S , v) < 0)
         return - 1;
                }
            }
        }
        break;
        case SSH_OPTIONS_COMPRESSION_S_C : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            if (strcasecmp (value , "yes") == 0)
            {
                if (ssh_options_set_algo (session , SSH_COMP_S_C , "zlib@openssh.com,zlib") < 0)
         return - 1;
            }
            else
            {
                if (strcasecmp (value , "no") == 0)
                {
                    if (ssh_options_set_algo (session , SSH_COMP_S_C , "none") < 0)
         return - 1;
                }
                else
                {
                    if (ssh_options_set_algo (session , SSH_COMP_S_C , v) < 0)
         return - 1;
                }
            }
        }
        break;
        case SSH_OPTIONS_COMPRESSION : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        if (ssh_options_set (session , SSH_OPTIONS_COMPRESSION_C_S , v) < 0)
         return - 1;
        if (ssh_options_set (session , SSH_OPTIONS_COMPRESSION_S_C , v) < 0)
         return - 1;
        break;
        case SSH_OPTIONS_COMPRESSION_LEVEL : if (value == NULL)
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            int* x;
            x = (int*) value;
            if (* x < 1 || * x > 9)
            {
                ssh_set_error_invalid (session);
                return - 1;
            }
            session -> opts . compressionlevel = * x & 0xff;
        }
        break;
        case SSH_OPTIONS_STRICTHOSTKEYCHECK : if (value == NULL)
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            int* x;
            x = (int*) value;
            session -> opts . StrictHostKeyChecking =
                                                      (* x & 0xff) > 0
                                                      ? 1
                                                      : 0;
        }
        session -> opts . StrictHostKeyChecking = * (int*) value;
        break;
        case SSH_OPTIONS_PROXYCOMMAND : v = value;
        if (v == NULL || v [ 0 ] == '\0')
        {
            ssh_set_error_invalid (session);
            return - 1;
        }
        else
        {
            SAFE_FREE (session -> opts . ProxyCommand);
            rc = strcasecmp (v , "none");
            if (rc != 0)
            {
                q = strdup (v);
                if (q == NULL)
                {
                    return - 1;
                }
                session -> opts . ProxyCommand = q;
            }
        }
        break;
        default : ssh_set_error (session , SSH_REQUEST_DENIED , "Unknown ssh option %d" , type);
        return - 1;
        break;
    }
    return 0;
}

int ssh_options_get_port (ssh_session session , unsigned int* port_target)
{
    if (session == NULL)
    {
        return - 1;
    }
    if (!session -> opts . port)
    {
        ssh_set_error_invalid (session);
        return - 1;
    }
    * port_target = session -> opts . port;
    return 0;
}

int ssh_options_get (ssh_session session , enum ssh_options_e type , char** value)
{
    char* src;
    src = NULL;
    if (session == NULL)
    {
        return SSH_ERROR;
    }
    if (value == NULL)
    {
        ssh_set_error_invalid (session);
        return SSH_ERROR;
    }
    switch (type)
    {
        case SSH_OPTIONS_HOST :
        {
            src = session -> opts . host;
            break;
        }
        case SSH_OPTIONS_USER :
        {
            src = session -> opts . username;
            break;
        }
        case SSH_OPTIONS_IDENTITY :
        {
            struct ssh_iterator* it;
            it = ssh_list_get_iterator (session -> opts . identity);
            if (it == NULL)
            {
                return SSH_ERROR;
            }
            src = ((char*) ((it) -> data));
            break;
        }
        default : ssh_set_error (session , SSH_REQUEST_DENIED , "Unknown ssh option %d" , type);
        return SSH_ERROR;
        break;
    }
    if (src == NULL)
    {
        return SSH_ERROR;
    }
    * value = strdup (src);
    if (* value == NULL)
    {
        ssh_set_error_oom (session);
        return SSH_ERROR;
    }
    return SSH_OK;
}

int ssh_options_getopt (ssh_session session , int* argcptr , char** argv)
{
    argcptr = malloc (sizeof (int));
    char* user;
    user = NULL;
    char* cipher;
    cipher = NULL;
    char* identity;
    identity = NULL;
    char* port;
    port = NULL;
    char** save;
    save = NULL;
    char** tmp;
    int i;
    i = 0;
    int argc;
    argc = * argcptr;
    int debuglevel;
    debuglevel = 0;
    int usersa;
    usersa = 0;
    int usedss;
    usedss = 0;
    int compress;
    compress = 0;
    int cont;
    cont = 1;
    int current;
    current = 0;
// defined(WITH_SSH1)
    int ssh1_V1;
    if ((_reconfig_WITH_SSH1))
         ssh1_V1 = 1;
// !defined(WITH_SSH1)
    int ssh1_V2;
    if ((!_reconfig_WITH_SSH1))
         ssh1_V2 = 0;
    int ssh2;
    ssh2 = 1;
    if ((_reconfig__MSC_VER))
         return - 1;
// !defined(_MSC_VER)
    int saveoptind_V3;
    if ((!_reconfig__MSC_VER))
         saveoptind_V3 = optind;
// !defined(_MSC_VER)
    int saveopterr_V3;
    if ((!_reconfig__MSC_VER))
    {
        saveopterr_V3 = opterr;
        opterr = 0;
        while (cont && ((i = getopt (argc , argv , "c:i:Cl:p:vb:rd12")) != - 1))
        {
            switch (i)
            {
                case 'l' : user = optarg;
                break;
                case 'p' : port = optarg;
                break;
                case 'v' : debuglevel ++;
                break;
                case 'r' : usersa ++;
                break;
                case 'd' : usedss ++;
                break;
                case 'c' : cipher = optarg;
                break;
                case 'i' : identity = optarg;
                break;
                case 'C' : compress ++;
                break;
                case '2' : ssh2 = 1;
                (
                 (_reconfig_WITH_SSH1)
                 ? (ssh1_V1 = 0)
                 : (ssh1_V2 = 0));
                break;
                case '1' : ssh2 = 0;
                (
                 (_reconfig_WITH_SSH1)
                 ? (ssh1_V1 = 1)
                 : (ssh1_V2 = 1));
                break;
                default :
                {
                    char opt [ 3 ];
                    opt [ 0 ] = '-';
                    opt [ 1 ] = ' ';
                    opt [ 1 ] = optopt;
                    tmp = realloc (save , (current + 1) * sizeof (char*));
                    if (tmp == NULL)
                    {
                        SAFE_FREE (save);
                        ssh_set_error_oom (session);
                        return - 1;
                    }
                    save = tmp;
                    save [ current ] = strdup (opt);
                    if (save [ current ] == NULL)
                    {
                        SAFE_FREE (save);
                        ssh_set_error_oom (session);
                        return - 1;
                    }
                    current ++;
                    if (optarg)
                    {
                        save [ current ++ ] = argv [ optind + 1 ];
                    }
                }
            }
        }
        opterr = saveopterr_V3;
        while (optind < argc)
        {
            tmp = realloc (save , (current + 1) * sizeof (char*));
            if (tmp == NULL)
            {
                SAFE_FREE (save);
                ssh_set_error_oom (session);
                return - 1;
            }
            save = tmp;
            save [ current ] = argv [ optind ];
            current ++;
            optind ++;
        }
        if (usersa && usedss)
        {
            ssh_set_error (session , SSH_FATAL , "Either RSA or DSS must be chosen");
            cont = 0;
        }
        ssh_options_set (session , SSH_OPTIONS_LOG_VERBOSITY , & debuglevel);
        optind = saveoptind_V3;
        if (!cont)
        {
            SAFE_FREE (save);
            return - 1;
        }
        for (i = 0; i < current; i ++)
        {
            argv [ i + 1 ] = save [ i ];
        }
        argv [ current + 1 ] = NULL;
        * argcptr = current + 1;
        SAFE_FREE (save);
        if (compress)
        {
            if (ssh_options_set (session , SSH_OPTIONS_COMPRESSION , "yes") < 0)
            {
                cont = 0;
            }
        }
        if (cont && cipher)
        {
            if (ssh_options_set (session , SSH_OPTIONS_CIPHERS_C_S , cipher) < 0)
            {
                cont = 0;
            }
            if (cont && ssh_options_set (session , SSH_OPTIONS_CIPHERS_S_C , cipher) < 0)
            {
                cont = 0;
            }
        }
        if (cont && user)
        {
            if (ssh_options_set (session , SSH_OPTIONS_USER , user) < 0)
            {
                cont = 0;
            }
        }
        if (cont && identity)
        {
            if (ssh_options_set (session , SSH_OPTIONS_IDENTITY , identity) < 0)
            {
                cont = 0;
            }
        }
        ssh_options_set (session , SSH_OPTIONS_PORT_STR , port);
        ssh_options_set (session , SSH_OPTIONS_SSH1 , (
                                                       (_reconfig_WITH_SSH1)
                                                       ? & ssh1_V1
                                                       : & ssh1_V2));
        ssh_options_set (session , SSH_OPTIONS_SSH2 , & ssh2);
        if (!cont)
        {
            return SSH_ERROR;
        }
        return SSH_OK;
    }
}

int ssh_options_parse_config (ssh_session session , const char* filename)
{
    char* expanded_filename;
    int r;
    if (session == NULL)
    {
        return - 1;
    }
    if (session -> opts . host == NULL)
    {
        ssh_set_error_invalid (session);
        return - 1;
    }
    if (session -> opts . sshdir == NULL)
    {
        r = ssh_options_set (session , SSH_OPTIONS_SSH_DIR , NULL);
        if (r < 0)
        {
            ssh_set_error_oom (session);
            return - 1;
        }
    }
    if (filename == NULL)
    {
        expanded_filename = ssh_path_expand_escape (session , "%d/config");
    }
    else
    {
        expanded_filename = ssh_path_expand_escape (session , filename);
    }
    if (expanded_filename == NULL)
    {
        return - 1;
    }
    r = ssh_config_parse_file (session , expanded_filename);
    if (r < 0)
    {
        goto out;
    }
    if (filename == NULL)
    {
        r = ssh_config_parse_file (session , "/etc/ssh/ssh_config");
    }
    out : free (expanded_filename);
    return r;
}

int ssh_options_apply (ssh_session session)
{
    struct ssh_iterator* it;
    char* tmp;
    int rc;
    if (session -> opts . sshdir == NULL)
    {
        rc = ssh_options_set (session , SSH_OPTIONS_SSH_DIR , NULL);
        if (rc < 0)
        {
            return - 1;
        }
    }
    if (session -> opts . username == NULL)
    {
        rc = ssh_options_set (session , SSH_OPTIONS_USER , NULL);
        if (rc < 0)
        {
            return - 1;
        }
    }
    if (session -> opts . knownhosts == NULL)
    {
        tmp = ssh_path_expand_escape (session , "%d/known_hosts");
    }
    else
    {
        tmp = ssh_path_expand_escape (session , session -> opts . knownhosts);
    }
    if (tmp == NULL)
    {
        return - 1;
    }
    free (session -> opts . knownhosts);
    session -> opts . knownhosts = tmp;
    if (session -> opts . ProxyCommand != NULL)
    {
        tmp = ssh_path_expand_escape (session , session -> opts . ProxyCommand);
        if (tmp == NULL)
        {
            return - 1;
        }
        free (session -> opts . ProxyCommand);
        session -> opts . ProxyCommand = tmp;
    }
    for (it = ssh_list_get_iterator (session -> opts . identity); it != NULL; it = it -> next)
    {
        char* id;
        id = (char*) it -> data;
        tmp = ssh_path_expand_escape (session , id);
        if (tmp == NULL)
        {
            return - 1;
        }
        free (id);
        it -> data = tmp;
    }
    return 0;
}

// defined(WITH_SERVER)
static int ssh_bind_options_set_algo_V0 (ssh_bind sshbind , int algo , const char* list)
{
    if (!verify_existing_algo (algo , list))
    {
        ssh_set_error (sshbind , SSH_REQUEST_DENIED , "Setting method: no algorithm for method \"%s\" (%s)\n" , ssh_kex_get_description (algo) , list);
        return - 1;
    }
    SAFE_FREE (sshbind -> wanted_methods [ algo ]);
    sshbind -> wanted_methods [ algo ] = strdup (list);
    if (sshbind -> wanted_methods [ algo ] == NULL)
    {
        ssh_set_error_oom (sshbind);
        return - 1;
    }
    return 0;
}

// defined(WITH_SERVER)
int ssh_bind_options_set_V0 (ssh_bind sshbind , enum ssh_bind_options_e type , const void* value)
{
    char* p;
    char* q;
    int i;
    if (sshbind == NULL)
    {
        return - 1;
    }
    switch (type)
    {
        case SSH_BIND_OPTIONS_HOSTKEY : if (value == NULL)
        {
            ssh_set_error_invalid (sshbind);
            return - 1;
        }
        else
        {
            if (ssh_bind_options_set_algo_V0 (sshbind , SSH_HOSTKEYS , value) < 0)
         return - 1;
        }
        break;
        case SSH_BIND_OPTIONS_BINDADDR : if (value == NULL)
        {
            ssh_set_error_invalid (sshbind);
            return - 1;
        }
        else
        {
            SAFE_FREE (sshbind -> bindaddr);
            sshbind -> bindaddr = strdup (value);
            if (sshbind -> bindaddr == NULL)
            {
                ssh_set_error_oom (sshbind);
                return - 1;
            }
        }
        break;
        case SSH_BIND_OPTIONS_BINDPORT : if (value == NULL)
        {
            ssh_set_error_invalid (sshbind);
            return - 1;
        }
        else
        {
            int* x;
            x = (int*) value;
            sshbind -> bindport = * x & 0xffff;
        }
        break;
        case SSH_BIND_OPTIONS_BINDPORT_STR : if (value == NULL)
        {
            sshbind -> bindport = 22 & 0xffff;
        }
        else
        {
            q = strdup (value);
            if (q == NULL)
            {
                ssh_set_error_oom (sshbind);
                return - 1;
            }
            i = strtol (q , & p , 10);
            if (q == p)
            {
                SAFE_FREE (q);
            }
            SAFE_FREE (q);
            sshbind -> bindport = i & 0xffff;
        }
        break;
        case SSH_BIND_OPTIONS_LOG_VERBOSITY : if (value == NULL)
        {
            ssh_set_error_invalid (sshbind);
            return - 1;
        }
        else
        {
            int* x;
            x = (int*) value;
            sshbind -> common . log_verbosity = * x & 0xffff;
        }
        break;
        case SSH_BIND_OPTIONS_LOG_VERBOSITY_STR : if (value == NULL)
        {
            sshbind -> common . log_verbosity = 0;
        }
        else
        {
            q = strdup (value);
            if (q == NULL)
            {
                ssh_set_error_oom (sshbind);
                return - 1;
            }
            i = strtol (q , & p , 10);
            if (q == p)
            {
                SAFE_FREE (q);
            }
            SAFE_FREE (q);
            sshbind -> common . log_verbosity = i & 0xffff;
        }
        break;
        case SSH_BIND_OPTIONS_DSAKEY : if (value == NULL)
        {
            ssh_set_error_invalid (sshbind);
            return - 1;
        }
        else
        {
            SAFE_FREE (sshbind -> dsakey);
            sshbind -> dsakey = strdup (value);
            if (sshbind -> dsakey == NULL)
            {
                ssh_set_error_oom (sshbind);
                return - 1;
            }
        }
        break;
        case SSH_BIND_OPTIONS_RSAKEY : if (value == NULL)
        {
            ssh_set_error_invalid (sshbind);
            return - 1;
        }
        else
        {
            SAFE_FREE (sshbind -> rsakey);
            sshbind -> rsakey = strdup (value);
            if (sshbind -> rsakey == NULL)
            {
                ssh_set_error_oom (sshbind);
                return - 1;
            }
        }
        break;
        case SSH_BIND_OPTIONS_BANNER : if (value == NULL)
        {
            ssh_set_error_invalid (sshbind);
            return - 1;
        }
        else
        {
            SAFE_FREE (sshbind -> banner);
            sshbind -> banner = strdup (value);
            if (sshbind -> banner == NULL)
            {
                ssh_set_error_oom (sshbind);
                return - 1;
            }
        }
        break;
        default : ssh_set_error (sshbind , SSH_REQUEST_DENIED , "Unknown ssh option %d" , type);
        return - 1;
        break;
    }
    return 0;
}