using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Xml;

namespace CheckIn
{
    public partial class CheckInForm : Form
    {
        IgnoredForm ignoredForm = new IgnoredForm(GetWorkspaceName());
        RecentMessagesForm recentMessages = new RecentMessagesForm(GetWorkspaceName());
        Point clickedPoint;
        BackgroundWorker listWorker = new BackgroundWorker();
        BackgroundWorker submitWorker = new BackgroundWorker();
        string changelist = "";

        public CheckInForm()
        {
            InitializeComponent();

            UpdateStatus("Loading...");

            UpdateChangesList();

            listWorker.DoWork += new DoWorkEventHandler(InitializeLists);
            listWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(InitializeListsCompleted);

            submitWorker.DoWork += new DoWorkEventHandler(SubmitFiles);
            submitWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(SubmitFilesCompleted);
        }

        private delegate void UpdateStatusDelegate(string message);
        private void UpdateStatus(string message)
        {
            if (statusLabel.InvokeRequired)
                statusLabel.Invoke(new UpdateStatusDelegate(UpdateStatus), message);
            else
            {
                statusLabel.Text = message;
                statusLabel.Update();
            }
        }

        private void DisableButtons()
        {
            modifiedFilesCheckBox.Enabled = false;
            addedFilesCheckBox.Enabled = false;
            deletedFilesCheckBox.Enabled = false;

            ignoredButton.Enabled = false;
            submitButton.Enabled = false;
            searchAdvancedLocationsCheckBox.Enabled = false;

            changesComboBox.Enabled = false;
        }

        private void EnableButtons()
        {
            modifiedFilesCheckBox.Enabled = true;
            addedFilesCheckBox.Enabled = true;
            deletedFilesCheckBox.Enabled = true;

            ignoredButton.Enabled = true;
            submitButton.Enabled = true;
            searchAdvancedLocationsCheckBox.Enabled = true;

            if (changesComboBox.Items.Count > 1)
                changesComboBox.Enabled = true;
        }

        private List<string> CheckForAdvancedOperations()
        {
            List<string> filesWithAdvancedOperations = new List<string>();
            List<string> pending = GetPendingChangelistNumbers();

            // The default changelist is never returned, so we need to
            // do a special check to see if it contains pending submits
            P4Process process = new P4Process("opened -C " + GetWorkspaceName() + " -c default");
            process.Start();
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
            {
                string line = process.StandardOutput.ReadLine();
                string data = GetTagData(line, "depotFile");
                if (data != "")
                {
                    pending.Add("default");
                    break;
                }
            }

            foreach (string changelist in pending)
            {
                process = new P4Process("opened -c " + changelist);
                process.Start();
                string file = "";
                while (!process.StandardOutput.EndOfStream || !process.HasExited)
                {
                    // When we hit a blank line, we've moved on to a new file
                    string line = process.ReadLine(process.StandardOutput);
                    if (line == "")
                    {
                        file = "";
                        continue;
                    }

                    string tagData = GetTagData(line, "clientFile");
                    if (tagData != "")
                        file = tagData;

                    // This program isn't meant to be a full replacement for P4V - users
                    // need to switch to that when they start doing advanced operations
                    // like branching, move/delete, etc
                    tagData = GetTagData(line, "action");
                    if (file != "" && tagData != "" && tagData != "add" && tagData != "edit" && tagData != "delete")
                        filesWithAdvancedOperations.Add(file + " (" + tagData + ")");
                }
            }

            return filesWithAdvancedOperations;
        }

        private delegate void HandleExceptionDelegate(Exception e, bool exit);
        private delegate void HandleExceptionStringDelegate(string message, bool exit);

        public static string GetTagData(string line, string tagName)
        {
            Regex pattern = new Regex(@"^\.\.\. " + tagName + " (.+)\r*$", RegexOptions.Multiline);
            return pattern.Match(line).Groups[1].ToString().Trim();
        }

