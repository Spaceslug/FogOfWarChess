# SlugChess Rules - version 1

SlugChess is a group variant of Chess where the two players have imperfect information. There are two sub-variants of SlugChess, but first the things they have in common compared to normal chess.

## Vision

SlugChess is a game of imperfect information. A player can only see certain parts of the chessboard, and the two players will almost never be able to see the exact same parts of the board. The fields of the board can be either be in-vision or out-of-vision. If a field is out-of-vision you can not see if a piece is currently or previously occupying the field. It is not possible to move one of your pieces through an out-of-vision field, or end it's movement in one. This is similar to how other pieces of the same color blocks movement. It is therefor possible for knights to "jump" over out-of-vision fields. If fields are in-vision the normal rules of Chess apply unless otherwise specified.

All fields of the board are out-of-vision unless one or more of your pieces grant out vision over that field. There are 3 different ways your pieces can grant you visions over a field
- _View Distance_ - if a field is within x distance from your piece the field is in-vision. Distance is measured in standard chess distance [(Chebyshev distance)](https://en.wikipedia.org/wiki/Chebyshev_distance).
- _View Movement_ - if it would in be possible to move to a field excluding the out-of-vision rules, that field is in-vision. This means pieces with this rule is able to move as in normal chess.
- _View Capture_ - if a piece is captured is will no longer give vision as it is removed from the board. Except for the field is was captured in. The field the capture happened in is in-vision for the player who lost a piece for one half-move. After this the field is out-of-vision.

The two SlugChess sub-variants have a different combination of these vision rules for different types of pieces.

## Shuffling of the officers

Like Fisher Random/Chess960 the officers of SlugChess are shuffled. Unlike Fisher Random the officers in SlugChess are shuffled completely random. That means that the two Bishops can end up on the same colors, and both Rook on the same side of the King. In addition there is NO CASTLING is SlugChess. This is because threating castling fields preventing castling would be a leak of information through the fog-of-war.

## No Checks

In SlugChess there is NO CHECKS and NO CHECKMATE. This is because checks preventing movement is a leak of information through the fog-of-war. Instead of winning by checkmating your opponent. You win is SlugChess by capturing your opponents King. The no checks rule means is is fully legal moves to move your King to a threatened field, and moving some other piece when your King is threatened.

## The two sub-variants of SlugChess

The rules of SlugChess is not a complete variant but a framework for the two implementations of SlugChess. These implementations are currently called _SlugChess TorchWip_ and _SlugChess SightWip_.

### SlugChess TorchWip

In _SlugChess TorchWip_ all pawns and Kings have a _View Distance_ of 1 and all the other officers a _View Distance_ of 2. No pieces have _View Movement_. _View Capture_ is in use for _SlugChess TorchWip_.

This variant rewards a slow and methodical approach. You will for the most part know what the opponent can currently see of your pieces.  

### SlugChess SightWip

In _SlugChess SightWip_ only Knights and Pawns have a _View Distance_, and the _View Distance_ is 1. All pieces have _View Movement_.  

This variant rewards a more aggressive playstyle. You will very rarely know what the opponent can see of your pieces, and you can often capture the opponents officers without them seeing it coming. 

## Changelog from SlugChess rules - version 0
- Everything
