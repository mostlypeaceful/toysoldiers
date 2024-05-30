#region File Information
//-----------------------------------------------------------------------------
// DebugNotificationDialog.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Threading;
using System.Windows.Forms;
using XDevkit;

namespace Atg.Samples.xbWatson.Forms
{
    public partial class DebugNotificationDialog : Form
    {
        private DebugMonitorForm parentForm;
        private IXboxEventInfo eventInfo;

        public DebugNotificationDialog(DebugMonitorForm parentForm, IXboxEventInfo eventInfo)
        {
            InitializeComponent();

            this.parentForm = parentForm;
            this.eventInfo = eventInfo;
        }

        #region Public Methods/Properties

        /// <summary>
        /// The detailed information about the debug notification.
        /// </summary>
        public string Details
        {
            set
            {
                detailsText.Lines = value.Split("\n".ToCharArray());
                detailsText.Select(0, 0);
            }
        }

        /// <summary>
        /// The icon for the debug notification.
        /// </summary>
        public NotificationIcon NotificationIcon
        {
            set
            {
                int index = (int)value;
                notificationIcon.Image = notificationIcons.Images[index];
            }
        }

        /// <summary>
        /// The summary of the debug notification.
        /// </summary>
        public string Overview { set { overviewLabel.Text = value; } }

        #endregion

        #region Event Handlers

        private void continueButton_Click(object sender, EventArgs e)
        {
            parentForm.Monitor.HandleNotificationDialogAction(AutomationAction.Continue, eventInfo);
            Close();
        }

        private void rebootButton_Click(object sender, EventArgs e)
        {
            parentForm.Monitor.HandleNotificationDialogAction(AutomationAction.Reboot, eventInfo);
            Close();
        }

        #endregion

        private void DebugNotificationDialog_Load(object sender, EventArgs e)
        {
            Dumpy dumpy = new Dumpy(parentForm);
            DialogResult result = dumpy.ShowDialog();
            if (result != DialogResult.Cancel)
                Close();
        }
    }

    /// <summary>
    /// The icons that can be displayed in the notification dialog.
    /// </summary>
    public enum NotificationIcon
    {
        Information,
        Warning,
        Error,
    }
}