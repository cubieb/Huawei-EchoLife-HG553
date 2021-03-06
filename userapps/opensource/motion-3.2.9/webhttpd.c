/*
 *      webhttpd.c
 *
 *      HTTP Control interface for motion.
 *
 *      Specs : http://www.lavrsen.dk/twiki/bin/view/Motion/MotionHttpAPI
 *
 *      Copyright 2004-2005 by Angel Carpintero  (ack@telefonica.net)
 *      This software is distributed under the GNU Public License Version 2
 *      See also the file 'COPYING'.
 *
 */
#include "motion.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

pthread_mutex_t httpd_mutex;

int warningkill; // This is a dummy variable use to kill warnings when not checking sscanf and similar functions

static const char* ini_template =
	"<html><head></head>\n"
	"<body>\n";

static const char *set_template =
	"<html><head><script language='javascript'>"
	"function show(){top.location.href="
	"'set?'+document.n.onames.options[document.n.onames.selectedIndex].value"
	"+'='+document.s.valor.value;"
	"}</script>\n</head>\n"
	"<body>\n";

static const char* end_template =
	"</body>\n"
	"</html>\n";

static const char* ok_response =
	"HTTP/1.1 200 OK\r\n"
	"Server: Motion-httpd/"VERSION"\r\n"
	"Connection: close\r\n"
	"Max-Age: 0\r\n"
	"Expires: 0\r\n"
	"Cache-Control: no-cache\r\n"
	"Cache-Control: private\r\n"
	"Pragma: no-cache\r\n"
	"Content-type: text/html\r\n\r\n";

static const char* ok_response_raw =
	"HTTP/1.1 200 OK\r\n"
	"Server: Motion-httpd/"VERSION"\r\n"
	"Connection: close\r\n"
	"Max-Age: 0\r\n"
	"Expires: 0\r\n"
	"Cache-Control: no-cache\r\n"
	"Cache-Control: private\r\n"
	"Pragma: no-cache\r\n"
	"Content-type: text/plain\r\n\r\n";


static const char* bad_request_response =
	"HTTP/1.0 400 Bad Request\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>\n"
	"<body>\n"
	"<h1>Bad Request</h1>\n"
	"<p>The server did not understand your request.</p>\n"
	"</body>\n"
	"</html>\n";

static const char* bad_request_response_raw =
	"HTTP/1.0 400 Bad Request\r\n"
	"Content-type: text/plain\r\n\r\n"
	"Bad Request";

static const char* not_found_response_template =
	"HTTP/1.0 404 Not Found\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>\n"
	"<body>\n"
	"<h1>Not Found</h1>\n"
	"<p>The requested URL was not found on the server.</p>\n"
	"</body>\n"
	"</html>\n";

static const char* not_found_response_template_raw =
	"HTTP/1.0 404 Not Found\r\n"
	"Content-type: text/plain\r\n\r\n"
	"Not Found";

static const char* not_found_response_valid =
	"HTTP/1.0 404 Not Valid\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>\n"
	"<body>\n"
	"<h1>Not Valid</h1>\n"
	"<p>The requested URL is not valid.</p>\n"
	"</body>\n"
	"</html>\n";

static const char* not_found_response_valid_raw =
	"HTTP/1.0 404 Not Valid\r\n"
	"Content-type: text/plain\r\n\r\n"
	"The requested URL is not valid.";

static const char* not_valid_syntax =
	"HTTP/1.0 404 Not Valid Syntax\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>\n"
	"<body>\n"
	"<h1>Not Valid Syntax</h1>\n"
	"</body>\n"
	"</html>\n";

static const char* not_valid_syntax_raw =
	"HTTP/1.0 404 Not Valid Syntax\r\n"
	"Content-type: text/plain\r\n\r\n"
	"Not Valid Syntax\n";

static const char* not_track =
	"HTTP/1.0 200 OK\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>\n"
	"<body>\n"
	"<h1>Tracking Not Enabled</h1>\n";

static const char* not_track_raw =
	"HTTP/1.0 200 OK\r\n"
	"Content-type: text/plain\r\n\r\n"
	"Tracking Not Enabled";

static const char* track_error =
	"HTTP/1.0 200 OK\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>\n"
	"<body>\n"
	"<h1>Track Error</h1>\n";

static const char* track_error_raw =
	"HTTP/1.0 200 OK\r\n"
	"Content-type: text/plain\r\n\r\n"
	"Track Error";

static const char* error_value =
	"HTTP/1.0 200 OK\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>\n"
	"<body>\n"
	"<h1>Value Error</h1>\n";

static const char* error_value_raw =
	"HTTP/1.0 200 OK\r\n"
	"Content-type: text/plain\r\n\r\n"
	"Value Error";
	
static const char* not_found_response_valid_command =
	"HTTP/1.0 404 Not Valid Command\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>\n"
	"<body>\n"
	"<h1>Not Valid Command</h1>\n"
	"<p>The requested URL is not valid Command.</p>\n"
	"</body>\n"
	"</html>\n";

static const char* not_found_response_valid_command_raw =
	"HTTP/1.0 404 Not Valid Command\r\n"
	"Content-type: text/plain\n\n"
	"Not Valid Command\n";

static const char* bad_method_response_template =
	"HTTP/1.0 501 Method Not Implemented\r\n"
	"Content-type: text/html\r\n\r\n"
	"<html>\n"
	"<body>\n"
	"<h1>Method Not Implemented</h1>\n"
	"<p>The method is not implemented by this server.</p>\n"
	"</body>\n"
	"</html>\n";

static const char* bad_method_response_template_raw =
	"HTTP/1.0 501 Method Not Implemented\r\n"
	"Content-type: text/plain\r\n\r\n"
	"Method Not Implemented\n";

static const char *request_auth_response_template=
	"HTTP/1.0 401 Authorization Required\r\n"
	"WWW-Authenticate: Basic realm=\"Motion Security Access\"\r\n";

static void send_template_ini_client(int client_socket, const char* template)
{
	ssize_t nwrite = 0;
	nwrite = write(client_socket, ok_response, strlen (ok_response));
	nwrite += write(client_socket, template, strlen(template));
	if (nwrite != (ssize_t)(strlen(ok_response) + strlen(template)))
		motion_log(LOG_ERR, 1, "httpd send_template_ini_client");
}

static void send_template_ini_client_raw(int client_socket)
{
	ssize_t nwrite = 0;
	nwrite = write(client_socket, ok_response_raw, strlen (ok_response_raw));
	if (nwrite != (ssize_t)strlen(ok_response_raw))
		motion_log(LOG_ERR, 1, "httpd send_template_ini_client_raw");
}

static void send_template(int client_socket, char *res)
{
	ssize_t nwrite = 0;
	nwrite = write(client_socket, res, strlen(res));
	if ( nwrite != (ssize_t)strlen(res))
		motion_log(LOG_ERR, 1, "httpd send_template failure write");
}

static void send_template_raw(int client_socket, char *res)
{
	ssize_t nwrite = 0;
	nwrite = write(client_socket, res, strlen(res));
}

static void send_template_end_client(int client_socket)
{
	ssize_t nwrite = 0;
	nwrite = write(client_socket, end_template, strlen(end_template));
}

static void response_client(int client_socket, const char* template, char *back)
{
	ssize_t nwrite = 0;
	nwrite = write(client_socket, template, strlen(template));
	if (back != NULL) {
		send_template(client_socket, back);
		send_template_end_client(client_socket);
	}
}


static int check_authentication(char *authentication, char *auth_base64, size_t size_auth, const char *conf_auth)
{
	int ret = 0;
	char *userpass = NULL;

	authentication = (char *) mymalloc(BASE64_LENGTH(size_auth) + 1);
	userpass = mymalloc(size_auth + 4);
	/* base64_encode can read 3 bytes after the end of the string, initialize it */
	memset(userpass, 0, size_auth + 4);
	strcpy(userpass, conf_auth);
	base64_encode(userpass, authentication, size_auth);
	free(userpass);

	if (!strcmp(authentication, auth_base64))
		ret=1;

	return ret;
}



/*
   This function decode the values from GET request following the http RFC.
*/

static int url_decode(char *urlencoded, int length)
{
	char *data=urlencoded;
	char *urldecoded=urlencoded;

	while (length > 0)  {
		if (*data == '%') {
			char c[3];
			int i;
			data++;
			length--;
			c[0] = *data++;
			length--;
			c[1] = *data;
			c[2] = 0;
			warningkill = sscanf(c, "%x", &i);
			if (i < 128)
				*urldecoded++ = (char)i;
			else {
				*urldecoded++ = '%';
				*urldecoded++ = c[0];
				*urldecoded++ = c[1];
			}
		} else if (*data=='+') {
			*urldecoded++=' ';
		
		}
		else {
			*urldecoded++=*data;
		}
		data++;
		length--;
	}
	*urldecoded = '\0';
	return 0;
}


