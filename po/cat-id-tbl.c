/* Automatically generated by po2tbl.sed from cronosII.pot.  */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "libgettext.h"

const struct _msg_ent _msg_tbl[] = {
  {"", 1},
  {"Unable to create structure for Cronos II Db: %s\n", 2},
  {"Unable to update structure for Spool Db: %s\n", 3},
  {"Unable to create structure for Spool Db (1): %s\n", 4},
  {"Unable to create structure for Spool Db (2): %s\n", 5},
  {"Success", 6},
  {"Data exception", 7},
  {"No such message", 8},
  {"Process is busy", 9},
  {"Unknown hostname", 10},
  {"Action cancelled by user", 11},
  {"Internal error", 12},
  {"Internal error: Net Object has reached its maximal allocated buffer", 13},
  {"Error connecting to IMAP server.", 14},
  {"Failed to login to IMAP server.", 15},
  {"Error reading from socket on IMAP host %s! Reader thread aborting!\n", 16},
  {"IMAP Server responded with 'BAD': %s\n", 17},
  {"\
A server reply with no tagged ending sent to c2_imap_check_server_reply(): \
%s\n", 18},
  {"This message claims to be multipart, but is broken.", 19},
  {"This message claims to be multipart but it seems to be broken.", 20},
  {"Server does not support APOP", 21},
  {"SMTP server did not respond to our sent messaage in a friendly way", 22},
  {"System Error: Unable to write message to disk for local SMTP command", 23},
  {"Internal C2 Error: Unable to fetch headers in email message", 24},
  {"\
Problem running local SMTP command to send messages -- Check SMTP settings", 25},
  {"Unable to connect to SMTP server", 26},
  {"SMTP server was not friendly on our connect! May not be RFC compliant", 27},
  {"SMTP server did not respond to 'EHLO in a friendly way", 28},
  {"SMTP server did not respond to HELO in a friendly way", 29},
  {"Internal C2 Error: Unable to fetch \"From:\" header in email message", 30},
  {"Internal C2 Error: Unable to fetch \"To:\" header in email message", 31},
  {"SMTP server did not reply to 'MAIL FROM:' in a friendly way", 32},
  {"SMTP server did not reply to 'DATA' in a friendly way", 33},
  {"SMTP server did not reply to 'RCPT TO:' in a friendly way", 34},
  {"Critical Internal Error: Creating Mutex Pipe\n", 35},
  {"Critical Internal Error: Using Mutex Pipe\n", 36},
  {"Attempting to unlock an already unlocked mutex\n", 37},
  {"Critical Internal Error: Writing to Mutex Pipe\n", 38},
  {"Bad syntaxes, quotes aren't closed.\n", 39},
  {"~/.c2/Inbox.mbx/1", 40},
  {"~/.c2/Inbox.mbx/index", 41},
  {"Remove directory %s", 42},
  {"Failed: %s\n", 43},
  {"Failed", 44},
  {"Create directory %s", 45},
  {"Create mailbox �%s�", 46},
  {"Remove file %s", 47},
  {"Copy %s to %s", 48},
  {"Done.", 49},
  {"Failed to load message: %s.", 50},
  {"Failed to save message: %s.", 51},
  {"Failed to load mailbox �%s�: %s.", 52},
  {"Failed to create mailbox �%s�: %s.", 53},
  {"Failed to save file: %s.", 54},
  {"Message saved successfully.", 55},
  {"File saved successfully.", 56},
  {"Action cancelled by user.", 57},
  {"There is no selected mailbox.", 58},
  {"Unknown reason", 59},
  {"Compose a new email.", 60},
  {"Set the Account field.", 61},
  {"Account", 62},
  {"Set the To field.", 63},
  {"Address", 64},
  {"Set the CC field.", 65},
  {"Set the BCC field.", 66},
  {"Set the Subject field.", 67},
  {"Subject", 68},
  {"Set the Body.", 69},
  {"Text", 70},
  {"Compose a new email decoding the argument as a mailto: link", 71},
  {"Set the active mailbox at start (default=Inbox)", 72},
  {"Inbox", 73},
  {"Check account for mail.", 74},
  {"Open the main window (default)", 75},
  {"Unable to find the proper mailbox", 76},
  {"Composer: Untitled", 77},
  {"\
You have choosen to use an external editor instead of the built-in with \
Cronos II.\n\
\n\
Click the button below to run the editor you selected in the Preferences \
dialog.\n\
\n\
Once you finish writting the message in the editor save the file and close \
it.\n\
After that you can manipulate the message using the tools provided in this \
window.\n", 78},
  {"Composer: %s", 79},
  {"Run external editor: %s", 80},
  {"unknown", 81},
  {"The account specified does not longer exist: %s", 82},
  {"\
The message is not asociated to an account, default account will be used.", 83},
  {"Unable to open temporal file for writing to external editor: %s", 84},
  {"On %s, %s wrote:\n", 85},
  {"Failed to open %s: %s", 86},
  {"Failed to stat %s: %s", 87},
  {"The account specified does not exist: %s", 88},
  {"%s (default)", 89},
  {"General", 90},
  {"Interface", 91},
  {"Advanced", 92},
  {"Preferences", 93},
  {"Account Editor", 94},
  {"Post Office Protocol 3 (POP3)", 95},
  {"Internet Message Access Protocol 4 (IMAP)", 96},
  {"SMTP", 97},
  {"Sendmail", 98},
  {"Plain", 99},
  {"APOP", 100},
  {"From", 101},
  {"Date", 102},
  {"Mailbox", 103},
  {"Receive �%s�", 104},
  {"Send �%s�", 105},
  {"Cancel the transfer", 106},
  {"No Inbox mailbox", 107},
  {"Resolving", 108},
  {"Connecting", 109},
  {"Loggining", 110},
  {"Login failed", 111},
  {"Getting mail list", 112},
  {"%u message.", 113},
  {"%u messages.", 114},
  {"Receiving %v of %u", 115},
  {"Synchronizing", 116},
  {"Completed", 117},
  {"Sending", 118},
  {"Stop all", 119},
  {"Close when finished.", 120},
  {"Send & Receive", 121},
  {"Check", 122},
  {"Check for incoming mails", 123},
  {"Send", 124},
  {"Send outgoing mails", 125},
  {"Search", 126},
  {"Search a message in existent mailboxes", 127},
  {"Save", 128},
  {"Save selected message", 129},
  {"Print", 130},
  {"Print selected message", 131},
  {"Delete", 132},
  {"Delete selected mails", 133},
  {"Copy", 134},
  {"Copy selected mails", 135},
  {"Move", 136},
  {"Move selected mails", 137},
  {"Compose", 138},
  {"Compose a new message", 139},
  {"Reply", 140},
  {"Reply selected message", 141},
  {"Reply All", 142},
  {"Reply selected message to all recipients", 143},
  {"Forward", 144},
  {"Forward selected message", 145},
  {"Previous", 146},
  {"Select previous message", 147},
  {"Next", 148},
  {"Select next message", 149},
  {"Contacts", 150},
  {"Open the Contacts Address Book", 151},
  {"Close", 152},
  {"Close the main window of Cronos II", 153},
  {"Exit", 154},
  {"Exit Cronos II", 155},
  {"\
Cronos II was not able to load the UI, you should check your installation.\n\
Aborting.\n", 156},
  {"Copying...", 157},
  {"Deleting...", 158},
  {"Moveing...", 159},
  {"Unable to find message.", 160},
  {"%d messages, %d new.", 161},
  {"You found the Eastern Egg!", 162},
  {"Neat, baby!", 163},
  {"New mailbox", 164},
  {"Toolbar Configuration", 165},
  {"Separator", 166},
  {"\
The account name you\n\
selected is already in use.\n\
\n\
Account names need to be unique.\n\
Please choose another one.", 167},
  {"Error", 168},
  {"Close Dialog", 169},
  {"\
The E-Mail address you\n\
entered is not valid.", 170},
  {"\
The account name you\n\
entered is not valid.", 171},
  {"You have not entered a valid hostname.", 172},
  {"\
This will permanently remove\n\
the account from your configuration.\n\
\n\
Are you sure you want this?", 173},
  {"Confirmation", 174},
  {"Information", 175},
  {"Actions at start", 176},
  {"Check for mail in active accounts.", 177},
  {"Load mailboxes", 178},
  {"Actions at exit", 179},
  {"Expunge mails in �Trash� mailbox.", 180},
  {"Timeouts", 181},
  {"Mark messages as readed after", 182},
  {"seconds.", 183},
  {"Check active accounts for incoming mail every", 184},
  {"minutes.", 185},
  {"Incoming mail", 186},
  {"Let know the user.", 187},
  {"Outgoing mail", 188},
  {"Save copy in �Sent Items� mailbox.", 189},
  {"Deleting mails", 190},
  {"Store deleted mails in �Trash�.", 191},
  {"Ask for confirmation when deleting.", 192},
  {"Archive expunged mails.", 193},
  {"General/Options", 194},
  {"Name", 195},
  {"Type", 196},
  {"Press this button to add a new account to the list.", 197},
  {"_Add", 198},
  {"Press this button to edit the information of the selected account.", 199},
  {"_Edit", 200},
  {"Press this button to remove the selected account of the list.", 201},
  {"_Remove", 202},
  {"Press this button to move up the selected account in the priority list", 203},
  {"_Up", 204},
  {"\
Press this button to move down the selected account in the priority list", 205},
  {"_Down", 206},
  {"General/Accounts", 207},
  {"Paths to prompt to by action", 208},
  {"Save files to disk:", 209},
  {"Get files from disk:", 210},
  {"Use smart path selection.", 211},
  {"General/Paths", 212},
  {"label129", 213},
  {"Press this button to add a plugin to Cronos II.", 214},
  {"Add", 215},
  {"Press this button to remove the selected plugin from Cronos II.", 216},
  {"Remove", 217},
  {"Press this button to configure the selected plugin", 218},
  {"Configure", 219},
  {"Description", 220},
  {"General/Plugins", 221},
  {"Font by occation", 222},
  {"Readed mails:", 223},
  {"Unreaded mails:", 224},
  {"Unreaded mailboxes:", 225},
  {"Font to be used for listing mails that have been opened.", 226},
  {"Select font for readed mais", 227},
  {"Pablo was in Paris.", 228},
  {"Font to be used for listing mails that have not been opened.", 229},
  {"Select font for unreaded mails", 230},
  {"Pablo was in Vienna.", 231},
  {"Font to be used for listing mailbox that have unreaded mails in it.", 232},
  {"Select font for unreaded mailbox", 233},
  {"Pablo was in Rome.", 234},
  {"Message body:", 235},
  {"Composer body:", 236},
  {"Interface/Fonts", 237},
  {"Images from net", 238},
  {"Download.", 239},
  {"Do not download.", 240},
  {"Ask for confirmation before downloading.", 241},
  {"Links", 242},
  {"Open using default Gnome browser.", 243},
  {"Do not open unless is a \"mailto:\" link.", 244},
  {"Ask for confirmation before acting.", 245},
  {"Interpret text/plain symbols.", 246},
  {"Interface/HTML", 247},
  {"Quotes", 248},
  {"Prepend with character", 249},
  {"Highlight with color\t", 250},
  {"Pick a color", 251},
  {"Editor", 252},
  {"Use internal editor.", 253},
  {"Use external editor:", 254},
  {"gnome-edit", 255},
  {"gedit", 256},
  {"gnp", 257},
  {"gvim", 258},
  {"Inteface/Composer", 259},
  {"Title format", 260},
  {"%a v.%v - %M (%m messages, %n new)", 261},
  {"%a v.%v - %M", 262},
  {"%a v.%v", 263},
  {"%M - %a %v", 264},
  {"Date format", 265},
  {"%d.%m.%Y %H:%M %z", 266},
  {"%Y.%m.%d %H:%M %z", 267},
  {"%A %d %B %Y %H:%M", 268},
  {"Attachments", 269},
  {"Display", 270},
  {"text/plain", 271},
  {"text/html", 272},
  {"by default.", 273},
  {"Interface/Misc", 274},
  {"Proxy", 275},
  {"FTP Proxy", 276},
  {"HTTP Proxy", 277},
  {"Port", 278},
  {"Copy values.", 279},
  {"Advanced/Misc", 280},
  {"\
This will permanently remove\n\
the selected account.\n\
Do you want to continue?", 281},
  {"Account Setup", 282},
  {"\
Cronos II supports multiple mail accounts and identities.\n\
\n\
With this tool you will be able to add new accounts\n\
and edit existing accounts.\n\
\n\
Further help is available in the �Help� menu", 283},
  {"Identity", 284},
  {"Required information", 285},
  {"Full Name:", 286},
  {"E-Mail Address:", 287},
  {"\
Name of person or organization which will appear as remitent in outgoing \
messages of this account.", 288},
  {"Optional information", 289},
  {"Organization:", 290},
  {"Reply-To Address:", 291},
  {"\
E-mail address where reply's to mail sent from this account will be sent.", 292},
  {"Submit Personal Card:", 293},
  {"[TODO]", 294},
  {"\
Please, fill in the information regarding the incoming mail.\n\
If you don't know the protocol the server uses you should ask your\n\
system administrator or your ISP.", 295},
  {"Protocol:", 296},
  {"Server information", 297},
  {"Hostname:", 298},
  {"Port:", 299},
  {"Port where Cronos II can access the mail server for your mails", 300},
  {"Mail server where Cronos II will go to fetch your mails", 301},
  {"Use Secure Server Layer (SSL).", 302},
  {"Username:", 303},
  {"Username Cronos II will use to identifies it self at the server", 304},
  {"Authentication", 305},
  {"Authentication method:", 306},
  {"Remember password.", 307},
  {"\
Please, fill in the information regarding the outgoing mail server.\n\
If you don't know the protocol your server uses you should ask your\n\
System Administrator, your ISP or use the Sendmail protocol.", 308},
  {"Server Protocol:", 309},
  {"\
Port where Cronos II can access the Mail Transfer Agent to deliver messages \
from this account", 310},
  {"\
Outgoing mail server where Cronos II will find the Mail Transfer Agent (MTA) \
to deliver messages from this account", 311},
  {"Username Cronos II will use to authenticate it self to the MTA", 312},
  {"Enable this option if your server requires authentication", 313},
  {"This server requires authentication.", 314},
  {"Password", 315},
  {"Options", 316},
  {"Account Information", 317},
  {"Name:", 318},
  {"Unique name that will identify this account", 319},
  {"Signature", 320},
  {"Signature file (plain):", 321},
  {"Signature file (HTML):", 322},
  {"Signature file that will be used when signing messages with no format", 323},
  {"Signature file that will be used when signing messages with HTML format", 324},
  {"Multiple server access options", 325},
  {"\
Enable this if you don't want Cronos II to delete mails in the server after \
fetching them", 326},
  {"Leave messages in the server.", 327},
  {"\
Enable this option if you want Cronos II to delete downloaded messages in \
the server after an specific period of time", 328},
  {"Remove messages older than", 329},
  {"\
Specify the period of time you want Cronos II to keep downloaded messages in \
the server", 330},
  {"days.", 331},
  {"\
Enable this option if you want to include this account in \"All Account\" \
checks", 332},
  {"Automatically check this account.", 333},
  {"Finished", 334},
  {"\
Congratulations, you have successfully configured\n\
your account for use with Cronos II.\n\
\n\
Press �Finish� to store the new configuration.", 335},
  {"\
The name you entered is\n\
not valid.", 336},
  {"Cronos II", 337},
  {"Compose a new E-Mail", 338},
  {"_E-Mail", 339},
  {"Create a new mailbox", 340},
  {"M_ailbox", 341},
  {"Add a new contact to your address book", 342},
  {"_Contact", 343},
  {"_Check Mail", 344},
  {"Check all accounts for new mail", 345},
  {"All Accounts", 346},
  {"_Send unsent mails", 347},
  {"Open the Address Book", 348},
  {"_Address Book", 349},
  {"_Save", 350},
  {"_Print", 351},
  {"mmmm, I smell a nice egg coming from the east...", 352},
  {"_From", 353},
  {"_Account", 354},
  {"_To", 355},
  {"_Carbon Copy", 356},
  {"_Blind Carbon Copy", 357},
  {"_Date", 358},
  {"_Subject", 359},
  {"_Priority", 360},
  {"_Mailbox", 361},
  {"_Speed Up", 362},
  {"_Import", 363},
  {"_Export", 364},
  {"Empty _Garbage", 365},
  {"Mess_age", 366},
  {"_Reply", 367},
  {"Reply _all", 368},
  {"_Forward", 369},
  {"Copy selected messages", 370},
  {"_Copy", 371},
  {"Move selected messages", 372},
  {"_Move", 373},
  {"Delete selected messages", 374},
  {"_Delete", 375},
  {"Expunge selected messages", 376},
  {"_Expunge", 377},
  {"Mar_k", 378},
  {"Mark selected message as important", 379},
  {"_Important", 380},
  {"Mark selected messages as unreaded", 381},
  {"_Unread", 382},
  {"Mark selected messages as readed", 383},
  {"_Read", 384},
  {"Mark selected messages as replied", 385},
  {"Repl_ied", 386},
  {"Mark selected messages as forwarded", 387},
  {"_Previous", 388},
  {"_Next", 389},
  {"Try to recover an expunged message", 390},
  {"Reco_very", 391},
  {"_User Manual", 392},
  {"_Quick Tips", 393},
  {"_Getting in Touch", 394},
  {"_Release Information", 395},
  {"About _Plugins", 396},
  {"Icon Menu Bar", 397},
  {"View Message Pane", 398},
  {"Show the Message Transfer dialog", 399},
  {"\
The mailbox name you\n\
entered is invalid.", 400},
  {"close this dialog", 401},
  {"Mailbox properties", 402},
  {"finished", 403},
  {"Warning", 404},
  {"\
Editing mailbox properties might\n\
result in the lost of all mails in it.", 405},
  {"Type here the name of the new mailbox", 406},
  {"Type:", 407},
  {"Select the type of access you want for this mailbox", 408},
  {"Extra data", 409},
  {"Type here the port where this mailbox is stored", 410},
  {"Type here the hostname where this mailbox is stored", 411},
  {"Password:", 412},
  {"Path:", 413},
  {"Select file", 414},
  {"C2 Addressbook", 415},
  {"Cronos II Addressbook", 416},
  {"_New File", 417},
  {"Open Addressbook file", 418},
  {"close addressbook", 419},
  {"New File", 420},
  {"New", 421},
  {"Open Address Book File", 422},
  {"Open", 423},
  {"Save Address Book File", 424},
  {"Search:", 425},
  {"Groups:", 426},
  {"E-Mail", 427},
  {"Comments", 428},
  {"Address Book vCard", 429},
  {"close card", 430},
  {"_New Mailbox", 431},
  {"Delete selected mailbox", 432},
  {"_Delete Mailbox", 433},
  {"\
Removal of a mailbox will permanently\n\
delete all mail stored in it and any\n\
mailbox depending of it.\n\
\n\
Are you sure you want to delete\n\
this mailbox?", 434},
  {"Remove this Mailbox", 435},
  {"Don't do anything. Close this dialog.", 436},
  {"Make a backup copy", 437},
  {"Archive mails (recommended).", 438},
  {"Incorrect Password", 439},
  {"Try this new information", 440},
  {"Account:", 441},
  {"\
The server has rejected the username/password\n\
combination: ", 442},
  {"\
\n\
Provide the correct information for logging in\n\
in the following fields.", 443},
  {"Getting in Touch", 444},
  {"Close this dialog", 445},
  {"\
Getting in Touch:\n\
\n\
There are several ways to get in touch with anyone\n\
from the Cronos II community (users or developers).\n\
\n\
Note that if you want to contact a particular developer for\n\
a special bug-report (or for whatever reason) you should\n\
check the Cronos II homepage for a list of contacts at\n\
_h_t_t_p_:_/_/_c_r_o_n_o_s_I_I_._s_o_u_r_c_e_f_o_r_g_e_._n_e_t_/\n\
\n\
_E_-_M_a_i_l:\n\
\n\
To send a mail to the developers you should write to\n\
our mailing list: \
_c_r_o_n_o_s_I_I_-_d_e_v_e_l_o_p_e_r_@_l_i_s_t_s_._s_o_u_r_c_e_f_o_r_g_e_._n_\
e_t.\n\
\n\
To contact other users of Cronos II send mail\n\
to the user's mailing list: \
_c_r_o_n_o_s_I_I_-_u_s_e_r_s_@_l_i_s_t_s_._s_o_u_r_c_e_f_o_r_g_e_._n_e_t.\n\
\n\
_I_R_C:\n\
\n\
You can chat with both users and developers in the\n\
OpenProjects network by connecting to the server\n\
_i_r_c_._g_n_o_m_e_._o_r_g_:_6_6_6_7 and the channel _#_c_r_o_n_o_s_I_I.", 446},
  {"\
There is no account configured yet.\n\
You need to do this before being\n\
able to compose mails.", 447},
  {"\
Not enough data was filled in,\n\
complete the missing data\n\
and try again.", 448},
  {"Text _beside icons", 449},
  {"Text _under icons", 450},
  {"_Text only", 451},
  {"_Icons only", 452},
  {"_Tooltips", 453},
  {"_Edit toolbar...", 454},
  {"Toolbar configuration", 455},
  {"Available elements", 456},
  {"label73", 457},
  {"label74", 458},
  {"Current Toolbar", 459},
  {"label75", 460},
  {"label76", 461},
  {"\
There is a file by that name already.\n\
\n\
Do you want to overwrite it?", 462},
  {"\
Are you sure you want to\n\
delete the selected mails?", 463},
  {"Do not ask again.", 464},
  {"\
Are you sure you want to\n\
expunge the selected mails?", 465},
  {"Release Information", 466},
  {"Welcome", 467},
  {"Known Bugs", 468},
  {"Missing Features", 469},
  {"Show this dialog at start.", 470},
  {"Select Mailbox", 471},
  {"You selected a directory.", 472},
  {"About", 473},
  {"Web Site", 474},
};

int _msg_tbl_length = 474;
