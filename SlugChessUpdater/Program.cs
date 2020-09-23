using SharpCompress.Archives;
using SharpCompress.Archives.SevenZip;
using SharpCompress.Common;
using SharpCompress.Readers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Dynamic;
using System.IO;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;

namespace SlugChessUpdater
{
    public static class Const
    {
#if DEBUG
        public static string VersionUrl = "https://spaceslug.no/slugchess-debug/latest/version.txt";
        public static string SlugChessUrl(int major, int minor, int patch) => $"https://spaceslug.no/slugchess-debug/releases/{PlatformString}/SlugChess_{major}_{minor}_{patch}.7z";
#else
        public static string VersionUrl = "https://spaceslug.no/slugchess/latest/version.txt";
        public static string SlugChessUrl(int major, int minor, int patch) => $"https://spaceslug.no/slugchess/releases/{PlatformString}/SlugChess_{major}_{minor}_{patch}.7z";
#endif
        public static string OldestWorkingAutoupdator = "https://spaceslug.no/slugchess/latest/oldest-working-autoupdater.txt";
        public static string PlatformString 
        { get {
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
                {
                    return "linux-x64";
                }
                else if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                {
                    return "win-x64";
                }
                throw new PlatformNotSupportedException("fucj esg");
            } 
        }
    }

    class Program
    {
        private static bool _downloadCompleted;
        private static string _downloadName = "non";
        private static int _downloadPercent = 0;

        public static string RootDir = (Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName) ?? ".") + "/";
        static void Main(string[] args)
        {
            try
            {

                List<string> argsList = args.ToList();
                if (argsList.Contains("--help"))
                {
                    Console.WriteLine(
                        "SlugChessUpdator help \n"+
                        "    All commands retun Error:[message] if something is wrong     \n" +
                        "--help                      - for this message                   \n" +
                        "--check-version xx.xx.xx    - to to check version relative       \n" +
                        "    to latest version, returns 0 for ok, 1 for must,             \n" +
                        "    2 for should update, 3 must fetch new version manualy        \n" +
                        "--run-update                - to update SlugChessAval,           \n" +
                        "    SlugChessAval has 2 sec to quit after starting update,       \n" +
                        "    SlugChessAval will start after update complete               \n" 
                        );
                }
                else if(argsList.Contains("--check-version"))
                {
                    List<int> currentVersion = argsList[argsList.IndexOf("--check-version") + 1].Split('.').ToList().ConvertAll(Convert.ToInt32);

                    List<int> oldestWorkingAutoupdater = WebFetcher.GetOldestWorkingAutoupdator().Split('.').ToList().ConvertAll(Convert.ToInt32);
                    if (currentVersion[0] == oldestWorkingAutoupdater[0])
                    {
                        if(currentVersion[1] == oldestWorkingAutoupdater[1])
                        {
                            if(currentVersion[2] < oldestWorkingAutoupdater[2])
                            {
                                Console.WriteLine("3");
                                return;
                            }
                        }
                        else if (currentVersion[1] < oldestWorkingAutoupdater[1])
                        {
                            Console.WriteLine("3");
                            return;
                        }
                    }
                    else if(currentVersion[0] < oldestWorkingAutoupdater[0])
                    {
                        Console.WriteLine("3");
                        return;
                    }

                    List<int> latestVersion = WebFetcher.GetLatestVersionString().Split('.').ToList().ConvertAll(Convert.ToInt32);
                    if (latestVersion[0] > currentVersion[0])
                    {
                        Console.WriteLine("1");
                        return;
                    }
                    else if(latestVersion[0] == currentVersion[0])
                    {
                        if (latestVersion[1] > currentVersion[1])
                        {
                            Console.WriteLine("1");
                            return;
                        }
                        else if (latestVersion[1] == currentVersion[1])
                        {
                            if(latestVersion[2] > currentVersion[2])
                            {
                                Console.WriteLine("2");
                                return;
                            }
                            else if (latestVersion[2] == currentVersion[2])
                            {
                                Console.WriteLine("0");
                                return;
                            }
                            else
                            {
                                Console.WriteLine("Error:current version is higher than latest");
                                return;
                            }
                        }
                        else
                        {
                            Console.WriteLine("Error:current version is higher than latest");
                            return;
                        }
                    }
                    else
                    {
                        Console.WriteLine("Error:current version is higher than latest");
                        return;
                    }
                }
                else if (argsList.Contains("--run-update"))
                {
                    //Do update
                    DeleteFiles();
                    DownloadAndExtractNewVersion(WebFetcher.GetLatestVersionAddress(), WebFetcher.GetLatestVersionString());
                    Console.WriteLine("Update complete. Launching SlugChess");
                    LaunchAval();
                }
                else if (argsList.Contains("--test"))
                {
                }
            }
            catch(PlatformNotSupportedException)
            {
                Console.WriteLine("Error: Could not detect plattform");
                Console.WriteLine("Press any to quit...");
                Console.ReadKey();
            }
            catch(Exception ex)
            {
                Console.WriteLine("unknown error: " + ex.Message);
                Console.WriteLine("Press any to quit...");
                Console.ReadKey();
            }
        }