        private void InitializeLists(object sender, DoWorkEventArgs e)
        {
            UpdateStatus("Processing directories...");

            List<List<string>> stringList = new List<List<string>>();
            List<string> modified = new List<string>();
            List<string> added = new List<string>();
            List<string> deleted = new List<string>();

            string projectDir = Environment.GetEnvironmentVariable("SigCurrentProject");
            string engineDir = Environment.GetEnvironmentVariable("SigEngine");
            if (!searchAdvancedLocationsCheckBox.Checked)
                engineDir += "\\Src\\Internal";

            string workspaceName = GetWorkspaceName();
            string workspaceDir = GetWorkspaceDirectory();

            // Get a list of all files in the current project
            List<string> fileList = new List<string>();
            List<string> ignoredList = ignoredForm.GetIgnoredList();
            ProcessDirectory(projectDir, ref fileList, ref ignoredList);
            ProcessDirectory(engineDir, ref fileList, ref ignoredList);

            // Find the modified file list
            P4Process process = new P4Process("opened -c " + changelist);
            process.Start();
            string file = "";
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
            {
                // When we hit a blank line, we've moved on to a new file
                string line = process.ReadLine(process.StandardOutput);
                if (line == "")
                {
                    file = "";
                    continue;
                }

                string tagData = GetTagData(line, "clientFile");
                if (tagData != "")
                    file = tagData;

                tagData = GetTagData(line, "action");
                if (tagData == "edit" && file != "")
                {
                    file = file.Replace("/", "\\");
                    file = file.Replace("\\\\" + workspaceName, workspaceDir);

                    UpdateStatus(file);
                    modified.Add(file);
                }
            }

            // Get a list of what the server thinks the workspace has
            // and compare it to the list we have from the local system
            process = new P4Process("have \"" + projectDir + "\\...\" \"" + engineDir + "\\...\"");
            process.Start();
            int fileCount = 0;
            file = "";
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
            {
                string line = process.ReadLine(process.StandardOutput);
                string tagData = GetTagData(line, "clientFile");
                if (tagData == "")
                    continue;

                file = tagData.Replace("/", "\\");
                file = file.Replace("\\\\" + workspaceName, workspaceDir);

                fileCount++;
                if ((fileCount % 50) == 0)
                    UpdateStatus(file);

                if (!File.Exists(file) && !modified.Contains(file))
                    deleted.Add(file);

                // Do a case-insensitive comparison when finding what to remove
                for (int i = 0; i < fileList.Count; i++)
                {
                    string fileListStr = fileList[i];
                    if (String.Compare(fileListStr, file, true) == 0)
                    {
                        fileList.RemoveAt(i);
                        break;
                    }
                }
            }

            // Whatever is left after the list comparison is what needs
            // to be added to the depot
            added.AddRange(fileList);

            stringList.Add(added);
            stringList.Add(modified);
            stringList.Add(deleted);
            e.Result = stringList;
        }

        void InitializeListsCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Error != null)
                statusLabel.Invoke(new HandleExceptionDelegate(Program.HandleException), e.Error, false);
            else if (e.Cancelled)
                return;

            List<string> filesWithAdvancedOperations = CheckForAdvancedOperations();
            if (filesWithAdvancedOperations.Count > 0)
            {
                string message = "The following files are marked for advanced operations not properly supported by CheckIn:\n";
                foreach (string advancedFile in filesWithAdvancedOperations)
                    message += "\n" + advancedFile;
                message += "\n\nPlease use P4V to submit this changelist!";

                UpdateStatus("Advanced operations present. Please use P4V to resolve!");
                MessageBox.Show(message);

                return;
            }

            List<List<string>> stringList = (List<List<string>>)e.Result;
            if (stringList == null)
                return;

            foreach (string file in stringList[0])
            {
                if (addedFilesCheckBox.Checked)
                    addedList.Items.Add(file).Checked = true;
                else
                    addedList.Items.Add(file).Checked = false;
            }

            foreach (string file in stringList[1])
            {
                if (modifiedFilesCheckBox.Checked)
                    modifiedList.Items.Add(file).Checked = true;
                else
                    modifiedList.Items.Add(file).Checked = false;
            }

            foreach (string file in stringList[2])
            {
                if (deletedFilesCheckBox.Checked)
                    deletedList.Items.Add(file).Checked = true;
                else
                    deletedList.Items.Add(file).Checked = false;
            }

            EnableButtons();

            UpdateStatus("All files processed.");

