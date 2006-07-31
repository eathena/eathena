
#include "baseparam.h"
#include "baseinet.h"
#include "basereentrant.h"

#include "core.h"
#include "socket.h"
#include "timer.h"
#include "showmsg.h"
#include "utils.h"

#include "char.h"
#include "irc.h"




// parameters
basics::CParam< bool             > use_irc("use_irc",false);
basics::CParam< bool             > irc_autojoin("irc_autojoin",false);

basics::CParam< basics::string<> > irc_nick("irc_nick","");
basics::CParam< basics::string<> > irc_pass("irc_pass","");

basics::CParam< basics::string<> > irc_channel("irc_channel","");
basics::CParam< basics::string<> > irc_channel_pass("irc_channel_pass","");
basics::CParam< basics::string<> > irc_trade_channel("irc_trade_channel","");

basics::CParam< basics::string<> > irc_server("irc_server", "0.0.0.0:0");
basics::CParam< unsigned short   > irc_port("irc_port", 6667);


// the irc connection fd
int irc_fd=-1;


// the userlist
basics::smap< basics::string<>, unsigned char> user_data;



void irc_send(const char *buf)
{
	if( session_isActive(irc_fd) && buf)
	{
		size_t len = 1+strlen(buf);
		session_checkbuffer(irc_fd, len);
		memcpy( WFIFOP(irc_fd,0), buf, len);
		WFIFOB(irc_fd,len-1) = 0x0A;
		WFIFOSET(irc_fd,len);
	}
}

void irc_announce(int fd)
{
	if( session_isActive(fd) )
	{
		// get data from packet
		unsigned short len = RFIFOW(fd, 2);
		if(len>5) // 4 byte header + 1 byte terminator
		{
			char* message = (char*)RFIFOP(fd, 4);
			len -= 4;
			message[len-1] = 0; // force a terminator

			char send_string[1024];
			snprintf(send_string, sizeof(send_string), "PRIVMSG %s :%s", (const char*)irc_channel(), message);
			irc_send(send_string);
		}
	}
}

