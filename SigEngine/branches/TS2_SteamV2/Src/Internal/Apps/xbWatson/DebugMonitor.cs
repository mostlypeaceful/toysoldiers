#region File Information
//-----------------------------------------------------------------------------
// DebugMonitor.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.IO;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using XDevkit;

namespace Atg.Samples.xbWatson
{
    /// <summary>
    /// Encapsulation of the XDevkit/XBDM functionality to monitor an Xbox 360 console.
    /// </summary>
    public class DebugMonitor
    {
        private delegate void VoidDelegate();

        private static XboxManagerClass xboxManager = new XboxManagerClass();

        private XboxConsole xboxConsole = null;
        private IXboxDebugTarget debugTarget = null;

        private ConnectionState connectionState = ConnectionState.NotConnected;
        private string runningProcess = string.Empty;

        private Forms.DebugMonitorForm owner;

        private XboxEvents_OnStdNotifyEventHandler onStdNotify;

        public DebugMonitor(Forms.DebugMonitorForm owner)
        {
            this.owner = owner;
            onStdNotify = new XboxEvents_OnStdNotifyEventHandler(xboxConsole_OnStdNotify);
        }

        #region Public Methods/Properties

        /// <summary>
        /// XDevkit/XBDM object for managing Xbox 360 consoles.
        /// </summary>
        public static XboxManager XboxManager { get { return xboxManager; } }

        /// <summary>
        /// The form associated with this object.
        /// </summary>
        public Forms.DebugMonitorForm Owner { get { return owner; } }

        /// <summary>
        /// The Xbox 360 console associated with this object.
        /// </summary>
        public XboxConsole XboxConsole { get { return xboxConsole; } }

        /// <summary>
        /// The name of the Xbox 360 console.
        /// </summary>
        public string Name
        {
            get
            {
                try
                {
                    return xboxConsole.Name;
                }
                catch (ArgumentNullException)
                {                
                    return string.Empty;
                }
            }
        }

        /// <summary>
        /// The IP address of the Xbox 360 console.  NOTE: This is the debug IP, not the title IP.
        /// </summary>
        public IPAddress IPAddress
        {
            get
            {
                try
                {
                    long ip = (long)EndianSwap(xboxConsole.IPAddress);
                    return new IPAddress(ip);
                }
                catch (NullReferenceException)
                {
                    return IPAddress.None;
                }
            }
        }

        /// <summary>
        /// The current state of the connection with the Xbox 360 console.
        /// </summary>
        public ConnectionState ConnectionState { get { return connectionState; } }

        /// <summary>
        /// Connects to the specified Xbox 360 console.
        /// </summary>
        public void Connect(string consoleNameOrIP)
        {
            // Spawn a thread.
            new Thread(new ParameterizedThreadStart(ConnectThread)).Start(consoleNameOrIP);
        }

        /// <summary>
        /// Close the connection to the Xbox 360 console.
        /// </summary>
        public void Disconnect()
        {
            // Spawn a thread.
            new Thread(new ThreadStart(DisconnectThread)).Start();
        }

        /// <summary>
        /// Reboots the Xbox 360 console.
        /// </summary>
        public void RebootConsole(bool cold)
        {
            try
            {
                xboxConsole.Reboot(null, null, null, cold ? XboxRebootFlags.Cold : 0);
            }
            catch (COMException e)
            {
                LogCOMException(e);
            }
            catch (NullReferenceException) { }
        }
        
        /// <summary>
        /// Generates a minidump (optionally with heap), saving to the specified file.
        /// </summary>
        public void SaveMinidump(string filename, bool withHeap)
        {
            try
            {
                // Update UI.
                try
                {
                    Owner.BeginInvoke(new VoidDelegate(delegate
                    {
                        Owner.Log.AppendLine(
                            LogFormatting.Default,
                            string.Format(
                                "Saving minidump{0} to {1}...",
                                withHeap ? " w/ heap" : string.Empty,
                                filename
                            )
                        );
                    }));
                }
                catch (InvalidOperationException) { }

                // NOTE: IXboxDebugTarget::WriteDump cannot always overwrite a file.
                // If the file exists, delete it first.
                if (File.Exists(filename))
                    File.Delete(filename);
                
                // Generate the minidump.
                XboxDumpFlags flags = withHeap ? XboxDumpFlags.WithFullMemory : XboxDumpFlags.Normal;
                debugTarget.WriteDump(filename, flags);
            }
            catch (COMException e)
            {
                LogCOMException(e);                
            }
        }

