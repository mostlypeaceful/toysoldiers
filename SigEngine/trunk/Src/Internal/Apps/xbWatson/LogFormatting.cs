#region File Information
//-----------------------------------------------------------------------------
// LogFormatting.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Drawing;
using System.Text;

namespace Atg.Samples.xbWatson
{
    /// <summary>
    /// The formatting options for an entry in the log.
    /// </summary>
    /// <remarks>
    /// This class generates RTF (rich-text format) data that can be used in a RichTextBox.
    /// </remarks>
    public class LogFormatting
    {
        private Font font = SystemFonts.DefaultFont;
        private Color color = SystemColors.WindowText;
        private Color? highlight = null;
        
        private string rtf;

        public LogFormatting()
        {
            UpdateRtf();
        }

        public LogFormatting(Font font, Color color)
        {
            this.font = font;
            this.color = color;

            UpdateRtf();
        }

        public LogFormatting(Font font, Color color, Color? highlight)
        {
            this.font = font;
            this.color = color;
            this.highlight = highlight;

            UpdateRtf();
        }

        #region Public Methods/Properties

        /// <summary>
        /// The font used to format the log entry.
        /// </summary>
        public Font Font
        {
            get { return font; }
            set { font = value; UpdateRtf(); }
        }

        /// <summary>
        /// The color of the text for the log entry.
        /// </summary>
        public Color Color
        {
            get { return color; }
            set { color = value; UpdateRtf(); }
        }

        /// <summary>
        /// The optional highlight color for the log entry.
        /// </summary>
        public Color? Highlight
        {
            get { return highlight; }
            set { highlight = value; UpdateRtf(); }
        }
        
        /// <summary>
        /// Format the input text as RTF.
        /// </summary>
        public string FormatText(string text)
        {
            return string.Format(@"{{{0}{1}}}", rtf, EscapeText(text));
        }

        /// <summary>
        /// Default formatting for log messages.
        /// </summary>
        public static LogFormatting Default =
            new LogFormatting(
                new Font(SystemFonts.DialogFont, FontStyle.Bold),
                Color.DarkSlateBlue,
                null
            );

        /// <summary>
        /// Formatting for debug output from a title.
        /// </summary>
        public static LogFormatting DebugOutput =
            new LogFormatting(
                new Font(FontFamily.GenericMonospace, SystemFonts.DialogFont.Size),
                Color.Black,
                null
            );

        /// <summary>
        /// Formatting for error messages.
        /// </summary>
        public static LogFormatting Error =
            new LogFormatting(
                new Font(SystemFonts.DialogFont, FontStyle.Bold),
                Color.OrangeRed,
                null
            );

        /// <summary>
        /// Formatting for error messages (fixed-width).
        /// </summary>
        public static LogFormatting ErrorFixed =
            new LogFormatting(
                new Font(FontFamily.GenericMonospace, SystemFonts.DialogFont.Size),
                Color.OrangeRed,
                null
            );

        
        #endregion

        #region Internal Methods/Properties

        /// <summary>
        /// Converts a GDI color into RTF format.
        /// </summary>
        private static string ColorToRtf(Color color)
        {
            return string.Format(@"\red{0}\green{1}\blue{2}", color.R, color.G, color.B);
        }

        /// <summary>
        /// Escape characters in the input text so they appear as literals in RTF.
        /// </summary>
        private static string EscapeText(string text)
        {
            StringBuilder result = new StringBuilder();

            // Process input text one line at a time.
            string[] lines = text.Split('\n');
            foreach (string line in lines)
            {
                // Insert newline as needed.
                if (result.Length > 0) result.Append(@"\par");

                // Escape special characters in RTF: \ { }
                string escaped = line.Replace(@"\", @"\\");
                escaped = escaped.Replace(@"{", @"\{");
                escaped = escaped.Replace(@"}", @"\}");

                result.AppendFormat(@"{{{0}}}", escaped);
            }

            return result.ToString();
        }

        /// <summary>
        /// Generate the RTF data to describe the formatting options.
        /// </summary>
        private void UpdateRtf()
        {
            StringBuilder result = new StringBuilder();

            result.Append(@"\rtf1\ansi");

            result.AppendFormat(@"{{\fonttbl\f0\fnil {0};}}", font.FontFamily.Name);

            result.AppendFormat(@"{{\colortbl;{0};{1};}}", ColorToRtf(color),
                                ColorToRtf(highlight.GetValueOrDefault()));

            result.Append(@"\pard\plain\cf1");
            if (highlight.HasValue) result.Append(@"\highlight2");

            if (font.Bold) result.Append(@"\b");
            if (font.Italic) result.Append(@"\i");
            if (font.Strikeout) result.Append(@"\strike");
            if (font.Underline) result.Append(@"\ul");
            result.AppendFormat(@"\fs{0}", (int)Math.Floor(font.SizeInPoints * 2.0f));

            rtf = result.ToString();
        }
        
        #endregion
    }
}
