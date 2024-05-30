using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

namespace GetLatest
{
    public partial class GetLatest : Form
    {
        enum tSyncStages
        {
            cSyncEngine,
            cSyncEngineBin,
            cSyncEngineDocs,
            cSyncGameProj,
            cCleanGame,
            cCopyBin,
            cCopyEngineBin,
            cCopyGame,
            cAssetGen,
            cFinished,

            cNumStages,
        };

        enum tScmTypes
        {
            svn,
            p4
        }

        private tSyncStages mSyncStage = tSyncStages.cSyncEngine;
        private System.Diagnostics.Process mCurrentStageProcess;
        private string mProjNameVar;
        private string mProjPathVar;
        private string mEngPathVar;
        private string mBuildMachineName;
        private System.DateTime mCleanGameLastWriteTime;
        private tScmTypes mScmType = tScmTypes.svn;

        private bool[] mFailures = new bool[(int)tSyncStages.cNumStages];
        private List<string> mConflictErrors = new List<string>();

        public GetLatest()
        {
            InitializeComponent();
        }

        private bool fStageComplete()
        {
            if (mCurrentStageProcess.HasExited)
            {
                if (mSyncStage < tSyncStages.cNumStages)
                    mFailures[(int)mSyncStage] |= mCurrentStageProcess.ExitCode != 0;

                return true;
            }

            return false;
        }

        private void fAdvanceStage()
        {
            ++mSyncStage;
        }