int irc_parse(int fd)
{
	///////////////////////////////////////////////////////////////////////////
	if(fd != irc_fd)
	{
		session_Remove(fd);
		return 0;
	}
	// else it is correct
	if( !session_isActive(fd) )
	{	// is disconnecting
		ShowMessage("Connection to login-server dropped (connection #%d).\n", fd);
		session_Remove(fd);// have it removed by do_sendrecv
		irc_fd = -1;
		return 0;
	}

	if( RFIFOREST(fd)>0 )
	{
		char source[256]="";
		char command[256]="";
		char target[256]="";
		char message[8192]="";
		char send_string[8192]="";
		char *source_nick=NULL;
		char *source_ident=NULL;
		char *source_host=NULL;

		
		size_t len = RFIFOREST(fd);
		const char *ptstart;
		char *ptrun, *ptend;
		ptstart = ptend = ptrun = (char*)RFIFOP(fd,0);
		const char *ptfinish = ptstart+len;


FILE *ll = fopen("irc.log", "a+b");
if(ll)
{
	fwrite(ptstart, len, 1, ll);
	fprintf(ll, "\n\n");
	fclose(ll);
}

		while( ptend<ptfinish )
		{
			// skip empty returns
			ptrun = ptend;
			while(*ptrun=='\r' || *ptrun=='\n') ++ptrun;

			// find the end of the line
			ptend = ptrun;
			while(ptend<ptfinish && *ptend!='\r' && *ptend!='\n') ++ptend;
			// if not propperly terminated with return, therefore this line is incomplete
			if(ptend>=ptfinish) break;

			// otherwise cut the line out
			*ptend++=0;

			// start processing the line
			sscanf(ptrun, ":%255s %255s %255s %8192[^\r\n]", source, command, target, message);

			if( strstr(source,"!") != NULL )
			{
				char *state_mgr=NULL;
				source_nick = strtok_r(source,"!",&state_mgr);
				source_ident = strtok_r(NULL,"@",&state_mgr);
				source_host = strtok_r(NULL,"%%",&state_mgr);
			}


			///////////////////////////////////////////////////////////////////
			if( 0==strcmp(command,"001") )
			{
				ShowStatus("IRC: Connected to IRC.\n");
				snprintf(send_string, sizeof(send_string), "PRIVMSG nickserv :identify %s", (const char *)irc_pass());
				irc_send(send_string);
				snprintf(send_string, sizeof(send_string), "JOIN %s %s", (const char *)irc_channel(), (const char *)irc_channel_pass());
				irc_send(send_string);
				snprintf(send_string, sizeof(send_string), "NAMES %s",(const char *)irc_channel());
				irc_send(send_string);
			}
			///////////////////////////////////////////////////////////////////
			else if( 0==strcmp(command,"433") )
			{
				ShowError("IRC: Nickname %s is already taken, IRC Client unable to connect.\n", (const char *)irc_nick());
				irc_send("QUIT");
				session_Remove(fd);// have it removed by do_sendrecv
				use_irc = 0;
			}
			///////////////////////////////////////////////////////////////////
			else if( 0==strcmp(command,"451") )
			{
				ShowWarning("IRC: Nickname %s not registered.\n", (const char *)irc_nick());
			}
			///////////////////////////////////////////////////////////////////
			else if((strcasecmp(command,"353")==0))
			{	// Names Reply
				ShowInfo("IRC: recieving NAMES\n");

				char *tok;
				char *lpnam=NULL;
				char channel[256]="";
				char names[1024]="";

				sscanf(message, "%*1[=@] %255s :%1023[^\r\n]",channel,names);
				tok=strtok_r(names," ", &lpnam);
				if(tok)
				{
					do
					{
						switch(tok[0])
						{
						case '~':
							user_data[tok+1] = ACCESS_OWNER;
							break;
						case '&':
							user_data[tok+1] = ACCESS_SOP;
							break;
						case '@':
							user_data[tok+1] = ACCESS_OP;
							break;
						case '%':
							user_data[tok+1] = ACCESS_HOP;
							break;
						case '+':
							user_data[tok+1] = ACCESS_VOICE;
							break;
						default:
							user_data[tok] = ACCESS_NORM;
							break;	
						}
					}
					while( (tok=strtok_r(NULL," ", &lpnam))!=NULL );
				}
			}
			///////////////////////////////////////////////////////////////////
			else if( 0==strcasecmp(source, "PING") )
			{
				snprintf(send_string, sizeof(send_string), "PONG %s", command);
				irc_send(send_string);
			}
			///////////////////////////////////////////////////////////////////
			else if( 0==strcasecmp(command,"privmsg") &&
					 (0==strcasecmp(target, (const char*)irc_channel()) || 0==strcasecmp(target,((const char*)irc_channel())+1) ||		// listen to channel
					  0==strcasecmp(target, (const char*)irc_nick())     ) )	// listen to wispers
			{
				char cmdname[256]="";
				char cmdargs[256]="";
				char header[256];
				if( 0==strcasecmp(target, (const char*)irc_nick()) )
				{	// notice to user only
					snprintf(header, sizeof(header), "NOTICE %s", source_nick);
				}
				else
				{	// message to the channel
					snprintf(header, sizeof(header), "PRIVMSG %s", (const char*)irc_channel());
				}

				///////////////////////////////////////////////////////////////
				if( sscanf(message,":@%255s %255[^\r\n]",cmdname,cmdargs)>0 )
				{	
					///////////////////////////////////////////////////////////
					// unpriviliged commands
					if(strcasecmp(cmdname,"users")==0)
					{	// Number of users online [Zido]
						int users=0;
// access char server's online_user_db, which as not been done yet
//						map_getallusers(&users);
						snprintf(send_string, sizeof(send_string), "%s :Users Online: %d", header, users);
						irc_send(send_string);
					// List all users online [Zido]
					}
					///////////////////////////////////////////////////////////
					else if(strcasecmp(cmdname,"who")==0)
					{
						int users=0;
// access char server's online_user_db, which as not been done yet
//						allsd=map_getallusers(&users);
						if(users>0)
						{
							int i=0;
							snprintf(send_string, sizeof(send_string), "%s :%d Users Online", header, users);
							irc_send(send_string);
							for(i=0;i<users;i++)
							{
//								snprintf(send_string, sizeof(send_string), "NOTICE %s :Name: \"%s\"",replyto,allsd[i]->status.name);
								irc_send(send_string);
							}
						}
						else
						{
							snprintf(send_string, sizeof(send_string), "%s :No Users Online",header);
							irc_send(send_string);
						}
					}
					///////////////////////////////////////////////////////////
					else if( user_data.exists(source_nick) && user_data[source_nick] >= ACCESS_OP )
					{
						if( 0==strcasecmp(cmdname,"kami") )
						{
							size_t sz = 1+snprintf(send_string, sizeof(send_string), "%s: %s", source_nick, cmdargs);
							if(sz>1)	// prevent buffer overflow
							{
								unsigned char buffer[1024];
								if(4+sz>sizeof(buffer))
									sz = sizeof(buffer)-4;
								// send irc_announce to map servers
								WBUFW(buffer,0) = 0x2b38;
								WBUFW(buffer,2) = 4+sz;
								safestrcpy((char*)WBUFP(buffer, 4), send_string, sz);
								mapif_sendall(buffer, 4+sz);

								snprintf(send_string, sizeof(send_string), "%s :Message Sent", header);
								irc_send(send_string);
							}
						}
						else if(strcasecmp(cmdname,"quit")==0)
						{	// shut down
							core_stoprunning();
						}
					}
					///////////////////////////////////////////////////////////
					else 
					{
						snprintf(send_string, sizeof(send_string), "%s :Access Denied", header);
					}
					///////////////////////////////////////////////////////////
				}
				///////////////////////////////////////////////////////////////
				// Refresh Names [Zido]
				else if( 0==strcasecmp(command,"JOIN") ||
						 0==strcasecmp(command,"PART") ||
						 0==strcasecmp(command,"MODE") ||
						 0==strcasecmp(command,"NICK") )
				{
// possibly just parse through and only change the changing user...
					ShowInfo("IRC: Refreshing User List\n");
					user_data.clear();
					snprintf(send_string, sizeof(send_string), "NAMES %s", (const char*)irc_channel());
					irc_send(send_string);
				}
				///////////////////////////////////////////////////////////////
				else if( 0==strcasecmp(command,"KICK") && irc_autojoin() )
				{	// Autojoin on kick [Zido]
					snprintf(send_string, sizeof(send_string), "JOIN %s %s", target, (const char*)irc_channel_pass());
					irc_send(send_string);
				}
			}
		}// end while
		RFIFOSKIP(fd, ptrun-ptstart);
	}
	return 0;
}