/*
    This function manages/parses the config action for motion ( set , get , write , list ).
*/

static int config(char *pointer, char *res, int length_uri, int thread, int client_socket, void *userdata)
{
	char question;
	char command[256] = {'\0'};
	int i;
	struct context **cnt = userdata;

	warningkill = sscanf (pointer, "%256[a-z]%c", command , &question);
	if (!strcmp(command,"list")) {
		pointer = pointer + 4;
		length_uri = length_uri - 4;
		if (length_uri == 0) {
			const char *value = NULL;
			char *retval = NULL;
			/*call list*/
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res, "<a href=/%d/config><- back</a>", thread);
				send_template(client_socket, res);

				for (i=0; config_params[i].param_name != NULL; i++) {
					value = config_params[i].print(cnt, NULL, i, thread);

					if (value == NULL) {
						retval=NULL;
						
						/* Only get the thread value for main thread */ 
						if (thread == 0)
							config_params[i].print(cnt, &retval, i, thread);
					
						/* thread value*/
						
						if (retval) {
							
							if (!strcmp(retval,"")) {
								free(retval);
								retval = strdup("No threads");
							} else {
								char *temp=retval;
								size_t retval_miss = 0;
								size_t retval_len = strlen(retval);
								int ind=0;
								char thread_strings[1024]={'\0'};
								
								while (retval_miss != retval_len) {
									while (*temp != '\n') {
										thread_strings[ind++] = *temp;
										retval_miss++;
										temp++;
									}
									temp++;
									thread_strings[ind++] = '<';
									thread_strings[ind++] = 'b';
									thread_strings[ind++] = 'r';
									thread_strings[ind++] = '>';
									retval_miss++;
								}
								free(retval);
								retval = NULL;
								retval = strdup(thread_strings);
							}
							
							sprintf(res, "<li><a href=/%d/config/set?%s>%s</a> = %s</li>\n", thread,
							        config_params[i].param_name, config_params[i].param_name, retval);
							free(retval);
						} else if (thread != 0) {
							/* get the value from main thread for the rest of threads */
							value = config_params[i].print(cnt, NULL, i, 0);
							
							sprintf(res,"<li><a href=/%d/config/set?%s>%s</a> = %s</li>\n", thread,
							        config_params[i].param_name, config_params[i].param_name,
							        value ? value : "(not defined)");
						} else {
							sprintf(res, "<li><a href=/%d/config/set?%s>%s</a> = %s</li>\n", thread,
							        config_params[i].param_name, config_params[i].param_name,
							        "(not defined)");
						}
						
					} else {
						sprintf(res,"<li><a href=/%d/config/set?%s>%s</a> = %s</li>\n",thread,
						        config_params[i].param_name, config_params[i].param_name, value);
					}
					send_template(client_socket, res);
				}

				sprintf(res,"<br><a href=/%d/config><- back</a>",thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				for (i=0; config_params[i].param_name != NULL; i++) {
					value=config_params[i].print(cnt, NULL, i, thread);
					if (value == NULL)
						value=config_params[i].print(cnt, NULL, i, 0);
					sprintf(res,"%s = %s\n", config_params[i].param_name, value);
					send_template_raw(client_socket, res);
				}
			}
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_found_response_valid_command, NULL);
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else if (!strcmp(command,"set")) {
		/* set?param_name=value */
		pointer = pointer + 3;
		length_uri = length_uri - 3;
		if ((length_uri != 0) && (question == '?')) {
			pointer++;
			length_uri--;
			warningkill = sscanf(pointer,"%256[a-z_]%c", command, &question);
			/*check command , question == '='  length_uri too*/
			if ((question == '=') && (command[0]!='\0')) {
				length_uri = length_uri - strlen(command) - 1;
				pointer = pointer + strlen(command) + 1;
				/* check if command exists and type of command and not end of URI */
				i=0;
				while (config_params[i].param_name != NULL) {
					if (!strcasecmp(command, config_params[i].param_name))
						break;
					i++;
				}

				if (config_params[i].param_name) {
					if (length_uri > 0) {
						char Value[1024]={'\0'};
						warningkill = sscanf(pointer,"%1024s", Value);
						length_uri = length_uri - strlen(Value);
						if ( (length_uri == 0) && (strlen(Value) > 0) ) {
							/* FIXME need to assure that is a valid value */
							url_decode(Value,strlen(Value));
							conf_cmdparse(cnt + thread, config_params[i].param_name, Value);
							if (cnt[0]->conf.control_html_output) {
								sprintf(res,
								        "<li><a href=/%d/config/set?%s>%s</a> = %s</li> <b>Done</b><br>\n"
								        "<a href=/%d/config/list><- back</a>\n",
								        thread, config_params[i].param_name,
								        config_params[i].param_name, Value, thread);
								
								send_template_ini_client(client_socket, ini_template);
								send_template(client_socket,res);
								send_template_end_client(client_socket);
							} else {
								send_template_ini_client_raw(client_socket);
								sprintf(res, "%s = %s\nDone\n", config_params[i].param_name, Value);
								send_template_raw(client_socket, res);
							}
						} else {
							/*error*/
							if (cnt[0]->conf.control_html_output)
								response_client(client_socket, not_valid_syntax, NULL);
							else
								response_client(client_socket, not_valid_syntax_raw, NULL);
						}
					} else {
						char *type = NULL;
						type = strdup(config_type(&config_params[i]));

						if (!strcmp(type,"string")) {
							char *value = NULL;
							conf_cmdparse(cnt+thread, config_params[i].param_name, value);
							free(type);
							type = strdup("(null)");
						} else if (!strcmp(type,"int")) {
							free(type);
							type = strdup("0");
							conf_cmdparse(cnt+thread, config_params[i].param_name, type);
						} else if (!strcmp(type,"bool")) {
							free(type);
							type = strdup("off");
							conf_cmdparse(cnt+thread, config_params[i].param_name, type);
						} else {
							free(type);
							type = strdup("unknown");
						}

						if (cnt[0]->conf.control_html_output) {
							sprintf(res,
							        "<li><a href=/%d/config/set?%s>%s</a> = %s</li> <b>Done</b><br>\n"
							        "<a href=/%d/config/list><- back</a>",
							        thread, config_params[i].param_name,
							        config_params[i].param_name, type, thread);
							
							send_template_ini_client(client_socket, ini_template);
							send_template(client_socket, res);
							send_template_end_client(client_socket);
						} else {
							send_template_ini_client_raw(client_socket);
							sprintf(res, "%s = %s\nDone\n", config_params[i].param_name,type);
							send_template_raw(client_socket, res);
						}
						free(type);

					}
				} else {
					/*error*/
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, not_found_response_valid_command, NULL);
					else
						response_client(client_socket, not_found_response_valid_command_raw, NULL);
				}
			} else {
				/* Show param_name dialogue only for html output */
				if ( (cnt[0]->conf.control_html_output) && (command[0]!='\0') &&
				     (((length_uri = length_uri - strlen(command)) == 0 )) ) {
					i=0;
					while (config_params[i].param_name != NULL) {
						if (!strcasecmp(command, config_params[i].param_name))
							break;
						i++;
					}
					/* param_name exists */
					if (config_params[i].param_name) {
						send_template_ini_client(client_socket, ini_template);
						if (!strcmp ("bool",config_type(&config_params[i])) )
							sprintf(res, "<b>Thread %d </b>\n"
								     "<form action=set?>\n"
								     "<b>%s</b>&nbsp;<select name='%s'>\n"
								     "<option value='on'>on</option>\n"
								     "<option value='off'>off</option></select>\n"
								     "<input type='submit' value='set'>\n"
								     "</form>\n"
								     "<a href=/%d/config/list><- back</a>\n", thread,
								     config_params[i].param_name, config_params[i].param_name, thread);
						else
							sprintf(res, "<b>Thread %d </b>\n"
								     "<form action=set?>\n"
								     "<b>%s</b>&nbsp;<input type=text name='%s' value=''>\n"
								     "<input type='submit' value='set'>\n"
								     "</form>\n"
								     "<a href=/%d/config/list><- back</a>\n", thread,
								     config_params[i].param_name, config_params[i].param_name, thread);
						send_template(client_socket, res);
						send_template_end_client(client_socket);
					} else {
						if (cnt[0]->conf.control_html_output)
							response_client(client_socket, not_found_response_valid_command, NULL);
						else
							response_client(client_socket, not_found_response_valid_command_raw, NULL);
					}
				} else {
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, not_found_response_valid_command, NULL);
					else
						response_client(client_socket, not_found_response_valid_command_raw, NULL);
				}
			}
		} else if (length_uri == 0) {
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, set_template);
				sprintf(res, "<b>Thread %d </b>\n<form name='n'>\n<select name='onames'>\n", thread);
				send_template(client_socket, res);
				for (i=0; config_params[i].param_name != NULL; i++) {
					sprintf(res, "<option value='%s'>%s</option>\n",
					        config_params[i].param_name, config_params[i].param_name);
					send_template(client_socket, res);
				}
				sprintf(res, "</select>\n</form>\n"
				             "<form action=set name='s'"
				            "ONSUBMIT='if (!this.submitted) return false; else return true;'>\n"
				             "<input type=text name='valor' value=''>\n"
				             "<input type='button' value='set' onclick='javascript:show()'>\n"
				             "</form>\n"
				             "<a href=/%d/config><- back</a>\n", thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"set needs param_name=value\n");
				send_template_raw(client_socket, res);
			}
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_found_response_valid_command, NULL);
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else if (!strcmp(command,"get")) {
		/* get?query=param_name */
		pointer = pointer + 3;
		length_uri = length_uri - 3;

		if ((length_uri > 7) && (question == '?')) {
			/* 8 -> query=param_name FIXME minimum length param_name */
			pointer++;
			length_uri--;
			warningkill = sscanf(pointer,"%256[a-z]%c", command, &question);

			if ( (question == '=') && (!strcmp(command,"query")) ) {
				pointer = pointer + 6;
				length_uri = length_uri - 6;
				warningkill = sscanf(pointer, "%256[a-z_]", command);
				/*check if command exist, length_uri too*/
				length_uri = length_uri-strlen(command);

				if (length_uri == 0) {
					const char *value = NULL;
					i = 0;
					while (config_params[i].param_name != NULL) {
						if (!strcasecmp(command, config_params[i].param_name))
							break;
						i++;
					}
					/* FIXME bool values or commented values maybe that should be
					solved with config_type */

					if (config_params[i].param_name) {
						const char *type=NULL;
						type = config_type(&config_params[i]);
						if (!strcmp(type,"unknown")) {
							/*error doesn't exists this param_name */
							if (cnt[0]->conf.control_html_output)
								response_client(client_socket, not_found_response_valid_command, NULL);
							else
								response_client(client_socket, not_found_response_valid_command_raw, NULL);
							return 1;
						} else {
							value = config_params[i].print(cnt, NULL, i, thread);
							if (value == NULL)
								value=config_params[i].print(cnt, NULL, i, 0);
							if (cnt[0]->conf.control_html_output) {
								send_template_ini_client(client_socket,ini_template);
								sprintf(res,"<li>%s = %s</li><br>\n"
								            "<a href=/%d/config/get><- back</a>\n",
								        config_params[i].param_name,value, thread);
								send_template(client_socket, res);
								send_template_end_client(client_socket);
							} else {
								send_template_ini_client_raw(client_socket);
								sprintf(res,"%s = %s\nDone\n", config_params[i].param_name,value);
								send_template_raw(client_socket, res);
							}
						}
					} else {
						/*error*/
						if (cnt[0]->conf.control_html_output)
							response_client(client_socket, not_found_response_valid_command, NULL);
						else
							response_client(client_socket, not_found_response_valid_command_raw, NULL);
					}
				} else {
					/*error*/
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, not_found_response_valid_command, NULL);
					else
						response_client(client_socket, not_found_response_valid_command_raw, NULL);
				}
			}
		} else if (length_uri == 0) {
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res,"<b>Thread %d </b><br>\n"
				            "<form action=get>\n"
				            "<select name='query'>\n", thread);
				send_template(client_socket, res);
				for (i=0; config_params[i].param_name != NULL; i++) {
					sprintf(res,"<option value='%s'>%s</option>\n",
					        config_params[i].param_name, config_params[i].param_name);
					send_template(client_socket, res);
				}
				sprintf(res,"</select>\n"
				            "<input type='submit' value='get'>\n"
				            "</form>\n"
				            "<a href=/%d/config><- back</a>\n", thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"get needs param_name\n");
				send_template_raw(client_socket, res);
			}
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_valid_syntax, NULL);
			else
				response_client(client_socket, not_valid_syntax_raw, NULL);
		}
	} else if (!strcmp(command,"write")) {
			pointer = pointer + 5;
			length_uri = length_uri - 5;
			if (length_uri == 0) {
				if (cnt[0]->conf.control_html_output) {
					send_template_ini_client(client_socket, ini_template);
					sprintf(res,"Are you sure? <a href=/%d/config/writeyes>Yes</a><br>\n"
					            "<a href=/%d/config>No</a>\n", thread, thread);
					send_template(client_socket, res);
					send_template_end_client(client_socket);
				} else {
					conf_print(cnt);
					send_template_ini_client_raw(client_socket);
					sprintf(res,"Thread %d write\nDone\n", thread);
					send_template_raw(client_socket, res);
				}
			} else {
				/*error*/
				if (cnt[0]->conf.control_html_output)
					response_client(client_socket, not_found_response_valid_command, NULL);
				else
					response_client(client_socket, not_found_response_valid_command_raw, NULL);
			}

	} else if (!strcmp(command, "writeyes")) {
		pointer = pointer + 8;
		length_uri = length_uri - 8;
		if (length_uri==0) {
			conf_print(cnt);
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res,"<b>Thread %d</b> write done !<br>\n"
				            "<a href=/%d/config><- back</a>\n", thread, thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"Thread %d write\nDone\n", thread);
				send_template_raw(client_socket, res);
			}
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output) {
				response_client(client_socket, not_found_response_valid_command, NULL);
			}
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else {
		/*error*/
		if (cnt[0]->conf.control_html_output)
			response_client(client_socket, not_found_response_valid_command, NULL);
		else
			response_client(client_socket, not_found_response_valid_command_raw, NULL);
	}

	return 1;
}