        #endregion

        #region Internal Methods/Properties

        /// <summary>
        /// Adds the current time to the specified text.
        /// </summary>
        private static string AddTimestamp(string text)
        {
            StringBuilder result = new StringBuilder();

            string timestamp = string.Format("{0} : ", DateTime.Now);
            string blanks = string.Empty.PadLeft(timestamp.Length, ' ');
            bool first = true;

            string[] lines = text.Split("\n".ToCharArray());
            foreach (string line in lines)
            {
                string trimmed = line.TrimEnd(" \t\n\r".ToCharArray());
                if (first)
                {
                    result.AppendFormat("{0}{1}", timestamp, trimmed);
                    first = false;
                }
                else
                {
                    result.AppendFormat("\n{0}{1}", blanks, trimmed);
                }
            }

            return result.ToString();
        }

        /// <summary>
        /// Worker thread for Connect.
        /// </summary>
        private void ConnectThread(object param)
        {
            string consoleNameOrIP = param as string;
            if (string.IsNullOrEmpty(consoleNameOrIP))
                return;

            lock (this)
            {
                // Only connect if there is no existing connection.
                if (connectionState != ConnectionState.NotConnected)
                    return;

                try
                {
                    connectionState = ConnectionState.IsConnecting;

                    // Update UI.
                    try
                    {
                        Owner.BeginInvoke(new VoidDelegate(delegate
                        {
                            Owner.Text = string.Format("{0} (?)", consoleNameOrIP);
                            Owner.StatusImage = xbWatson.Forms.StatusImage.Connecting;
                            Owner.RunningProcess = string.Empty;
                            Owner.Log.AppendLine(
                                LogFormatting.Default,
                                string.Format("Connecting to {0}...", consoleNameOrIP)
                            );
                        }));
                    }
                    catch (InvalidOperationException) { }

                    // Open the specified Xbox 360 console.
                    xboxConsole = XboxManager.OpenConsole(consoleNameOrIP);

                    // Receive notifications.
                    xboxConsole.OnStdNotify += onStdNotify;

                    // Attach as debugger.
                    debugTarget = xboxConsole.DebugTarget;
                    debugTarget.ConnectAsDebugger("xbWatson", XboxDebugConnectFlags.Force);
                }
                catch (COMException e)
                {
                    // Update UI.
                    try
                    {
                        Owner.BeginInvoke(new VoidDelegate(delegate
                        {
                            Owner.StatusImage = xbWatson.Forms.StatusImage.Disconnected;
                            Owner.RunningProcess = string.Empty;
                        }));
                    }
                    catch (InvalidOperationException) { }

                    LogCOMException(e);

                    connectionState = ConnectionState.NotConnected;
                    Disconnect();
                }
                catch (NullReferenceException)
                {
                    // Update UI.
                    try
                    {
                        Owner.BeginInvoke(new VoidDelegate(delegate
                        {
                            Owner.StatusImage = xbWatson.Forms.StatusImage.Disconnected;
                            Owner.RunningProcess = string.Empty;
                        }));
                    }
                    catch (InvalidOperationException) { }

                    connectionState = ConnectionState.NotConnected;
                    Disconnect();
                }
            }
        }
        
        /// <summary>
        /// Continues execution of the Xbox 360 console.
        /// </summary>
        private void ContinueExecution(IXboxEventInfo eventInfo)
        {
            try
            {
                bool isStopped = (eventInfo.Info.IsThreadStopped != 0);

                if (isStopped)
                {
                    eventInfo.Info.Thread.Continue(true);

                    bool notStopped;
                    debugTarget.Go(out notStopped);
                }
            }
            catch (COMException e)
            {
                LogCOMException(e);
            }
        }

