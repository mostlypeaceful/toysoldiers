using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;
using System.Threading;
using System.Xml;
using System.Xml.XPath;

namespace PostBuildProcess
{
    class PostBuildProcess
    {
        FogBugz fogBugz = null;
        LogIt logIt = new LogIt(@"C:\postbuildprocess_log.txt");
        string buildStatus = "";
        string buildLabel = "";
        string artifactDirectory = "";
        string projectName = "";
        string projectDirectory = "";
        string fogbugzProjectToReference = "";
        UInt32 latestChangelist = 0;
        string xedkDirectory = "";

        public PostBuildProcess()
        {
            IDictionary environmentVariables = Environment.GetEnvironmentVariables();
            buildStatus = (String)environmentVariables["ccnetintegrationstatus"];
            buildLabel = (String)environmentVariables["ccnetlabel"];
            artifactDirectory = (String)environmentVariables["ccnetartifactdirectory"];
            projectName = (String)environmentVariables["sigcurrentprojectname"];
            projectDirectory = (String)environmentVariables["sigcurrentproject"];
            fogbugzProjectToReference = (String)environmentVariables["bugreferenceproject"];
            latestChangelist = Convert.ToUInt32((String)environmentVariables["latestbuiltchangelist"]);
            xedkDirectory = (String)environmentVariables["xedk"];

            // We need to know which project we're referencing
            if (fogbugzProjectToReference == null || fogbugzProjectToReference == "")
            {
                logIt.LogLine("No FogBugz project given.");
                return;
            }

            bool successfulBuild = false;
            if (buildStatus == "Success")
                successfulBuild = true;

            fogBugz = new FogBugz(@"http://signal/fogbugz/api.asp");

            // First get all unique changelists involved in this build
            logIt.LogLine("Getting all changelists...");
            List<Changelist> changelists = GetChangelists(artifactDirectory + "\\modifications.xml");
            if (changelists.Count > 0)
            {
                // Go through each changelist and get any case IDs that need
                // to be resolved
                foreach (Changelist changelist in changelists)
                {
                    // Track the latest changelist involved in this build
                    UInt32 changelistNumber = Convert.ToUInt32(changelist.changeNumber);
                    if (changelistNumber > latestChangelist)
                        latestChangelist = changelistNumber;

                    List<string> caseIDs = GetCaseIDsToResolve(changelist);
                    foreach (string caseID in caseIDs)
                    {
                        // Don't bother to log in until we know we need to
                        if (!fogBugz.CurrentlyLoggedIn())
                        {
                            if (!fogBugz.LoginToFogBugz("Steven Carroll", "Signal01", ref logIt))
                                break;
                        }

                        logIt.LogLine("Updating Case " + caseID);
                        UpdateCase(caseID, changelist.username, changelist.changeNumber, successfulBuild);
                    }
                }

                if (latestChangelist > 0)
                {
                    string changelist = latestChangelist.ToString();
                    UpdateEXEDescriptionMetadata("Changelist " + changelist);
                    Environment.SetEnvironmentVariable("latestbuiltchangelist", changelist, EnvironmentVariableTarget.User);
                }
            }

            MoveStatFiles();
        }

        ~PostBuildProcess()
        {
            if (fogBugz != null)
            {
                logIt.LogLine("Logging out of FogBugz...");
                fogBugz.LogoutOfFogBugz(ref logIt);
            }
        }