/*
    This function manages/parses the actions for motion ( makemovie , snapshot , restart , quit ).
*/

static int action(char *pointer, char *res, int length_uri, int thread, int client_socket, void *userdata)
{
	/* parse action commands */
	char command[256] = {'\0'};
	struct context **cnt = userdata;

	warningkill = sscanf (pointer, "%256[a-z]" , command);
	if (!strcmp(command,"makemovie")) {
		pointer = pointer + 9;
		length_uri = length_uri - 9;
		if (length_uri == 0) {
			/*call makemovie*/

			if (thread == 0) {
				int i = 0;
				while (cnt[++i])
					cnt[i]->makemovie=1;
			} else {
				cnt[thread]->makemovie=1;
			}

			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res,"makemovie for thread %d done<br>\n"
				            "<a href=/%d/action><- back</a>\n", thread, thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"makemovie for thread %d\nDone\n", thread);
				send_template_raw(client_socket, res);
			}
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket,not_found_response_valid_command,NULL);
			else
				response_client(client_socket,not_found_response_valid_command_raw,NULL);
		}
	} else if (!strcmp(command,"snapshot")) {
		pointer = pointer + 8;
		length_uri = length_uri - 8;
		if (length_uri == 0) {
			/*call snapshot*/

			if (thread == 0) {
				int i = 0;
				while (cnt[++i])
					cnt[i]->snapshot=1;
			} else {
				cnt[thread]->snapshot=1;
			}

			cnt[thread]->snapshot = 1;
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res,"snapshot for thread %d done<br>\n"
				            "<a href=/%d/action><- back</a>\n", thread, thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"snapshot for thread %d\nDone\n", thread);
				send_template_raw(client_socket, res);
			}
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_found_response_valid_command, NULL);
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else if (!strcmp(command, "restart")) {
		pointer = pointer + 7;
		length_uri = length_uri - 7;
		if (length_uri == 0) {
			int i = 0;
			do {
				motion_log(LOG_DEBUG, 0, "httpd restart");
				kill(getpid(),1);
			} while (cnt[++i]);

			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res,"restart in progress ... bye<br>\n<a href='/'>Home</a>");
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"restart in progress ...\nDone\n");
				send_template_raw(client_socket, res);
			}
			return 0; // to restart
		} else {
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket,not_found_response_valid_command,NULL);
			else
				response_client(client_socket,not_found_response_valid_command_raw,NULL);
		}
	} else if (!strcmp(command,"quit")) {
		pointer = pointer + 4;
		length_uri = length_uri - 4;
		if (length_uri == 0) {
			int i = 0;
			/*call quit*/
			do {
				motion_log(LOG_DEBUG, 0, "httpd quitting");
				cnt[i]->makemovie = 1;
				cnt[i]->finish = 1;
			} while (cnt[++i]);

			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res,"quit in progress ... bye");
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"quit in progress ... bye\nDone\n");
				send_template_raw(client_socket, res);
			}
			return 0; // to quit
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_found_response_valid_command, NULL);
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else {
		if (cnt[0]->conf.control_html_output)
			response_client(client_socket, not_found_response_valid_command, NULL);
		else
			response_client(client_socket, not_found_response_valid_command_raw, NULL);
	}

	return 1;
}

