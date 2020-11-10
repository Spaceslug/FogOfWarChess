## Not assigned to version yet
- propper user login system (Make Loggin call a lifetime call)
- sign the assembly with a certificate to make windows happy
  - seams i have to pay money to get this
- learn how to debug in the IDE
- a check to evaluate the validity of time spent on move. Change the value if it seams wrong. 
- go back and forwards in the move while playing a match
- castling checks threatened fields, and works with FisherRandom 
- print the Rules of a SlugChess game to text
- print the VisionRules of a SlugChess game to text
- add commands to the server in the message box
- reenable signal handling on server
- updator of slugchessclient (fetch newest version for spaceslug.no)
- client with only unary and server streamer calls for Match and ChatMessages
- server saves all matches that have been played
- use Python script to upload new SlugChess version to spaceslug.no
- password and name on hosted games
- elo system 
- matchmaker using elo
- use json save app state on Aval

## What does the versions mean (only from v1.0.0 and onwards)
The client and server versions follow eachother. It is impossible to use a release client with 
higher number than release server as new client is only release after server is updated.

### Build version is lower
It is recomended to update. Hosted games can still be played but not matchmaking (elo games)

Minor fixes goes here that should not break games between users

### Minor version is lower
You need to update in order to login

Game breaking changes but no changes to the chesscom communication api or function

### Major
Changes to chesscom commuication api or underlating function.

## BUGS:
- in 0.10.0 stack overflow in SelectedItem in Chessboard when pressing Browse Game after previus game done
- might be race condition when surrendering


### version 1.0.0
- make this a stable version
- spectate a PGN (PGNs on server)
- full PGN support ()
- Print rules
- make a propper logger on server. Write to log file and console. Delete logfile on restart
- a check to evaluate the validity of time spent on move. Change the value if it seems wrong. 
- go over what lines are logged on the server
- make a propper logging system
- [ ] auto updator can update to spesific version. A command line option. Mayby post link to available releases page

### version 0.18.0


### version 0.17.0
- [ ] make a propper version system for slugchesscore shared lib. A build of slugchess server needs a spesific version of core. When you publish server release you supply a core version (think more on it). symlink to more spsific version to support multiple versions. Currently slugchesscoretest rpath does not work and will only use export version of lib
- [ ] makefile SlugChessServer have a define for debug build. Compile different .o files for debug and regular
- [ ] rename 'libslugchess.so' to 'libslugchess-core.so'


### version 0.16.0
- [ ] Formalize Torch and Sight. Both a text and chesscom description of the rules. They should temporerly be known as TorchWip and SightWip.
- [ ] password system
- [ ] save userdata {username as id and elo. Passwords come later}
- [ ] simple elo system (same as FIDE)

### version 0.15.0
- [x] load up and click through PGNs.
  - [x] Make Match.Model propper observable type. Less of the properties. More observable.
  - [x] Make server set bool if PGN was parsed successfully.
  - [x] select viewpoint when watching replay
  - [ ] add no_vision_rules as a vision rule to Core
- [x] Bind ServerVisionRulset with CreateGame view selector. 

### version 1.14.1
- [x] bug- Server kan motta move etter match er ferdig. Fører til krash
- [x] (could not replicate)bug- Server fjerner ikke bruker ordentlig ved heartbeat failed
- [x] Updater new retries when .7z handle not released during cleanup

### version 0.14.0
- [x] record games played (PGN). In a replay folder for users and a replay folder on server

### version 0.13.1 (Done)
- [x] client can be lagging behind 1 message when finishing matches
- [x] nothing happens on client when time runs out. When out of time client sends random move
- [x] selecteble pices with zero moves are sent from server
- [x] sort capture pices according to color instead of cronologicaly
- [x] auto updator is busted again. looks for files in wring locations

### version 0.13.0 (Done)
- [x] move match logic to MatchModel. Ex: IsCurrentPlayersTurn obs lives there, code for match events is there and the Chesscom call
- [x] if server is wrong version or is not there at all. Show a propper message to the user trying to log on


### version 0.12.1 (Done)
- [x] finish install.sh for linux. make seperate for debug also
- [x] click again on a pice to deselect
- [x] clean up selection colors


