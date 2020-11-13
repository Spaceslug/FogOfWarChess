# Publishing
NOTES:
- run publish-release.py from script folder to build and publish win, linux (and osx) releases. They will be marked as latest
- folders on webhost must have public execute permision for user to be able to download files from them.
- run publish-debug.py to publich seperate debug builds
- spaceslug.no/slugchess/latest/oldest-working-autoupdater.txt must be set manualy

- NetCoreBeauty to clean root Ex:  dotnet publish -c Release -r win-x64 --self-contained true -p:PublishTrimmed=true
added updater deleting "libs"