/*
   This function manages/parses the detection actions for motion ( status , start , pause ).
*/

static int detection(char *pointer, char *res, int length_uri, int thread, int client_socket, void *userdata)
{
	char command[256]={'\0'};
	struct context **cnt=userdata;

	warningkill = sscanf (pointer, "%256[a-z]" , command);
	if (!strcmp(command,"status")) {
		pointer = pointer + 6;
		length_uri = length_uri - 6;
		if (length_uri == 0) {
			/*call status*/
			
			if (cnt[thread]->pause)
				sprintf(res, "Thread %d Detection status PAUSE\n", thread);
			else
				sprintf(res, "Thread %d Detection status ACTIVE\n", thread);

			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				send_template(client_socket, res);
				sprintf(res, "<br><a href=/%d/detection><- back</a>\n", thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				send_template_raw(client_socket, res);
			}
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_found_response_valid_command, NULL);
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else if (!strcmp(command, "start")) {
		pointer = pointer + 5;
		length_uri = length_uri - 5;
		if (length_uri == 0) {
			/*call start*/
			int i = 0;

			cnt[thread]->pause = 0;

			if (thread == 0) {
				do {
					cnt[i]->pause = 0;
				} while (cnt[++i]);
			} else {
				cnt[thread]->pause = 0;
			}

			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket,ini_template);
				sprintf(res,"Thread %i Detection resumed<br>\n"
				            "<a href=/%d/detection><- back</a>\n", thread, thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"Thread %i Detection resumed\nDone\n", thread);
				send_template_raw(client_socket, res);
			}
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_found_response_valid_command, NULL);
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else if (!strcmp(command,"pause")){
		pointer = pointer + 5;
		length_uri = length_uri - 5;
		if (length_uri==0) {
			/*call pause*/
			int i = 0;
			
			cnt[thread]->pause=1;

			if (thread == 0) {
				do {
					cnt[i]->pause = 1;
				} while (cnt[++i]);
			} else {
				cnt[thread]->pause = 1;
			}
			
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res,"Thread %i Detection paused<br>\n"
				            "<a href=/%d/detection><- back</a>\n",thread,thread);
				send_template(client_socket,res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"Thread %i Detection paused\nDone\n", thread);
				send_template_raw(client_socket, res);
			}
		} else {
			/*error*/
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_found_response_valid_command, NULL);
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else if (!strcmp(command,"connection")){
		pointer = pointer + 10;
		length_uri = length_uri - 10;
		if (length_uri==0) {
			
			/*call connection*/	
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				if (thread == 0){
					int i = 0;
					do{
						sprintf(res,"Thread %i %s<br>\n",i, 
								(cnt[i]->lost_connection)?CONNECTION_KO:CONNECTION_OK);
						send_template(client_socket,res);
					}while (cnt[++i]);
					sprintf(res,"<a href=/%d/detection><- back</a>\n",thread);
				}else{
					sprintf(res,"Thread %i %s<br>\n"
							"<a href=/%d/detection><- back</a>\n",thread, (cnt[thread]->lost_connection)? CONNECTION_KO: CONNECTION_OK, thread);
				}	
				send_template(client_socket,res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				if (thread == 0){
					int i = 0;
					do{
						sprintf(res,"Thread %i %s\n", i,
								(cnt[i]->lost_connection)? CONNECTION_KO: CONNECTION_OK);
						send_template_raw(client_socket, res);
					}while (cnt[++i]);
				}else{		
					sprintf(res,"Thread %i %s\n", thread,(cnt[thread]->lost_connection)? CONNECTION_KO: CONNECTION_OK);
					send_template_raw(client_socket, res);
				}	
				
			}	
		}else{
			/*error*/
			 if (cnt[0]->conf.control_html_output)
				 response_client(client_socket, not_found_response_valid_command, NULL);
			 else
				 response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}	
	} else {
		if (cnt[0]->conf.control_html_output)
			response_client(client_socket, not_found_response_valid_command, NULL);
		else
			response_client(client_socket, not_found_response_valid_command_raw, NULL);
	}

	return 1;
}


/*
   This function manages/parses the track action for motion ( set , pan , tilt , auto ).
*/