        private List<Changelist> GetChangelists(string path)
        {
            List<Changelist> changelists = new List<Changelist>();

            if (!File.Exists(path))
            {
                logIt.LogLine("Could not find modification file: " + path);
                return changelists;
            }

            // Open the modification file and get each Modification node
            XmlTextReader reader = new XmlTextReader(new StreamReader(path));
            XPathDocument doc = new XPathDocument(reader);
            XPathNavigator nav = doc.CreateNavigator();
            XPathNodeIterator nodeIterator = (XPathNodeIterator)nav.Evaluate("ArrayOfModification/Modification");
            logIt.LogLine(nodeIterator.Count + " modification nodes found.");
            foreach (XPathNavigator node in nodeIterator)
            {
                if (!node.HasChildren)
                    continue;

                Changelist newChangelist = new Changelist();

                // Iterate through the child nodes
                node.MoveToFirstChild();
                do
                {
                    switch (node.Name)
                    {
                        case "UserName":
                            newChangelist.username = node.Value;
                            break;
                        case "ChangeNumber":
                            newChangelist.changeNumber = node.Value;
                            break;
                        case "Comment":
                            newChangelist.comment = node.Value;
                            break;
                    }
                }
                while (node.MoveToNext());

                // Just a little sanity check
                if (newChangelist.changeNumber == "")
                    continue;

                // Everything is based on the changelist number,
                // so don't add nodes with the same one twice
                bool duplicate = false;
                foreach (Changelist changelist in changelists)
                {
                    if (newChangelist.changeNumber == changelist.changeNumber)
                    {
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate)
                    changelists.Add(newChangelist);
            }
            reader.Close();

            return changelists;
        }

        private List<string> GetCaseIDsToResolve(Changelist changelist)
        {
            List<string> caseIDs = new List<string>();

            // Parse the changelist comment looking for cases to resolve
            Regex regex = new Regex(@"\s*RESOLVE*\s*IDs*\s*[#:; ]+((?<caseid>\d+[ ,:;#]*)+)", RegexOptions.IgnoreCase | RegexOptions.Multiline);
            MatchCollection matches = regex.Matches(changelist.comment);
            if (matches.Count > 0)
                logIt.LogLine("A RESOLVEID match was found.");
            foreach (Match match in matches)
            {
                if (match.Groups.Count < 3)
                    continue;

                CaptureCollection captures = match.Groups[2].Captures;
                if (captures.Count < 1)
                    continue;

                logIt.LogLine(captures.Count + " resolve IDs found."); 
                foreach (Capture capture in captures)
                {
                    // Replace any non-digit in the captured value
                    // with nothing
                    string caseID = Regex.Replace(capture.Value, "\\D+", "");
                    if (caseID != "")
                        caseIDs.Add(caseID);
                }
            }

            return caseIDs;
        }

        private void UpdateCase(string caseID, string user, string changeNumber, bool successfulBuild)
        {
            if (caseID == null || caseID == "")
                return;

            Case fbCase = new Case(caseID, ref fogBugz, ref logIt);

            // We don't need to update inactive cases
            if (!fbCase.active)
                return;

            // Only update cases for the project we're working on
            if (fbCase.project != fogbugzProjectToReference && fbCase.project != "SigEngine")
                return;

            string command = "";
            string description = user + " submitted a fix for this bug in changelist " + changeNumber + ".";

            if (fbCase.active && buildStatus == "Success")
            {
                command = "resolve";
                description += " A build that included this was successful. This bug is now resolved.";
            }
            else if (fbCase.active && buildStatus != "Success")
            {
                command = "edit";
                description += " A build that included this failed. This bug is still not resolved.";
            }

            // If there is a failed build, a note will be left about it;
            // if we encounter the same note, we don't need to update
            if (description == fbCase.lastUpdate)
                return;

            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("ixBug", caseID);
            args.Add("sEvent", description);
            args.Add("cmd", command);
            string response = fogBugz.CallRESTAPIFiles(ref logIt, args, null);
            if (response.Contains("error"))
            {
                System.Xml.XmlTextReader reader = new System.Xml.XmlTextReader(new StringReader(response));
                System.Xml.XPath.XPathDocument doc = new System.Xml.XPath.XPathDocument(reader);
                System.Xml.XPath.XPathNavigator nav = doc.CreateNavigator();

                string errorMessage = nav.Evaluate("string(response/error)").ToString();
            }
        }

        private void UpdateEXEDescriptionMetadata(string description)
        {
            if (projectDirectory == "" || description == "")
                return;

            List<string> paths = new List<string>();
            paths.Add(projectDirectory + @"\Builds\Current\Bin\game_xbox360_debug\default.exe");
            paths.Add(projectDirectory + @"\Builds\Current\Bin\game_xbox360_internal\default.exe");
            paths.Add(projectDirectory + @"\Builds\Current\Bin\game_xbox360_profile\default.exe");
            paths.Add(projectDirectory + @"\Builds\Current\Bin\game_xbox360_release\default.exe");

            foreach (string path in paths)
            {
                if (!File.Exists(path))
                    continue;

                Process proc = new Process();
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.RedirectStandardOutput = true;
                proc.StartInfo.CreateNoWindow = true;
                proc.StartInfo.FileName = artifactDirectory + @"\..\bin\verpatch.exe";
                proc.StartInfo.Arguments = "/va " + path + " /s desc \"" + description + "\"";
                proc.Start();
                proc.WaitForExit();
            }
        }

        private void MoveStatFiles()
        {
            string buildDir = @"C:\BuildStats\" + projectName + @"\" + latestChangelist;

            string xbconsoletypeEXE = xedkDirectory + "\\bin\\win32\\xbconsoletype.exe";
            if (!File.Exists(xbconsoletypeEXE))
            {
                Console.WriteLine("Could not find " + xbconsoletypeEXE);
                return;
            }

            string xbcpEXE = xedkDirectory + "\\bin\\win32\\xbcp.exe";
            if (!File.Exists(xbcpEXE))
            {
                Console.WriteLine("Could not find " + xbcpEXE);
                return;
            }

            string xbdelEXE = xedkDirectory + "\\bin\\win32\\xbdel.exe";
            if (!File.Exists(xbdelEXE))
            {
                Console.WriteLine("Could not find " + xbdelEXE);
                return;
            }

            Process proc;
            bool connectionSuccessful = false;
            for (int i = 0; i < 10; ++i)
            {
                // The console is usually rebooting while this
                // process is running and we have to wait for it
                proc = new Process();
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.RedirectStandardOutput = true;
                proc.StartInfo.RedirectStandardError = true;
                proc.StartInfo.CreateNoWindow = true;
                proc.StartInfo.FileName = xbconsoletypeEXE;
                proc.Start();
                proc.WaitForExit();

                string standard = proc.StandardOutput.ReadToEnd();
                string error = proc.StandardOutput.ReadToEnd();

                if (standard != "")
                {
                    connectionSuccessful = true;
                    break;
                }

                Console.WriteLine("Waiting for console to reboot...");

                Thread.Sleep(1000);
            }
            if (!connectionSuccessful)
            {
                Console.WriteLine("Could not connect to dev kit.");
                return;
            }

            proc = new Process();
            proc.StartInfo.FileName = xbcpEXE;
            proc.StartInfo.Arguments = "/t /r \"devkit:\\" + projectName + "\\*_stats.txt\" \"" + buildDir + "\"";
            proc.Start();
            proc.WaitForExit();

            proc = new Process();
            proc.StartInfo.FileName = xbdelEXE;
            proc.StartInfo.Arguments = "/f \"devkit:\\" + projectName + "\\*_stats.txt\"";
            proc.Start();
            proc.WaitForExit();
        }
    }
}