### version 0.12.0 (Done)
- [x] add audio to Aval 
- [x] python script for uploading new versions of Aval and maintain a manifest of versions for updater
- [x] auto updator for Aval


### version 0.11.1 (Done)
- [x] clean up TODOs on servinator
- [x] Alive heartbeat must be fixed. Make sure it works as intended.

### version 0.11.0 (Done)
- [x] make messager globaly available in SCS so messages can be sent to a user anywhere in the code
- [x] a worker that periodicly run tasks on server
  - [x] Task: Check last hearbeat on logged in
  - [x] Task: Remove matches that should no longer run
  - [x] Task: Log metrics like ammout of logged in users, current matches
- [x] Do the things that should happen when user is logged out e.g stop matches, end all log calls
- [x] need to separate messages to player. Both can't listen to match_events because they sometime need differert events


### version 0.10.1 (Done)
- [x] leave game and conced
- [x] ask for and accept draw
- [x] see if Draw commands can be made with async commands istead of callin UIAsync https://reactiveui.net/docs/guidelines/framework/asynchronous-commands
- [x] fix cancel on hosting not can   celing hosting

### version 0.10.0 (Done)
- [x] back and forward in match( clients are sendt state so should be easy)
- [x] a way to see where and when pices died
- [x] switch to aval with all the same features as client
  - [x] Fix match terminating when one player exits client
  - [x] Messages not sent between players in a match
  
### version 0.9.0 (Done)
- [x] implement double fisher random
- [x] fix release uploader

### version 0.8.1 (Done)
- [x] fix looking for game bug
- [x] fix check helper bug, (check mus be handled inside slugchess. shadowmoves is not good enough)
- [x] Make less cases of invalid state on client. Only enable things when logged in
- [x] implement Alive Hearbeat (Clients must send heartbeat every 1 min)

### version 0.8.0 (Done)
- [x] Create custom games with custom rules
- [x] create a new object orientet structure to the server code

### version 0.7.1 (Done)
- [x] create a new object orientet structure to the server code
  - [x] rework some thread to use conditional varible instead of thread sleep. This to improve speed
- [x] Print SAN at end of game

version 0.7.0 (Done)
-(x)chose new IDE to work with on linux
-(x)move all move validation to server/rules not on server
  -(x)slugchess on server (missing end of game)
-(x)critical game breaking bug. Server stops excepting massages and computer crashes
-(x)timer is fucked no time is removed

version 0.6.2 (Done)
-(x)sound when running out of time
-(x) bug - time over minutes of time used are ignored

version 0.6.1 (Done)
-(x)finaly implement view move field (have vision on all tiles where your pices can move to)
    -calling this Sea rules

version 0.5.2 (Done)
-can click on opponents pices to see moves

version 0.5.1 (Done)
-(X)prevent move through hidden fields

version 0.4.2 (Done)
-(x)remove unessesary files like x86 stuff and .xml and .pdb

version 0.4.1 (Done)
-(x)chess clock

version 0.3.3 (Done)
-(x)bug: last move not cleared when starting a new match.
-(x)bug: text log only scrolles when "Opponet did move!" or opponent won

version 0.3.2 (Done)
-(x)make it a only x64 program 

version 0.3.1 (Done)
-(x)(optional rule) Last captured field is visible for both players
-(x)chat system
-(x)bug: losing player can continue to play
-(x)bug: loser does not get message that they lost. Server does not send message

version 0.2.4 (Done)
-(x)add debug port
-(x)checker of version number. Server must match clients. Minor not same is fine.
-(x)fix no message scroll after match is done
-(x)fix killed pices not cleared after new match is started
-(x)add sound when match found

version 0.2.3 (Done)
-(x)fix that user that does not have current turn freezes on exit
--(x)changing to a dedicated read thread on server
-(x) added autogeneration of csharp proto 

version 0.2.2 (Done)
-(x) make autoexport to spaceslug.no
-(x) make service for slugChessServinator
-(x) show check from visible enemies


version 0.2.1 (Done)
-message moveResult contains MatchEvents
-Match rules on server
-add end of game

version 0.1.2 (Done)
-opponent move sound
-probably sleep loop fail