static int track(char *pointer, char *res, int length_uri, int thread, int client_socket, void *userdata)
{
	char question;
	char command[256] = {'\0'};
	struct context **cnt = userdata;

	warningkill = sscanf(pointer, "%256[a-z]%c", command, &question);
	if (!strcmp(command, "set")) {
		pointer=pointer+3;length_uri=length_uri-3;
		/* FIXME need to check each value */
		/* Relative movement set?pan=0&tilt=0 | set?pan=0 | set?tilt=0*/
		/* Absolute movement set?x=0&y=0 | set?x=0 | set?y=0 */

		if ((question == '?') && (length_uri > 2)) {
			char panvalue[12] = {'\0'}, tiltvalue[12] = {'\0'};
			char x_value[12] = {'\0'}, y_value[12] = {'\0'};
			struct context *setcnt;
			int pan = 0, tilt = 0, X = 0 , Y = 0;

			pointer++;
			length_uri--;
			/* set?pan=value&tilt=value */
			/* set?x=value&y=value */
			/* pan= or x= | tilt= or y= */

			warningkill = sscanf (pointer, "%256[a-z]%c" , command, &question);

			if (( question != '=' ) || (command[0] == '\0')) {
				/* no valid syntax */
				motion_log(LOG_WARNING, 0, "httpd debug race 1");
				if (cnt[0]->conf.control_html_output)
					response_client(client_socket, not_valid_syntax, NULL);
				else
					response_client(client_socket, not_valid_syntax_raw, NULL);
				return 1;
			}

			pointer++;
			length_uri--;

			/* Check first parameter */

			if (!strcmp(command, "pan")) {
				pointer = pointer + 3;
				length_uri = length_uri - 3;
				pan = 1;
				if ((warningkill = sscanf(pointer, "%10[-0-9]", panvalue))){
					pointer = pointer + strlen(panvalue);
					length_uri = length_uri - strlen(panvalue);
				}
			}
			else if (!strcmp(command, "tilt")) {
				pointer = pointer + 4;
				length_uri = length_uri - 4;
				tilt = 1;
				if ((warningkill = sscanf(pointer, "%10[-0-9]", tiltvalue))){
					pointer = pointer + strlen(tiltvalue);
					length_uri = length_uri - strlen(tiltvalue);
				}
			}
			else if (!strcmp(command, "x")) {
				pointer++;
				length_uri--;
				X = 1;
				if ((warningkill = sscanf(pointer, "%10[-0-9]", x_value))){
					pointer = pointer + strlen(x_value);
					length_uri = length_uri - strlen(x_value);
				}
			}
			else if (!strcmp(command, "y")) {
				pointer++;
				length_uri--;
				Y = 1;
				if ((warningkill = sscanf (pointer, "%10[-0-9]" , y_value))){
					pointer = pointer + strlen(y_value);
					length_uri = length_uri - strlen(y_value);
				}
			} else {
				/* no valid syntax */
				motion_log(LOG_WARNING, 0, "httpd debug race 2");
				if (cnt[0]->conf.control_html_output)
					response_client(client_socket, not_valid_syntax, NULL);
				else
					response_client(client_socket, not_valid_syntax_raw, NULL);
				return 1;
			}


			/* first value check for error */

			
			if ( !warningkill ) {
				motion_log(LOG_WARNING, 0, "httpd debug race 3");
				/* error value */
				if (cnt[0]->conf.control_html_output)
					response_client(client_socket, error_value, NULL);
				else
					response_client(client_socket, error_value_raw, NULL);
				return 1;
			}

			

			/* Only one parameter (pan= ,tilt= ,x= ,y= ) */
			if (length_uri == 0) {
				if (pan) {
					struct coord cent;
					struct context *pancnt;

					/* move pan */

					pancnt = cnt[thread];
					cent.width = pancnt->imgs.width;
					cent.height = pancnt->imgs.height;
					cent.y = 0;
					cent.x = atoi(panvalue);
					// Add the number of frame to skip for motion detection
					cnt[thread]->moved = track_move(pancnt, pancnt->video_dev, &cent, &pancnt->imgs, 1);
					if (cnt[thread]->moved) {
						if (cnt[0]->conf.control_html_output) {
							send_template_ini_client(client_socket, ini_template);
							sprintf(res,"track set relative pan=%s<br>\n"
							            "<a href=/%d/track><- back</a>\n", panvalue, thread);
							send_template(client_socket, res);
							send_template_end_client(client_socket);
						} else {
							send_template_ini_client_raw(client_socket);
							sprintf(res,"track set relative pan=%s\nDone\n", panvalue);
							send_template_raw(client_socket, res);
						}
					} else {
					/*error in track action*/
						if (cnt[0]->conf.control_html_output) {
							sprintf(res, "<a href=/%d/track><- back</a>\n", thread);
							response_client(client_socket, track_error, res);
						}
						else
							response_client(client_socket, track_error_raw, NULL);
					}
				} else if (tilt) {
					struct coord cent;
					struct context *tiltcnt;

					/* move tilt */

					tiltcnt = cnt[thread];
					cent.width = tiltcnt->imgs.width;
					cent.height = tiltcnt->imgs.height;
					cent.x = 0;
					cent.y = atoi(tiltvalue);
					// Add the number of frame to skip for motion detection
					cnt[thread]->moved=track_move(tiltcnt, tiltcnt->video_dev, &cent, &tiltcnt->imgs, 1);
					if (cnt[thread]->moved){	
						if (cnt[0]->conf.control_html_output) {
							send_template_ini_client(client_socket, ini_template);
							sprintf(res,"track set relative tilt=%s<br>\n"
							            "<a href=/%d/track><- back</a>\n", tiltvalue, thread);
							send_template(client_socket, res);
							send_template_end_client(client_socket);
						} else {
							send_template_ini_client_raw(client_socket);
							sprintf(res,"track set relative tilt=%s\nDone\n",tiltvalue);
							send_template_raw(client_socket, res);
						}
					} else {
						/*error in track action*/
						if (cnt[0]->conf.control_html_output) {
							sprintf(res,"<a href=/%d/track><- back</a>\n", thread);
							response_client(client_socket, track_error, res);
						}
						else
							response_client(client_socket, track_error_raw, NULL);
					}
				} else if (X){
					/* X */
					setcnt = cnt[thread];
					// 1000 is out of range for pwc
					cnt[thread]->moved = track_center(setcnt, setcnt->video_dev, 1, atoi(x_value), 1000);
					if (cnt[thread]->moved) {
						if (cnt[0]->conf.control_html_output) {
							send_template_ini_client(client_socket, ini_template);
							sprintf(res,"track set absolute x=%s<br>\n"
							            "<a href=/%d/track><- back</a>\n", x_value, thread);
							send_template(client_socket, res);
							send_template_end_client(client_socket);
						} else {
							send_template_ini_client_raw(client_socket);
							sprintf(res,"track set absolute x=%s\nDone\n", x_value);
							send_template_raw(client_socket, res);
						}
					} else {
						/*error in track action*/
						if (cnt[0]->conf.control_html_output) {
							sprintf(res,"<a href=/%d/track><- back</a>\n", thread);
							response_client(client_socket, track_error, res);
						}
						else
							response_client(client_socket, track_error_raw, NULL);
					}

				} else {
					/* Y */
					setcnt = cnt[thread];
					// 1000 is out of range for pwc
					cnt[thread]->moved = track_center(setcnt, setcnt->video_dev, 1, 1000, atoi(y_value));
					if (cnt[thread]->moved) {
						if (cnt[0]->conf.control_html_output) {
							send_template_ini_client(client_socket, ini_template);
							sprintf(res,"track set absolute y=%s<br>\n"
							            "<a href=/%d/track><- back</a>\n", y_value, thread);
							send_template(client_socket, res);
							send_template_end_client(client_socket);
						} else {
							send_template_ini_client_raw(client_socket);
							sprintf(res,"track set absolute y=%s\nDone\n", y_value);
							send_template_raw(client_socket, res);
						}
					} else {
						/*error in track action*/
						if (cnt[0]->conf.control_html_output) {
							sprintf(res,"<a href=/%d/track><- back</a>", thread);
							response_client(client_socket, track_error, res);
						}
						else
							response_client(client_socket, track_error_raw, NULL);
					}
				}
				return 1;
			}


			/* Check Second parameter */

			warningkill = sscanf (pointer, "%c%256[a-z]" ,&question, command);
			if ( ( question != '&' ) || (command[0] == '\0') ){
				motion_log(LOG_WARNING, 0, "httpd debug race 4");
				if ( strstr(pointer,"&")){
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, error_value, NULL);
					else
						response_client(client_socket, error_value_raw, NULL);
				}	
				/* no valid syntax */
				else{	
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, not_valid_syntax, NULL);
					else
						response_client(client_socket, not_valid_syntax_raw, NULL);
				}	
				return 1;
			}

			pointer++;
			length_uri--;

			if (!strcmp(command, "pan")){
				pointer = pointer + 3;
				length_uri = length_uri - 3;
				if ( (pan) || (!tilt) || (X) || (Y) ) {
					motion_log(LOG_WARNING, 0, "httpd debug race 5");
					/* no valid syntax */
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, not_valid_syntax, NULL);
					else
						response_client(client_socket, not_valid_syntax_raw, NULL);
					return 1;
				}
				pan=2;
				warningkill = sscanf (pointer, "%c%10[-0-9]" ,&question, panvalue);
			}
			else if (!strcmp(command, "tilt")) {
				pointer = pointer + 4;
				length_uri = length_uri - 4;
				if ( (tilt) || (!pan) || (X) || (Y) ) {
					/* no valid syntax */
					motion_log(LOG_WARNING, 0, "httpd debug race 6");
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, not_valid_syntax, NULL);
					else
						response_client(client_socket, not_valid_syntax_raw, NULL);
					return 1;
				}
				tilt = 2;
				warningkill = sscanf (pointer, "%c%10[-0-9]" ,&question, tiltvalue);
			}
			else if	(!strcmp(command, "x")) {
				pointer++;
				length_uri--;
				if ( (X) || (!Y) || (pan) || (tilt) ) {
					motion_log(LOG_WARNING, 0, "httpd debug race 7");
					
					/* no valid syntax */
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, not_valid_syntax, NULL);
					else
						response_client(client_socket, not_valid_syntax_raw, NULL);
					return 1;
				}
				X = 2;
				warningkill = sscanf (pointer, "%c%10[-0-9]" ,&question, x_value);
			}
			else if	(!strcmp(command, "y")) {
				pointer++;
				length_uri--;
				if ( (Y) || (!X) || (pan) || (tilt) ){
					motion_log(LOG_WARNING, 0, "httpd debug race 8");
					/* no valid syntax */
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, not_valid_syntax, NULL);
					else
						response_client(client_socket, not_valid_syntax_raw, NULL);
					return 1;
				}
				Y = 2;
				warningkill = sscanf (pointer, "%c%10[-0-9]" ,&question, y_value);
			} else {
				motion_log(LOG_WARNING, 0, "httpd debug race 9");
				/* no valid syntax */
				if (cnt[0]->conf.control_html_output)
					response_client(client_socket, not_valid_syntax, NULL);
				else
					response_client(client_socket, not_valid_syntax_raw, NULL);
				return 1;
			}

			/* Second value check */

			if ( ( warningkill < 2 ) && (question != '=') ) {
				motion_log(LOG_WARNING, 0, "httpd debug race 10");
				/* no valid syntax */
				if (cnt[0]->conf.control_html_output)
					response_client(client_socket, not_valid_syntax, NULL);
				else
					response_client(client_socket, not_valid_syntax_raw, NULL);
				return 1;
			}else if (( question == '=') && ( warningkill == 1)){
				motion_log(LOG_WARNING, 0, "httpd debug race 11");
				if (cnt[0]->conf.control_html_output)
					response_client(client_socket, error_value, NULL);
				else
					response_client(client_socket, error_value_raw, NULL);
				return 1;
			}
			
			
			if (pan == 2){
				pointer = pointer + strlen(panvalue) + 1;
				length_uri = length_uri - strlen(panvalue) - 1;	
			}
			else if (tilt == 2){
				pointer = pointer + strlen(tiltvalue) + 1;
				length_uri = length_uri - strlen(tiltvalue) - 1;
			}
			else if (X == 2){
				pointer = pointer + strlen(x_value) + 1;
				length_uri = length_uri - strlen(x_value) - 1;
			}
			else{
				pointer = pointer + strlen(y_value) + 1;
				length_uri = length_uri - strlen(y_value) - 1;
			}	
			
			

			if (length_uri != 0) {
				motion_log(LOG_WARNING, 0, "httpd debug race 12");
				if (cnt[0]->conf.control_html_output)
					response_client(client_socket, error_value, NULL);
				else
					response_client(client_socket, error_value_raw, NULL);
				return 1;
			}


			/* track set absolute ( x , y )*/

			if ( X && Y ) {
				setcnt = cnt[thread];
				cnt[thread]->moved = track_center(setcnt, setcnt->video_dev, 1, atoi(x_value), atoi(y_value));
				if (cnt[thread]->moved) {
					if (cnt[0]->conf.control_html_output){
						send_template_ini_client(client_socket, ini_template);
						sprintf(res,"track set x=%s y=%s<br>\n"
						            "<a href=/%d/track><- back</a>\n", x_value, y_value, thread);
						send_template(client_socket, res);
						send_template_end_client(client_socket);
					} else {
						send_template_ini_client_raw(client_socket);
						sprintf(res,"track set x=%s y=%s\nDone\n", x_value, y_value);
						send_template_raw(client_socket, res);
					}
				} else {
					/*error in track action*/
					if (cnt[0]->conf.control_html_output) {
						sprintf(res,"<a href=/%d/track><- back</a>\n", thread);
						response_client(client_socket, track_error, res);
					}
					else
						response_client(client_socket, track_error_raw, NULL);
				}
				/* track set relative ( pan , tilt )*/
			} else {
				struct coord cent;
				struct context *relativecnt;

				/* move pan */

				relativecnt = cnt[thread];
				cent.width = relativecnt->imgs.width;
				cent.height = relativecnt->imgs.height;
				cent.y = 0;
				cent.x = atoi(panvalue);
				// Add the number of frame to skip for motion detection
				cnt[thread]->moved = track_move(relativecnt, relativecnt->video_dev, &cent, &relativecnt->imgs, 1);
				if (cnt[thread]->moved){
					/* move tilt */
					relativecnt = cnt[thread];
					cent.width = relativecnt->imgs.width;
					cent.height = relativecnt->imgs.height;
					cent.x = 0;
					cent.y = atoi(tiltvalue);
					SLEEP(1,0);
					cnt[thread]->moved = track_move(relativecnt, relativecnt->video_dev, &cent, &relativecnt->imgs, 1);
					if (cnt[thread]->moved){
						if (cnt[0]->conf.control_html_output) {
							send_template_ini_client(client_socket, ini_template);
							sprintf(res,"track pan=%s tilt=%s<br>\n"
							            "<a href=/%d/track><- back</a>\n", panvalue, tiltvalue, thread);
							send_template(client_socket, res);
							send_template_end_client(client_socket);
						} else {
							send_template_ini_client_raw(client_socket);
							sprintf(res,"track pan=%s tilt=%s\nDone\n", panvalue, tiltvalue);
							send_template_raw(client_socket, res);
						}
						return 1;
					}
					else{
						/*error in track tilt*/	
						if (cnt[0]->conf.control_html_output) {
							sprintf(res,"<a href=/%d/track><- back</a>\n", thread);
							response_client(client_socket, track_error, res);
						}else response_client(client_socket, track_error_raw, NULL);
					}
				}

				/*error in track pan*/
				if (cnt[0]->conf.control_html_output) {
					sprintf(res,"<a href=/%d/track><- back</a>\n", thread);
					response_client(client_socket, track_error, res);
				} else
					response_client(client_socket, track_error_raw, NULL);
			}
		} else if (length_uri == 0) {
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res,"<b>Thread %d</b><br>\n"
				            "<form action='set'>\n"
				            "Pan<input type=text name='pan' value=''>\n"
				            "Tilt<input type=text name='tilt' value=''>\n"
				            "<input type=submit value='set relative'>\n"
				            "</form>\n"
				            "<form action='set'>\n"
				            "X<input type=text name='x' value=''>\n"
				            "Y<input type=text name='y' value=''>\n"
				            "<input type=submit value='set absolute'>\n"
				            "</form>\n"
				            "<a href=/%d/track><- back</a>\n", thread, thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"set needs a pan/tilt or x/y values\n");
				send_template_raw(client_socket, res);
			}
		} else {
			/* error not valid command */
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_found_response_valid_command, NULL);
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else if (!strcmp(command,"status")) {
		pointer = pointer+6;
		length_uri = length_uri-6;
		if (length_uri==0) {
			if (cnt[0]->conf.control_html_output) {
				if (cnt[thread]->track.active)
					sprintf(res,"<b>Thread %d</b><br>track auto enabled<br>\n",thread);
				else
					sprintf(res,"<b>Thread %d</b><br>track auto disabled<br>\n",thread);
				send_template_ini_client(client_socket, ini_template);
				send_template(client_socket, res);
				sprintf(res, "<a href=/%d/track><- back</a>\n",thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				if (cnt[thread]->track.active)
					sprintf(res,"Thread %d\n track auto enabled\nDone\n",thread);
				else
					sprintf(res,"Thread %d\n track auto disabled\nDone\n",thread);
				send_template_ini_client_raw(client_socket);
				send_template_raw(client_socket, res);
			}
		} else {
			/* error not valid command */
			if (cnt[0]->conf.control_html_output) {
				response_client(client_socket, not_found_response_valid_command, NULL);
			}
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else if (!strcmp(command,"auto")) {
		pointer = pointer + 4;
		length_uri = length_uri - 4;
		if ((question == '?') && (length_uri > 0)) {
			char query[256] = {'\0'};
			pointer++;
			length_uri--;
			/* value= */

			warningkill = sscanf (pointer, "%256[a-z]%c",query,&question);
			if ((question == '=') && (!strcmp(query,"value")) ) {
				pointer = pointer + 6;
				length_uri = length_uri - 6;
				warningkill = sscanf (pointer, "%256[-0-9a-z]" , command);
				if ((command!=NULL) && (strlen(command) > 0)) {
					struct context *autocnt;

					/* auto value=0|1|status*/

					if (!strcmp(command,"status")) {
						if (cnt[0]->conf.control_html_output) {
							if (cnt[thread]->track.active)
								sprintf(res, "<b>Thread %d</b><br>track auto enabled\n", thread);
							else
								sprintf(res, "<b>Thread %d</b><br>track auto disabled\n", thread);
							send_template_ini_client(client_socket, ini_template);
							send_template(client_socket, res);
							sprintf(res,"<a href=/%d/track><- back</a>", thread);
							send_template(client_socket, res);
							send_template_end_client(client_socket);
						} else {
							if (cnt[thread]->track.active)
								sprintf(res, "Thread %d\n track auto enabled\nDone\n", thread);
							else
								sprintf(res, "Thread %d\n track auto disabled\nDone\n", thread);
							send_template_ini_client_raw(client_socket);
							send_template_raw(client_socket, res);
						}
					} else {
						int active;
						active = atoi(command);
						if (active > -1 && active < 2) {
							autocnt = cnt[thread];
							autocnt->track.active = atoi(command);
							if (cnt[0]->conf.control_html_output) {
								send_template_ini_client(client_socket, ini_template);
								sprintf(res,"track auto	%s<br><a href=/%d/track><- back</a>",
								        command,thread);
								send_template(client_socket, res);
								send_template_end_client(client_socket);
							} else {
								send_template_ini_client_raw(client_socket);
								sprintf(res,"track auto %s\nDone\n",command);
								send_template_raw(client_socket, res);
							}
						} else {
							if (cnt[0]->conf.control_html_output)
								response_client(client_socket, not_found_response_valid_command, NULL);
							else
								response_client(client_socket, not_found_response_valid_command_raw, NULL);
						}
					}
				} else {
					if (cnt[0]->conf.control_html_output)
						response_client(client_socket, not_found_response_valid_command, NULL);
					else
						response_client(client_socket, not_found_response_valid_command_raw, NULL);
				}
			} else {
				if (cnt[0]->conf.control_html_output)
					response_client(client_socket, not_found_response_valid_command, NULL);
				else
					response_client(client_socket, not_found_response_valid_command_raw, NULL);
			}
		}
		else if (length_uri == 0) {
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket, ini_template);
				sprintf(res,"<b>Thread %d</b>\n"
				            "<form action='auto'><select name='value'>\n"
				            "<option value='0'>Disable</option><option value='1'>Enable</option>\n"
				            "<option value='status'>status</option>\n"
				            "</select><input type=submit value='set'>\n"
				            "</form>\n"
				            "<a href=/%d/track><- back</a>\n", thread, thread);
				send_template(client_socket, res);
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"auto needs a value 0,1 or status\n");
				send_template_raw(client_socket, res);
			}
		} else {
			if (cnt[0]->conf.control_html_output)
				response_client(client_socket, not_found_response_valid_command, NULL);
			else
				response_client(client_socket, not_found_response_valid_command_raw, NULL);
		}
	} else {
		if (cnt[0]->conf.control_html_output)
			response_client(client_socket, not_found_response_valid_command, NULL);
		else
			response_client(client_socket, not_found_response_valid_command_raw, NULL);
	}

	return 1;
}



