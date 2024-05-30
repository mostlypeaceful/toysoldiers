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
        private const string dumpDir = @"\\signal\share\SignalServer\crashdumps";
        private const string fogbugzApiUrl = @"http://signal/fogbugz/api.asp";
        private static int callCount = 0;
        
        private string fogbugzToken = "";
        private LoginPrompt fogbugzLoginPrompt = new LoginPrompt();

        private DebugMonitorForm parentForm;

        BackgroundWorker submitWorker = new BackgroundWorker();

        private delegate void VoidDelegate();
        private delegate void LogSaveFileDelegate(String stream, RichTextBoxStreamType streamType);
  
        private void DisableSubmitButton()
        {
            if (submitButton.InvokeRequired)
                // Calls itself on the thread that can actually update it
                submitButton.Invoke(new VoidDelegate(DisableSubmitButton));
            else
                submitButton.Enabled = false;
        }

        private void EnableSubmitButton()
        {
            if (submitButton.InvokeRequired)
                // Calls itself on the thread that can actually update it
                submitButton.Invoke(new VoidDelegate(EnableSubmitButton));
            else
                submitButton.Enabled = true;
        }

        public Dumpy(DebugMonitorForm parentForm)
        {
            InitializeComponent();

            this.parentForm = parentForm;

            submitWorker.DoWork += new DoWorkEventHandler(Submit);
            submitWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(SubmitCompleted);

            ResetState();
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

        private void CopyConsoleFile(string consolePath, string sharePath, string filename)
        {
            string consoleFilePath = consolePath + "\\" + filename;
            string shareFilePath = sharePath + "\\" + filename;

            bool fileFound = false;
            XDevkit.IXboxFiles projectFileList = parentForm.Monitor.XboxConsole.DirectoryFiles(consolePath);
            foreach (XDevkit.IXboxFile file in projectFileList)
            {
                if (String.Compare(consoleFilePath, file.Name, true) == 0)
                {
                    fileFound = true;
                    break;
                }
            }
            if (fileFound)
            {
                UpdateStatus("Copying " + filename + " from console...");
                parentForm.Monitor.XboxConsole.ReceiveFile(shareFilePath, consoleFilePath);
            }
        }

        private void Submit(object sender, DoWorkEventArgs e)
        {
            List<object> submitArgs = (List<object>)e.Argument;
            int logTextLength = (int)submitArgs[0];
            string descText = (string)submitArgs[1];

            DisableSubmitButton();

            string consoleName = parentForm.Monitor.XboxConsole.Name;
            string projectName = GetActiveProject();
            string consolePath = @"DEVKIT:\" + projectName;

            DateTime now = DateTime.Now;
            string date = now.ToString("MM-dd-yy");
            string time = now.ToString("H.mm");

            string userDir = dumpDir + @"\" + consoleName;
            string dateDir = userDir + @"\" + date;
            string timeDir = dateDir + @"\" + time;

            // Create a structure of <console_name>\<date>\<time>
            if (!Directory.Exists(dumpDir))
                throw (new DirectoryNotFoundException());
            if (!Directory.Exists(dateDir))
                Directory.CreateDirectory(dateDir);
            if (!Directory.Exists(timeDir))
                Directory.CreateDirectory(timeDir);

            string sharePath = timeDir;

            // Copy the DMP file
            string shareDumpPath = sharePath + @"\dump.dmp";
            UpdateStatus("Copying DMP file from console...");
            parentForm.Monitor.SaveMinidump(shareDumpPath, true);

            CopyConsoleFile(consolePath, sharePath, "default.exe");
            CopyConsoleFile(consolePath, sharePath, "default.xex");
            CopyConsoleFile(consolePath, sharePath, "default.pdb");
            CopyConsoleFile(consolePath, sharePath, "default.xdb");

            // Create the description text file
            string shareDescPath = sharePath + @"\desc.txt";
            if (descTextBox.Text != "")
            {
                UpdateStatus("Creating description text file...");
                using (TextWriter writer = new StreamWriter(shareDescPath))
                    writer.Write(descTextBox.Text);
            }

            string shareDebugLogPath = sharePath + @"\debugLog.rtf";

            if (logTextLength > 0)
            {
                if (parentForm.Log.InvokeRequired)
                    parentForm.Log.Invoke(new LogSaveFileDelegate(parentForm.Log.SaveFile), shareDebugLogPath, RichTextBoxStreamType.RichText);
                else
                    parentForm.Log.SaveFile(shareDebugLogPath, RichTextBoxStreamType.RichText);
            }

            // Check for FogBugz case links
            if (fogbugzIDTextBox.Text != "")
            {
                if (LoginToFogBugz())
                {
                    string idText = fogbugzIDTextBox.Text;
                    bool retry = true;
                    while (retry)
                    {
                        Dictionary<string, string> args = new Dictionary<string, string>();
                        args.Add("cmd", "edit");
                        args.Add("token", fogbugzToken);
                        args.Add("ixBug", idText);
                        args.Add("sEvent", descTextBox.Text + "\n\n" + sharePath);
                        string response = CallRESTAPIFiles(fogbugzApiUrl, args, null);
                        if (response.Contains("error"))
                        {
                            System.Xml.XmlTextReader reader = new System.Xml.XmlTextReader(new StringReader(response));
                            System.Xml.XPath.XPathDocument doc = new System.Xml.XPath.XPathDocument(reader);
                            System.Xml.XPath.XPathNavigator nav = doc.CreateNavigator();

                            string errorMessage = nav.Evaluate("string(response/error)").ToString();

                            string valueString = "";
                            DialogResult result = InputBox("Error Editing Case", errorMessage += " Enter case number:", ref valueString);
                            if (result == DialogResult.OK)
                                idText = valueString;
                            else
                                retry = false;
                        }
                        else
                            retry = false;
                    }
                }
            }
        }

        void SubmitCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            ResetState();

            MessageBox.Show("Submit complete.");

            Close();
        }

        private void ResetState()
        {
            descTextBox.Text = "";

            UpdateStatus("You must enter a bug description before proceeding.");

            DisableSubmitButton();
        }

        // From the FogBugz APITest C# project
        public string CallRESTAPIFiles(string sURL, Dictionary<string, string> rgArgs, Dictionary<string, byte[]>[] rgFiles)
        {
            if (callCount++ > 20)
                throw(new Exception("Way too many API calls!"));

            string sBoundaryString = getRandomString(30);
            string sBoundary = "--" + sBoundaryString;
            ASCIIEncoding encoding = new ASCIIEncoding();
            UTF8Encoding utf8encoding = new UTF8Encoding();
            HttpWebRequest http = (HttpWebRequest)HttpWebRequest.Create(sURL);
            http.Method = "POST";
            http.AllowWriteStreamBuffering = true;
            http.ContentType = "multipart/form-data; boundary=" + sBoundaryString;
            string vbCrLf = "\r\n";

            Queue parts = new Queue();

            // Add all the normal arguments
            foreach (System.Collections.Generic.KeyValuePair<string, string> i in rgArgs)
            {
                parts.Enqueue(encoding.GetBytes(sBoundary + vbCrLf));
                parts.Enqueue(encoding.GetBytes("Content-Type: text/plain; charset=\"utf-8\"" + vbCrLf));
                parts.Enqueue(encoding.GetBytes("Content-Disposition: form-data; name=\"" + i.Key + "\"" + vbCrLf + vbCrLf));
                parts.Enqueue(utf8encoding.GetBytes(i.Value));
                parts.Enqueue(encoding.GetBytes(vbCrLf));
            }

            // Add all the files
            if (rgFiles != null)
            {
                foreach (Dictionary<string, byte[]> j in rgFiles)
                {
                    parts.Enqueue(encoding.GetBytes(sBoundary + vbCrLf));
                    parts.Enqueue(encoding.GetBytes("Content-Disposition: form-data; name=\""));
                    parts.Enqueue(j["name"]);
                    parts.Enqueue(encoding.GetBytes("\"; filename=\""));
                    parts.Enqueue(j["filename"]);
                    parts.Enqueue(encoding.GetBytes("\"" + vbCrLf));
                    parts.Enqueue(encoding.GetBytes("Content-Transfer-Encoding: base64" + vbCrLf));
                    parts.Enqueue(encoding.GetBytes("Content-Type: "));
                    parts.Enqueue(j["contenttype"]);
                    parts.Enqueue(encoding.GetBytes(vbCrLf + vbCrLf));
                    parts.Enqueue(j["data"]);
                    parts.Enqueue(encoding.GetBytes(vbCrLf));
                }
            }

            parts.Enqueue(encoding.GetBytes(sBoundary + "--"));

            // Calculate the content length
            int nContentLength = 0;
            foreach (Byte[] part in parts)
                nContentLength += part.Length;
            http.ContentLength = nContentLength;

            // Write the post
            Stream stream = http.GetRequestStream();
            string sent = "";
            foreach (Byte[] part in parts)
            {
                stream.Write(part, 0, part.Length);
                sent += encoding.GetString(part);
            }
            stream.Close();

            // Read the result
            Stream r = http.GetResponse().GetResponseStream();
            StreamReader reader = new StreamReader(r);
            string retValue = reader.ReadToEnd();
            reader.Close();

            return retValue;
        }

        // From the FogBugz APITest C# project
        public string getRandomString(int nLength)
        {
            string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXTZabcdefghiklmnopqrstuvwxyz";
            string s = "";
            System.Random rand = new System.Random();
            for (int i = 0; i < nLength; i++)
            {
                int rnum = (int)Math.Floor((double)rand.Next(0, chars.Length - 1));
                s += chars.Substring(rnum, 1);
            }
            return s;
        }

        // From http://www.codeproject.com/dotnet/ContentType.asp
        private string GetMIMEType(string filepath)
        {
            RegistryPermission regPerm = new RegistryPermission(RegistryPermissionAccess.Read, "\\\\HKEY_CLASSES_ROOT");
            FileInfo fi = new FileInfo(filepath);
            RegistryKey classesRoot = Registry.ClassesRoot;
            string dotExt = fi.Extension.ToLower();
            RegistryKey typeKey = classesRoot.OpenSubKey("MIME\\Database\\Content Type");

            foreach (string keyname in typeKey.GetSubKeyNames())
            {
                RegistryKey curKey = classesRoot.OpenSubKey("MIME\\Database\\Content Type\\" + keyname);
                if (curKey.GetValue("Extension") != null && curKey.GetValue("Extension").ToString().ToLower() == dotExt)
                    return keyname;
            }

            return "";
        }

        private bool LoginToFogBugz()
        {
            fogbugzLoginPrompt.username = (string)Application.UserAppDataRegistry.GetValue("FogbugzUsername", "");
            fogbugzLoginPrompt.password = (string)Application.UserAppDataRegistry.GetValue("FogbugzPassword", "");

            // If we don't have saved data, by default make the user decide to START saving it;
            // if we do have saved data, by default make the user decide to STOP saving it
            if (fogbugzLoginPrompt.username == "" || fogbugzLoginPrompt.password == "")
                fogbugzLoginPrompt.rememberMyInfo = false;
            else
                fogbugzLoginPrompt.rememberMyInfo = true;

            DialogResult tryAgain = DialogResult.Yes;
            while (tryAgain == DialogResult.Yes)
            {
                if (fogbugzLoginPrompt.username == "" || fogbugzLoginPrompt.password == "")
                    fogbugzLoginPrompt.ShowDialog();

                Dictionary<string, string> args = new Dictionary<string, string>();
                args.Add("cmd", "logon");
                args.Add("email", fogbugzLoginPrompt.username);
                args.Add("password", fogbugzLoginPrompt.password);

                string result = CallRESTAPIFiles(fogbugzApiUrl, args, null);

                System.Xml.XmlTextReader reader = new System.Xml.XmlTextReader(new StringReader(result));
                System.Xml.XPath.XPathDocument doc = new System.Xml.XPath.XPathDocument(reader);
                System.Xml.XPath.XPathNavigator nav = doc.CreateNavigator();

                fogbugzToken = nav.Evaluate("string(response/token)").ToString();
                if (fogbugzToken == "")
                {
                    tryAgain = MessageBox.Show("Login failed. Try again?", "Login Failed", MessageBoxButtons.YesNo);

                    fogbugzLoginPrompt.username = "";
                    fogbugzLoginPrompt.password = "";
                }
                else
                    tryAgain = DialogResult.No;

                if (fogbugzLoginPrompt.rememberMyInfo)
                {
                    Application.UserAppDataRegistry.SetValue("FogBugzUsername", fogbugzLoginPrompt.username, RegistryValueKind.String);
                    Application.UserAppDataRegistry.SetValue("FogBugzPassword", fogbugzLoginPrompt.password, RegistryValueKind.String);
                }
                else
                {
                    Application.UserAppDataRegistry.DeleteValue("FogBugzUsername", false);
                    Application.UserAppDataRegistry.DeleteValue("FogBugzPassword", false);
                }
            }

            return true;
        }

        private bool LogoutOfFogBugz()
        {
            if (fogbugzToken == "")
                return true;

            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("cmd", "logoff");
            args.Add("token", fogbugzToken);

            string result = CallRESTAPIFiles(fogbugzApiUrl, args, null);

            return true;
        }

        private void descTextBox_TextChanged(object sender, EventArgs e)
        {
            if (descTextBox.Text != "")
                EnableSubmitButton();
            else
                DisableSubmitButton();
        }

        private void fogbugzIDTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            // Don't allow anything but numbers in the FogBugz ID field
            const char Delete = (char)8;
            e.Handled = !Char.IsDigit(e.KeyChar) && e.KeyChar != Delete;
        }

        private void submitButton_Click(object sender, EventArgs e)
        {
            saveMinidumpButton.Enabled = false;
            fogbugzIDTextBox.Enabled = false;
            descTextBox.Enabled = false;

            List<object> submitArgs = new List<object>();
            submitArgs.Add(parentForm.Log.Text.Length);
            submitArgs.Add(descTextBox.Text);
            StartBackgroundWorker(submitWorker, submitArgs);
        }

        private void Dumpy_FormClosing(object sender, FormClosingEventArgs e)
        {
            LogoutOfFogBugz();
        }

        private void CompressFiles(string zipFilename, string[] filesToAdd)
        {
            // Open a stream to the ZIP file that will contain our files
            using (Package zipStream = ZipPackage.Open(zipFilename, FileMode.Create))
            {
                foreach (string filePath in filesToAdd)
                {
                    // Create an area in the ZIP file for the file that's being added
                    string filename = filePath.Substring(filePath.LastIndexOf("\\") + 1);
                    //PackagePart zipFileAreaStream = zipStream.CreatePart(PackUriHelper.CreatePartUri(new Uri(filename, UriKind.Relative)), "binary/octet-stream", CompressionOption.Maximum);
                    PackagePart zipFileAreaStream = zipStream.CreatePart(PackUriHelper.CreatePartUri(new Uri(filename, UriKind.Relative)), "", CompressionOption.Maximum);

                    // Copy the data to the ZIP area we created
                    using (FileStream fileStream = new FileStream(filePath, FileMode.Open, FileAccess.Read))
                        CopyStream(fileStream, zipFileAreaStream.GetStream());
                }
            }
        }

        // From http://msdn.microsoft.com/en-us/library/ms568067.aspx
        private static void CopyStream(Stream source, Stream target)
        {
            const int bufSize = 0x1000;
            byte[] buf = new byte[bufSize];
            int bytesRead = 0;
            while ((bytesRead = source.Read(buf, 0, bufSize)) > 0)
                target.Write(buf, 0, bytesRead);
        }

        /// <summary>
        /// Worker thread for saving a minidump.
        /// </summary>
        private void SaveMinidumpThread()
        {
            parentForm.Monitor.SaveMinidump(saveFileDialog.FileName, true);
        }

        private void saveMinidumpButton_Click(object sender, EventArgs e)
        {
            DialogResult result = saveFileDialog.ShowDialog(this);
            if (result == DialogResult.OK)
            {
                // Spawn a thread to save the dump.
                new Thread(new ThreadStart(SaveMinidumpThread)).Start();
            }
        }

        public static DialogResult InputBox(string title, string promptText, ref string value)
        {
            Form form = new Form();
            Label label = new Label();
            TextBox textBox = new TextBox();
            Button buttonOk = new Button();
            Button buttonCancel = new Button();

            form.Text = title;
            label.Text = promptText;
            textBox.Text = value;

            buttonOk.Text = "OK";
            buttonCancel.Text = "Cancel";
            buttonOk.DialogResult = DialogResult.OK;
            buttonCancel.DialogResult = DialogResult.Cancel;

            label.SetBounds(9, 20, 372, 13);
            textBox.SetBounds(12, 36, 372, 20);
            buttonOk.SetBounds(228, 72, 75, 23);
            buttonCancel.SetBounds(309, 72, 75, 23);

            label.AutoSize = true;
            textBox.Anchor = textBox.Anchor | AnchorStyles.Right;
            buttonOk.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            buttonCancel.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;

            form.ClientSize = new Size(396, 107);
            form.Controls.AddRange(new Control[] { label, textBox, buttonOk, buttonCancel });
            form.ClientSize = new Size(Math.Max(300, label.Right + 10), form.ClientSize.Height);
            form.FormBorderStyle = FormBorderStyle.FixedDialog;
            form.StartPosition = FormStartPosition.CenterScreen;
            form.MinimizeBox = false;
            form.MaximizeBox = false;
            form.AcceptButton = buttonOk;
            form.CancelButton = buttonCancel;

            DialogResult dialogResult = form.ShowDialog();
            value = textBox.Text;
            return dialogResult;
        }

        private void StartBackgroundWorker(BackgroundWorker worker, object argument)
        {
            // We can't make a single background worker do two things at once
            if (worker.IsBusy)
                return;

            worker.RunWorkerAsync(argument);
        }
    }
}
