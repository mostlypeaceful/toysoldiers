#region File Information
//-----------------------------------------------------------------------------
// Log.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace Atg.Samples.xbWatson
{
    /// <summary>
    /// Extension to the RichTextBox control to provide features useful for logging.
    /// </summary>
    public class Log : RichTextBox
    {
        private delegate void VoidDelegate();
    
        private bool scrollToRecentMessages;
        private List<string> allFilters = new List<string>();
        private Dictionary<string, Color> filters = new Dictionary<string, Color>();
        private Atg.Samples.xbWatson.Forms.MainForm mainWatsonForm = null;
        private bool lookForWatsonForm = true;
    
        public Log()
        {
            // Default properties for a log.
            AutoWordSelection = true;
            DetectUrls = false;
            EnableAutoDragDrop = false;
            Multiline = true;
            ReadOnly = true;
            ScrollBars = RichTextBoxScrollBars.ForcedBoth;
            ShowSelectionMargin = true;
            WordWrap = false;
            
            BackColor = Color.White;
            ForeColor = Color.Black;
        }

        #region Public Methods/Properties

        /// <summary>
        /// Specifies whether the log should scroll to show recent messages.
        /// </summary>
        public bool ScrollToRecentMessages
        {
            get { return scrollToRecentMessages; }
            set { scrollToRecentMessages = value; }
        }

        /// <summary>
        /// Appends text with the specified formatting, including a newline.  (Callable from any thread.)
        /// </summary>
        public void AppendLine(LogFormatting formatting, string text)
        {
            BeginInvoke(new VoidDelegate(delegate
            {
                AppendTextInternal(formatting, text, true);
            }));
        }
        
        /// <summary>
        /// Appends text with the specified formatting.  (Callable from any thread.)
        /// </summary>
        public void AppendText(LogFormatting formatting, string text)
        {
            BeginInvoke(new VoidDelegate(delegate
            {
                AppendTextInternal(formatting, text, false);
            }));
        }

        #endregion

        #region Internal Methods/Properties

        private const int WM_SETREDRAW = 0xb;
        private const int WM_USER = 0x400;
        private const int EM_GETEVENTMASK = (WM_USER + 59);
        private const int EM_SETEVENTMASK = (WM_USER + 69);

        [DllImport("user32", CharSet = CharSet.Auto)]
        private extern static IntPtr SendMessage(IntPtr hWnd, int msg, int wParam, IntPtr lParam);

        /// <summary>
        /// If the parent has changed, (we've been added as a child control to something)
        /// search for the main watson form if we haven't already found it.
        /// </summary>
        protected override void OnParentChanged (EventArgs e) {
            base.OnParentChanged(e);

            if (mainWatsonForm == null) {
                FindMainWatsonForm(this.Parent);
            }
        }

        /// <summary>
        /// Recursively travels up the parent tree to try and find the main watson form.
        /// </summary>
        /// <param name="parent"></param>
        private void FindMainWatsonForm (Control parent) {
            if (parent == null)
                return;

            if (parent is Atg.Samples.xbWatson.Forms.MainForm) {
                mainWatsonForm = (Atg.Samples.xbWatson.Forms.MainForm)parent;
                return;
            }
            else {
                FindMainWatsonForm(parent.Parent);
            }
        }

        /// <summary>
        /// Appends text with the specified formatting.
        /// </summary>
        private void AppendTextInternal(LogFormatting formatting, string text, bool newline)
        {
            if (mainWatsonForm != null) {
                foreach( var ignoreString in mainWatsonForm.CurrentIgnoredStrings )
                    if( text.Contains(ignoreString) )
                        return;
            }
            else if (mainWatsonForm == null && lookForWatsonForm) {
                lookForWatsonForm = false;
                FindMainWatsonForm(this.Parent);
            }

            // Update the control.
            IntPtr eventMask = IntPtr.Zero;
            try
            {
                // Stop redrawing or sending events.
                SendMessage(Handle, WM_SETREDRAW, 0, IntPtr.Zero);
                eventMask = SendMessage(Handle, EM_GETEVENTMASK, 0, IntPtr.Zero);

                // Capture original selection.
                int length = SelectionLength;
                int start = SelectionStart;
                int first = GetCharIndexFromPosition(new Point(1, 1));

                // Sometimes text from multiple sources comes in a single string, separated
                // only by newlines - split each line and parse it separately
                string[] splitText = text.Split("\n".ToCharArray());
                text = "";
                for (int i = 0; i < splitText.Length; ++i)
                {
                    string line = splitText[i];

                    string tag = GetLogTag(line);
                    Color c = Color.ForestGreen;
                    if( !GetFilterColor(tag,ref c) )
                        continue;
                    formatting.Color = c;

                    // Handle newline.
                    if (newline && TextLength > 0)
                        line = string.Format("\n{0}", line);

                    // Format the input text.
                    string formatted = formatting.FormatText(line);

                    // Append the text as RTF.
                    Select(TextLength, 0);
                    SelectedRtf = formatted;
                }

                if (!scrollToRecentMessages)
                {
                    // Restore original selection.
                    Select(first, 0);
                    ScrollToCaret();
                }
                else
                {
                    // Scroll to end.
                    Select(TextLength, 0);
                    ScrollToCaret();
                }
            }
            finally
            {
                // Enable events and redrawing.
                SendMessage(Handle, EM_SETEVENTMASK, 0, eventMask);
                SendMessage(Handle, WM_SETREDRAW, 1, IntPtr.Zero);

                // Redraw.
                Refresh();
            }
        }

        public bool GetFilterColor(string tag, ref Color c)
        {
            if (!allFilters.Contains(tag))
                return true;

            if (!filters.ContainsKey(tag))
                return false;

            c = filters[tag];
            return true;
        }

        public string GetLogTag(string text)
        {
            // Handle "Log:" type tags
            if (text.Length < 4)
                return "None";
            else if (text.Substring(0, 4) == "Log:")
            {
                Match tagMatch = Regex.Match(text, @"^Log:(\w+).*");
                if (tagMatch.Groups.Count < 2)
                    throw (new FormatException("The \"Log:\" tag doesn't appear to contain a valid name: " + text));

                return tagMatch.Groups[1].Value;
            }

            // Handle "!WARNING!" type tags
            if (text.Length < 9)
                return "None";
            else if (text.Substring(0, 9) == "!WARNING!")
                return "!WARNING!";

            return "None";
        }

        public void AddToListOfAllFilters(string filter)
        {
            allFilters.Add(filter);
        }

        public void AddFilter(string filter, Color color)
        {
            if (!filters.ContainsKey(filter))
                filters.Add(filter, color);
        }

        public void RemoveFilter(string filter)
        {
            if (filters.ContainsKey(filter))
                filters.Remove(filter);
        }
        
        #endregion
    }
}