/*
	parses the action requested for motion ( config , action , detection , track ) and call
	to action function if needed.
*/

static int handle_get(int client_socket, const char* url, void *userdata)
{
	struct context **cnt=userdata;
	if (*url == '/' ){
		int i=0;
		char *res=NULL;
		res = malloc(2048);

		/* get the number of threads */
		while (cnt[++i]);
		/* ROOT_URI -> GET / */
		if (! (strcmp (url, "/")) ) {
			int y=0;
			if (cnt[0]->conf.control_html_output) {
				send_template_ini_client(client_socket,ini_template);
				sprintf(res,"<b>Motion "VERSION" Running [%d] Threads</b><br>\n"
				            "<a href='/0/'>All</a><br>\n", i);
				send_template(client_socket, res);
				for (y=1; y<i; y++) {
					sprintf(res,"<a href='/%d/'>Thread %d</a><br>\n", y, y);
					send_template(client_socket, res);
				}
				send_template_end_client(client_socket);
			} else {
				send_template_ini_client_raw(client_socket);
				sprintf(res,"Motion "VERSION" Running [%d] Threads\n0\n",i);
				send_template_raw(client_socket, res);
				for (y=1; y<i; y++) {
					sprintf(res, "%d\n", y);
					send_template_raw(client_socket, res);
				}
			}
		}
		else {
			char command[256] = {'\0'};
			char slash;
			int thread = -1;
			int length_uri = 0;
			char *pointer = (char *)url;

			length_uri = strlen(url);
			/* Check for Thread number first -> GET /2 */
			pointer++;
			length_uri--;
			warningkill = sscanf (pointer, "%i%c", &thread, &slash);

			if ((thread != -1) && (thread < i)) {
				/* thread_number found */
				if (thread > 9){
					pointer = pointer + 2;
					length_uri = length_uri - 2;
				}else{
					pointer++;
					length_uri--;	
				}
				
				if (slash == '/') {   /* slash found /2/ */
					pointer++;
					length_uri--;
				} 

				if (length_uri!=0) {
					warningkill = sscanf (pointer, "%256[a-z]%c" , command , &slash);

					/* config */
					if (!strcmp(command,"config")) {
						pointer = pointer + 6;
						length_uri = length_uri - 6;
						if (length_uri == 0) {
							if (cnt[0]->conf.control_html_output) {
								send_template_ini_client(client_socket, ini_template);
								sprintf(res,"<b>Thread %d</b><br>\n"
								            "<a href=/%d/config/list>list</a><br>\n"
								            "<a href=/%d/config/write>write</a><br>\n"
								            "<a href=/%d/config/set>set</a><br>\n"
								            "<a href=/%d/config/get>get</a><br>\n"
								            "<a href=/%d/><- back</a>\n",
								        thread, thread, thread, thread, thread, thread);
								send_template(client_socket, res);
								send_template_end_client(client_socket);
							} else {
								send_template_ini_client_raw(client_socket);
								sprintf(res,"Thread %d\nlist\nwrite\nset\nget\n", thread);
								send_template_raw(client_socket, res);
							}
						} else if ((slash == '/') && (length_uri >= 4)) {
							/*call config() */
							pointer++;
							length_uri--;
							config(pointer, res, length_uri, thread, client_socket, cnt);
						} else {
							if (cnt[0]->conf.control_html_output)
								response_client(client_socket, not_found_response_valid_command, NULL);
							else
								response_client(client_socket, not_found_response_valid_command_raw, NULL);
						}
					}
					/* action */
					else if (!strcmp(command,"action")) {
						pointer = pointer + 6;
						length_uri = length_uri - 6;
						/* call action() */
						if (length_uri == 0) {
							if (cnt[0]->conf.control_html_output) {
								send_template_ini_client(client_socket, ini_template);
								sprintf(res,"<b>Thread %d</b><br>\n"
								            "<a href=/%d/action/makemovie>makemovie</a><br>\n"
								            "<a href=/%d/action/snapshot>snapshot</a><br>\n"
								            "<a href=/%d/action/restart>restart</a><br>\n"
								            "<a href=/%d/action/quit>quit</a><br>\n"
								            "<a href=/%d/><- back</a>\n",
								        thread,thread,thread,thread,thread,thread);
								send_template(client_socket, res);
								send_template_end_client(client_socket);
							} else {
								send_template_ini_client_raw(client_socket);
								sprintf(res,"Thread %d\nmakemovie\nsnapshot\nrestart\nquit\n", thread);
								send_template_raw(client_socket, res);
							}
						} else if ((slash == '/') && (length_uri > 4)) {
							int ret = 1;
							pointer++;
							length_uri--;
							ret = action(pointer, res, length_uri, thread, client_socket, cnt);
							free(res);
							return ret;

						} else {
							if (cnt[0]->conf.control_html_output)
								response_client(client_socket, not_found_response_valid_command,NULL);
							else
								response_client(client_socket, not_found_response_valid_command_raw,NULL);
						}
					}
					/* detection */
					else if (!strcmp(command,"detection")) {
						pointer = pointer + 9;
						length_uri = length_uri - 9;
						if (length_uri == 0) {
							if (cnt[0]->conf.control_html_output) {
								send_template_ini_client(client_socket, ini_template);
								sprintf(res,"<b>Thread %d</b><br>\n"
								            "<a href=/%d/detection/status>status</a><br>\n"
								            "<a href=/%d/detection/start>start</a><br>\n"
								            "<a href=/%d/detection/pause>pause</a><br>\n"
								            "<a href=/%d/detection/connection>connection</a><br>\n"
								            "<a href=/%d/><- back</a>\n",
								        thread, thread, thread, thread, thread, thread);
								send_template(client_socket, res);
								send_template_end_client(client_socket);
							} else {
								send_template_ini_client_raw(client_socket);
								sprintf(res,"Thread %d\nstatus\nstart\npause\nconnection\n", thread);
								send_template_raw(client_socket, res);
							}
						} else if ((slash == '/') && (length_uri > 5)) {
							pointer++;
							length_uri--;
							/* call detection() */
							detection(pointer, res, length_uri, thread, client_socket, cnt);
						} else {
							if (cnt[0]->conf.control_html_output)
								response_client(client_socket, not_found_response_valid_command, NULL);
							else
								response_client(client_socket, not_found_response_valid_command_raw, NULL);
						}
					}
					/* track */
					else if (!strcmp(command,"track")) {
						pointer = pointer + 5;
						length_uri = length_uri - 5;
						if (length_uri == 0) {
							if (cnt[0]->conf.control_html_output) {
								send_template_ini_client(client_socket, ini_template);
								sprintf(res,"<b>Thread %d</b><br>\n"
								            "<a href=/%d/track/set>track set pan/tilt</a><br>\n"
								            "<a href=/%d/track/auto>track auto</a><br>\n"
								            "<a href=/%d/track/status>track status</a><br>\n"
								            "<a href=/%d/><- back</a>\n",
								        thread, thread, thread, thread, thread);
								send_template(client_socket, res);
								send_template_end_client(client_socket);
							} else {
								send_template_ini_client_raw(client_socket);
								sprintf(res,"Thread %d\nset pan/tilt\nauto\nstatus\n", thread);
								send_template_raw(client_socket, res);
							}
						}
						else if ((slash == '/') && (length_uri >= 4)) {
							pointer++;
							length_uri--;
							/* call track() */
							if (cnt[thread]->track.type) {
								track(pointer, res, length_uri, thread, client_socket, cnt);
							} else {
								/* error track not enable */
								if (cnt[0]->conf.control_html_output) {
									sprintf(res,"<a href=/%d/><- back</a>\n",thread);
									response_client(client_socket, not_track,res);
								}
								else
									response_client(client_socket, not_track_raw,NULL);
							}
						} else {
							if (cnt[0]->conf.control_html_output) {
								sprintf(res,"<a href=/%d/><- back</a>\n",thread);
								response_client(client_socket, not_found_response_valid_command, res);
							}
							else
								response_client(client_socket, not_found_response_valid_command_raw, NULL);
						}
					} else {
						if (cnt[0]->conf.control_html_output) {
							sprintf(res,"<a href=/%d/><- back</a>\n",thread);
							response_client(client_socket, not_found_response_valid_command, res);
						}
						else
							response_client(client_socket, not_found_response_valid_command_raw, NULL);
					}
				} else {
					/* /thread_number/ requested */
					if (cnt[0]->conf.control_html_output) {
						send_template_ini_client(client_socket,ini_template);
						sprintf(res,"<b>Thread %d</b><br>\n"
						            "<a href='/%d/config'>config</a><br>\n"
						            "<a href='/%d/action'>action</a><br>\n"
						            "<a href='/%d/detection'>detection</a><br>\n"
						            "<a href='/%d/track'>track</a><br>\n"
						            "<a href=/><- back</a>\n",
						        thread, thread, thread, thread, thread);
						send_template(client_socket, res);
						send_template_end_client(client_socket);
					} else {
						send_template_ini_client_raw(client_socket);
						sprintf(res,"Thread %d\nconfig\naction\ndetection\ntrack\n", thread);
						send_template_raw(client_socket, res);
					}
				}
			} else {
				if (cnt[0]->conf.control_html_output) {
					sprintf(res,"<a href=/><- back</a>\n");
					response_client(client_socket, not_found_response_valid, res);
				}
				else
					response_client(client_socket, not_found_response_valid_raw, NULL);
			}
		}
		free(res);
	} else {
		if (cnt[0]->conf.control_html_output)
			response_client(client_socket, not_found_response_template,NULL);
		else
			response_client(client_socket, not_found_response_template_raw,NULL);
	}

	return 1;
}