        private void fLaunchStage()
        {
            switch (mSyncStage)
            {
                case tSyncStages.cSyncEngine:
                    {
                        switch (mScmType)
                        {
                            case tScmTypes.svn:
                                {
                                    this.OutputLog.AppendText("\r\n====================== Sync'ing SVN \r\n");
                                    this.OutputLog.AppendText("Syncing " + mEngPathVar + "\r\n");
                                    mCurrentStageProcess = fSpawnProcess("svn.exe", "update " + mEngPathVar, "");
                                }
                                break;

                            case tScmTypes.p4:
                                {
                                    this.OutputLog.AppendText("\r\n====================== Sync'ing P4 \r\n");
                                    mCurrentStageProcess = fSpawnProcess("p4", @"sync " + mEngPathVar + @"\...", "");
                                }
                                break;
                        }

                    }
                    break;

                case tSyncStages.cSyncEngineBin:
                    {
                        this.OutputLog.AppendText("\r\n");

                        switch (mScmType)
                        {
                            case tScmTypes.svn:
                                this.OutputLog.AppendText("Syncing " + mEngPathVar + "\\Bin\r\n");
                                mCurrentStageProcess = fSpawnProcess("svn.exe", "update " + mEngPathVar + @"\Bin", "");
                                break;

                            case tScmTypes.p4:
                                mCurrentStageProcess = fSpawnProcess("p4", @"sync " + mEngPathVar + @"\Bin\...", "");
                                break;
                        }
                    }
                    break;

                case tSyncStages.cSyncEngineDocs:
                    {
                        this.OutputLog.AppendText("\r\n");



                        switch (mScmType)
                        {
                            case tScmTypes.svn:
                                this.OutputLog.AppendText("Syncing " + mEngPathVar + "\\Docs\r\n");
                                mCurrentStageProcess = fSpawnProcess("svn.exe", "update " + mEngPathVar + @"\Docs", "");
                                break;

                            case tScmTypes.p4:
                                mCurrentStageProcess = fSpawnProcess("p4", @"sync " + mEngPathVar + @"\Docs\...", "");
                                break;
                        }

                    }
                    break;

                case tSyncStages.cSyncGameProj:
                    {
                        this.OutputLog.AppendText("\r\n");

                        switch (mScmType)
                        {
                            case tScmTypes.svn:
                                this.OutputLog.AppendText("Syncing " + mProjPathVar + "\r\n");
                                mCurrentStageProcess = fSpawnProcess("svn.exe", "update " + mProjPathVar, "");
                                break;

                            case tScmTypes.p4:
                                mCurrentStageProcess = fSpawnProcess("p4", @"sync " + mProjPathVar + @"\...", "");
                                break;
                        }
                    }
                    break;

                case tSyncStages.cCleanGame:
                    {
                        try
                        {
                            System.IO.FileInfo cleanGameFileInfo = new System.IO.FileInfo(mProjPathVar + "\\cleangame");
                            cleanGameFileInfo.Refresh();
                            if (cleanGameFileInfo.LastWriteTime == mCleanGameLastWriteTime)
                                this.OutputLog.AppendText("\r\ncleangame is up-to-date\r\n");
                            else
                            {
                                // delete game folder
                                this.OutputLog.AppendText("\r\n====================== Cleaning Game Folder \r\n");
                                mCurrentStageProcess = fShellCommand("\"%SigEngine%\\Bin\\CleanGame.cmd\"", "");
                            }
                        }
                        catch { }
                    }
                    break;

                case tSyncStages.cCopyGame:
                    {
                        this.OutputLog.AppendText("\r\n====================== Copying Built Assets From Build Server \r\n");
                        mCurrentStageProcess = fShellCommand("xcopy", @"\\" + mBuildMachineName + @"\" + mProjNameVar + @"\Project\Builds\Current\Game " + mProjPathVar + @"\Game /D /S /C /I /Y /R");
                    }
                    break;

                case tSyncStages.cCopyBin:
                    {
                        mCurrentStageProcess = fShellCommand("xcopy", @"\\" + mBuildMachineName + @"\" + mProjNameVar + @"\Project\Builds\Current\Bin " + mProjPathVar + @"\Bin /D /S /C /I /Y /R");
                    }
                    break;

                case tSyncStages.cCopyEngineBin:
                    {
                        mCurrentStageProcess = fShellCommand("xcopy", @"\\" + mBuildMachineName + @"\" + mProjNameVar + @"\Project\Builds\Current\SigEngine\Bin " + mEngPathVar + @"\Bin /D /S /C /I /Y /K /R");
                    }
                    break;

                case tSyncStages.cAssetGen:
                    {
                        this.OutputLog.AppendText("\r\n====================== Building Local Assets \r\n");
                        mCurrentStageProcess = fSpawnProcess("AssetGen.exe", "-np -m -r -pcdx9 -xbox360 -game2xbox -log 1", mProjPathVar + @"\Res");
                    }
                    break;

                case tSyncStages.cFinished:
                    {
                        fFinishedStage();
                    }
                    break;
            }
        }

        private void fFinishedStage()
        {
            this.OutputLog.AppendText("\r\n\r\n====================== GetLatest is complete.\r\n");

            bool syncError =
                mFailures[(int)tSyncStages.cSyncEngine] ||
                mFailures[(int)tSyncStages.cSyncEngineBin] ||
                mFailures[(int)tSyncStages.cSyncEngineDocs] ||
                mFailures[(int)tSyncStages.cSyncGameProj];

            bool copyError =
                mFailures[(int)tSyncStages.cCopyGame] ||
                mFailures[(int)tSyncStages.cCopyBin] ||
                mFailures[(int)tSyncStages.cCopyEngineBin];

            bool assetGenError =
                mFailures[(int)tSyncStages.cAssetGen];

            // Check for unresolved conflicts manually, because if the conflict
            // has already been reported by P4 once, it won't be reported again
            if (mScmType == tScmTypes.p4)
            {
                System.Diagnostics.Process process = new System.Diagnostics.Process();
                process.StartInfo.FileName = "p4";
                process.StartInfo.Arguments = "resolve -n";
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.RedirectStandardInput = true;
                process.StartInfo.RedirectStandardError = true;
                process.StartInfo.CreateNoWindow = true;
                process.Start();
                while (!process.StandardOutput.EndOfStream)
                {
                    string output = process.StandardOutput.ReadLine();
                    syncError = true;
                    mFailures[(int)mSyncStage] = true;
                    mConflictErrors.Add(output);
                }
            }

            if (syncError)
            {
                if (mConflictErrors.Count > 0)
                {
                    // Dump all conflicts.
                    this.OutputLog.AppendText("\r\n\r\n====================== Conflict(s)\r\n");
                    foreach (string errorStr in mConflictErrors)
                        this.OutputLog.AppendText(errorStr + "\r\n");
                    switch (mScmType)
                    {
                        case tScmTypes.svn:
                            MessageBox.Show("Conflicts must be resolved from Windows Explorer using the TortoiseSVN Right-Click commands. If you need help, just ask.", "SVN Conflict(s) while updating.");
                            break;
                        case tScmTypes.p4:
                            MessageBox.Show("Conflicts must be resolved from P4V. If you need help, just ask.", "P4 Conflict(s) while updating.");
                            break;
                    }
                }
                else
                {
                    switch (mScmType)
                    {
                        case tScmTypes.svn:
                            MessageBox.Show("Unknown SVN sync error, ask for help.", "SVN Error(s) while updating.");
                            break;
                        case tScmTypes.p4:
                            MessageBox.Show("Unknown P4 sync error, ask for help.", "P4 Error(s) while updating.");
                            break;
                    }
                }
            }
            else if (copyError)
            {
                // First check that the actual project folder exists on the remote machine
                // because if it doesn't, something is wrong with the build machine
                bool isBuilding = false;
                if (Directory.Exists(@"\\" + mBuildMachineName + @"\" + mProjNameVar))
                    isBuilding = !Directory.Exists(@"\\" + mBuildMachineName + @"\" + mProjNameVar + @"\Project\Builds\Current\Game");

                if (isBuilding)
                    MessageBox.Show("A clean build is likely being done on the build machine. Try to obtain assets again after the current build completes.", "Clean Build is running!");
                else
                    MessageBox.Show("The Build Server is probably down. Ask for help.", "Error(s) copying files from Build Server");
            }
            else if (assetGenError)
                MessageBox.Show("If you encounter problems running the game, these errors may be the cause, so ask for help.", "Error(s) running AssetGen");
            else
                Application.Exit(); // Only exit on good cases.
        }

        private void fTick(object sender, System.EventArgs eArgs)
        {
            if (fStageComplete())
            {
                fAdvanceStage();
                fLaunchStage();
            }
        }

        private System.Diagnostics.Process fShellCommand(string cmd, string args)
        {
            return fSpawnProcess(@"cmd.exe ", @"/D /c " + cmd + " " + args, "");
        }

        private delegate void AddListBoxItemDelegate(string item);
        private void fAddToOutputLog(string item)
        {
            if (item == null)
                return;

            if (this.OutputLog.InvokeRequired)
            {
                // Pass the invocation to the real thread (I guess?).
                this.OutputLog.Invoke(new AddListBoxItemDelegate(this.fAddToOutputLog), item);
            }
            else
            {
                switch (mScmType)
                {
                    case tScmTypes.svn:
                        {
                            // Eat these lines because the user can't respond to this through input.
                            if (item.Contains("Select: (p) postpone, (df) diff-full, (e) edit,")
                                || item.Contains(" (mc) mine-conflict, (tc) theirs-conflict,")
                                || item.Contains("        (s) show all options: "))

                                return;

                            // This is the real thread.
                            this.OutputLog.AppendText(item + "\r\n");

                            if (item.Contains("Conflict discovered"))
                            {
                                mFailures[(int)mSyncStage] = true;
                                mCurrentStageProcess.StandardInput.WriteLine("p");
                                mConflictErrors.Add(item);
                            }
                        }
                        break;

                    case tScmTypes.p4:
                        {
                            this.OutputLog.AppendText(item + "\r\n");
                            if (item.Contains("must resolve") || item.Contains("Submit failed"))
                            {
                                mFailures[(int)mSyncStage] = true;
                                mCurrentStageProcess.StandardInput.WriteLine("p");
                                mConflictErrors.Add(item);
                            }
                        }
                        break;
                }
            }
        }

        private void OutputReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            fAddToOutputLog(e.Data);
        }

        private System.Diagnostics.Process fSpawnProcess(string cmd, string args, string workingDir)
        {
            System.Diagnostics.Process spawnedProc = new System.Diagnostics.Process();
            spawnedProc.StartInfo.FileName = cmd;
            spawnedProc.StartInfo.Arguments = args;
            spawnedProc.StartInfo.UseShellExecute = false;
            spawnedProc.StartInfo.RedirectStandardOutput = true;
            spawnedProc.StartInfo.RedirectStandardInput = true;
            spawnedProc.StartInfo.RedirectStandardError = true;
            spawnedProc.StartInfo.CreateNoWindow = true;
            spawnedProc.StartInfo.WorkingDirectory = workingDir;

            spawnedProc.OutputDataReceived += OutputReceived;
            spawnedProc.ErrorDataReceived += OutputReceived;

            spawnedProc.Start();

            spawnedProc.BeginOutputReadLine();
            spawnedProc.BeginErrorReadLine();

            return spawnedProc;
        }

        private void fFormClosing(object sender, FormClosingEventArgs e)
        {
            if (!mCurrentStageProcess.HasExited)
            {
                // Try to close out the sync process since the user wants to cancel the sync.
                // Does it do this automatically?
                mCurrentStageProcess.CloseMainWindow();
            }
        }

        private void GetLatest_Shown(object sender, EventArgs e)
        {
            // Check for any open processes that could conflict with this.
            List<string> procsOpen = new List<string>();

            System.Diagnostics.Process[] foundProcs = null;

            foundProcs = System.Diagnostics.Process.GetProcessesByName("maya");
            foreach (System.Diagnostics.Process proc in foundProcs)
                procsOpen.Add(proc.ProcessName);

            foundProcs = System.Diagnostics.Process.GetProcessesByName("SigScript");
            foreach (System.Diagnostics.Process proc in foundProcs)
                procsOpen.Add(proc.ProcessName);

            foundProcs = System.Diagnostics.Process.GetProcessesByName("SigAnim");
            foreach (System.Diagnostics.Process proc in foundProcs)
                procsOpen.Add(proc.ProcessName);

            foundProcs = System.Diagnostics.Process.GetProcessesByName("SigFx");
            foreach (System.Diagnostics.Process proc in foundProcs)
                procsOpen.Add(proc.ProcessName);

            foundProcs = System.Diagnostics.Process.GetProcessesByName("SigEd");
            foreach (System.Diagnostics.Process proc in foundProcs)
                procsOpen.Add(proc.ProcessName);

            foundProcs = System.Diagnostics.Process.GetProcessesByName("SigAtlas");
            foreach (System.Diagnostics.Process proc in foundProcs)
                procsOpen.Add(proc.ProcessName);

            foundProcs = System.Diagnostics.Process.GetProcessesByName("Shade");
            foreach (System.Diagnostics.Process proc in foundProcs)
                procsOpen.Add(proc.ProcessName);

            foundProcs = System.Diagnostics.Process.GetProcessesByName("ProjectSelector");
            foreach (System.Diagnostics.Process proc in foundProcs)
                procsOpen.Add(proc.ProcessName);

            foundProcs = System.Diagnostics.Process.GetProcessesByName("AssetGen");
            foreach (System.Diagnostics.Process proc in foundProcs)
                procsOpen.Add(proc.ProcessName);

            if (procsOpen.Count > 0)
            {
                const string cMessageBoxTitle = "Close Conflicting Applications";
                string display = "Please close the following applications before updating:\n";
                foreach (string proc in procsOpen)
                    display += proc + "\n";

                MessageBox.Show(display, cMessageBoxTitle);
                Application.Exit();
                return;
            }

            // Force xbWatson to be closed
            System.Diagnostics.Process[] watsonProcs = System.Diagnostics.Process.GetProcessesByName("xbWatson");
            foreach (System.Diagnostics.Process proc in watsonProcs)
                proc.Kill();

            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(fFormClosing);

            mProjNameVar = System.Environment.GetEnvironmentVariable("SigCurrentProjectName");
            mProjPathVar = System.Environment.GetEnvironmentVariable("SigCurrentProject");
            mEngPathVar = System.Environment.GetEnvironmentVariable("SigEngine");
            mBuildMachineName = ("build-" + mProjNameVar).ToLower( );
            this.OutputLog.AppendText("Getting latest from server: [" + mBuildMachineName + "] \r\n");

            // Switch SCM types passed on arguments being passed in
            string[] args = Environment.GetCommandLineArgs();
            if (args.Length > 0)
            {
                foreach (string arg in args)
                {
                    if (arg == "p4")
                        mScmType = tScmTypes.p4;
                }
            }

            try
            {
                System.IO.FileInfo fileInfo = new System.IO.FileInfo(mProjPathVar + "\\cleangame");
                fileInfo.Refresh();
                mCleanGameLastWriteTime = fileInfo.LastWriteTime;
            }
            catch
            {
                mCleanGameLastWriteTime = new System.DateTime( );
            }

            // Launch initial stage.
            fLaunchStage();

            // Launch timer to handle progressing through different sync stages.
            Timer t = new Timer( );
            t.Interval = 1;
            t.Tick += new System.EventHandler( this.fTick );
            t.Start( ) ;
        }
    }
}