int irc_connect_timer(int tid, unsigned long tick, int32 id, basics::numptr data)
{
	if(use_irc)
	{
		if( !session_isActive(irc_fd) )
		{	// reconnect
			basics::netaddress addr = irc_server();
			if( addr.port()==0 )
				addr.port() = irc_port(); 

			if( irc_nick()=="" )
			{
				ShowError("no irc user/pass specified. disabling irc bot.\n");
				use_irc = 0;
			}
			else if( addr.port()==0 )
			{
				ShowError("no irc port specified. disabling irc bot.\n");
				use_irc = 0;
			}
			else if( addr == basics::ipnone )
			{
				ShowError("Unable to resolve irc address '%s'. disabling irc bot.\n", (const char*)irc_server());
				use_irc = 0;
			}
			else if( !session_isActive( (irc_fd = make_connection(addr,addr.port())) ) )
			{
				ShowError("Cannot connect to '%s' (%s). disabling irc bot.\n", (const char*)irc_server(), (const char*)addr.tostring());
				use_irc = 0;
			}
			else
			{	// ok

				// prepare the session
				session[irc_fd]->func_parse   = irc_parse;
				session[irc_fd]->rdata_tick = 0;

				// send login
				char buf[1024];
				snprintf(buf, sizeof(buf), "NICK %s", (const char *)irc_nick());
				irc_send(buf);

				// USER command description from IRC RFC 2812.
				// user <name> <mode> * : <real name>
				// the numeric mode parameter is used to automatically set user modes when registering with the server. 
				// It is a bit mask, with bit 2 representing user mode w and bit 3 representing user mode i.
				// so using a value of 8 means you are asking to get set invisible. 
				// Currently, only these two bits are used
				snprintf(buf, sizeof(buf), "USER %s 8 * : %s Bot", (const char *)irc_nick(), server_name);
				irc_send(buf);
			}
		}
	}
	return 0;
}

void irc_final(void)
{
	session_Delete(irc_fd);
	irc_fd = -1;
}

void irc_init(void)
{
	basics::CParamBase::loadFile("conf/irc_athena.conf");

	add_timer_func_list(irc_connect_timer, "irc_connect_timer");
	add_timer_interval(gettick(), 60000, irc_connect_timer, 0, 0);
}