/*
 -TODO-
 As usually web clients uses nonblocking connect/read
 read_client should handle nonblocking sockets.
*/

static int read_client(int client_socket, void *userdata, char *auth)
{
	int alive = 1;
	int ret = 1;
	char buffer[1024] = {'\0'};
	int length = 1024;
	struct context **cnt = userdata;

	/* lock the mutex */
	pthread_mutex_lock(&httpd_mutex);

	while (alive)
	{
		int nread = 0, readb = -1;

		nread = read (client_socket, buffer, length);

		if (nread <= 0) {
			motion_log(LOG_ERR, 1, "httpd First read");
			pthread_mutex_unlock(&httpd_mutex);
			return -1;
		}
		else {
			char method[sizeof (buffer)];
			char url[sizeof (buffer)];
			char protocol[sizeof (buffer)];
			char *authentication=NULL;

			buffer[nread] = '\0';

			warningkill = sscanf (buffer, "%s %s %s", method, url, protocol);

			while ((strstr (buffer, "\r\n\r\n") == NULL) && (readb!=0) && (nread < length)){
				readb = read (client_socket, buffer+nread, sizeof (buffer) - nread);

				if (readb == -1){
					nread = -1;
					break;
				}

				nread +=readb;
				
				if (nread > length) {
					motion_log(LOG_ERR, 1, "httpd End buffer reached waiting for buffer ending");
					break;
				}
				buffer[nread] = '\0';
			}

			/* Make sure the last read didn't fail.  If it did, there's a
			problem with the connection, so give up.  */
			if (nread == -1) {
				motion_log(LOG_ERR, 1, "httpd READ");
				pthread_mutex_unlock(&httpd_mutex);
				return -1;
			}
			alive = 0;

			/* Check Protocol */
			if (strcmp (protocol, "HTTP/1.0") && strcmp (protocol, "HTTP/1.1")) {
				/* We don't understand this protocol.  Report a bad response.  */
				if (cnt[0]->conf.control_html_output)
					warningkill = write (client_socket, bad_request_response, sizeof (bad_request_response));
				else
					warningkill = write (client_socket, bad_request_response_raw, sizeof (bad_request_response_raw));

				pthread_mutex_unlock(&httpd_mutex);
				return -1;
			}
			else if (strcmp (method, "GET")) {
				/* This server only implements the GET method.  If client
				uses other method, report the failure.  */
				char response[1024];
				if (cnt[0]->conf.control_html_output)
					snprintf (response, sizeof (response),bad_method_response_template, method);
				else
					snprintf (response, sizeof (response),bad_method_response_template_raw, method);
				warningkill = write (client_socket, response, strlen (response));
				pthread_mutex_unlock(&httpd_mutex);
				return -1;
			} else if ( auth != NULL) {
				if ( (authentication = strstr(buffer,"Basic")) ) {
					char *end_auth = NULL;
					authentication = authentication + 6;

					if ( (end_auth  = strstr(authentication,"\r\n")) ) {
						authentication[end_auth - authentication] = '\0';
					} else {
						char response[1024];
						snprintf (response, sizeof (response),request_auth_response_template, method);
						warningkill = write (client_socket, response, strlen (response));
						pthread_mutex_unlock(&httpd_mutex);
						return -1;
					}

					if ( !check_authentication(auth, authentication,
					                           strlen(cnt[0]->conf.control_authentication),
					                           cnt[0]->conf.control_authentication)) {
						char response[1024]={'\0'};
						snprintf(response, sizeof (response), request_auth_response_template, method);
						warningkill = write (client_socket, response, strlen (response));
						pthread_mutex_unlock(&httpd_mutex);
						return -1;
					} else {
						ret = handle_get (client_socket, url, cnt);
						/* A valid auth request.  Process it.  */
					}
				} else {
					// Request Authorization
					char response[1024]={'\0'};
					snprintf (response, sizeof (response),request_auth_response_template, method);
					warningkill = write (client_socket, response, strlen (response));
					pthread_mutex_unlock(&httpd_mutex);
					return -1;
				}
			} else {
				ret=handle_get (client_socket, url, cnt);
				/* A valid request.  Process it.  */
			}
		}
	}
	pthread_mutex_unlock(&httpd_mutex);

	return ret;
}