        /// <summary>
        /// Worker thread for Disconnect.
        /// </summary>
        private void DisconnectThread()
        {
            lock (this)
            {
                try
                {
                    if (debugTarget != null)
                    {
                        // Detach from the Xbox 360 console.
                        debugTarget.DisconnectAsDebugger();

                        FullyReleaseComObject(debugTarget);
                        debugTarget = null;
                    }

                    if (xboxConsole != null)
                    {
                        // Stop receiving notifications.
                        xboxConsole.OnStdNotify -= onStdNotify;

                        FullyReleaseComObject(xboxConsole);
                        xboxConsole = null;
                    }
                }
                catch (COMException e)
                {
                    LogCOMException(e);
                }
                finally
                {
                    // Update UI.
                    try
                    {
                        Owner.BeginInvoke(new VoidDelegate(delegate
                        {
                            Owner.StatusImage = xbWatson.Forms.StatusImage.Disconnected;
                            Owner.RunningProcess = string.Empty;
                        }));
                    }
                    catch (InvalidOperationException) { }

                    connectionState = ConnectionState.NotConnected;
                }
            }
        }

        /// <summary>
        /// Swaps the byte-order of the specified value.
        /// </summary>
        private static uint EndianSwap(uint value)
        {
            return (uint)(((value & 0x000000ff) << 24)
                | ((value & 0x0000ff00) << 8)
                | ((value & 0x00ff0000) >> 8)
                | ((value & 0xff000000) >> 24));
        }

        /// <summary>
        /// Force a COM object's reference count to zero.
        /// </summary>
        private static void FullyReleaseComObject(object o)
        {
            int refcount;
            do
            {
                // TODO: "RaceOnRCWCleanup was detected"
                refcount = Marshal.ReleaseComObject(o);
            } while (refcount > 0);
        }
        
        /// <summary>
        /// Write the COM exception to the log window.
        /// </summary>
        private void LogCOMException(COMException e)
        {
            // Generate the log message.
            StringBuilder message = new StringBuilder();
            message.Append("Error: ");

            string translated = xboxManager.TranslateError(e.ErrorCode);
            if (!string.IsNullOrEmpty(translated))
                message.Append(translated);
            else
                message.Append("Unknown exception");

            message.AppendFormat(" (HRESULT = 0x{0:X8})", e.ErrorCode);
            
            // Update UI.
            try
            {
                Owner.BeginInvoke(new VoidDelegate(delegate
                {
                    Owner.Log.AppendLine(LogFormatting.Error, message.ToString());
                }));
            }
            catch (InvalidOperationException) { }
        }
        
        private const int MaximumAttempts = 5;
        private const int AttemptDelay = 500;
        
        /// <summary>
        /// Worker thread to obtain running process information from the Xbox 360 console.
        /// </summary>
        private void RunningProcessThread()
        {
            string name = string.Empty;
        
            int attempts = 0;
            for (;;)
            {
                try
                {
                    name = xboxConsole.RunningProcessInfo.ProgramName;
                    if (!string.IsNullOrEmpty(name))
                        break;
                }
                catch (COMException) { }
                catch (NullReferenceException) { break; }
                
                if (++attempts >= MaximumAttempts)
                    break;
                
                // Wait before trying again.
                Thread.Sleep(AttemptDelay);
            }
            
            runningProcess = name;

            // Update UI.
            try
            {
                Owner.BeginInvoke(new VoidDelegate(delegate
                {
                    Owner.RunningProcess = name;
                }));
            }
            catch (InvalidOperationException) { }
        }
        
        /// <summary>
        /// Sets the return value on the Xbox 360 console.
        /// </summary>
        private void SetReturnValue(IXboxEventInfo eventInfo, long value)
        {
            IXboxStackFrame context = eventInfo.Info.Thread.TopOfStack;
            try
            {
                context.SetRegister64(XboxRegisters64.r3, value);
                context.FlushRegisterChanges();
            }
            catch (COMException e)
            {
                LogCOMException(e);
            }
            finally
            {
                if (context != null)
                    FullyReleaseComObject(context);
            }
        }
        
