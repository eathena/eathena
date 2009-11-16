// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _IF_LOGIN_H_
#define _IF_LOGIN_H_

int parse_fromlogin(int fd);
bool loginif_is_connected(void);

void loginif_charserver_login(void);
void loginif_auth_request(int account_id, int login_id1, int login_id2, int sex, int ip, int fd);
void loginif_user_count(int users);
void loginif_request_account_data(int account_id);
void loginif_ping(void);
void loginif_change_email(int account_id, const char* old_email, const char* new_email);
void loginif_account_status(int account_id, int status);
void loginif_account_ban(int account_id, short year, short month, short day, short hour, short minute, short second);
void loginif_account_changesex(int account_id);
void loginif_save_accreg2(unsigned char* buf, int len);
void loginif_account_unban(int account_id);
void loginif_char_online(int account_id);
void loginif_char_offline(int account_id);
void loginif_online_accounts_list(void);
void loginif_request_accreg2(int account_id, int char_id);
void loginif_all_offline(void);

void do_init_loginif(void);
void do_final_loginif(void);

#endif /* _IF_LOGIN_H_ */