            if (changesComboBox.Items.Count > 1)
                multipleChangelistsToolTip.Show("Multiple changelists are present!", changesComboBox, changesComboBox.Width, -(changesComboBox.Height * 2), 3000);
        }

        private void CheckInForm_Load(object sender, EventArgs e)
        {
            string[] args = Environment.GetCommandLineArgs();

            bool readOnly = false;
            foreach (string arg in args)
            {
                if (arg == "readonly")
                {
                    UpdateStatus("Read-Only Mode!");
                    readOnly = true;
                }
            }

            if (readOnly)
                changelistDescTextBox.Enabled = false;
            else
                StartCheckInBackgroundWorker(listWorker);
        }

        public bool PathIsIgnored(string targetPath, ref List<string> ignoredList)
        {
            bool ignored = false;
            foreach (string pattern in ignoredList)
            {
                if (Regex.Match(targetPath, pattern, RegexOptions.IgnoreCase).Success)
                {
                    ignored = true;
                    break;
                }
            }

            return ignored;
        }

        public void ProcessDirectory(string targetDirectory, ref List<string> fileList, ref List<string> ignoredList)
        {
            if (PathIsIgnored(targetDirectory, ref ignoredList))
                return;

            // Process the list of files found in the directory
            string[] fileEntries = Directory.GetFiles(targetDirectory);
            foreach (string fileName in fileEntries)
            {
                // The directory name's been checked and is OK, but
                // before adding any file from the directory, it also
                // needs to be checked
                if (!PathIsIgnored(fileName, ref ignoredList))
                    fileList.Add(fileName);
            }

            // Recurse into subdirectories of this directory.
            string[] subdirectoryEntries = Directory.GetDirectories(targetDirectory);
            foreach (string subdirectory in subdirectoryEntries)
                ProcessDirectory(subdirectory, ref fileList, ref ignoredList);
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void ignoredButton_Click(object sender, EventArgs e)
        {
            ignoredForm.ShowDialog();

            // Refresh the lists of files if they've updated their ignore list
            if (ignoredForm.okClicked)
                StartCheckInBackgroundWorker(listWorker);
        }

        private void StartCheckInBackgroundWorker(BackgroundWorker worker)
        {
            StartCheckInBackgroundWorker(worker, null);
        }

        private void StartCheckInBackgroundWorker(BackgroundWorker worker, object argument)
        {
            // Don't launch any other threads while we're trying to submit
            if (submitWorker.IsBusy)
                return;

            // We can't make a single background worker do two things at once
            if (worker.IsBusy)
                return;

            DisableButtons();

            // Clear the lists before initializing them again
            addedList.Items.Clear();
            modifiedList.Items.Clear();
            deletedList.Items.Clear();

            worker.RunWorkerAsync(argument);
        }

        private string MapLocalFileToDepotFile(string localFile)
        {
            string depotFile = "";

            P4Process process = new P4Process("where \"" + localFile + "\"");
            process.Start();
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
            {
                string line = process.ReadLine(process.StandardOutput);
                string tagData = GetTagData(line, "depotFile");
                if (tagData != "")
                {
                    depotFile = tagData;
                    break;
                }
            }

            return depotFile;
        }

        private string MapDepotFileToLocalFile(string depotFile)
        {
            string localFile = "";

            P4Process process = new P4Process("where \"" + depotFile + "\"");
            process.Start();
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
            {
                string line = process.ReadLine(process.StandardOutput);
                string tagData = GetTagData(line, "path");
                if (tagData != "")
                {
                    localFile = tagData;
                    break;
                }
            }

            return localFile;
        }

        private void SubmitFiles(object sender, DoWorkEventArgs e)
        {
            List<string> errors;

            List<List<string>> submitLists = (List<List<string>>)e.Argument;
            List<string> exclusionList = submitLists[0];
            List<string> addedList = submitLists[1];
            List<string> deletedList = submitLists[2];
            string desc = submitLists[3][0];

            // From http://www.perforce.com/perforce/doc.091/manuals/p4guide/03_using.html:
            // When you enter information into a Perforce form, observe the following rules:
            // * Field names (for example, View:) must be flush left (not indented) and must end with a colon.
            // * Values (your entries) must be on the same line as the field name, or indented with tabs on the lines beneath the field name.
            desc = desc.Replace("\n", "\n\t");
            desc += "\n";

            // Create a modified changelist minus the unchecked modified files
            string updatedChangelist = "";
            bool pastFilesLine = false;
            Process changeProcess = new Process();
            changeProcess.StartInfo.UseShellExecute = false;
            changeProcess.StartInfo.RedirectStandardOutput = true;
            changeProcess.StartInfo.RedirectStandardError = true;
            changeProcess.StartInfo.RedirectStandardInput = true;
            changeProcess.StartInfo.CreateNoWindow = true;
            changeProcess.StartInfo.FileName = "p4.exe";
            changeProcess.StartInfo.Arguments = "change -o";
            if (changelist != "default")
                changeProcess.StartInfo.Arguments += " " + changelist;
            changeProcess.Start();
            while (!changeProcess.StandardOutput.EndOfStream || !changeProcess.HasExited)
            {
                string line = changeProcess.StandardOutput.ReadLine();

                // Always add the blank lines back in
                if (line != "")
                {
                    if (pastFilesLine)
                    {
                        bool removeLine = false;
                        foreach (string file in exclusionList)
                        {
                            string mappedFile = MapLocalFileToDepotFile(file);
                            if (line.ToLower().Contains(mappedFile.ToLower()))
                            {
                                UpdateStatus("Removing " + file + " from changelist...");

                                removeLine = true;
                                break;
                            }
                        }
                        if (removeLine)
                            continue;
                    }
                    else
                    {
                        if (line == "Files:")
                            pastFilesLine = true;

                        // The description tag changes depending on whether it's
                        // a default or numbered list
                        if (line.Contains("<enter description here>"))
                            line = line.Replace("<enter description here>", desc);
                        else if (line.Contains("<saved by Perforce>"))
                            line = line.Replace("<saved by Perforce>", desc);
                    }
                }

                updatedChangelist += line + "\r\n";
            }

            // Write the modified changelist and get a changelist number back
            string changeListNumber = "";
            P4Process inputProcess = new P4Process("change -i");
            if (changelist != "default")
                changeProcess.StartInfo.Arguments += " " + changelist;
            inputProcess.Start();
            inputProcess.StandardInput.Write(updatedChangelist.ToCharArray());
            inputProcess.StandardInput.Close();
            while (!inputProcess.StandardOutput.EndOfStream || !inputProcess.HasExited)
            {
                string line = inputProcess.ReadLine(inputProcess.StandardOutput);
                changeListNumber = Regex.Match(line, @"\d+").Value;
            }
            if (changeListNumber == "")
                throw (new Exception("Could not get a numeric changelist label!\n" + updatedChangelist));

            P4Process process = new P4Process("");
            foreach (string file in addedList)
            {
                UpdateStatus("Notifying the server that " + file + " has been added.");

                process.StartInfo.Arguments = "add -c " + changeListNumber + " \"" + file + "\"";
                process.Start();
                errors = new List<string>();
                while (!process.StandardError.EndOfStream || !process.HasExited)
                {
                    string line = process.ReadLine(process.StandardError);
                    if (line != "")
                        errors.Add(line);
                }
                if (errors.Count > 0)
                {
                    string message = "Could not open " + file + " for adding:";
                    foreach (string error in errors)
                        message += "\r\n" + error;

                    throw (new Exception(message));
                }
            }

            foreach (string file in deletedList)
            {
                UpdateStatus("Notifying the server that " + file + " has been deleted.");

                process.StartInfo.Arguments = "delete -c " + changeListNumber + " \"" + file + "\"";
                process.Start();
                errors = new List<string>();
                while (!process.StandardError.EndOfStream || !process.HasExited)
                {
                    string line = process.ReadLine(process.StandardError);
                    if (line != "")
                        errors.Add(line);
                }
                if (errors.Count > 0)
                {
                    string message = "Could not open " + file + " for deleting:";
                    foreach (string error in errors)
                        message += "\r\n" + error;

                    throw (new Exception(message));
                }
            }

            UpdateStatus("Submitting changes to the server...");
            process.StartInfo.Arguments = "submit -c " + changeListNumber;
            process.Start();
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
                UpdateStatus(process.ReadLine(process.StandardOutput));

            errors = new List<string>();
            while (!process.StandardError.EndOfStream)
            {
                string line = process.ReadLine(process.StandardError);
                if (line != "")
                    errors.Add(line);
            }
            if (errors.Count > 0)
            {
                string message = "Could not submit:";
                foreach (string error in errors)
                    message += "\r\n" + error;

                throw (new Exception(message));
            }

            UpdateStatus("All files have been submitted...");
        }

        private void SubmitFilesCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Error == null)
                Close();
            else
            {
                statusLabel.Invoke(new HandleExceptionStringDelegate(Program.HandleException), e.Error.Message, false);
                changelistDescTextBox.Enabled = true;
                StartCheckInBackgroundWorker(listWorker);
            }
        }

        private void submitButton_Click(object sender, EventArgs e)
        {
            if (modifiedList.CheckedIndices.Count <= 0 && addedList.CheckedIndices.Count <= 0 && deletedList.CheckedIndices.Count <= 0)
            {
                MessageBox.Show("There is nothing to submit!");
                return;
            }

            if (changelistDescTextBox.Text == "")
            {
                MessageBox.Show("You must enter a description!");
                return;
            }

            recentMessages.AddMessageToTop(changelistDescTextBox.Text);

            UpdateStatus("Ensuring all local files are ready for submission...");
            List<string> missingFiles = new List<string>();
            bool pastFilesLine = false;
            Process changeProcess = new Process();
            changeProcess.StartInfo.UseShellExecute = false;
            changeProcess.StartInfo.RedirectStandardOutput = true;
            changeProcess.StartInfo.RedirectStandardError = true;
            changeProcess.StartInfo.RedirectStandardInput = true;
            changeProcess.StartInfo.CreateNoWindow = true;
            changeProcess.StartInfo.FileName = "p4.exe";
            changeProcess.StartInfo.Arguments = "change -o";
            if (changelist != "default")
                changeProcess.StartInfo.Arguments += " " + changelist;
            changeProcess.Start();
            while (!changeProcess.StandardOutput.EndOfStream || !changeProcess.HasExited)
            {
                string line = changeProcess.StandardOutput.ReadLine();
                if (line == "")
                    continue;

                if (pastFilesLine)
                {
                    string file = MapDepotFileToLocalFile(line.Split("#".ToCharArray())[0].Trim());
                    string action = line.Split("#".ToCharArray())[1].Trim();
                    UpdateStatus("Checking file " + file + " exists locally...");
                    if (!File.Exists(file) && action != "delete")
                        missingFiles.Add(file);
                }
                else if (line == "Files:")
                    pastFilesLine = true;
            }
            if (missingFiles.Count > 0)
            {
                string message = "The following files are not on your system, but appear in your changelist as an add or edit:\n\n";
                foreach (string file in missingFiles)
                    message += MapDepotFileToLocalFile(file) + "\n";
                message += "\n Do you want to revert these files and continue the submit?";
                DialogResult result = MessageBox.Show(message, "Local File(s) Missing", MessageBoxButtons.OKCancel);
                if (result != DialogResult.OK)
                    return;

                string errors = "";
                foreach (string file in missingFiles)
                {
                    P4Process process = new P4Process("revert \"" + file + "\"");
                    process.Start();
                    process.WaitForExit();
                    string error = process.ReadToEnd(process.StandardError);
                    if (error != null && error != "")
                        errors += "\n\n" + error;
                }
                if (errors != "")
                {
                    statusLabel.Invoke(new HandleExceptionStringDelegate(Program.HandleException), errors, false);
                    return;
                }
            }

            List<List<string>> submitLists = new List<List<string>>();

            // We actually pass the items that we DON'T
            // want to check in because we've already notified
            // the server that they're going to be submitted and
            // now we need to tell it that we aren't
            List<string> excluded = new List<string>();
            foreach (ListViewItem item in modifiedList.Items)
            {
                if (!item.Checked)
                {
                    UpdateStatus("Exclude: " + item.Text);
                    excluded.Add(item.Text);
                }
            }
            submitLists.Add(excluded);

            List<string> added = new List<string>();
            foreach (ListViewItem item in addedList.Items)
            {
                if (item.Checked)
                {
                    UpdateStatus("Add: " + item.Text);
                    added.Add(item.Text);
                }
                else
                {
                    UpdateStatus("Exclude: " + item.Text);
                    excluded.Add(item.Text);
                }
            }
            submitLists.Add(added);

            List<string> deleted = new List<string>();
            foreach (ListViewItem item in deletedList.Items)
            {
                if (item.Checked)
                {
                    UpdateStatus("Delete: " + item.Text);
                    deleted.Add(item.Text);
                }
                else
                {
                    UpdateStatus("Exclude: " + item.Text);
                    excluded.Add(item.Text);
                }
            }
            submitLists.Add(deleted);

            List<string> desc = new List<string>();
            desc.Add(changelistDescTextBox.Text);
            submitLists.Add(desc);

            cancelButton.Enabled = false;
            changelistDescTextBox.Enabled = false;
            StartCheckInBackgroundWorker(submitWorker, submitLists);
        }

        private string GetWorkspaceDirectory()
        {
            string rootDirectory = "";
            try
            {
                P4Process process = new P4Process("client -o");
                process.Start();
                string clientDesc = process.ReadToEnd(process.StandardOutput);
                string errors = process.ReadToEnd(process.StandardError);

                if (errors != "")
                    statusLabel.Invoke(new HandleExceptionStringDelegate(Program.HandleException), errors, false);

                // Pull the root directory out of the client description
                rootDirectory = GetTagData(clientDesc, "Root");

                // Correct the slashes and get rid of any trailing newlines
                rootDirectory = rootDirectory.Replace('/', '\\').Trim();

                if (rootDirectory == "")
                    statusLabel.Invoke(new HandleExceptionStringDelegate(Program.HandleException), "The workspace directory wasn't found.", false);
            }
            catch (Exception e)
            {
                statusLabel.Invoke(new HandleExceptionDelegate(Program.HandleException), e, false);
            }

            return rootDirectory;
        }

        private void addedFilesCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (addedFilesCheckBox.Checked)
            {
                for (int i = 0; i < addedList.Items.Count; i++)
                    addedList.Items[i].Checked = true;
            }
            else
            {
                for (int i = 0; i < addedList.Items.Count; i++)
                    addedList.Items[i].Checked = false;
            }
        }

        private void modifiedFilesCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (modifiedFilesCheckBox.Checked)
            {
                for (int i = 0; i < modifiedList.Items.Count; i++)
                    modifiedList.Items[i].Checked = true;
            }
            else
            {
                for (int i = 0; i < modifiedList.Items.Count; i++)
                    modifiedList.Items[i].Checked = false;
            }
        }

        private void deletedFilesCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (deletedFilesCheckBox.Checked)
            {
                for (int i = 0; i < deletedList.Items.Count; i++)
                    deletedList.Items[i].Checked = true;
            }
            else
            {
                for (int i = 0; i < deletedList.Items.Count; i++)
                    deletedList.Items[i].Checked = false;
            }
        }

        public static string GetP4SetVariable(string variableName)
        {
            P4Process process = new P4Process("set " + variableName);
            process.Start();
            process.WaitForExit();
            string value = process.ReadToEnd(process.StandardOutput);

            string[] splitData = value.Split("=".ToCharArray(), 2);
            if (splitData.Length >= 2)
                value = splitData[1];
            value = value.Replace("(set)", "").Trim();

            return value;
        }

        public static string GetWorkspaceName()
        {
            return GetP4SetVariable("P4CLIENT");
        }

        private void DisplayContextMenu(Form form, Dictionary<string, EventHandler> menuItems)
        {
            ContextMenu contextMenu = new ContextMenu();

            foreach (KeyValuePair<string, EventHandler> pair in menuItems)
            {
                string label = (string)pair.Key;
                EventHandler handler = (EventHandler)pair.Value;

                MenuItem menuItem = new MenuItem(label, handler);
                if (handler == null)
                    menuItem.Enabled = false;

                contextMenu.MenuItems.Add(menuItem);
            }

            contextMenu.Collapse += new EventHandler(contextMenu_Collapse);

            // Assign this context menu to the form's context menu
            form.ContextMenu = contextMenu;
        }

        private void DisplayContextMenu(Form form, EventHandler revertUnchanged, EventHandler revert, EventHandler diff, EventHandler history)
        {
            Dictionary<string, EventHandler> menuItems = new Dictionary<string, EventHandler>();

            menuItems.Add("Revert Unchanged Files", revertUnchanged);
            menuItems.Add("Revert File(s)", revert);
            menuItems.Add("Diff File", diff);
            menuItems.Add("Show History", history);

            DisplayContextMenu(form, menuItems);
        }

        void revertMenuItem_deleted_Click(object sender, EventArgs e)
        {
            List<ListViewItem> files = new List<ListViewItem>();
            foreach (int index in deletedList.SelectedIndices)
                files.Add(deletedList.Items[index]);

            List<ListViewItem> successfulReverts = RevertFiles(files, true);
            foreach (ListViewItem revertedFile in successfulReverts)
                deletedList.Items.Remove(revertedFile);
        }

        void revertMenuItem_modified_Click(object sender, EventArgs e)
        {
            List<ListViewItem> files = new List<ListViewItem>();
            foreach (int index in modifiedList.SelectedIndices)
                files.Add(modifiedList.Items[index]);

            List<ListViewItem> successfulReverts = RevertFiles(files, false);
            foreach (ListViewItem revertedFile in successfulReverts)
                modifiedList.Items.Remove(revertedFile);
        }

        void revertUnchangedMenuItem_modified_Click(object sender, EventArgs e)
        {
            List<ListViewItem> files = new List<ListViewItem>();
            if (modifiedList.SelectedItems.Count > 0)
            {
                foreach (ListViewItem item in modifiedList.SelectedItems)
                    files.Add(item);
            }
            else
            {
                foreach (ListViewItem item in modifiedList.Items)
                    files.Add(item);
            }
            List<ListViewItem> successfulReverts = RevertUnchangedFiles(files);
            foreach (ListViewItem revertedFile in successfulReverts)
                modifiedList.Items.Remove(revertedFile);
        }

        void diffMenuItem_Click(object sender, EventArgs e)
        {
            ListViewItem selected = modifiedList.GetItemAt(clickedPoint.X, clickedPoint.Y);
            if (selected == null)
                return;

            DiffFile(selected.Text);
        }

        void historyMenuItem_Click(object sender, EventArgs e)
        {
            ListViewItem selected = modifiedList.GetItemAt(clickedPoint.X, clickedPoint.Y);
            if (selected == null)
                return;

            HistoryFile(selected.Text);
        }

        void DiffFile(string file)
        {
            string tempFile = Path.GetTempFileName();
            tempFile = Path.ChangeExtension(tempFile, "sigtmp");
            File.Delete(tempFile);

            P4Process process = new P4Process("print -o \"" + tempFile + "\" \"" + file + "\"");
            process.Start();
            process.WaitForExit();

            // This will allow us to delete the file when the program is closed
            File.SetAttributes(tempFile, FileAttributes.Normal);

            string depotFile = null;
            string rev = null;
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
            {
                string line = process.ReadLine(process.StandardOutput);

                string tagData = GetTagData(line, "depotFile");
                if (tagData != "")
                    depotFile = tagData;

                tagData = GetTagData(line, "rev");
                if (tagData != "")
                    rev = tagData;
            }

            string diffProgram = "";
            string altArguments = "";

            // Try getting the P4V diff settings
            string settingsFile = Environment.ExpandEnvironmentVariables(@"%USERPROFILE%\.p4qt\appsettings.xml");
            if (File.Exists(settingsFile))
            {
                XmlDocument settings = new XmlDocument();
                settings.Load(settingsFile);

                // From http://www.csharp-examples.net/xml-nodes-by-attribute-value/:
                // First check if we should use an external program - then get it's path and argument pattern
                XmlNode node = settings.SelectSingleNode("PropertyList/Associations[@varName='Diff Associations']/RunExternal");
                if (node != null && node.InnerText == "true")
                {
                    node = settings.SelectSingleNode("PropertyList/Associations[@varName='Diff Associations']/Association[@varName='Default Association']/Application");
                    if (node != null)
                        diffProgram = node.InnerText;

                    // Allow the use of the argument pattern given to P4V
                    node = settings.SelectSingleNode("PropertyList/Associations[@varName='Diff Associations']/Association[@varName='Default Association']/Arguments");
                    if (node != null)
                        altArguments = node.InnerText;
                }
            }

            // Check the Perforce registry settings
            if (diffProgram == "")
                diffProgram = GetP4SetVariable("P4DIFF");

            // Check the standard Windows environment variables
            if (diffProgram == "")
                diffProgram = Environment.GetEnvironmentVariable("P4DIFF");

            // If we still haven't found another diff program, fall back on the default
            if (diffProgram == null || diffProgram == "")
            {
                process.StartInfo.FileName = "p4merge.exe";
                process.StartInfo.Arguments = "-nl \"" + depotFile + "#" + rev + "\" \"" + tempFile + "\" \"" + file + "\"";
            }
            else
            {
                process.StartInfo.FileName = diffProgram;

                if (altArguments != "")
                {
                    altArguments = altArguments.Replace("%1", "\"" + tempFile + "\"");
                    altArguments = altArguments.Replace("%2", "\"" + file + "\"");
                    process.StartInfo.Arguments = altArguments;
                }
                else
                    process.StartInfo.Arguments = "\"" + tempFile + "\" \"" + file + "\"";
            }

            process.Start();
        }

        void LaunchP4V(string arguments)
        {
            string port = GetP4SetVariable("P4PORT");
            string user = GetP4SetVariable("P4USER");

            Process process = new Process();
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.RedirectStandardInput = true;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.FileName = "p4v";
            process.StartInfo.Arguments = "-p " + port + " -c " + GetWorkspaceName() + " -u " + user;
            if (arguments != null && arguments != "")
                process.StartInfo.Arguments += " -cmd \"" + arguments + "\"";
            process.Start();
        }

        void HistoryFile(string file)
        {
            LaunchP4V("history " + file);
        }

        List<ListViewItem> RevertUnchangedFiles(List<ListViewItem> files)
        {
            List<ListViewItem> successList = new List<ListViewItem>();

            foreach (ListViewItem file in files)
            {
                P4Process process = new P4Process("revert -a \"" + file.Text + "\"");
                process.Start();
                process.WaitForExit();
                string error = process.ReadToEnd(process.StandardError);
                string output = process.ReadToEnd(process.StandardOutput);
                if ((error == null || error == "") && (output.Contains("reverted")))
                    successList.Add(file);
            }

            return successList;
        }

        List<ListViewItem> RevertFiles(List<ListViewItem> files, bool force)
        {
            string message = "The following will be reverted:\n\n";
            foreach (ListViewItem file in files)
                message += "\n" + file.Text;

            List<ListViewItem> successList = new List<ListViewItem>();

            DialogResult result = MessageBox.Show(message, "Revert File(s)", MessageBoxButtons.OKCancel);
            if (result == DialogResult.OK)
            {
                string errors = "";
                foreach (ListViewItem file in files)
                {
                    P4Process process = new P4Process("revert \"" + file.Text + "\"");
                    process.Start();
                    process.WaitForExit();
                    string error = process.ReadToEnd(process.StandardError);
                    if (error == null || error == "")
                        successList.Add(file);
                    else
                    {
                        if (force)
                        {
                            process = new P4Process("sync -f \"" + file.Text + "\"");
                            process.Start();
                            process.WaitForExit();
                            error = process.ReadToEnd(process.StandardError);
                            if (error == null || error == "")
                                successList.Add(file);
                            else
                                errors += error + "\n";
                        }
                        else
                            errors += error + "\n";
                    }
                }
                if (errors != "")
                    statusLabel.Invoke(new HandleExceptionStringDelegate(Program.HandleException), errors, false);
            }

            return successList;
        }

        void contextMenu_Collapse(object sender, EventArgs e)
        {
            // When the context menu is closed, disassociate it from the form
            ContextMenu = null;
        }

        private void deletedList_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                clickedPoint = new Point(e.X, e.Y);

                ListViewItem selected = deletedList.GetItemAt(clickedPoint.X, clickedPoint.Y);
                if (selected != null)
                    DisplayContextMenu(this, null, revertMenuItem_deleted_Click, null, historyMenuItem_Click);
                else
                    DisplayContextMenu(this, null, null, null, null);
            }
        }

        private void modifiedList_DoubleClick(object sender, EventArgs e)
        {
            ListViewItem selected = modifiedList.GetItemAt(clickedPoint.X, clickedPoint.Y);
            if (selected == null)
                return;

            DiffFile(selected.Text);
        }

        private void modifiedList_MouseDown(object sender, MouseEventArgs e)
        {
            clickedPoint = new Point(e.X, e.Y);

            if (e.Button == MouseButtons.Right)
            {
                EventHandler revertUnchanged = null;

                // Don't enable this if there's no files listed
                if (modifiedList.Items.Count > 0)
                    revertUnchanged = revertUnchangedMenuItem_modified_Click;

                ListViewItem selected = modifiedList.GetItemAt(clickedPoint.X, clickedPoint.Y);
                if (selected != null)
                    DisplayContextMenu(this, revertUnchanged, revertMenuItem_modified_Click, diffMenuItem_Click, historyMenuItem_Click);
                else
                    DisplayContextMenu(this, revertUnchanged, null, null, null);
            }
        }

        private void CheckInForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (submitWorker.IsBusy)
            {
                DialogResult result = MessageBox.Show("A submit is in progress. Are you sure you want to exit?", "Submit in Progress", MessageBoxButtons.OKCancel);
                if (result != DialogResult.OK)
                {
                    e.Cancel = true;
                    return;
                }
            }

            if (changelistDescTextBox.Text != "")
                recentMessages.AddMessageToTop(changelistDescTextBox.Text);

            string[] tempFiles = Directory.GetFiles(Path.GetTempPath(), "*.sigtmp");
            foreach (string tempFile in tempFiles)
            {
                try
                {
                    File.Delete(tempFile);
                }
                catch (Exception ex)
                {
                    statusLabel.Invoke(new HandleExceptionDelegate(Program.HandleException), ex, false);
                }
            }

            if (WindowState == FormWindowState.Normal)
            {
                Properties.Settings.Default.CheckInForm_Size = Size;
                Properties.Settings.Default.CheckInForm_Location = Location;
            }
            else
            {
                // If the form is maximized or minimized, we want to start the program
                // the next time with what it would have been restored to
                Properties.Settings.Default.CheckInForm_Size = RestoreBounds.Size;
                Properties.Settings.Default.CheckInForm_Location = RestoreBounds.Location;
            }

            Properties.Settings.Default.CheckInForm_State = WindowState;
            Properties.Settings.Default.CheckInForm_addedList_ColumnHeaderWidth = addedList.Columns[0].Width;
            Properties.Settings.Default.CheckInForm_modifiedList_ColumnHeaderWidth = modifiedList.Columns[0].Width;
            Properties.Settings.Default.CheckInForm_deletedList_ColumnHeaderWidth = deletedList.Columns[0].Width;

            Properties.Settings.Default.CheckInForm_topSplitPanel_SplitterDistance = topSplitContainer.SplitterDistance;
            Properties.Settings.Default.CheckInForm_bottomSplitPanel_SplitterDistance = bottomSplitContainer.SplitterDistance;

            Properties.Settings.Default.Save();
        }

        private void CheckInForm_Shown(object sender, EventArgs e)
        {
            // Restore the form size and location from the last time it was run
            addedList.Columns[0].Width = Properties.Settings.Default.CheckInForm_addedList_ColumnHeaderWidth;
            modifiedList.Columns[0].Width = Properties.Settings.Default.CheckInForm_modifiedList_ColumnHeaderWidth;
            deletedList.Columns[0].Width = Properties.Settings.Default.CheckInForm_deletedList_ColumnHeaderWidth;

            WindowState = Properties.Settings.Default.CheckInForm_State;
            Size = Properties.Settings.Default.CheckInForm_Size;
            Location = Properties.Settings.Default.CheckInForm_Location;

            topSplitContainer.SplitterDistance = Properties.Settings.Default.CheckInForm_topSplitPanel_SplitterDistance;
            bottomSplitContainer.SplitterDistance = Properties.Settings.Default.CheckInForm_bottomSplitPanel_SplitterDistance;
        }

        private void recentMessagesButton_Click(object sender, EventArgs e)
        {
            recentMessages.ShowDialog();
            string selectedMessage = recentMessages.SelectedMessage;
            if (selectedMessage != null && selectedMessage != "")
                changelistDescTextBox.Text = selectedMessage;
        }

        private void CheckInForm_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.F5)
                StartCheckInBackgroundWorker(listWorker);
        }

        private void addedList_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                clickedPoint = new Point(e.X, e.Y);

                ListViewItem selected = addedList.GetItemAt(clickedPoint.X, clickedPoint.Y);
                if (selected != null)
                {
                    Dictionary<string, EventHandler> menuItems = new Dictionary<string, EventHandler>();
                    menuItems.Add("Delete File(s)", deleteFilesMenuItem_added_Click);
                    DisplayContextMenu(this, menuItems);
                }
            }
        }

        List<ListViewItem> DeleteFiles(List<ListViewItem> files)
        {
            string message = "The following will be deleted:\n\n";
            foreach (ListViewItem file in files)
                message += "\n" + file.Text;

            List<ListViewItem> successList = new List<ListViewItem>();

            DialogResult result = MessageBox.Show(message, "Delete File(s)", MessageBoxButtons.OKCancel);
            if (result == DialogResult.OK)
            {
                string errors = "";
                foreach (ListViewItem file in files)
                {
                    P4Process process = new P4Process("revert \"" + file.Text + "\"");
                    process.Start();
                    process.WaitForExit();
                    string error = process.ReadToEnd(process.StandardError);
                    if (error == null || error == "" || error.Contains("file(s) not opened on this client") || error.Contains(", abandoned"))
                    {
                        if (File.Exists(file.Text))
                        {
                            // Clear any read-only flag if necessary
                            FileInfo fileInfo = new FileInfo(file.Text);
                            if (fileInfo != null && fileInfo.IsReadOnly)
                                fileInfo.Attributes = (FileAttributes)(Convert.ToInt32(fileInfo.Attributes) - Convert.ToInt32(FileAttributes.ReadOnly));

                            File.Delete(file.Text);
                        }

                        successList.Add(file);
                    }
                    else
                        errors += error + "\n";
                }
                if (errors != "")
                    statusLabel.Invoke(new HandleExceptionStringDelegate(Program.HandleException), errors, false);
            }

            return successList;
        }

        void deleteFilesMenuItem_added_Click(object sender, EventArgs e)
        {
            List<ListViewItem> files = new List<ListViewItem>();
            foreach (int index in addedList.SelectedIndices)
                files.Add(addedList.Items[index]);

            List<ListViewItem> successfulDeletes = DeleteFiles(files);
            foreach (ListViewItem deletedFile in successfulDeletes)
                addedList.Items.Remove(deletedFile);
        }

        private void changesComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            changelist = changesComboBox.SelectedItem.ToString();
            StartCheckInBackgroundWorker(listWorker);
        }

        private List<string> GetPendingChangelistNumbers()
        {
            List<string> pending = new List<string>();

            P4Process process = new P4Process("changes -c " + GetWorkspaceName() + " -s pending");
            process.Start();
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
            {
                string line = process.StandardOutput.ReadLine();
                string data = GetTagData(line, "change");
                if (data != "")
                    pending.Add(data);
            }

            return pending;
        }

        private void UpdateChangesList()
        {
            changesComboBox.Items.Clear();

            // Default is always available
            changesComboBox.Items.Add("default");

            List<string> pending = GetPendingChangelistNumbers();
            foreach (string pendingChangelist in pending)
                changesComboBox.Items.Add(pendingChangelist);

            changesComboBox.SelectedIndex = 0;
        }
    }
}