        /// <summary>
        /// Displays the notification dialog to the user based on automation settings.
        /// </summary>
        private DialogResult ShowNotificationDialog(IXboxEventInfo eventInfo, string title, string overview, string details)
        {
            DialogResult result = DialogResult.None;

            // Update UI.
            try
            {
                Owner.BeginInvoke(new VoidDelegate(delegate
                {
                    Owner.Log.AppendLine(LogFormatting.Error, title);
                    Owner.Log.AppendLine(LogFormatting.ErrorFixed, details);
                }));
            }
            catch (InvalidOperationException) { }

            AutomationSetting setting = new AutomationSetting(eventInfo.Info.Event);
            setting = Program.GetSetting<AutomationSetting>(eventInfo.Info.Event, setting);
            if (setting.Action == AutomationAction.Prompt)
            {
                // Prompt the user.
                try
                {
                    Owner.Invoke(new VoidDelegate(delegate
                    {
                        Forms.DebugNotificationDialog dialog = new Forms.DebugNotificationDialog(owner);

                        dialog.Text = title;
                        dialog.NotificationIcon = Forms.NotificationIcon.Error;
                        dialog.Overview = overview.ToString();
                        dialog.Details = details.ToString();

                        result = dialog.ShowDialog(Owner);
                    }));
                }
                catch (InvalidOperationException) { }
            }
            else
            {
                // Use automation settings.
                switch (setting.Action)
                {
                    case AutomationAction.Continue: result = DialogResult.Retry; break;
                    case AutomationAction.Break: result = DialogResult.Cancel; break;
                    case AutomationAction.Reboot: result = DialogResult.Abort; break;
                }

                // Automated minidump.
                if (setting.SaveMinidump)
                {
                    // Folder to save minidump.
                    string path = Program.GetSetting<string>("MinidumpSaveLocation", Path.GetTempPath());

                    // Base name of minidump.
                    string filename = string.Empty;
                    if (!string.IsNullOrEmpty(runningProcess))
                        filename = Path.GetFileNameWithoutExtension(runningProcess);
                    if (string.IsNullOrEmpty(filename))
                        filename = Name;
                    if (string.IsNullOrEmpty(filename))
                        filename = "Minidump";

                    // Full filename with unique suffix.
                    filename = string.Format(@"{0}\{1}-{2}.dmp",
                        path, filename, DateTime.Now.Ticks);

                    SaveMinidump(filename, setting.WithHeap);
                }

                // Update UI.
                try
                {
                    string report = string.Format("\nAutomated Action: {0}", setting.Action);

                    Owner.BeginInvoke(new VoidDelegate(delegate
                    {
                        Owner.Log.AppendLine(LogFormatting.Default, report);
                    }));
                }
                catch (InvalidOperationException) { }
            }

            return result;
        }

        #endregion

        #region Event Handlers

        /// <summary>
        /// Handle the AssertionFailed (DM_ASSERT) notification.
        /// </summary>
        private void OnAssertionFailed(IXboxEventInfo eventInfo)
        {
            // Title.
            string title = string.Format("Assertion Failed - {0}", Owner.Text);

            // Overview.
            StringBuilder overview = new StringBuilder();
            overview.AppendFormat("An assertion failed on thread 0x{0:X8} \"{1}\".",
                eventInfo.Info.Thread.ThreadId,
                eventInfo.Info.Thread.ThreadInfo.Name);

            // Details.
            StringBuilder details = new StringBuilder();
            details.AppendFormat("{0}\n\n", overview);
            details.AppendFormat("{0}\n", eventInfo.Info.Message);

            // Display dialog.
            DialogResult result = ShowNotificationDialog(eventInfo, title, overview.ToString(), details.ToString());

            // Handle user action.
            switch (result)
            {
                case DialogResult.Retry: // Continue
                    SetReturnValue(eventInfo, 'i');
                    ContinueExecution(eventInfo);
                    break;

                case DialogResult.Cancel: // Break
                    SetReturnValue(eventInfo, 'b');
                    ContinueExecution(eventInfo);
                    break;

                case DialogResult.Abort: // Reboot
                    RebootConsole(true);
                    break;
            }
        }

