# eAthena

### About

**eAthena** - is MMORPG emulator written and actively developed from ~2004 up to ~2011.
Since around 2011 year after some drama and not only, whole old community (https://eathena.ws) died,
and split to few modern and actual up to date emulators:
- [hercules](https://github.com/HerculesWS/Hercules)
- [rAthena](https://github.com/rathena/rathena)
- [3CeAM](https://sourceforge.net/projects/v1-3ceam/)

This project almost dead, and not supported anymore.
Strongly recommend you to take a look for projects linked  above.

The repository good source for different information and tracking history changes.
Also, optionally in 22 November 2018 the branch has been updated, and now support VS 2017 Community.

The differences between emulators above not too big, almost all of them interconnected in different systems or features.
But use different codebase because of split in 2011 and continuing developed what you can track on their official forums.

## How to try?

### Linux (debian based distros (ubuntu, etc)

#### 1. You need install requirements:

> apt-get install git make gcc mysql-server libmysqlclient-dev zlib1g-dev libpcre3-dev screen

**Where:**
- git - a tool for cloning/pushing, etc whatever to work with repository and tracking changes
- make and gcc - for compilation
- MySQL-server (required 5.6 version, not 5.7) - where will be stored database
- libmysqlclient-dev - required for the emulator to work with database
- libpcre3-dev - required for pcre support in script engine and not only
- screen - a tool for simply running the emulator in the background on the server and returning back to console every time when it needed.

**Note:** MySQL >= 5.7 not supported.
Please consider to use MySQL-Server <= 5.6

#### 2. You need to create a non-root user under what will be running the emulator

> adduser eathena

Enter pass and make sure nobody except you know it.

#### 3. Clone and configure before compilation

> git clone https://github.com/anacondaqq/eathena.git

Then you will see `/conf/` folder, almost all settings there.

Files what required to edit to make emulator works:

- map_athena.conf

```
userid: s1 <--- change to your username (what will be used in login table (userid) on account with ID = 1)
passwd: p1 <--- change to your username (what will be used in login table (user_pass) on account with ID = 1)

map_ip: 127.0.0.1 - change to your public IP address (or non-touch if you run it on local pc)
```

- char_athena.conf

```
userid: s1 <--- change to your username (what will be used in login table (userid) on account with ID = 1)
passwd: p1 <--- change to your username (what will be used in login table (user_pass) on account with ID = 1)

char_ip: 127.0.0.1 - change to your public IP address (or non-touch if you run it on local pc)
```

- inter_athena.conf - where you will set user/pass to connect your databases on MySQL server

```
sql.db_hostname: 127.0.0.1
sql.db_port: 3306
sql.db_username: change_to_main_mysql_db_user
sql.db_password: change_to_main_mysql_db_pass
sql.db_database: name_of_main_db
sql.codepage:

// MySQL Character SQL server
char_server_ip: 127.0.0.1
char_server_port: 3306
char_server_id: change_to_main_mysql_db_user
char_server_pw: change_to_main_mysql_db_pass
char_server_db: name_of_main_db

// MySQL Map SQL Server
map_server_ip: 127.0.0.1
map_server_port: 3306
map_server_id: change_to_main_mysql_db_user
map_server_pw: change_to_main_mysql_db_pass
map_server_db: name_of_main_db

// MySQL Log SQL Database
log_db_ip: 127.0.0.1
log_db_port: 3306
log_db_id: change_to_log_mysql_db_user
log_db_pw: change_to_log_mysql_db_pass
log_db_db: name_of_log_db
log_codepage:
```

#### 4. MySQL configuration

Required: MySQL <= 5.6 only, current emulator not work fine on MySQL >= 5.7

**Create a MySQL database and users:**

1. Login to your mysql-server `mysql -uroot -p<your password>`
2. Create two databases, one for main game db, second where will be stored emulator logs
3. Main db: > create database `name_of_main_db`; # please replace `name_of_main_db` to your own, example: myserver_db
4. Logs db: > create database `name_of_log_db`; # please replace `name_of_log_db` to your own, example: myserver_log
5. Now, need to create two separate users with some privilegies for both of created databases:
6. Main DB User: > `GRANT create, select, insert, delete, alter, update, drop ON name_of_main_db.* TO 'change_to_main_mysql_db_user'@'localhost' IDENTIFIED BY 'change_to_main_mysql_db_pass';` # please replace values here to correct one what you will use (and in config above too)
7. Main LOG User: > `GRANT create, select, insert, delete, alter, update, drop ON name_of_log_db.* TO 'change_to_log_mysql_db_user'@'localhost' IDENTIFIED BY 'change_to_log_mysql_db_pass';` # please replace values here to correct one what you will use (and in config above too)

**Import SQL Tables**

1. mysql -u root -p<replace_to_your_pass> name_of_main_db < sql/main.sql
2. mysql -u root -p<replace_to_your_pass> name_of_main_log < sql/logs.sql

Now, you need to edit a login table and change `s1` `p1` values there to yours in configs.
For example i have in `char_athena.conf` and `map_athena.conf` next data:

```
userid: thisismyuser
passwd: mycoolpassword
```

So, I need to do next than:

1. Login to your mysql console (`mysql -uroot -p<yourpass>`)
2. Use proper database: > `use name_of_main_db;`
3. Display current database structure: > `describe login;`
4. Display current values: > `select * FROM 'login' \G;`
5. Replace `s1`, `p1` on your main administrative account (for server purposes): > `update login set userid='thisismyuser', user_pass='mycoolpassword' WHERE account_id='1';`
6. Now mysql configuration is complete, and you can able to compile emulator and connect (see below)


#### 5. Compilation

```
1. cd eathena
2. chmod +x configure
3. ./configure
4. make sql
```

Make all created files executable:
```
chmod +x *_sql
chmdo +x run-server.sh
```

#### 6. Launching

1. Go to your emulator folder with `cd` (cd path/to/your/emulator)
2. Now enter: `screen -S loginserv` hit enter, and you will be under screen session (to which you can always return back easily)
3. Now enter `./login-server_sql` and press enter
4. Hit CTRL + A, then press just D key without holding anything (yes, two combinations) at this point you will send the screen to detach state, and you will be able to launch another emulator windows
5. Now `screen -S charserver`, hit enter
6. `./char-server_sql` press enter, then CTRL+A; and then D for detaching
7. Now last map-server: `screen -S mapserver`, hit enter
8. `./map-server_sql` press enter, then CTRL+A; and then D for detaching

__That it, now the whole emulator must be running fine.__
**Note:** If it running without any errors, you will not see any WARNINGS or ERRORS messages in screen consoles.

**How access each of the server? Super easy:**

1. `screen -ls` - will display available screens in the detached state to which you can switch
2. `screen -X <name_of_screen>`, for example, if you wish to see map-server `screen -S mapserver`
3. If you will press CTRL + A and will use Arrow Up / Down you able to scroll up or down.
4. If you wish to cancel the focus to another server, you need send screen to detach mod, hit `CTRL+A,  then press D`

**How restart server?**
1. Log in to each screen, and press CTRL+C
2. Repeat Launching steps.

Optionally you can use athena_start.sh (personally me, don't like it, too buggy)

#### 7. How connect from client to the server?

For this question, you will not receive an answer here.
Use or herc.ws, or rathena.org, boards.

### Windows (7, 10, server, etc)

1. (Download Visual Studio Community 2017)[https://visualstudio.microsoft.com/vs/community/] or whatever is actual when you reading the information.
2. In installer select C++ (around 6.66GB to download and install for VS 2017) and install
3. When you will finish instalation, just launch eathena-17.sln
4. CTRL + SHIFT + B, or `Build -> Build`.
5. If you see any of errors in output, right click on the whole solution -> Retarget Solution, OK, and try again.
6. What about mysql part, configuration, etc, it's the same, so find any portable or like that mysql server and do almost everything above except adding a user.

That it.