/*
   acceptnonblocking
   
   This function waits timeout seconds for listen socket.
   Returns :
   	-1 if the timeout expires or on accept error.
	curfd (client socket) on accept success.
*/

static int acceptnonblocking(int serverfd, int timeout)
{
	int curfd;
	socklen_t namelen = sizeof(struct sockaddr_in);
	struct sockaddr_in client;
	struct timeval tm;
	fd_set fds;

	tm.tv_sec = timeout; /* Timeout in seconds */
	tm.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(serverfd,&fds);
	
	if( select (serverfd + 1, &fds, NULL, NULL, &tm) > 0){
		if(FD_ISSET(serverfd, &fds)) { 
			if((curfd = accept(serverfd, (struct sockaddr*)&client, &namelen))>0)
				return(curfd);
		}	
	}

	return -1;
} 


/*
   Main function: Create the listening socket and waits client requests.
*/

void httpd_run(struct context **cnt)
{
	int sd, client_socket_fd;
	int client_sent_quit_message = 1, val=1;
	int closehttpd = 0;
	struct sockaddr_in servAddr;
	struct sigaction act;
	char *authentication = NULL;

	/* Initialize the mutex */
	pthread_mutex_init(&httpd_mutex, NULL);


	/* set signal handlers TO IGNORE */
	memset(&act,0,sizeof(act));
	sigemptyset(&act.sa_mask);
	act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE,&act,NULL);
	sigaction(SIGCHLD,&act,NULL);

	/* create socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd<0) {
		motion_log(LOG_ERR, 1, "httpd socket");
		return;
	}

	/* bind server port */
	servAddr.sin_family = AF_INET;
	if (cnt[0]->conf.control_localhost)
		servAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(cnt[0]->conf.control_port);

    printf("control_port : %d \r\n", cnt[0]->conf.control_port);

	/* Reuse Address */ 
	
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof( int ) ) ;

	if (bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
		motion_log(LOG_ERR, 1, "httpd bind()");
		close(sd);
		return;
	}

	if (listen(sd,5) == -1){
		motion_log(LOG_ERR, 1, "httpd listen()");
		close(sd);
		return;
	}

	motion_log(LOG_DEBUG, 0, "motion-httpd/"VERSION" running, accepting connections");
	motion_log(LOG_DEBUG, 0, "motion-httpd: waiting for data on port TCP %d", cnt[0]->conf.control_port);

	if (cnt[0]->conf.control_authentication != NULL ) {
		char *userpass = NULL;
		size_t auth_size = strlen(cnt[0]->conf.control_authentication);

		authentication = (char *) mymalloc(BASE64_LENGTH(auth_size) + 1);
		userpass = mymalloc(auth_size + 4);
		/* base64_encode can read 3 bytes after the end of the string, initialize it */
		memset(userpass, 0, auth_size + 4);
		strcpy(userpass, cnt[0]->conf.control_authentication);
		base64_encode(userpass, authentication, auth_size);
		free(userpass);
	}

	while ((client_sent_quit_message!=0) && (!closehttpd)) { 

		client_socket_fd = acceptnonblocking(sd, 1);

		if (client_socket_fd<0) {
			if ((!cnt[0]) || (cnt[0]->finish)){
				motion_log(-1, 1, "httpd - Finishing");
				closehttpd = 1;
			}
		} else {
			/* Get the Client request */
			client_sent_quit_message = read_client (client_socket_fd, cnt, authentication);
			motion_log(-1, 1, "httpd - Read from client");

			/* Close Connection */
			if (client_socket_fd)
				close(client_socket_fd);
		}

	}

	if (authentication != NULL) free(authentication);
	close(sd);
	motion_log(LOG_DEBUG, 0, "httpd Closing");
	pthread_mutex_destroy(&httpd_mutex);
}

void *motion_web_control(void *arg)
{
	struct context **cnt=arg;
    printf("\nmotion_web_control %d\n",getpid());
	httpd_run(cnt);
	pthread_exit(NULL);
}
