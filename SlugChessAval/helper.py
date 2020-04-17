#!/usr/bin/env python3

boardpos = ["a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
]
print(boardpos)
back = 1
num = 56
ticker = 1
row = 0
for x in range(64):
    
    
    str = """      <Border Name=\"{0}\" Grid.Row=\"{1}\" Grid.Column=\"{2}\" PointerReleased=\"ChessfieldClicked\" Background=\"{{DynamicResource {3}}}\" 
        BorderBrush=    \"{{Binding FieldBoard[{4}].BorderBrush}}\" 
        BorderThickness=\"{{Binding FieldBoard[{4}].BorderThickness}}\" 
        Opacity=        \"{{Binding FieldBoard[{4}].Opacity}}\"
        CornerRadius=   \"{{Binding FieldBoard[{4}].CornerRadius}}\" >
          <DockPanel Background=\"{{Binding FieldBoard[{4}].ImageBackgroundBrush}}\">
            <Image Source=\"{{Binding FieldBoard[{4}].Image}}\" />
          </DockPanel>
        </Border>"""
    
    print(str.format(boardpos[num], row, ticker-1, "BlackField" if back%2 == 1 else "WhiteField", num))
    
    back += 1
    ticker += 1
    num += 1
    if ticker == 9:
        num -= 16
        ticker = 1
        row += 1
        back += 1