        static void DeleteFiles()
        {
            Console.WriteLine("Deleting old version");
            string[] filePaths = Directory.GetFiles($"{RootDir}");
            foreach (string filePath in filePaths)
            {
                var name = new FileInfo(filePath).Name;
                var nameLower = name.ToLower();
                if (nameLower != "appstate.json" && nameLower != "slugchessupdater.exe" && nameLower != "slugchessupdater")
                {
                    //Console.WriteLine(nameLower);
                    File.Delete(filePath);
                }
            }
            if(Directory.Exists($"{RootDir}Assets")) Directory.Delete($"{RootDir}Assets", true);

            Console.WriteLine("Deleting old files complete");

        }

        static void DownloadAndExtractNewVersion(string adressOfLatestVersion, string latestVersion)
        {
            Console.WriteLine("Downloading SlugChess " + latestVersion + ": " + adressOfLatestVersion);
            Directory.CreateDirectory($"{RootDir}temp");

            WebClient myWebClient = new WebClient();
            //myWebClient.DownloadFile(adressOfLatestVersion, $"{RootDir}temp/latest.7z");

            // Specify that the DownloadFileCallback method gets called
            // when the download completes.
            myWebClient.DownloadFileCompleted += new AsyncCompletedEventHandler(DownloadFileCallback);
            // Specify a progress notification handler.
            myWebClient.DownloadProgressChanged += new DownloadProgressChangedEventHandler(DownloadProgressCallback);
            _downloadName = "SlugChess " + latestVersion;
            _downloadCompleted = false;
            _downloadPercent = 0;
            myWebClient.DownloadFileAsync(new Uri(adressOfLatestVersion), $"{RootDir}temp/latest.7z");

            //Wait until download completed
            while (!_downloadCompleted) Thread.Sleep(250);
            Console.WriteLine("Extracting SlugChess " + latestVersion);
            Console.WriteLine("Size of achive: " + new FileInfo($"{RootDir}temp/latest.7z").Length);
            using (var archive = SevenZipArchive.Open($"{RootDir}temp/latest.7z"))
            {
                var reader = archive.ExtractAllEntries();
                reader.CompressedBytesRead += new EventHandler<CompressedBytesReadEventArgs>(CompressedBytesReadCallback);
                while (reader.MoveToNextEntry())
                {
                    if (!reader.Entry.IsDirectory)
                    {
                        reader.WriteEntryToDirectory($"{RootDir}temp", new ExtractionOptions() { ExtractFullPath = true, Overwrite = true });
                    }
                }
            }
            Console.WriteLine("Moving files");
            foreach (var file in new DirectoryInfo($"{RootDir}temp/SlugChess/").GetFiles())
            {
                if (File.Exists($"{RootDir}{file.Name}"))
                {
                    Console.WriteLine("File allready exists: " + file.Name);
                    if(file.Name == "SlugChessUpdater.exe")
                    {
                        file.MoveTo($"{RootDir}{file.Name}.new");
                    }
                }
                else
                {
                    file.MoveTo($"{RootDir}{file.Name}");
                }
            }
            foreach (var dir in new DirectoryInfo($"{RootDir}temp/SlugChess/").GetDirectories())
            {
                dir.MoveTo($"{RootDir}{dir.Name}");
            }
            Console.WriteLine("Deleting temp files");
            Directory.Delete($"{RootDir}temp", true);
        }

        static void LaunchAval()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Process.Start("SlugChessAval.exe", "--no-updatecheck");
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                Process.Start("SlugChessAval", "--no-updatecheck");
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                //bla bla bla
                throw new PlatformNotSupportedException();
            }
            else
            {
                throw new PlatformNotSupportedException();
            }
        }

        private static void DownloadProgressCallback(object sender, DownloadProgressChangedEventArgs e)
        {
            // Displays the operation identifier, and the transfer progress.
            if(_downloadPercent < e.ProgressPercentage)
            {
                _downloadPercent = e.ProgressPercentage;
                Console.WriteLine("{0}    downloaded {1} of {2} bytes. {3} % complete...",
                    _downloadName,
                    e.BytesReceived,
                    e.TotalBytesToReceive,
                    e.ProgressPercentage);
            }

        }

        private static void DownloadFileCallback(object sender, AsyncCompletedEventArgs e)
        {
            if (e.Cancelled)
            {
                Console.WriteLine("Download failed. Error: " + e.Error.Message);
            }
            else
            {
                Console.WriteLine(_downloadName + " download finished");
                _downloadCompleted = true;
            }
        }

        private static void CompressedBytesReadCallback(object? sender, CompressedBytesReadEventArgs e)
        {
            Console.WriteLine("Compressed bytes read: " + e.CompressedBytesRead);
        }



    }


    public static class WebFetcher
    {
        public static string GetOldestWorkingAutoupdator()
        {
            WebClient client = new WebClient();
            Stream stream = client.OpenRead(Const.OldestWorkingAutoupdator);
            StreamReader reader = new StreamReader(stream);
            return reader.ReadToEnd();
        }
        public static string GetLatestVersionString()
        {
            WebClient client = new WebClient();
            Stream stream = client.OpenRead(Const.VersionUrl);
            StreamReader reader = new StreamReader(stream);
            return reader.ReadToEnd();
        }

        public static string GetLatestVersionAddress()
        {
            List<int> version = GetLatestVersionString().Split('.').ToList().ConvertAll(Convert.ToInt32);
            return Const.SlugChessUrl(version[0], version[1], version[2]);
        }
    }
}
