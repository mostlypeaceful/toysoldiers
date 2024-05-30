#region File Information
//-----------------------------------------------------------------------------
// AutomationSetting.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using XDevkit;

namespace Atg.Samples.xbWatson
{
    /// <summary>
    /// The automation settings for a particular debug notification event.
    /// </summary>
    [Serializable]
    public class AutomationSetting
    {
        private readonly XboxDebugEventType eventType;

        private AutomationAction action = AutomationAction.Prompt;
        private bool saveMinidump = false;
        private bool withHeap = false;

        public AutomationSetting(XboxDebugEventType eventType)
        {
            this.eventType = eventType;
        }

        #region Public Methods/Properties

        /// <summary>
        /// The debug notification event for which this automation setting applies.
        /// </summary>
        public XboxDebugEventType EventType { get { return eventType; } }
        
        /// <summary>
        /// Which action to perform upon receiving the debug notification event.
        /// </summary>
        public AutomationAction Action
        { 
            get { return action; }
            set { action = value; }
        }
        
        /// <summary>
        /// Whether to generate a minidump in response to the debug notification event.
        /// </summary>
        public bool SaveMinidump
        {
            get { return saveMinidump; }
            set { saveMinidump = value; }
        }
        
        /// <summary>
        /// Whether the minidump should contain the heap.
        /// </summary>
        public bool WithHeap
        {
            get { return withHeap; }
            set { withHeap = value; }
        }

        public override string ToString()
        {
            // Convert event type to friendly display name.
            switch (eventType)
            {
                case XboxDebugEventType.AssertionFailed:
                    return "Assertion Failed";
                
                case XboxDebugEventType.Exception:
                    return "Exception";
                
                case XboxDebugEventType.RIP:
                    return "Fatal Error (RIP)";
                
                default:
                    return eventType.ToString();
            }
        }

        #endregion
    }

    /// <summary>
    /// The supported actions in response to a debug notification event.
    /// </summary>
    public enum AutomationAction
    {
        Prompt,
        Continue,
        Break,
        Reboot,
    }
}