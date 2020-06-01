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
- GameState from server does not clear all previuos moves even though the pice has moved
-



### version 1.0.0
- make this a stable version
- spectate a PGN (PGNs on server)
- full PGN support ()
- Print rules

### version 0.12.0
- add audio to Aval
- python script for uploading new versions of Aval and maintain a manifest of versions for updator
- auto updator for Aval

### version 0.11.0
- a worker that periodicly run tasks on server
  - Task: Check last hearbeat on logged in
  - Task: Remove matches that should no longer run
  - Task: Log metrics like ammout of logged in users, current matches
- Do the things that should happen when user i logged out e.g stop matches

### version 0.10.0
- back and forward in match( clients are sendt state so should be easy)
- a way to see where and when pices died
- switch to aval with all the same features as client
  - Alive heartbeat must be fixed. Make sure it works as intended.
  
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