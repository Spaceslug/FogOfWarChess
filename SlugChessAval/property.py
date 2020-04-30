#!/usr/bin/env python3

import sys
a = sys.argv[2]
a = a.replace(a[0], a[0].lower(), 1) 
priv = "_" + a

str = """public {0} {1}
{{ 
    get => {2};
    set => this.RaiseAndSetIfChanged(ref {2}, value);
}}
private {0} {2};"""

print(str.format(sys.argv[1], sys.argv[2], priv))