/* Automatically generated by po2tbl.sed from cronosII.pot.  */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "libgettext.h"

const struct _msg_ent _msg_tbl[] = {
  {"", 1},
  {"Unable to create structure for Cronos II Db: %s\n", 2},
  {"Success", 3},
  {"Data exception", 4},
  {"No such message", 5},
  {"Process is busy", 6},
  {"Unknown hostname", 7},
  {"Action cancelled by user", 8},
  {"Internal error", 9},
  {"Internal error: Net Object has reached its maximal allocated buffer", 10},
  {"Internal socket read operation failed, connection is most likely broken", 11},
  {"\
Internal socket write operation failed, connection is most likely broken", 12},
  {"Error connecting to IMAP server.", 13},
  {"Error reading from socket on IMAP host %s! Reader thread aborting!\n", 14},
  {"IMAP Server responded with 'BAD': %s\n", 15},
  {"\
A server reply with no tagged ending sent to c2_imap_check_server_reply(): %\
s\n", 16},
  {"That IMAP mailbox is non-selectable", 17},
  {"\
Invalid IMAP mailbox. Try re-starting CronosII and resycning your IMAP \
mailbox tree.", 18},
  {"Invalid SELECT command status returned by IMAP server", 19},
  {"This message claims to be multipart, but is broken.", 20},
  {"This message claims to be multipart but it seems to be broken.", 21},
  {"Server does not support APOP", 22},
  {"Internal socket read operation failed", 23},
  {"Internal socket write operation failed", 24},
  {"SMTP server did not respond to our sent messaage in a friendly way", 25},
  {"System Error: Unable to write message to disk for local SMTP command", 26},
  {"Internal C2 Error: Unable to fetch headers in email message", 27},
  {"\
Problem running local SMTP command to send messages -- Check SMTP settings", 28},
  {"Unable to connect to SMTP server", 29},
  {"SMTP server was not friendly on our connect! May not be RFC compliant", 30},
  {"SMTP server did not respond to 'EHLO in a friendly way", 31},
  {"SMTP server did not respond to HELO in a friendly way", 32},
  {"Internal C2 Error: Unable to fetch \"From:\" header in email message", 33},
  {"Internal C2 Error: Unable to fetch \"To:\" header in email message", 34},
  {"SMTP server did not reply to 'MAIL FROM:' in a friendly way", 35},
  {"SMTP server did not reply to 'DATA' in a friendly way", 36},
  {"SMTP server did not reply to 'RCPT TO:' in a friendly way", 37},
  {"Attempting to lock/unlock a mutex which has already been destroyed", 38},
  {"Critical Internal Error: Creating Mutex Pipe\n", 39},
  {"Critical Internal Error: Using Mutex Pipe\n", 40},
  {"Attempting to unlock an already unlocked mutex\n", 41},
  {"Critical Internal Error: Writing to Mutex Pipe\n", 42},
  {"Bad syntaxes, quotes aren't closed.\n", 43},
  {"~/.c2/Inbox.mbx/1", 44},
  {"~/.c2/Inbox.mbx/index", 45},
  {"Remove directory %s", 46},
  {"Failed: %s\n", 47},
  {"Failed", 48},
  {"Create directory %s", 49},
  {"Create mailbox �%s�", 50},
  {"Remove file %s", 51},
  {"Copy %s to %s", 52},
  {"Done.", 53},
  {"Failed to load message: %s.", 54},
  {"Failed to save message: %s.", 55},
  {"Failed to load mailbox �%s�: %s.", 56},
  {"Failed to create mailbox �%s�: %s.", 57},
  {"Failed to save file: %s.", 58},
  {"Message saved successfully.", 59},
  {"File saved successfully.", 60},
  {"Action cancelled by user.", 61},
  {"There is no selected mailbox.", 62},
  {"Unknown reason", 63},
  {"Compose a new email.", 64},
  {"Set the Account field.", 65},
  {"Account", 66},
  {"Set the To field.", 67},
  {"Address", 68},
  {"Set the CC field.", 69},
  {"Set the BCC field.", 70},
  {"Set the Subject field.", 71},
  {"Subject", 72},
  {"Set the Body.", 73},
  {"Text", 74},
  {"Compose a new email decoding the argument as a mailto: link", 75},
  {"Set the active mailbox at start (default=Inbox)", 76},
  {"Inbox", 77},
  {"Check account for mail.", 78},
  {"Open the main window (default)", 79},
  {"\
Don't open any windows; instead act as a server for quick startup of new \
Cronos II instances", 80},
  {"The mailbox \"%s\", specified in command line, does not exist.", 81},
  {"Select a message", 82},
  {"Unable to find the proper mailbox", 83},
  {"Composer: Untitled", 84},
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
window.\n", 85},
  {"Composer: %s", 86},
  {"Run external editor: %s", 87},
  {"unknown", 88},
  {"The account specified does not longer exist: %s", 89},
  {"\
The message is not asociated to an account, default account will be used.", 90},
  {"Unable to open temporal file for writing to external editor: %s", 91},
  {"On %s, %s wrote:\n", 92},
  {"Failed to open %s: %s", 93},
  {"Failed to stat %s: %s", 94},
  {"The account specified does not exist: %s", 95},
  {"%s (default)", 96},
  {"General", 97},
  {"Interface", 98},
  {"Advanced", 99},
  {"Preferences", 100},
  {"Account Editor", 101},
  {"Post Office Protocol 3 (POP3)", 102},
  {"Internet Message Access Protocol 4 (IMAP)", 103},
  {"SMTP", 104},
  {"Sendmail", 105},
  {"Plain", 106},
  {"APOP", 107},
  {"From", 108},
  {"Date", 109},
  {"Mailboxes", 110},
  {"Receive �%s�", 111},
  {"Send �%s�", 112},
  {"Cancel the transfer", 113},
  {"No Inbox mailbox", 114},
  {"Resolving", 115},
  {"Connecting", 116},
  {"Loggining", 117},
  {"Login failed", 118},
  {"Getting mail list", 119},
  {"%u message.", 120},
  {"%u messages.", 121},
  {"Receiving %v of %u", 122},
  {"Synchronizing", 123},
  {"Completed", 124},
  {"Sending", 125},
  {"Stop all", 126},
  {"Close when finished.", 127},
  {"Send & Receive", 128},
  {"Spool (local)", 129},
  {"Check", 130},
  {"Check for incoming mails", 131},
  {"Send", 132},
  {"Send outgoing mails", 133},
  {"Search", 134},
  {"Search a message in existent mailboxes", 135},
  {"Save", 136},
  {"Save selected message", 137},
  {"Print", 138},
  {"Print selected message", 139},
  {"Delete", 140},
  {"Delete selected mails", 141},
  {"Copy", 142},
  {"Copy selected mails", 143},
  {"Move", 144},
  {"Move selected mails", 145},
  {"Compose", 146},
  {"Compose a new message", 147},
  {"Reply", 148},
  {"Reply selected message", 149},
  {"Reply All", 150},
  {"Reply selected message to all recipients", 151},
  {"Forward", 152},
  {"Forward selected message", 153},
  {"Previous", 154},
  {"Select previous message", 155},
  {"Next", 156},
  {"Select next message", 157},
  {"Contacts", 158},
  {"Open the Contacts Address Book", 159},
  {"Close", 160},
  {"Close the main window of Cronos II", 161},
  {"Exit", 162},
  {"Close all windows of Cronos II", 163},
  {"\
Cronos II was not able to load the UI, you should check your installation.\n\
Aborting.\n", 164},
  {"Deleting...", 165},
  {"%d messages, %d new.", 166},
  {"You found the Eastern Egg!", 167},
  {"Neat, baby!", 168},
  {"New mailbox", 169},
  {"Separator", 170},
  {"Toolbar Configuration", 171},
  {"\
The account name you\n\
selected is already in use.\n\
\n\
Account names need to be unique.\n\
Please choose another one.", 172},
  {"Error", 173},
  {"Close Dialog", 174},
  {"\
The E-Mail address you\n\
entered is not valid.", 175},
  {"\
The account name you\n\
entered is not valid.", 176},
  {"You have not entered a valid hostname.", 177},
  {"\
This will permanently remove\n\
the account from your configuration.\n\
\n\
Are you sure you want this?", 178},
  {"Confirmation", 179},
  {"Information", 180},
  {"Actions at start", 181},
  {"Check for mail in active accounts.", 182},
  {"Load mailboxes", 183},
  {"Actions at exit", 184},
  {"Expunge mails in �Trash� mailbox.", 185},
  {"Timeouts", 186},
  {"Mark messages as readed after", 187},
  {"seconds.", 188},
  {"Check active accounts for incoming mail every", 189},
  {"minutes.", 190},
  {"Incoming mail", 191},
  {"Let know the user.", 192},
  {"Outgoing mail", 193},
  {"Save copy in �Sent Items� mailbox.", 194},
  {"Deleting mails", 195},
  {"Store deleted mails in �Trash�.", 196},
  {"Ask for confirmation when deleting.", 197},
  {"Archive expunged mails.", 198},
  {"General/Options", 199},
  {"Name", 200},
  {"Type", 201},
  {"Press this button to add a new account to the list.", 202},
  {"_Add", 203},
  {"Press this button to edit the information of the selected account.", 204},
  {"_Edit", 205},
  {"Press this button to remove the selected account of the list.", 206},
  {"_Remove", 207},
  {"Press this button to move up the selected account in the priority list", 208},
  {"_Up", 209},
  {"\
Press this button to move down the selected account in the priority list", 210},
  {"_Down", 211},
  {"General/Accounts", 212},
  {"Paths to prompt to by action", 213},
  {"Save files to disk:", 214},
  {"Get files from disk:", 215},
  {"Use smart path selection.", 216},
  {"General/Paths", 217},
  {"label129", 218},
  {"Press this button to add a plugin to Cronos II.", 219},
  {"Add", 220},
  {"Press this button to remove the selected plugin from Cronos II.", 221},
  {"Remove", 222},
  {"Press this button to configure the selected plugin", 223},
  {"Configure", 224},
  {"Description", 225},
  {"General/Plugins", 226},
  {"Font by occation", 227},
  {"Readed mails:", 228},
  {"Unreaded mails:", 229},
  {"Unreaded mailboxes:", 230},
  {"Font to be used for listing mails that have been opened.", 231},
  {"Select font for readed mais", 232},
  {"Pablo was in Paris.", 233},
  {"Font to be used for listing mails that have not been opened.", 234},
  {"Select font for unreaded mails", 235},
  {"Pablo was in Vienna.", 236},
  {"Font to be used for listing mailbox that have unreaded mails in it.", 237},
  {"Select font for unreaded mailbox", 238},
  {"Pablo was in Rome.", 239},
  {"Message body:", 240},
  {"Composer body:", 241},
  {"Interface/Fonts", 242},
  {"Images from net", 243},
  {"Download.", 244},
  {"Do not download.", 245},
  {"Ask for confirmation before downloading.", 246},
  {"Links", 247},
  {"Open using default Gnome browser.", 248},
  {"Do not open unless is a \"mailto:\" link.", 249},
  {"Ask for confirmation before acting.", 250},
  {"Interpret text/plain symbols.", 251},
  {"Interface/HTML", 252},
  {"Quotes", 253},
  {"Prepend with character", 254},
  {"Highlight with color\t", 255},
  {"Pick a color", 256},
  {"Editor", 257},
  {"Use internal editor.", 258},
  {"Use external editor:", 259},
  {"gnome-edit", 260},
  {"gedit", 261},
  {"gnp", 262},
  {"gvim", 263},
  {"Inteface/Composer", 264},
  {"Title format", 265},
  {"%a v.%v - %M (%m messages, %n new)", 266},
  {"%a v.%v - %M", 267},
  {"%a v.%v", 268},
  {"%M - %a %v", 269},
  {"Date format", 270},
  {"%d.%m.%Y %H:%M %z", 271},
  {"%Y.%m.%d %H:%M %z", 272},
  {"%A %d %B %Y %H:%M", 273},
  {"Attachments", 274},
  {"Display", 275},
  {"text/plain", 276},
  {"text/html", 277},
  {"by default.", 278},
  {"Interface/Misc", 279},
  {"Proxy", 280},
  {"FTP Proxy", 281},
  {"HTTP Proxy", 282},
  {"Port", 283},
  {"Copy values.", 284},
  {"Advanced/Misc", 285},
  {"\
This will permanently remove\n\
the selected account.\n\
Do you want to continue?", 286},
  {"Account Setup", 287},
  {"\
Cronos II supports multiple mail accounts and identities.\n\
\n\
With this tool you will be able to add new accounts\n\
and edit existing accounts.\n\
\n\
Further help is available in the �Help� menu", 288},
  {"Identity", 289},
  {"Required information", 290},
  {"Full Name:", 291},
  {"E-Mail Address:", 292},
  {"\
Name of person or organization which will appear as remitent in outgoing \
messages of this account.", 293},
  {"Optional information", 294},
  {"Organization:", 295},
  {"Reply-To Address:", 296},
  {"\
E-mail address where reply's to mail sent from this account will be sent.", 297},
  {"Submit Personal Card:", 298},
  {"[TODO]", 299},
  {"\
Please, fill in the information regarding the incoming mail.\n\
If you don't know the protocol the server uses you should ask your\n\
system administrator or your ISP.", 300},
  {"Protocol:", 301},
  {"Server information", 302},
  {"Hostname:", 303},
  {"Port:", 304},
  {"Port where Cronos II can access the mail server for your mails", 305},
  {"Mail server where Cronos II will go to fetch your mails", 306},
  {"Username:", 307},
  {"Username Cronos II will use to identifies it self at the server", 308},
  {"Use Secure Server Layer (SSL).", 309},
  {"Password:", 310},
  {"Authentication", 311},
  {"Authentication method:", 312},
  {"Remember password.", 313},
  {"\
Please, fill in the information regarding the outgoing mail server.\n\
If you don't know the protocol your server uses you should ask your\n\
System Administrator, your ISP or use the Sendmail protocol.", 314},
  {"Server Protocol:", 315},
  {"\
Port where Cronos II can access the Mail Transfer Agent to deliver messages \
from this account", 316},
  {"\
Outgoing mail server where Cronos II will find the Mail Transfer Agent (MTA) \
to deliver messages from this account", 317},
  {"Username Cronos II will use to authenticate it self to the MTA", 318},
  {"Enable this option if your server requires authentication", 319},
  {"This server requires authentication.", 320},
  {"Password", 321},
  {"Options", 322},
  {"Account Information", 323},
  {"Name:", 324},
  {"Unique name that will identify this account", 325},
  {"Signature", 326},
  {"Signature file (plain):", 327},
  {"Signature file (HTML):", 328},
  {"Signature file that will be used when signing messages with no format", 329},
  {"Signature file that will be used when signing messages with HTML format", 330},
  {"Multiple server access options", 331},
  {"\
Enable this if you don't want Cronos II to delete mails in the server after \
fetching them", 332},
  {"Leave messages in the server.", 333},
  {"\
Enable this option if you want Cronos II to delete downloaded messages in \
the server after an specific period of time", 334},
  {"Remove messages older than", 335},
  {"\
Specify the period of time you want Cronos II to keep downloaded messages in \
the server", 336},
  {"days.", 337},
  {"\
Enable this option if you want to include this account in \"All Account\" \
checks", 338},
  {"Automatically check this account.", 339},
  {"Finished", 340},
  {"\
Congratulations, you have successfully configured\n\
your account for use with Cronos II.\n\
\n\
Press �Finish� to store the new configuration.", 341},
  {"\
The name you entered is\n\
not valid.", 342},
  {"Cronos II", 343},
  {"Compose a new E-Mail", 344},
  {"_E-Mail", 345},
  {"Create a new mailbox", 346},
  {"M_ailbox...", 347},
  {"Add a new contact to your address book", 348},
  {"_Contact...", 349},
  {"Open a new main window", 350},
  {"_Window", 351},
  {"_Check Mail", 352},
  {"Check all accounts for new mail", 353},
  {"All Accounts", 354},
  {"_Send unsent mails", 355},
  {"Open the Address Book", 356},
  {"_Address Book", 357},
  {"_Save...", 358},
  {"_Print...", 359},
  {"mmmm, I smell a nice egg coming from the east...", 360},
  {"_Toolbar", 361},
  {"_Mailboxes List", 362},
  {"Mail _Preview", 363},
  {"_Headers", 364},
  {"_Network Traffic", 365},
  {"Mail _Source", 366},
  {"Network _Traffic", 367},
  {"_Mailbox", 368},
  {"_Speed Up...", 369},
  {"_Import...", 370},
  {"_Export...", 371},
  {"Empty _Garbage", 372},
  {"Mess_age", 373},
  {"_Reply", 374},
  {"Reply to _all", 375},
  {"_Forward", 376},
  {"Copy selected messages", 377},
  {"_Copy...", 378},
  {"Move selected messages", 379},
  {"_Move...", 380},
  {"Delete selected messages", 381},
  {"_Delete", 382},
  {"Expunge selected messages", 383},
  {"_Expunge", 384},
  {"Mar_k", 385},
  {"Mark selected message as important", 386},
  {"_Important", 387},
  {"Mark selected messages as unreaded", 388},
  {"_Unreaded", 389},
  {"Mark selected messages as readed", 390},
  {"_Readed", 391},
  {"Mark selected messages as replied", 392},
  {"Repl_ied", 393},
  {"Mark selected messages as forwarded", 394},
  {"_Forwarded", 395},
  {"_Previous", 396},
  {"_Next", 397},
  {"Try to recover an expunged message", 398},
  {"Reco_ver", 399},
  {"F_ilters", 400},
  {"_Rule Manager...", 401},
  {"_Apply rule to selected mailbox", 402},
  {"_Features", 403},
  {"_Download new features from Internet...", 404},
  {"_User Manual", 405},
  {"_Quick Tips...", 406},
  {"_Getting in Touch...", 407},
  {"_Release Information...", 408},
  {"About _Plugins...", 409},
  {"_Filters:", 410},
  {"To", 411},
  {"Body", 412},
  {"contains", 413},
  {"is equal to", 414},
  {"Cas_e sensitive", 415},
  {"Show the Message Transfer dialog", 416},
  {"\
The mailbox name you\n\
entered is invalid.", 417},
  {"close this dialog", 418},
  {"Mailbox properties", 419},
  {"finished", 420},
  {"Warning", 421},
  {"\
Editing mailbox properties might\n\
result in the lost of all mails in it.", 422},
  {"Type here the name of the new mailbox", 423},
  {"Type:", 424},
  {"Select the type of access you want for this mailbox", 425},
  {"Extra data", 426},
  {"Type here the port where this mailbox is stored", 427},
  {"Type here the hostname where this mailbox is stored", 428},
  {"Path:", 429},
  {"Select file", 430},
  {"C2 Addressbook", 431},
  {"Cronos II Addressbook", 432},
  {"_New File", 433},
  {"Open Addressbook file", 434},
  {"close addressbook", 435},
  {"New File", 436},
  {"New", 437},
  {"Open Address Book File", 438},
  {"Open", 439},
  {"Save Address Book File", 440},
  {"Search:", 441},
  {"Groups:", 442},
  {"E-Mail", 443},
  {"Comments", 444},
  {"Address Book vCard", 445},
  {"close card", 446},
  {"_New Mailbox", 447},
  {"Delete selected mailbox", 448},
  {"_Delete Mailbox", 449},
  {"\
Removal of a mailbox will permanently\n\
delete all mail stored in it and any\n\
mailbox depending of it.\n\
\n\
Are you sure you want to delete\n\
this mailbox?", 450},
  {"Remove this Mailbox", 451},
  {"Don't do anything. Close this dialog.", 452},
  {"Make a backup copy", 453},
  {"Archive mails (recommended).", 454},
  {"Incorrect Password", 455},
  {"Try this new information", 456},
  {"Account:", 457},
  {"\
The server has rejected the username/password\n\
combination: ", 458},
  {"\
\n\
Provide the correct information for logging in\n\
in the following fields.", 459},
  {"Getting in Touch", 460},
  {"Close this dialog", 461},
  {"\
There is no account configured yet.\n\
You need to do this before being\n\
able to compose mails.", 462},
  {"\
Not enough data was filled in,\n\
complete the missing data\n\
and try again.", 463},
  {"Text _beside icons", 464},
  {"Text _under icons", 465},
  {"_Text only", 466},
  {"_Icons only", 467},
  {"_Tooltips", 468},
  {"_Edit toolbar...", 469},
  {"Toolbar configuration", 470},
  {"Available elements", 471},
  {"label73", 472},
  {"label74", 473},
  {"Current Toolbar", 474},
  {"label75", 475},
  {"label76", 476},
  {"\
There is a file by that name already.\n\
\n\
Do you want to overwrite it?", 477},
  {"\
Are you sure you want to\n\
delete the selected mails?", 478},
  {"Do not ask again.", 479},
  {"\
Are you sure you want to\n\
expunge the selected mails?", 480},
  {"Release Information", 481},
  {"Welcome", 482},
  {"Known Bugs", 483},
  {"Missing Features", 484},
  {"Show this dialog at start.", 485},
  {"Select Mailbox", 486},
  {"You selected a directory.", 487},
  {"About", 488},
  {"Web Site", 489},
  {"Attachments Tool", 490},
  {"Save selected attachments to disk", 491},
  {"Mail Source", 492},
  {"Add features", 493},
  {"Add Features", 494},
  {"\
Cronos II is prepared to add new cool features in any time you want.\n\
\n\
What follow is a list of all available features of Cronos II that you can \
download from the Internet and install in a quick and easy way.", 495},
  {"New Features available", 496},
  {"Downloading...", 497},
  {"Rule Manager", 498},
  {"_Add rule", 499},
  {"_Edit rule", 500},
  {"_Remove rule", 501},
  {"Rule Editor", 502},
  {"_Open Message", 503},
  {"_Expunge...", 504},
  {"_Mark", 505},
  {"Outbox", 506},
  {"Sent Items", 507},
  {"Trash", 508},
  {"Drafts", 509},
};

int _msg_tbl_length = 509;
