using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.IO.Packaging;
using System.Net;
using System.Security.Permissions;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using Microsoft.Win32;

namespace Atg.Samples.xbWatson.Forms
{
    public partial class Dumpy : Form
    {
        private const string dumpDir = @"\\Shares\Shared\QA";

        private DebugMonitorForm parentForm;

        BackgroundWorker submitWorker = new BackgroundWorker();

        private delegate void VoidDelegate();
        private delegate void LogSaveFileDelegate(String stream, RichTextBoxStreamType streamType);

        public Dumpy(DebugMonitorForm parentForm)
        {
            InitializeComponent();

            this.parentForm = parentForm;

            submitWorker.DoWork += new DoWorkEventHandler(Submit);
            submitWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(SubmitCompleted);
            submitWorker.WorkerSupportsCancellation = true;

            List<object> submitArgs = new List<object>();
            submitArgs.Add(parentForm.Log.Text.Length);
            StartBackgroundWorker(submitWorker, submitArgs);
        }

        private delegate void UpdateStatusDelegate(string text);
        private void UpdateStatus(string text)
        {
            if (statusLabel.InvokeRequired)
                // Calls itself on the thread that can actually update it
                statusLabel.Invoke(new UpdateStatusDelegate(UpdateStatus), text);
            else
            {
                statusLabel.Text = text;
                statusLabel.Update();
            }
        }

        private string GetActiveProject()
        {
            // Find the active project
            string activeProject = "";
            string projectProfiles = (string)Registry.GetValue(@"HKEY_CURRENT_USER\Software\SignalStudios\ProjectProfiles", "", "");
            if (projectProfiles != "")
            {
                string[] projects = projectProfiles.Split(";".ToCharArray());
                foreach (string project in projects)
                {
                    string active = (string)Registry.GetValue(@"HKEY_CURRENT_USER\Software\SignalStudios\ProjectProfiles\" + project, "ActiveProfile", "0");
                    if (active == "1")
                    {
                        activeProject = project;
                        break;
                    }
                }
            }
            if (activeProject == "")
                throw (new Exception("No active project found."));

            return activeProject;
        }

        private void Submit(object sender, DoWorkEventArgs e)
        {
            List<object> submitArgs = (List<object>)e.Argument;
            int logTextLength = (int)submitArgs[0];

            string consoleName = parentForm.Monitor.XboxConsole.Name;
            string projectName = GetActiveProject();
            string consolePath = @"DEVKIT:\" + projectName;

            DateTime now = DateTime.Now;
            string date = now.ToString("MM-dd-yy");
            string time = now.ToString("H.mm");

            string projDir = dumpDir + "\\" + projectName + "\\Crashdumps";
            string userDir = projDir + "\\" + consoleName;
            string dateDir = userDir + "\\" + date;
            string timeDir = dateDir + "\\" + time;

            // Create a structure of <project>\<console_name>\<date>\<time>
            if (!Directory.Exists(dumpDir))
                throw (new DirectoryNotFoundException());
            if (!Directory.Exists(dateDir))
                Directory.CreateDirectory(dateDir);
            if (!Directory.Exists(timeDir))
                Directory.CreateDirectory(timeDir);

            string sharePath = timeDir;

            // The progress bar can only be updated by the thread
            // it was created on
            progressBar.BeginInvoke((MethodInvoker)delegate()
            {
                progressBar.Minimum = 1;
                progressBar.Maximum = 3;
                progressBar.Value = 1;
                progressBar.Step = 1;
            });

            // Copy the dump file
            string shareDumpPath = sharePath + @"\dump.dmp";
            UpdateStatus("Copying DMP file from console...");
            parentForm.Monitor.SaveMinidump(shareDumpPath, ((MainForm) parentForm.ParentForm).SaveHeapWithMinidump );
            progressBar.BeginInvoke((MethodInvoker)delegate()
            {
                progressBar.PerformStep();
            });

            // Copy the log file
            string shareDebugLogPath = sharePath + @"\debugLog.rtf";
            if (logTextLength > 0)
            {
                if (parentForm.Log.InvokeRequired)
                    parentForm.Log.Invoke(new LogSaveFileDelegate(parentForm.Log.SaveFile), shareDebugLogPath, RichTextBoxStreamType.RichText);
                else
                    parentForm.Log.SaveFile(shareDebugLogPath, RichTextBoxStreamType.RichText);
            }
            progressBar.BeginInvoke((MethodInvoker)delegate()
            {
                progressBar.PerformStep();
            });
            
            // Save a screenshot
            UpdateStatus("Saving screen capture...");
            parentForm.Monitor.XboxConsole.ScreenShot(sharePath + @"\capture.bmp");
            progressBar.BeginInvoke((MethodInvoker)delegate()
            {
                progressBar.PerformStep();
            });
        }

        void SubmitCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            Close();
        }

        private void StartBackgroundWorker(BackgroundWorker worker, object argument)
        {
            // We can't make a single background worker do two things at once
            if (worker.IsBusy)
                return;

            worker.RunWorkerAsync(argument);
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            submitWorker.CancelAsync();
        }
    }
}