        /// <summary>
        /// Handle the Exception (DM_EXCEPTION) notification.
        /// </summary>
        private void OnException(IXboxEventInfo eventInfo)
        {
            // Title.
            string title = string.Format("Exception - {0}", Owner.Text);
            
            // Overview.
            StringBuilder overview = new StringBuilder();
            overview.AppendFormat("An exception occurred on thread 0x{0:X8} \"{1}\".",
                eventInfo.Info.Thread.ThreadId,
                eventInfo.Info.Thread.ThreadInfo.Name);

            // Details.
            StringBuilder details = new StringBuilder();
            details.AppendFormat("{0}\n\n", overview);
            details.AppendFormat(" Exception: 0x{0:X8}\n", eventInfo.Info.Code);
            details.AppendFormat("   Address: 0x{0:X8}\n", eventInfo.Info.Address);

            StringBuilder parameters = new StringBuilder();
            for (uint index = 0; index < eventInfo.Info.ParameterCount; ++index)
                parameters.AppendFormat("0x{0:X8} ", eventInfo.Info.Parameters[index]);
            details.AppendFormat("Parameters: {0}\n", parameters.ToString());

            switch (eventInfo.Info.Code)
            {
                case 0xc0000005: // STATUS_ACCESS_VIOLATION
                    {
                        bool isReading = eventInfo.Info.Parameters[0] == 0;
                        uint address = eventInfo.Info.Parameters[1];

                        details.AppendFormat("\nAccess violation {0} memory at 0x{1:X8}.\n",
                            isReading ? "reading" : "writing", address);
                    }
                    break;
            }

            // Display dialog.
            DialogResult result = ShowNotificationDialog(eventInfo, title, overview.ToString(), details.ToString());

            // Handle user action.
            switch (result)
            {
                case DialogResult.Retry: // Continue
                    ContinueExecution(eventInfo);
                    break;

                case DialogResult.Cancel: // Break
                    break;

                case DialogResult.Abort: // Reboot
                    RebootConsole(true);
                    break;
            }
        }

        /// <summary>
        /// Handle the RIP (DM_RIP) notification.
        /// </summary>
        private void OnRIP(IXboxEventInfo eventInfo)
        {
            // Title.
            string title = string.Format("Fatal Error (RIP) - {0}", Owner.Text);

            // Overview.
            StringBuilder overview = new StringBuilder();
            overview.AppendFormat("A fatal error (RIP) occurred on thread 0x{0:X8} \"{1}\".",
                eventInfo.Info.Thread.ThreadId,
                eventInfo.Info.Thread.ThreadInfo.Name);

            // Details.
            StringBuilder details = new StringBuilder();
            details.AppendFormat("{0}\n\n", overview);
            details.AppendFormat("RIP: {0}\n", eventInfo.Info.Message);

            // Display dialog.
            DialogResult result = ShowNotificationDialog(eventInfo, title, overview.ToString(), details.ToString());

            // Handle user action.
            switch (result)
            {
                case DialogResult.Retry: // Continue
                    ContinueExecution(eventInfo);
                    break;

                case DialogResult.Cancel: // Break
                    break;

                case DialogResult.Abort: // Reboot
                    RebootConsole(true);
                    break;
            }
        }

        /// <summary>
        /// Handle the ExecStateChange (DM_EXEC) notification.
        /// </summary>
        private void OnExecStateChange(IXboxEventInfo eventInfo)
        {
            // Confirm we are connected.
            if (connectionState == ConnectionState.IsConnecting)
            {
                connectionState = ConnectionState.IsConnected;

                // Update UI.
                string nameAndIP = string.Format("{0} ({1})", Name, IPAddress);
                try
                {
                    Owner.BeginInvoke(new VoidDelegate(delegate
                    {
                        Owner.Text = nameAndIP;
                        owner.Log.AppendLine(
                            LogFormatting.Default,
                            string.Format("Connected to {0}.", nameAndIP)
                        );
                    }));
                }
                catch (InvalidOperationException) { }
            }

            // Update status and current running process.
            Forms.StatusImage statusImage = Forms.StatusImage.Disconnected;
            switch (eventInfo.Info.ExecState)
            {
                case XboxExecutionState.Pending:
                case XboxExecutionState.PendingTitle:
                    statusImage = Forms.StatusImage.Pending;
                    break;
                
                case XboxExecutionState.Rebooting:
                case XboxExecutionState.RebootingTitle:
                    statusImage = Forms.StatusImage.Rebooting;
                    break;
                
                case XboxExecutionState.Running:
                    statusImage = Forms.StatusImage.Running;
                    break;
                
                case XboxExecutionState.Stopped:
                    statusImage = Forms.StatusImage.Stopped;
                    break;
            }
            try
            {
                Owner.BeginInvoke(new VoidDelegate(delegate
                {
                    Owner.StatusImage = statusImage;
                }));
            }
            catch (InvalidOperationException) { }

            new Thread(new ThreadStart(RunningProcessThread)).Start();


            // Watch for reboots.
            if (eventInfo.Info.ExecState == XboxExecutionState.Rebooting ||
                eventInfo.Info.ExecState == XboxExecutionState.RebootingTitle)
            {
                try
                {
                    Owner.BeginInvoke(new VoidDelegate(delegate
                    {
                        if (Owner.ClearWindowAfterReboot)
                            Owner.Log.Clear();

                        Owner.Log.AppendLine(LogFormatting.Default, "\nConsole is rebooting...");
                    }));
                }
                catch (InvalidOperationException) { }
            }
        }

        private void OnExecutionBreak(IXboxEventInfo eventInfo)
        {
            string title = string.Format("Execution Broken - {0}", Owner.Text);

            StringBuilder overview = new StringBuilder();
            overview.AppendFormat("Execution broke on thread 0x{0:X8} \"{1}\".",
                eventInfo.Info.Thread.ThreadId,
                eventInfo.Info.Thread.ThreadInfo.Name);

            StringBuilder details = new StringBuilder();
            details.AppendFormat("{0}\n\n", overview);
            details.AppendFormat("{0}\n", eventInfo.Info.Message);

            DialogResult result = ShowNotificationDialog(eventInfo, title, overview.ToString(), details.ToString());
            switch (result)
            {
                case DialogResult.Retry: // Continue
                    ContinueExecution(eventInfo);
                    break;

                case DialogResult.Cancel: // Break
                    break;

                case DialogResult.Abort: // Reboot
                    RebootConsole(true);
                    break;
            }
        }

        /// <summary>
        /// Handler for debug notifications from the Xbox 360 console.
        /// </summary>
        void xboxConsole_OnStdNotify(XboxDebugEventType eventCode, IXboxEventInfo eventInfo)
        {
            switch (eventCode)
            {
                case XboxDebugEventType.ExecStateChange: // DM_EXEC
                    OnExecStateChange(eventInfo);
                    break;

                case XboxDebugEventType.DebugString: // DM_DEBUGSTR
                    {
                        // Log the message.
                        string message = eventInfo.Info.Message.TrimEnd(" \t\n".ToCharArray());
                        if (!string.IsNullOrEmpty(message))
                        {
                            if (Owner.AddTimestamps)
                                message = AddTimestamp(message);

                            Owner.Log.AppendLine(LogFormatting.DebugOutput, message);
                        }

                        ContinueExecution(eventInfo);
                    }
                    break;

                case XboxDebugEventType.ExecutionBreak:
                    OnExecutionBreak(eventInfo);
                    break;

                case XboxDebugEventType.AssertionFailed: // DM_ASSERT
                    OnAssertionFailed(eventInfo);
                    break;
                
                case XboxDebugEventType.Exception: // DM_EXCEPTION
                    {
                        // Only handle first-chance exceptions.
                        if (eventInfo.Info.Flags != XboxExceptionFlags.FirstChance)
                            break;
                    
                        // Ignore exception caused by SetThreadName.
                        if (eventInfo.Info.Code == 0x406D1388)
                            break;
                        
                        OnException(eventInfo);
                    }
                    break;
                
                case XboxDebugEventType.RIP: // DM_RIP
                    OnRIP(eventInfo);
                    break;
            }

            // NOTE: The .NET CLR garbage collector does not clean up
            // these COM objects as soon as we would like.

            // Clean up COM objects.
            XBOX_EVENT_INFO info = eventInfo.Info;

            FullyReleaseComObject(eventInfo);

            if (info.Module != null)
                FullyReleaseComObject(info.Module);

            if (info.Section != null)
                FullyReleaseComObject(info.Section);

            if (info.Thread != null)
                FullyReleaseComObject(info.Thread);
        }

        #endregion
    }

    public enum ConnectionState
    {
        NotConnected,
        IsConnecting,
        IsConnected,
    }
}
